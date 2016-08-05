#ifndef REBOURS_MAL_LOADER_DETAIL_LOAD_PROPS_MACH_HPP_INCLUDED
#   define REBOURS_MAL_LOADER_DETAIL_LOAD_PROPS_MACH_HPP_INCLUDED

#   include <rebours/MAL/loader/detail/mutable_sections_table.hpp>
#   include <rebours/MAL/loader/special_sections.hpp>
#   include <rebours/MAL/loader/detail/address_fixing.hpp>
#   include <rebours/MAL/loader/detail/std_pair_hash.hpp>
#   include <rebours/MAL/loader/address.hpp>
#   include <rebours/MAL/loader/platform.hpp>
#   include <rebours/MAL/loader/file_props.hpp>
#   include <rebours/MAL/loader/dependencies_graph.hpp>
#   include <rebours/MAL/loader/init_fini.hpp>
#   include <rebours/MAL/loader/relocations.hpp>
#   include <rebours/MAL/loader/warnings.hpp>
#   include <unordered_map>
#   include <iostream>
#   include <sstream>
#   include <vector>
#   include <string>
#   include <tuple>
#   include <set>
#   include <memory>


#   define LOAD_MACH_WARNING_IN(mach_file,SSTREAM_EXPRESSION) \
            {\
                std::stringstream  sstr;\
                sstr << SSTREAM_EXPRESSION;\
                std::string const warning_message = sstr.str();\
                load_props.add_warning(mach_file,warning_message);\
                std::cout << "WARNING: " << warning_message << "\n";\
            }
#   define LOAD_MACH_WARNING(SSTREAM_EXPRESSION)  LOAD_MACH_WARNING_IN(mach_props->path(),SSTREAM_EXPRESSION)

#   define LOAD_MACH_ERROR(MSG)  dynamic_cast<std::ostringstream&>(std::ostringstream{} << MSG).str()


namespace loader { namespace detail {


struct mach_symbol_id
{
    mach_symbol_id(std::string const&  binary_name,
                   uint32_t const  ordinal,
                   std::string const&  symbol_name,
                   uint8_t const  type_flags,
                   uint8_t const  section_number,
                   uint16_t const  description,
                   uint64_t const  value
                   );

    std::string const&  binary_name() const { return m_binary_name; }
    uint32_t  ordinal() const { return m_ordinal; }
    std::string const&  symbol_name() const { return m_symbol_name; }
    uint8_t  type_flags() const { return m_type_flags; }
    uint8_t  section_number() const { return m_section_number; }
    uint16_t  description() const { return m_description; }
    uint64_t  value() const { return m_value; }

    bool  is_address_set() const { return m_address != 0ULL; }
    loader::address  address() const;
    void  set_address(loader::address const  address);

    std::string  full_name() const;

private:
    std::string  m_binary_name;
    uint32_t  m_ordinal;
    std::string  m_symbol_name;
    uint8_t  m_type_flags;
    uint8_t  m_section_number;
    uint16_t  m_description;
    uint64_t  m_value;
    loader::address  m_address;
};


//bool  operator==(mach_symbol_id const&  l, mach_symbol_id const&  r);
//bool  operator<(mach_symbol_id const&  l, mach_symbol_id const&  r);


struct load_props_mach
{
    load_props_mach(std::string const&  root_file,
                    std::vector<std::string> const&  ignored_files,
                    std::vector<std::string> const&  search_dirs);
    std::string const&  root_file() const { return m_root_file; }
    std::vector<std::string> const&  ignored_files() const { return m_ignored_files; }
    std::vector<std::string> const&  search_dirs() const { return m_search_dirs; }

    void  set_platform(platform_ptr const  platform);
    platform_ptr  platform() const { return m_platform; }

    void  set_entry_point(address const  entry_point);
    address  entry_point() const { return m_entry_point; }

    std::string  add_section(section_ptr const  new_section);
    mutable_sections_table_ptr  sections_table() { return m_sections_table; }
    address  first_highest_free_vm_page() const;
    address  first_free_vm_page_before(address const  adr) const;
    address  first_free_vm_page(address const  start_address, uint64_t const  size) const;

    void  add_special_section(std::string const&  binary_file,
                              std::string const&  section_name,
                              special_section_properties_ptr const  section_props);
    std::shared_ptr<loader::special_sections_table>  special_sections() const { return m_special_sections; }

    void  add_file_props(file_props_ptr const  file_props);
    std::shared_ptr<loader::files_table> files_table() const { return m_files_table; }

