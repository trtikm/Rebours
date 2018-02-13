#include <rebours/MAL/loader/detail/load_props_elf.hpp>
#include <rebours/utility/assumptions.hpp>
#include <rebours/utility/invariants.hpp>
#include <rebours/utility/file_utils.hpp>
#include <fstream>
#include <algorithm>

namespace loader { namespace detail {


load_props_elf::load_props_elf(std::string const&  root_file,
                               std::vector<std::string> const&  ignored_files,
                               std::vector<std::string> const&  search_dirs)
    : m_root_file(root_file)
    , m_ignored_files(ignored_files)
    , m_search_dirs(search_dirs)
    , m_platform()
    , m_entry_point(0ULL)
    , m_sections_table(new loader::sections_table)
    , m_special_sections(new loader::special_sections_table)
    , m_files_table(new loader::files_table)
    , m_skipped_files(new loader::skipped_files)
    , m_file_forward_dependencies(new loader::file_dependencies_map)
    , m_file_backward_dependencies(new loader::file_dependencies_map)
    , m_file_id_generator(0U)
    , m_parent_abi_loader()
    , m_init_functions(new special_functions_of_files)
    , m_fini_functions(new special_functions_of_files)
    , m_visible_symbol_table(new symbol_table)
    , m_hidden_symbol_table(new symbol_table)
    , m_from_symbol_indices_to_files()
    , m_performed_relocations(new loader::relocations)
    , m_skipped_relocations(new loader::relocations)
    , m_address_fixes{new std::unordered_map<std::string,address_fixes_map_ptr>}
    , m_warnings(new loader::warnings)
{
    (*m_file_forward_dependencies)[m_root_file] = {};
    (*m_file_backward_dependencies)[m_root_file] = {};
}

void  load_props_elf::set_platform(platform_ptr const  platform)
{
    ASSUMPTION( !this->platform().operator bool() );
    m_platform = platform;
}

void  load_props_elf::set_entry_point(address const  entry_point)
{
    m_entry_point = entry_point;
}

std::string  load_props_elf::add_section(section_ptr const  new_section)
{
    auto const  insert_result =
            m_sections_table->insert( sections_table::value_type{new_section->start_address(),new_section} );
    if (!insert_result.second)
    {
        std::stringstream  sstr;
        sstr << "The sections table already contains a section at address: " << new_section->start_address();
        return sstr.str();
    }
    auto const  it = insert_result.first;
    if (it != m_sections_table->begin())
    {
        auto const  pre_it = std::prev(it);
        if (pre_it->second->end_address() > it->second->start_address())
            return "The inserted section overlaps with its predecessor section.";
    }
    if (std::next(it) != m_sections_table->end())
    {
        auto const  post_it = std::next(it);
        if (it->second->end_address() > post_it->second->start_address())
            return "The inserted section overlaps with its successor section.";
    }
    return "";
}

void  load_props_elf::add_special_section(std::string const&  binary_file,
                                          std::string const&  section_name,
                                          special_section_properties_ptr const  section_props)
{
    ASSUMPTION(m_files_table->count(binary_file) != 0ULL);
    ASSUMPTION(m_special_sections->count(binary_file) == 0ULL ||
               m_special_sections->at(binary_file).count(section_name) == 0ULL);
    ASSUMPTION(
            [](special_section_properties_ptr const  section_props,
               mutable_sections_table_ptr const  sections_table) {
                for (address  adr = section_props->start_address(); adr < section_props->end_address(); ++adr)
                    if (!find_section(adr,sections_table).operator bool())
                        return false;
                return true;
            }(section_props,sections_table())
        );
    ((*m_special_sections)[binary_file]).insert({section_name,section_props});
}

void  load_props_elf::add_file_props(file_props_ptr const  file_props)
{
    ASSUMPTION(std::find(m_skipped_files->cbegin(),m_skipped_files->cend(),file_props->path()) == m_skipped_files->cend());
    auto const  insert_result =
            m_files_table->insert(loader::files_table::value_type(file_props->path(),file_props));
    ASSUMPTION(insert_result.second);
    (void)insert_result;
}

void  load_props_elf::add_skipped_file(std::string const&  file_pathname)
{
    ASSUMPTION(m_files_table->count(file_pathname) == 0ULL);
    m_skipped_files->insert(file_pathname);
}

void  load_props_elf::add_file_dependence(std::string const&  src_file, std::string const&  dst_file)
{
    ASSUMPTION(std::find(m_skipped_files->cbegin(),m_skipped_files->cend(),src_file) != m_skipped_files->cend() ||
               m_files_table->count(src_file) > 0ULL);
    ASSUMPTION(std::find(m_skipped_files->cbegin(),m_skipped_files->cend(),dst_file) != m_skipped_files->cend() ||
               m_files_table->count(dst_file) > 0ULL);

    {
        auto& successors =
                m_file_forward_dependencies->insert(file_dependencies_map::value_type(src_file,{})).first->second;
        if (std::find(successors.cbegin(),successors.cend(),dst_file) == successors.cend())
            successors.push_back(dst_file);
        m_file_forward_dependencies->insert(file_dependencies_map::value_type(dst_file,{}));
    }
    {
        auto& predecessors =
                m_file_backward_dependencies->insert(file_dependencies_map::value_type(dst_file,{})).first->second;
        if (std::find(predecessors.cbegin(),predecessors.cend(),src_file) == predecessors.cend())
            predecessors.push_back(src_file);
        m_file_backward_dependencies->insert(file_dependencies_map::value_type(src_file,{}));
    }
}

void  load_props_elf::add_init_function(std::string const& file, address const  fn_address)
{
    ASSUMPTION(m_init_functions->count(file) == 0 ||
               std::find(m_init_functions->at(file).cbegin(),m_init_functions->at(file).cend(),fn_address)
                    == m_init_functions->at(file).cend());
    ASSUMPTION(find_section(fn_address,sections_table()).operator bool() &&
               find_section(fn_address,sections_table())->has_execute_access());
    (*m_init_functions)[file].push_back(fn_address);
}

void  load_props_elf::add_fini_function(std::string const& file, address const  fn_address)
{
    ASSUMPTION(m_fini_functions->count(file) == 0 ||
               std::find(m_fini_functions->at(file).cbegin(),m_fini_functions->at(file).cend(),fn_address)
                    == m_fini_functions->at(file).cend());
    ASSUMPTION(find_section(fn_address,sections_table()).operator bool() &&
               find_section(fn_address,sections_table())->has_execute_access());
    (*m_fini_functions)[file].push_back(fn_address);
}

void  load_props_elf::add_visible_symbol(std::string const& definition_file,  uint64_t const  symbol_table_index,
                                         relocation_symbol_id const& symbol_id, std::string const&  symbol_type,
                                         uint64_t const  symbol_value, uint64_t const symbol_size)
{
    ASSUMPTION(!has_visible_symbol(definition_file,symbol_table_index));
    ASSUMPTION(!has_hidden_symbol(definition_file,symbol_table_index));
    //ASSUMPTION(m_from_symbol_indices_to_files.count(symbol_table_index) == 0ULL);
    m_visible_symbol_table->insert({{definition_file,symbol_table_index},
                                    std::make_tuple(symbol_id,symbol_type,symbol_value,symbol_size)});
    //m_from_symbol_indices_to_files.insert({symbol_table_index,definition_file});
}

bool  load_props_elf::has_visible_symbol(std::string const& definition_file,  uint64_t const  symbol_table_index)
{
    return m_visible_symbol_table->find({definition_file,symbol_table_index}) != m_visible_symbol_table->cend();
}

relocation_symbol_id const& load_props_elf::visible_symbol_id(std::string const& definition_file,  uint64_t const  symbol_table_index)
{
    ASSUMPTION(has_visible_symbol(definition_file,symbol_table_index));
    auto const  it = m_visible_symbol_table->find({definition_file,symbol_table_index});
    return std::get<0>(it->second);
}

std::string  load_props_elf::visible_symbol_type(std::string const& definition_file,  uint64_t const  symbol_table_index)
{
    ASSUMPTION(has_visible_symbol(definition_file,symbol_table_index));
    auto const  it = m_visible_symbol_table->find({definition_file,symbol_table_index});
    return std::get<1>(it->second);
}

uint64_t  load_props_elf::visible_symbol_value(std::string const& definition_file,  uint64_t const  symbol_table_index)
{
    ASSUMPTION(has_visible_symbol(definition_file,symbol_table_index));
    auto const  it = m_visible_symbol_table->find({definition_file,symbol_table_index});
    return std::get<2>(it->second);
}

uint64_t  load_props_elf::visible_symbol_size(std::string const& definition_file,  uint64_t const  symbol_table_index)
{
    ASSUMPTION(has_visible_symbol(definition_file,symbol_table_index));
    auto const  it = m_visible_symbol_table->find({definition_file,symbol_table_index});
    return std::get<3>(it->second);
}


void  load_props_elf::add_hidden_symbol(std::string const& definition_file,  uint64_t const  symbol_table_index,
                                        relocation_symbol_id const& symbol_id, std::string const&  symbol_type,
                                        uint64_t const  symbol_value, uint64_t const symbol_size)
{
    ASSUMPTION(!has_symbol(definition_file,symbol_table_index));
    ASSUMPTION(!has_hidden_symbol(definition_file,symbol_table_index));
    m_hidden_symbol_table->insert({{definition_file,symbol_table_index},
                                   std::make_tuple(symbol_id,symbol_type,symbol_value,symbol_size)});
}

bool  load_props_elf::has_hidden_symbol(std::string const& definition_file,  uint64_t const  symbol_table_index)
{
    return m_hidden_symbol_table->find({definition_file,symbol_table_index}) != m_hidden_symbol_table->cend();
}

relocation_symbol_id const& load_props_elf::hidden_symbol_id(std::string const& definition_file,  uint64_t const  symbol_table_index)
{
    ASSUMPTION(has_hidden_symbol(definition_file,symbol_table_index));
    auto const  it = m_hidden_symbol_table->find({definition_file,symbol_table_index});
    return std::get<0>(it->second);
}

std::string  load_props_elf::hidden_symbol_type(std::string const& definition_file,  uint64_t const  symbol_table_index)
{
    ASSUMPTION(has_hidden_symbol(definition_file,symbol_table_index));
    auto const  it = m_hidden_symbol_table->find({definition_file,symbol_table_index});
    return std::get<1>(it->second);
}

uint64_t  load_props_elf::hidden_symbol_value(std::string const& definition_file,  uint64_t const  symbol_table_index)
{
    ASSUMPTION(has_hidden_symbol(definition_file,symbol_table_index));
    auto const  it = m_hidden_symbol_table->find({definition_file,symbol_table_index});
    return std::get<2>(it->second);
}

uint64_t  load_props_elf::hidden_symbol_size(std::string const& definition_file,  uint64_t const  symbol_table_index)
{
    ASSUMPTION(has_hidden_symbol(definition_file,symbol_table_index));
    auto const  it = m_hidden_symbol_table->find({definition_file,symbol_table_index});
    return std::get<3>(it->second);
}


bool  load_props_elf::has_symbol(std::string const& definition_file,  uint64_t const  symbol_table_index)
{
    return has_visible_symbol(definition_file,symbol_table_index) || has_hidden_symbol(definition_file,symbol_table_index);
}

relocation_symbol_id const& load_props_elf::symbol_id(std::string const& definition_file,  uint64_t const  symbol_table_index)
{
    return has_visible_symbol(definition_file,symbol_table_index) ? visible_symbol_id(definition_file,symbol_table_index) :
                                                                    hidden_symbol_id(definition_file,symbol_table_index) ;
}

std::string  load_props_elf::symbol_type(std::string const& definition_file,  uint64_t const  symbol_table_index)
{
    return has_visible_symbol(definition_file,symbol_table_index) ? visible_symbol_type(definition_file,symbol_table_index) :
                                                                    hidden_symbol_type(definition_file,symbol_table_index) ;
}

uint64_t  load_props_elf::symbol_value(std::string const& definition_file,  uint64_t const  symbol_table_index)
{
    return has_visible_symbol(definition_file,symbol_table_index) ? visible_symbol_value(definition_file,symbol_table_index) :
                                                                    hidden_symbol_value(definition_file,symbol_table_index) ;
}

uint64_t  load_props_elf::symbol_size(std::string const& definition_file,  uint64_t const  symbol_table_index)
{
    return has_visible_symbol(definition_file,symbol_table_index) ? visible_symbol_size(definition_file,symbol_table_index) :
                                                                    hidden_symbol_size(definition_file,symbol_table_index) ;
}

std::string  load_props_elf::compute_symbol_address(std::string const& relocated_file,  uint64_t const  symbol_table_index,
                                                    address& output_address)
{
    if (!has_symbol(relocated_file,symbol_table_index))
        return "The relocated symbol was not found in the symbol table.";
    relocation_symbol_id const&  id = symbol_id(relocated_file,symbol_table_index);
    for (auto const& elem : *m_visible_symbol_table)
        if (elem.first.first != relocated_file && std::get<0>(elem.second) == id)
        {
            output_address = fixed_address_for(elem.first.first,std::get<2>(elem.second));
            return "";
        }
    for (auto const& elem : *m_visible_symbol_table)
        if (std::get<0>(elem.second) == id)
        {
            output_address = fixed_address_for(elem.first.first,std::get<2>(elem.second));
            return "";
        }

    auto const  it = m_hidden_symbol_table->find({relocated_file,symbol_table_index});
    if (it != m_hidden_symbol_table->cend() && std::get<1>(it->second) != loader::relocation_type::UNKNOWN())
    {
        output_address = fixed_address_for(relocated_file,std::get<2>(it->second));
        return "";
    }

    return "The relocated symbol was not found in none of the loaded binaries.";
}

std::string  load_props_elf::compute_definition_file_from_symbol_index(uint64_t const  symbol_table_index) const
{
    auto const  it = m_from_symbol_indices_to_files.find(symbol_table_index);
    return it == m_from_symbol_indices_to_files.cend() ? "" : it->second;
}

special_section::elf_tls_ptr  load_props_elf::tls(std::string const&  loaded_file)
{
    auto const  elf_specials = special_sections()->find(loaded_file);
    if (elf_specials != special_sections()->end())
    {
        auto const  it = elf_specials->second.find(special_section_name::thread_local_storage());
        if (it != elf_specials->second.end())
            return std::dynamic_pointer_cast<special_section::elf_tls const>(it->second);
    }
    return {};
}

void  load_props_elf::add_performed_relocation(loader::relocation const&  reloc)
{
    m_performed_relocations->insert({reloc.start_address(),reloc});
}

void  load_props_elf::add_skipped_relocation(loader::relocation const&  reloc)
{
    m_skipped_relocations->insert({reloc.start_address(),reloc});
}


void  load_props_elf::add_section_address_fix(std::string const&  elf_path, address const  orig_adr, address const  fixed_adr)
{
    auto it = m_address_fixes->find(elf_path);
    if (it == m_address_fixes->end())
    {
        address_fixes_map_ptr const  map{new address_fixes_map};
        map->insert({orig_adr,fixed_adr});
        m_address_fixes->insert({elf_path,map});
    }
    else
    {
        ASSUMPTION(it->second->count(orig_adr) == 0ULL);
        it->second->insert({orig_adr,fixed_adr});
    }
}


address_fixes_map_ptr  load_props_elf::address_fixes(std::string const&  elf_path)
{
    auto const it = m_address_fixes->find(elf_path);
    ASSUMPTION(it != m_address_fixes->end());
    return it->second;
}

bool  load_props_elf::can_address_be_fixed(std::string const&  elf_path, address const  orig_adr) const
{
    auto const it = m_address_fixes->find(elf_path);
    if (it == m_address_fixes->end())
        return false;
    return detail::can_address_be_fixed(orig_adr,*it->second);
}

address  load_props_elf::fixed_base_address_for(std::string const&  elf_path) const
{
    auto const it = m_address_fixes->find(elf_path);
    ASSUMPTION(it != m_address_fixes->end());
    return fixed_base_address(it->second);
}

address  load_props_elf::fixed_address_for(std::string const&  elf_path, address const  orig_adr) const
{
    auto const it = m_address_fixes->find(elf_path);
    ASSUMPTION(it != m_address_fixes->end());
    return detail::adjust_address_to_fixed_sections(orig_adr,*it->second);
}


void  load_props_elf::add_warning(std::string const&  elf_path, std::string const&  warning_message)
{
    ASSUMPTION(!warning_message.empty());
    (*m_warnings)[elf_path].push_back(warning_message);
}


}}
