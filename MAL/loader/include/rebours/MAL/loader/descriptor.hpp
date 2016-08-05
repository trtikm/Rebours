#ifndef REBOURS_MAL_LOADER_DESCRIPTOR_HPP_INCLUDED
#   define REBOURS_MAL_LOADER_DESCRIPTOR_HPP_INCLUDED

#   include <rebours/MAL/loader/platform.hpp>
#   include <rebours/MAL/loader/sections_table.hpp>
#   include <rebours/MAL/loader/special_sections.hpp>
#   include <rebours/MAL/loader/file_props.hpp>
#   include <rebours/MAL/loader/dependencies_graph.hpp>
#   include <rebours/MAL/loader/init_fini.hpp>
#   include <rebours/MAL/loader/relocations.hpp>
#   include <rebours/MAL/loader/warnings.hpp>
#   include <string>
#   include <vector>
#   include <memory>

namespace loader {


/**
 * It represents a binary file when it is loaded into the memory. It is a root
 * access point to all information about the loaded binary file. All information
 * is read only. Look at definitions of return types of access methods to see
 * what data are actually provided.
 *
 * Instances are constructed in 'load' functions declared in the headr file:
 *      Loader/load.hpp
 *
 * An ellaborate example of how to access the data in the descriptor is a HTML
 * dump inplemented in function 'dump_html' declared in the header file:
 *      Loader/dump.hpp
 */
struct descriptor
{
    /**
     * It takes a (shared) ownership of data passed as arguments, except the 'entry_point'
     * which is coppied.
     */
    descriptor(platform_ptr const  platform,
               sections_table_ptr const  sections_table,
               special_sections_table_ptr const  special_sections,
               address const  entry_point,
               files_table_ptr const  files_table,
               skipped_files_ptr const  skipped_files,
               dependencies_graph_ptr const  dependencies_of_loaded_files, //!< It excludes the skipped files.
               dependencies_graph_ptr const  dependencies_of_all_files, //!< It includes the skipped files.
               init_functions_ptr const init_function,
               fini_functions_ptr const  fini_functions,
               relocations_ptr const  performed_relocations,
               relocations_ptr const  skipped_relocations,
               symbol_table_ptr const  visible_symbol_table,
               symbol_table_ptr const  hidden_symbol_table,
               warnings_ptr const  warnings
               );
    platform_ptr  platform() const { return m_platform; }
    sections_table_ptr  sections_table() const { return m_sections_table; }
    special_sections_table_ptr  special_sections() const { return m_special_sections; }
    address  entry_point() const { return m_entry_point; }
    files_table_ptr  files_table() const { return m_files_table; }
    skipped_files_ptr  skipped_files() const { return m_skipped_files; }
    dependencies_graph_ptr  dependencies_of_loaded_files() const { return m_dependencies_of_loaded_files; }
    dependencies_graph_ptr  dependencies_of_all_files() const { return m_dependencies_of_all_files; }
    init_functions_ptr  init_functions() const { return m_init_functions; }
    fini_functions_ptr  fini_functions() const { return m_fini_functions; }
    relocations_ptr  performed_relocations() const { return m_performed_relocations; }
    relocations_ptr  skipped_relocations() const { return m_skipped_relocations; }
    symbol_table_ptr  visible_symbol_table() const { return m_visible_symbol_table; }
    symbol_table_ptr  hidden_symbol_table() const { return m_hidden_symbol_table; }
    warnings_ptr  warnings() const { return m_warnings; }
private:
    platform_ptr  m_platform;
    sections_table_ptr  m_sections_table;
    special_sections_table_ptr  m_special_sections;
    address  m_entry_point;
    files_table_ptr  m_files_table;
    skipped_files_ptr  m_skipped_files;
    dependencies_graph_ptr  m_dependencies_of_loaded_files;
    dependencies_graph_ptr  m_dependencies_of_all_files;
    init_functions_ptr  m_init_functions;
    fini_functions_ptr  m_fini_functions;
    relocations_ptr  m_performed_relocations;
    relocations_ptr  m_skipped_relocations;
    symbol_table_ptr  m_visible_symbol_table;
    symbol_table_ptr  m_hidden_symbol_table;
    warnings_ptr  m_warnings;
};


typedef std::shared_ptr<descriptor const>  descriptor_ptr;


}

#endif