    void  add_skipped_file(std::string const&  file_pathname);
    std::shared_ptr<loader::skipped_files>  skipped_files() const { return m_skipped_files; }
    bool  is_skipped_file(std::string const&  file_pathname) const { return m_skipped_files->count(file_pathname) != 0ULL; }

    void  add_file_dependence(std::string const&  src_file, std::string const&  dst_file);
    std::shared_ptr<file_dependencies_map>  file_forward_dependencies() { return m_file_forward_dependencies; }
    std::shared_ptr<file_dependencies_map>  file_backward_dependencies() { return m_file_backward_dependencies; }

    std::shared_ptr<special_functions_of_files>  init_functions() const { return m_init_functions; }
    std::shared_ptr<special_functions_of_files>  fini_functions() const { return m_fini_functions; }

    void  add_init_function(std::string const& file, address const  fn_address);
    void  add_fini_function(std::string const& file, address const  fn_address);

    void  add_performed_relocation(loader::relocation const&  reloc);
    void  add_skipped_relocation(loader::relocation const&  reloc);
    std::shared_ptr<loader::relocations>  performed_relocations() const { return m_performed_relocations; }
    std::shared_ptr<loader::relocations>  skipped_relocations() const { return m_skipped_relocations; }

    void  add_address_fix(std::string const&  mach_path, address const  orig_adr, address const  fixed_adr);
    address  fixed_address_for(std::string const&  mach_path, uint32_t const  segment_index) const;
    address  prefered_address_for(std::string const&  mach_path, uint32_t const  segment_index) const;
    address  fixed_base_address_for(std::string const&  mach_path) const { return fixed_address_for(mach_path,0U); }
    address  prefered_base_address_for(std::string const&  mach_path) const { return prefered_address_for(mach_path,0U); }

    void  add_library(std::string const&  mach_path, std::string const&  library_mach_path);
    std::string const&  get_library(std::string const&  mach_path, uint32_t const  library_index);
    bool  has_library(std::string const&  mach_path, uint32_t const  library_index);

    bool  is_system_library(std::string const&  mach_path) const { return m_system_libs.count(mach_path) != 0ULL; }
    void  add_system_library(std::string const&  mach_path) { m_system_libs.insert(mach_path); }

    void  add_symbol(mach_symbol_id const&  symbol_id);
    mach_symbol_id&  get_symbol(std::string const&  binary_name, uint32_t const  ordinal);
    mach_symbol_id&  get_symbol(std::string const&  binary_name, std::string const&  symbol_name);
    bool  has_symbol(std::string const&  binary_name, std::string const&  symbol_name);
    uint32_t  get_max_symbol_id(std::string const&  binary_name);
    void  set_symbol_address(std::string const&  binary_name, uint32_t const  ordinal, address const  adr);
    void  set_symbol_address(std::string const&  binary_name, std::string const&  symbol_name, address const  adr);
    address  symbol_address(std::string const&  binary_name, uint32_t const  ordinal) const;
    address  symbol_address(std::string const&  binary_name, std::string const&  symbol_name) const;

    void  add_warning(std::string const&  mach_path, std::string const&  warning_message);
    std::shared_ptr<loader::warnings>  warnings() const { return m_warnings; }

private:

    std::string  m_root_file;
    std::vector<std::string>  m_ignored_files;
    std::vector<std::string>  m_search_dirs;
    platform_ptr  m_platform;
    address  m_entry_point;
    mutable_sections_table_ptr  m_sections_table;
    std::shared_ptr<loader::special_sections_table>  m_special_sections;
    std::shared_ptr<loader::files_table>  m_files_table;
    std::shared_ptr<loader::skipped_files>  m_skipped_files;
    std::shared_ptr<file_dependencies_map>  m_file_forward_dependencies;
    std::shared_ptr<file_dependencies_map>  m_file_backward_dependencies;
    std::shared_ptr<special_functions_of_files>  m_init_functions;
    std::shared_ptr<special_functions_of_files>  m_fini_functions;
    std::shared_ptr<loader::relocations>  m_performed_relocations;
    std::shared_ptr<loader::relocations>  m_skipped_relocations;
    std::shared_ptr<std::unordered_map<std::string,std::vector<std::pair<address,address> > > >  m_address_fixes;
    std::shared_ptr<std::unordered_map<std::string,std::vector<std::string> > >  m_libraries;
    std::set<std::string>  m_system_libs;
    std::unordered_map<std::string,std::vector<mach_symbol_id> >  m_symbols;
    std::shared_ptr<loader::warnings>  m_warnings;
};


}}

#endif
