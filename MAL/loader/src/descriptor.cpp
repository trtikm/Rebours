#include <rebours/MAL/loader/descriptor.hpp>

namespace loader {


descriptor::descriptor(platform_ptr const  platform,
                       sections_table_ptr const  sections_table,
                       special_sections_table_ptr const  special_sections,
                       address const  entry_point,
                       files_table_ptr const  files_table,
                       skipped_files_ptr const  skipped_files,
                       dependencies_graph_ptr const  dependencies_of_loaded_files,
                       dependencies_graph_ptr const  dependencies_of_all_files,
                       init_functions_ptr const init_function,
                       fini_functions_ptr const  fini_functions,
                       relocations_ptr const  performed_relocations,
                       relocations_ptr const  skipped_relocations,
                       symbol_table_ptr const  visible_symbol_table,
                       symbol_table_ptr const  hidden_symbol_table,
                       warnings_ptr const  warnings
                       )
    : m_platform(platform)
    , m_sections_table(sections_table)
    , m_special_sections(special_sections)
    , m_entry_point(entry_point)
    , m_files_table(files_table)
    , m_skipped_files(skipped_files)
    , m_dependencies_of_loaded_files(dependencies_of_loaded_files)
    , m_dependencies_of_all_files(dependencies_of_all_files)
    , m_init_functions(init_function)
    , m_fini_functions(fini_functions)
    , m_performed_relocations(performed_relocations)
    , m_skipped_relocations(skipped_relocations)
    , m_visible_symbol_table(visible_symbol_table)
    , m_hidden_symbol_table(hidden_symbol_table)
    , m_warnings(warnings)
{}


}
