#ifndef REBOURS_MAL_LOADER_DETAIL_LOAD_PROPS_PE_HPP_INCLUDED
#   define REBOURS_MAL_LOADER_DETAIL_LOAD_PROPS_PE_HPP_INCLUDED

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
#   include <memory>


#   define LOAD_PE_WARNING_IN(pe_file,SSTREAM_EXPRESSION) \
            {\
                std::stringstream  sstr;\
                sstr << SSTREAM_EXPRESSION;\
                std::string const warning_message = sstr.str();\
                load_props.add_warning(pe_file,warning_message);\
                std::cout << "WARNING: " << warning_message << "\n";\
            }
#   define LOAD_PE_WARNING(SSTREAM_EXPRESSION)  LOAD_PE_WARNING_IN(pe_props->path(),SSTREAM_EXPRESSION)


namespace loader { namespace detail {


struct pe_symbol_id
{
    pe_symbol_id(std::string const&  binary_name, uint32_t const  ordinal, std::string const&  symbol_name);

    std::string const&  binary_name() const { return m_binary_name; }
    uint32_t  ordinal() const { return m_ordinal; }
    std::string const&  symbol_name() const { return m_symbol_name; }

    std::string  full_name() const;

private:
    std::string  m_binary_name;
    uint32_t  m_ordinal;
    std::string  m_symbol_name;
};


std::string  parse_forwarder(std::string const&  this_binary_name,
                             std::string const&  forwarder,
                             std::string&  binary_name,
                             uint32_t&  ordinal,
                             std::string&  symbol_name);

bool  operator==(pe_symbol_id const&  l, pe_symbol_id const&  r);
bool  operator<(pe_symbol_id const&  l, pe_symbol_id const&  r);


struct load_props_pe
{
    load_props_pe(std::string const&  root_file,
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
    address  highest_end_address() const;

    void  add_special_section(std::string const&  binary_file,
                              std::string const&  section_name,
                              special_section_properties_ptr const  section_props);
    std::shared_ptr<loader::special_sections_table>  special_sections() const { return m_special_sections; }

    void  add_file_props(file_props_ptr const  file_props);
    std::shared_ptr<loader::files_table> files_table() const { return m_files_table; }

    void  add_skipped_file(std::string const&  file_pathname);
    std::shared_ptr<loader::skipped_files>  skipped_files() const { return m_skipped_files; }

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

    void  add_section_address_fix(std::string const&  pe_path, address const  orig_adr, address const  fixed_adr);
    std::unordered_map<std::string,address_fixes_map_ptr>& address_fixes() { return *m_address_fixes; }
    address_fixes_map_ptr  address_fixes(std::string const&  pe_path);
    bool  can_address_be_fixed(std::string const&  pe_path, address const  orig_adr) const;
    address  fixed_base_address_for(std::string const&  pe_path) const;
    address  fixed_address_for(std::string const&  pe_path, address const  orig_adr) const;
    address  highest_address_fix() const;

    void  add_symbol(pe_symbol_id const&  symbol_id, address const  adr);
    address  symbol_address(std::string const&  binary_name, uint32_t const  ordinal) const;
    address  symbol_address(std::string const&  binary_name, std::string const&  symbol_name) const;
    address  symbol_address(pe_symbol_id const&  symbol_id) const;

    void  add_warning(std::string const&  pe_path, std::string const&  warning_message);
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
    std::shared_ptr<std::unordered_map<std::string,address_fixes_map_ptr> >  m_address_fixes;
    std::map<pe_symbol_id,address>  m_symbols;
    std::shared_ptr<loader::warnings>  m_warnings;
};


}}

#endif
