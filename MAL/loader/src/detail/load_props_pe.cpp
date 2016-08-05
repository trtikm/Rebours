#include <rebours/MAL/loader/detail/load_props_pe.hpp>
#include <rebours/MAL/loader/assumptions.hpp>
#include <rebours/MAL/loader/invariants.hpp>
#include <rebours/MAL/loader/file_utils.hpp>
#include <fstream>
#include <algorithm>

namespace loader { namespace detail {


pe_symbol_id::pe_symbol_id(std::string const&  binary_name, uint32_t const  ordinal, std::string const&  symbol_name)
    : m_binary_name(binary_name)
    , m_ordinal(ordinal)
    , m_symbol_name(symbol_name)
{
    ASSUMPTION(!m_binary_name.empty());
    ASSUMPTION(m_ordinal != 0U || !m_symbol_name.empty());
}

std::string  pe_symbol_id::full_name() const
{
    std::ostringstream  ostr;
    ostr << "[" << binary_name() << "]";
    if (ordinal() != 0U)
        ostr << "[" << ordinal() << "]";
    else
        ostr << "[]";
    if (!symbol_name().empty())
        ostr << "[" << symbol_name() << "]";
    else
        ostr << "[]";

    return ostr.str();
}

std::string  parse_forwarder(std::string const&  this_binary_name,
                             std::string const&  forwarder,
                             std::string&  binary_name,
                             uint32_t&  ordinal,
                             std::string&  symbol_name)
{
    if (forwarder.empty())
        return "The forwarder is empty.";
    auto const  ord_pos = forwarder.find_first_not_of(".#");
    if (forwarder.find_first_of(".#") == std::string::npos)
    {
        binary_name = this_binary_name;
        ordinal = 0U;
        symbol_name = forwarder;
    }
    else
    {
        if (ord_pos == 0U)
            return "Cannot parse binary name from the forwarder.";
        binary_name = forwarder.substr(0U,ord_pos);
        std::istringstream  istr(forwarder.substr(ord_pos+2U));
        istr >> ordinal;
        if (ordinal == 0U)
            return "Symbol's forwarder has an invalid ordinal.";
        symbol_name.clear();
    }
    return "";
}

bool  operator==(pe_symbol_id const&  l, pe_symbol_id const&  r)
{
    return l.binary_name() == r.binary_name() &&
           l.ordinal() == r.ordinal() &&
           l.symbol_name() == r.symbol_name() ;
}

bool  operator<(pe_symbol_id const&  l, pe_symbol_id const&  r)
{
    if (l.binary_name() < r.binary_name())
        return true;
    if (l.ordinal() < r.ordinal())
        return true;
    return l.symbol_name() < r.symbol_name();
}



load_props_pe::load_props_pe(std::string const&  root_file,
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
    , m_init_functions(new special_functions_of_files)
    , m_fini_functions(new special_functions_of_files)
    , m_performed_relocations(new loader::relocations)
    , m_skipped_relocations(new loader::relocations)
    , m_address_fixes{new std::unordered_map<std::string,address_fixes_map_ptr>}
    , m_symbols{}
    , m_warnings(new loader::warnings)
{
    (*m_file_forward_dependencies)[m_root_file] = {};
    (*m_file_backward_dependencies)[m_root_file] = {};
}

void  load_props_pe::set_platform(platform_ptr const  platform)
{
    ASSUMPTION( !this->platform().operator bool() );
    m_platform = platform;
}

void  load_props_pe::set_entry_point(address const  entry_point)
{
    m_entry_point = entry_point;
}

std::string  load_props_pe::add_section(section_ptr const  new_section)
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

address  load_props_pe::highest_end_address() const
{
    address  result = 0ULL;
    for (auto const& elem : *m_sections_table)
        if (result < elem.second->end_address())
            result = elem.second->end_address();
    return result;
}

void  load_props_pe::add_special_section(std::string const&  binary_file,
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

void  load_props_pe::add_file_props(file_props_ptr const  file_props)
{
    ASSUMPTION(std::find(m_skipped_files->cbegin(),m_skipped_files->cend(),file_props->path()) == m_skipped_files->cend());
    auto const  insert_result =
            m_files_table->insert(loader::files_table::value_type(file_props->path(),file_props));
    ASSUMPTION(insert_result.second);
    (void)insert_result;
}

void  load_props_pe::add_skipped_file(std::string const&  file_pathname)
{
    ASSUMPTION(m_files_table->count(file_pathname) == 0ULL);
    m_skipped_files->insert(file_pathname);
}

void  load_props_pe::add_file_dependence(std::string const&  src_file, std::string const&  dst_file)
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

void  load_props_pe::add_init_function(std::string const& file, address const  fn_address)
{
    ASSUMPTION(m_init_functions->count(file) == 0 ||
               std::find(m_init_functions->at(file).cbegin(),m_init_functions->at(file).cend(),fn_address)
                    == m_init_functions->at(file).cend());
    ASSUMPTION(find_section(fn_address,sections_table()).operator bool() &&
               find_section(fn_address,sections_table())->has_execute_access());
    (*m_init_functions)[file].push_back(fn_address);
}

void  load_props_pe::add_fini_function(std::string const& file, address const  fn_address)
{
    ASSUMPTION(m_fini_functions->count(file) == 0 ||
               std::find(m_fini_functions->at(file).cbegin(),m_fini_functions->at(file).cend(),fn_address)
                    == m_fini_functions->at(file).cend());
    ASSUMPTION(find_section(fn_address,sections_table()).operator bool() &&
               find_section(fn_address,sections_table())->has_execute_access());
    (*m_fini_functions)[file].push_back(fn_address);
}

void  load_props_pe::add_performed_relocation(loader::relocation const&  reloc)
{
    m_performed_relocations->insert({reloc.start_address(),reloc});
}

void  load_props_pe::add_skipped_relocation(loader::relocation const&  reloc)
{
    m_skipped_relocations->insert({reloc.start_address(),reloc});
}


void  load_props_pe::add_section_address_fix(std::string const&  pe_path, address const  orig_adr, address const  fixed_adr)
{
    auto it = m_address_fixes->find(pe_path);
    if (it == m_address_fixes->end())
    {
        address_fixes_map_ptr const  map{new address_fixes_map};
        map->insert({orig_adr,fixed_adr});
        m_address_fixes->insert({pe_path,map});
    }
    else
    {
        ASSUMPTION(it->second->count(orig_adr) == 0ULL);
        it->second->insert({orig_adr,fixed_adr});
    }
}


address_fixes_map_ptr  load_props_pe::address_fixes(std::string const&  pe_path)
{
    auto const it = m_address_fixes->find(pe_path);
    ASSUMPTION(it != m_address_fixes->end());
    return it->second;
}

bool  load_props_pe::can_address_be_fixed(std::string const&  pe_path, address const  orig_adr) const
{
    auto const it = m_address_fixes->find(pe_path);
    if (it == m_address_fixes->end())
        return false;
    return detail::can_address_be_fixed(orig_adr,*it->second);
}

address  load_props_pe::fixed_base_address_for(std::string const&  pe_path) const
{
    auto const it = m_address_fixes->find(pe_path);
    ASSUMPTION(it != m_address_fixes->end());
    return fixed_base_address(it->second);
}

address  load_props_pe::fixed_address_for(std::string const&  pe_path, address const  orig_adr) const
{
    auto const it = m_address_fixes->find(pe_path);
    ASSUMPTION(it != m_address_fixes->end());
    return detail::adjust_address_to_fixed_sections(orig_adr,*it->second);
}

address  load_props_pe::highest_address_fix() const
{
    address  result = 0ULL;
    for (auto const& elem : *m_address_fixes)
        if (!elem.second->empty())
        {
            address const  highest = std::prev(elem.second->cend())->second;
            if (result < highest)
                result = highest;
        }
    return result;
}

void  load_props_pe::add_symbol(pe_symbol_id const&  symbol_id, address const  adr)
{
    ASSUMPTION(!symbol_id.binary_name().empty());
    ASSUMPTION(symbol_id.ordinal() != 0U || !symbol_id.symbol_name().empty());
    ASSUMPTION(adr != 0ULL);
    auto const  result = m_symbols.insert(std::map<pe_symbol_id,address>::value_type{symbol_id,adr});
    ASSUMPTION(result.second);
    (void)result;
}

address  load_props_pe::symbol_address(std::string const&  binary_name, uint32_t const  ordinal) const
{
    ASSUMPTION(!binary_name.empty());
    ASSUMPTION(ordinal != 0U);
    for (auto const& elem : m_symbols)
        if (elem.first.binary_name() == binary_name && elem.first.ordinal() == ordinal)
            return elem.second;
    return 0ULL;
}

address  load_props_pe::symbol_address(std::string const&  binary_name, std::string const&  symbol_name) const
{
    ASSUMPTION(!binary_name.empty());
    ASSUMPTION(!symbol_name.empty());
    for (auto const& elem : m_symbols)
        if (elem.first.binary_name() == binary_name && elem.first.symbol_name() == symbol_name)
            return elem.second;
    return 0ULL;
}

address  load_props_pe::symbol_address(pe_symbol_id const&  symbol_id) const
{
    auto const  it = m_symbols.find(symbol_id);
    if (it != m_symbols.cend())
        return it->second;
    return symbol_id.ordinal() != 0U ? symbol_address(symbol_id.binary_name(),symbol_id.ordinal()) :
                                       symbol_address(symbol_id.binary_name(),symbol_id.symbol_name()) ;
}

void  load_props_pe::add_warning(std::string const&  pe_path, std::string const&  warning_message)
{
    ASSUMPTION(!warning_message.empty());
    (*m_warnings)[pe_path].push_back(warning_message);
}


}}
