#ifndef REBOURS_MAL_LOADER_DETAIL_LOAD_PROPS_ELF_HPP_INCLUDED
#   define REBOURS_MAL_LOADER_DETAIL_LOAD_PROPS_ELF_HPP_INCLUDED

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


#   define LOAD_ELF_WARNING_IN(elf_file,SSTREAM_EXPRESSION) \
            {\
                std::stringstream  sstr;\
                sstr << SSTREAM_EXPRESSION;\
                std::string const warning_message = sstr.str();\
                load_props.add_warning(elf_file,warning_message);\
                std::cout << "WARNING: " << warning_message << "\n";\
            }
#   define LOAD_ELF_WARNING(SSTREAM_EXPRESSION)  LOAD_ELF_WARNING_IN(elf_props->path(),SSTREAM_EXPRESSION)


namespace loader { namespace detail {


struct load_props_elf
{
    load_props_elf(std::string const&  root_file,
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

    uint32_t  get_fresh_file_id() { return ++m_file_id_generator; }

    std::string const&  parent_abi_loader() const { return m_parent_abi_loader; }
    void  set_parent_abi_loader(std::string const& abi_loader) { m_parent_abi_loader = abi_loader; }

    std::shared_ptr<special_functions_of_files>  init_functions() const { return m_init_functions; }
    std::shared_ptr<special_functions_of_files>  fini_functions() const { return m_fini_functions; }

    void  add_init_function(std::string const& file, address const  fn_address);
    void  add_fini_function(std::string const& file, address const  fn_address);

    void  add_visible_symbol(std::string const& definition_file,  uint64_t const  symbol_table_index,
                             relocation_symbol_id const& symbol_id, std::string const&  symbol_type,
                             uint64_t const  symbol_value, uint64_t const symbol_size);
    bool  has_visible_symbol(std::string const& definition_file,  uint64_t const  symbol_table_index);
    relocation_symbol_id const& visible_symbol_id(std::string const& definition_file,  uint64_t const  symbol_table_index);
    std::string  visible_symbol_type(std::string const& definition_file,  uint64_t const  symbol_table_index);
    uint64_t  visible_symbol_value(std::string const& definition_file,  uint64_t const  symbol_table_index);
    uint64_t  visible_symbol_size(std::string const& definition_file,  uint64_t const  symbol_table_index);
    std::shared_ptr<symbol_table>  visible_symbol_table() const { return m_visible_symbol_table; }

    void  add_hidden_symbol(std::string const& definition_file,  uint64_t const  symbol_table_index,
                            relocation_symbol_id const& symbol_id, std::string const&  symbol_type,
                            uint64_t const  symbol_value, uint64_t const symbol_size);
    bool  has_hidden_symbol(std::string const& definition_file,  uint64_t const  symbol_table_index);
    relocation_symbol_id const& hidden_symbol_id(std::string const& definition_file,  uint64_t const  symbol_table_index);
    std::string  hidden_symbol_type(std::string const& definition_file,  uint64_t const  symbol_table_index);
    uint64_t  hidden_symbol_value(std::string const& definition_file,  uint64_t const  symbol_table_index);
    uint64_t  hidden_symbol_size(std::string const& definition_file,  uint64_t const  symbol_table_index);
    std::shared_ptr<symbol_table>  hidden_symbol_table() const { return m_hidden_symbol_table; }

    bool  has_symbol(std::string const& definition_file,  uint64_t const  symbol_table_index);
    relocation_symbol_id const& symbol_id(std::string const& definition_file,  uint64_t const  symbol_table_index);
    std::string  symbol_type(std::string const& definition_file,  uint64_t const  symbol_table_index);
    uint64_t  symbol_value(std::string const& definition_file,  uint64_t const  symbol_table_index);
    uint64_t  symbol_size(std::string const& definition_file,  uint64_t const  symbol_table_index);
    std::string  compute_symbol_address(std::string const& relocated_file,  uint64_t const  symbol_table_index,
                                        address& output_address);

    std::string  compute_definition_file_from_symbol_index(uint64_t const  symbol_table_index) const;

    special_section::elf_tls_ptr  tls(std::string const&  loaded_file);


//    bool  has_symbol(uint64_t const  symbol_table_index);
//    relocation_symbol_id const& symbol_id(uint64_t const  symbol_table_index);
//    std::string  symbol_type(uint64_t const  symbol_table_index);
//    uint64_t  symbol_value(uint64_t const  symbol_table_index);
//    uint64_t  symbol_size(uint64_t const  symbol_table_index);
//    std::string  compute_symbol_address(uint64_t const  symbol_table_index, address& output_address);

    void  add_performed_relocation(loader::relocation const&  reloc);
    void  add_skipped_relocation(loader::relocation const&  reloc);
    std::shared_ptr<loader::relocations>  performed_relocations() const { return m_performed_relocations; }
    std::shared_ptr<loader::relocations>  skipped_relocations() const { return m_skipped_relocations; }

    void  add_section_address_fix(std::string const&  elf_path, address const  orig_adr, address const  fixed_adr);
    std::unordered_map<std::string,address_fixes_map_ptr>& address_fixes() { return *m_address_fixes; }
    address_fixes_map_ptr  address_fixes(std::string const&  elf_path);
    bool  can_address_be_fixed(std::string const&  elf_path, address const  orig_adr) const;
    address  fixed_base_address_for(std::string const&  elf_path) const;
    address  fixed_address_for(std::string const&  elf_path, address const  orig_adr) const;

    void  add_warning(std::string const&  elf_path, std::string const&  warning_message);
    std::shared_ptr<loader::warnings>  warnings() const { return m_warnings; }

private:
//    typedef std::unordered_map<std::pair<std::string,               // binary file
//                                         uint64_t                   // index into the symbol table
//                                         >,
//                               std::tuple<relocation_symbol_id,     // symbol name
//                                          std::string,              // relocation type (DATA, FUNCTION, UNKNOWN)
//                                          uint64_t,                 // symbol's value (e.g. its address)
//                                          uint64_t                  // size of the symbol in memory
//                                          > >
//            symbol_table;

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
    uint32_t  m_file_id_generator;
    std::string  m_parent_abi_loader;
    std::shared_ptr<special_functions_of_files>  m_init_functions;
    std::shared_ptr<special_functions_of_files>  m_fini_functions;
    std::shared_ptr<symbol_table>  m_visible_symbol_table;
    std::shared_ptr<symbol_table>  m_hidden_symbol_table;
    std::unordered_map<uint64_t,std::string>  m_from_symbol_indices_to_files;
    std::shared_ptr<loader::relocations>  m_performed_relocations;
    std::shared_ptr<loader::relocations>  m_skipped_relocations;
    std::shared_ptr<std::unordered_map<std::string,address_fixes_map_ptr> >  m_address_fixes;
    std::shared_ptr<loader::warnings>  m_warnings;
};


}}

#endif
