#include <rebours/MAL/loader/detail/load_props_mach.hpp>
#include <rebours/MAL/loader/assumptions.hpp>
#include <rebours/MAL/loader/invariants.hpp>
#include <rebours/MAL/loader/file_utils.hpp>
#include <fstream>
#include <algorithm>

namespace loader { namespace detail {


mach_symbol_id::mach_symbol_id(std::string const&  binary_name,
                               uint32_t const  ordinal,
                               std::string const&  symbol_name,
                               uint8_t const  type_flags,
                               uint8_t const  section_number,
                               uint16_t const  description,
                               uint64_t const  value)
    : m_binary_name(binary_name)
    , m_ordinal(ordinal)
    , m_symbol_name(symbol_name)
    , m_type_flags(type_flags)
    , m_section_number(section_number)
    , m_description(description)
    , m_value(value)
    , m_address(0ULL)
{
    ASSUMPTION(!m_binary_name.empty());
    ASSUMPTION(!m_symbol_name.empty());
}

std::string  mach_symbol_id::full_name() const
{
    std::ostringstream  ostr;
    ostr << "[" << binary_name() << "]";
    ostr << "[" << ordinal() << "]";
    if (!symbol_name().empty())
        ostr << "[" << symbol_name() << "]";
    else
        ostr << "[]";

    return ostr.str();
}

loader::address  mach_symbol_id::address() const
{
    return m_address;
}

void  mach_symbol_id::set_address(loader::address const  address)
{
    ASSUMPTION(address != 0ULL && (!is_address_set() || m_address == address));
    m_address = address;
}

//bool  operator==(mach_symbol_id const&  l, mach_symbol_id const&  r)
//{
//    return l.binary_name() == r.binary_name() &&
//           l.ordinal() == r.ordinal() &&
//           l.symbol_name() == r.symbol_name() ;
//}

//bool  operator<(mach_symbol_id const&  l, mach_symbol_id const&  r)
//{
//    if (l.binary_name() < r.binary_name())
//        return true;
//    if (l.ordinal() < r.ordinal())
//        return true;
//    return l.symbol_name() < r.symbol_name();
//}


load_props_mach::load_props_mach(std::string const&  root_file,
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
    , m_address_fixes{new std::unordered_map<std::string,std::vector<std::pair<address,address> > >}
    , m_libraries{new std::unordered_map<std::string,std::vector<std::string> >}
    , m_system_libs{}
    , m_symbols{}
    , m_warnings(new loader::warnings)
{
    (*m_file_forward_dependencies)[m_root_file] = {};
    (*m_file_backward_dependencies)[m_root_file] = {};
}

void  load_props_mach::set_platform(platform_ptr const  platform)
{
    ASSUMPTION( !this->platform().operator bool() );
    m_platform = platform;
}

void  load_props_mach::set_entry_point(address const  entry_point)
{
    m_entry_point = entry_point;
}

std::string  load_props_mach::add_section(section_ptr const  new_section)
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

address  load_props_mach::first_highest_free_vm_page() const
{
    return m_sections_table->empty() ?
                0ULL :
                aligned_high(m_sections_table->rbegin()->second->end_address(),0x1000ULL);
}

address  load_props_mach::first_free_vm_page_before(address const  adr) const
{
    if (m_sections_table->empty())
        return 0ULL;
    auto const  it = std::prev(m_sections_table->lower_bound(adr));
    address const  result = aligned_high(it->second->end_address(),0x1000ULL);
    return (result >= adr) ? first_highest_free_vm_page() : result;
}

address  load_props_mach::first_free_vm_page(address const  start_address, uint64_t const  size) const
{
    address  result = aligned_low(start_address,0x1000ULL);

    auto  it = m_sections_table->lower_bound(result);
    if (it == m_sections_table->cend())
        return result;
    if (it->first >= result + size)
    {
        if (it == m_sections_table->cbegin())
            return result;
        it = std::prev(it);
        if (it->second->end_address() <= result)
            return result;
    }

    for ( ; it != m_sections_table->cend(); ++it)
    {
        result = aligned_high(it->second->end_address(),0x1000ULL);
        auto nxt_it = std::next(it);
        if (nxt_it != m_sections_table->cend() && nxt_it->first >= result + size)
            return result;
    }

    return result;
}

void  load_props_mach::add_special_section(std::string const&  binary_file,
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

void  load_props_mach::add_file_props(file_props_ptr const  file_props)
{
    ASSUMPTION(std::find(m_skipped_files->cbegin(),m_skipped_files->cend(),file_props->path()) == m_skipped_files->cend());
    auto const  insert_result =
            m_files_table->insert(loader::files_table::value_type(file_props->path(),file_props));
    ASSUMPTION(insert_result.second);
    (void)insert_result;
}

void  load_props_mach::add_skipped_file(std::string const&  file_pathname)
{
    ASSUMPTION(m_files_table->count(file_pathname) == 0ULL);
    m_skipped_files->insert(file_pathname);
}

void  load_props_mach::add_file_dependence(std::string const&  src_file, std::string const&  dst_file)
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

void  load_props_mach::add_init_function(std::string const& file, address const  fn_address)
{
    ASSUMPTION(m_init_functions->count(file) == 0 ||
               std::find(m_init_functions->at(file).cbegin(),m_init_functions->at(file).cend(),fn_address)
                    == m_init_functions->at(file).cend());
    ASSUMPTION(find_section(fn_address,sections_table()).operator bool() &&
               find_section(fn_address,sections_table())->has_execute_access());
    (*m_init_functions)[file].push_back(fn_address);
}

void  load_props_mach::add_fini_function(std::string const& file, address const  fn_address)
{
    ASSUMPTION(m_fini_functions->count(file) == 0 ||
               std::find(m_fini_functions->at(file).cbegin(),m_fini_functions->at(file).cend(),fn_address)
                    == m_fini_functions->at(file).cend());
    ASSUMPTION(find_section(fn_address,sections_table()).operator bool() &&
               find_section(fn_address,sections_table())->has_execute_access());
    (*m_fini_functions)[file].push_back(fn_address);
}

void  load_props_mach::add_performed_relocation(loader::relocation const&  reloc)
{
    m_performed_relocations->insert({reloc.start_address(),reloc});
}

void  load_props_mach::add_skipped_relocation(loader::relocation const&  reloc)
{
    m_skipped_relocations->insert({reloc.start_address(),reloc});
}


void  load_props_mach::add_address_fix(std::string const&  mach_path, address const  orig_adr, address const  fixed_adr)
{
    auto it = m_address_fixes->find(mach_path);
    if (it == m_address_fixes->end())
        m_address_fixes->insert({mach_path,{{orig_adr,fixed_adr}}});
    else
        it->second.push_back({orig_adr,fixed_adr});
}

address  load_props_mach::fixed_address_for(std::string const&  mach_path, uint32_t const  segment_index) const
{
    auto const it = m_address_fixes->find(mach_path);
    ASSUMPTION(it != m_address_fixes->end());
    ASSUMPTION(segment_index < it->second.size());
    return it->second.at(segment_index).second;
}

address  load_props_mach::prefered_address_for(std::string const&  mach_path, uint32_t const  segment_index) const
{
    auto const it = m_address_fixes->find(mach_path);
    ASSUMPTION(it != m_address_fixes->end());
    ASSUMPTION(segment_index < it->second.size());
    return it->second.at(segment_index).first;
}


void  load_props_mach::add_library(std::string const&  mach_path, std::string const&  library_mach_path)
{
    auto it = m_libraries->find(mach_path);
    if (it == m_libraries->end())
        m_libraries->insert({mach_path,{library_mach_path}});
    else
        it->second.push_back(library_mach_path);
}

std::string const&  load_props_mach::get_library(std::string const&  mach_path, uint32_t const  library_index)
{
    auto const it = m_libraries->find(mach_path);
    ASSUMPTION(it != m_libraries->end());
    ASSUMPTION(library_index < it->second.size());
    return it->second.at(library_index);
}

bool  load_props_mach::has_library(std::string const&  mach_path, uint32_t const  library_index)
{
    auto const it = m_libraries->find(mach_path);
    ASSUMPTION(it != m_libraries->end());
    return library_index < it->second.size();
}


void  load_props_mach::add_symbol(mach_symbol_id const&  symbol_id)
{
    auto it = m_symbols.find(symbol_id.binary_name());
    if (it == m_symbols.end())
        m_symbols.insert({symbol_id.binary_name(),{symbol_id}});
    else
        it->second.push_back(symbol_id);
}

uint32_t  load_props_mach::get_max_symbol_id(std::string const&  binary_name)
{
    ASSUMPTION(!binary_name.empty());
    auto it = m_symbols.find(binary_name);
    ASSUMPTION(it != m_symbols.end());
    uint32_t  result = 0U;
    for (auto& elem : it->second)
        if (result < elem.ordinal())
            result = elem.ordinal();
    return result;
}

mach_symbol_id&  load_props_mach::get_symbol(std::string const&  binary_name, uint32_t const  ordinal)
{
    ASSUMPTION(!binary_name.empty());
    auto it = m_symbols.find(binary_name);
    ASSUMPTION(it != m_symbols.end());
    for (auto& elem : it->second)
        if (elem.ordinal() == ordinal)
            return elem;
    UNREACHABLE();
}

mach_symbol_id&  load_props_mach::get_symbol(std::string const&  binary_name, std::string const&  symbol_name)
{
    ASSUMPTION(!symbol_name.empty());
    if (binary_name.empty())
    {
        for (auto& rec : m_symbols)
            for (auto& elem : rec.second)
            if (elem.symbol_name() == symbol_name)
                return elem;
    }
    else
    {
        auto it = m_symbols.find(binary_name);
        ASSUMPTION(it != m_symbols.end());
        for (auto& elem : it->second)
            if (elem.symbol_name() == symbol_name)
                return elem;
    }
    UNREACHABLE();
}

bool  load_props_mach::has_symbol(std::string const&  binary_name, std::string const&  symbol_name)
{
    ASSUMPTION(!symbol_name.empty());
    if (binary_name.empty())
    {
        for (auto& rec : m_symbols)
            for (auto& elem : rec.second)
            if (elem.symbol_name() == symbol_name)
                return true;
    }
    else
    {
        auto it = m_symbols.find(binary_name);
        ASSUMPTION(it != m_symbols.end());
        for (auto& elem : it->second)
            if (elem.symbol_name() == symbol_name)
                return true;
    }
    return false;
}

void  load_props_mach::set_symbol_address(std::string const&  binary_name, uint32_t const  ordinal, address const  adr)
{
    ASSUMPTION(!binary_name.empty());
    ASSUMPTION(adr != 0ULL);
    auto it = m_symbols.find(binary_name);
    ASSUMPTION(it != m_symbols.end());
    for (auto& elem : it->second)
        if (elem.ordinal() == ordinal)
        {
            elem.set_address(adr);
            return;
        }
    UNREACHABLE();
}

void  load_props_mach::set_symbol_address(std::string const&  binary_name, std::string const&  symbol_name, address const  adr)
{
    ASSUMPTION(!binary_name.empty());
    ASSUMPTION(adr != 0ULL);
    auto it = m_symbols.find(binary_name);
    ASSUMPTION(it != m_symbols.end());
    for (auto& elem : it->second)
        if (elem.symbol_name() == symbol_name)
        {
            elem.set_address(adr);
            return;
        }
    UNREACHABLE();
}

address  load_props_mach::symbol_address(std::string const&  binary_name, uint32_t const  ordinal) const
{
    ASSUMPTION(!binary_name.empty());
    auto it = m_symbols.find(binary_name);
    ASSUMPTION(it != m_symbols.end());
    for (auto& elem : it->second)
        if (elem.ordinal() == ordinal)
            return elem.address();
    return 0ULL;
}

address  load_props_mach::symbol_address(std::string const&  binary_name, std::string const&  symbol_name) const
{
    ASSUMPTION(!binary_name.empty());
    ASSUMPTION(!symbol_name.empty());
    auto it = m_symbols.find(binary_name);
    ASSUMPTION(it != m_symbols.end());
    for (auto& elem : it->second)
        if (elem.symbol_name() == symbol_name)
            return elem.address();
    return 0ULL;
}


void  load_props_mach::add_warning(std::string const&  mach_path, std::string const&  warning_message)
{
    ASSUMPTION(!warning_message.empty());
    (*m_warnings)[mach_path].push_back(warning_message);
}


}}
