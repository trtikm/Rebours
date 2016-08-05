#ifndef REBOURS_MAL_LOADER_SPECIAL_SECTIONS_HPP_INCLUDED
#   define REBOURS_MAL_LOADER_SPECIAL_SECTIONS_HPP_INCLUDED

/**
 * Special sections hold an information parsed from loaded sections (see Loader/sections_table.hpp)
 * which is than used by an underlying system for managing execution of the loaded process.
 */

/**
 * This header file contains a definition of a base class for all special sections.
 */
#   include <rebours/MAL/loader/special_sections/special_section_properties.hpp>

/**
 * These headers contain concrete definitions of individual special sections.
 */
#   include <rebours/MAL/loader/special_sections/exceptions.hpp>
#   include <rebours/MAL/loader/special_sections/load_configuration_structure.hpp>
#   include <rebours/MAL/loader/special_sections/thread_local_storage_initialisers.hpp>
#   include <rebours/MAL/loader/special_sections/elf_tls.hpp>

#   include <map>

namespace loader {


namespace special_section_name {

/**
 * Here are string constants identifying kinds of special sections.
 */

std::string  thread_local_storage();
std::string  thread_local_storage_initialisers();
std::string  resources();
std::string  load_configuration_structure();
std::string  exceptions();


}

/**
 * It is a table of special sections ordered by their names from the namespace loader::special_section_name
 */
typedef std::map<std::string,                       //!< Special section name form the namespace loader::special_section_name
                 special_section_properties_ptr     //!< Properties of the section (see Loader/special_sections/special_section_properties.hpp)
                 >
        special_sections;

/**
 * For each loaded file (the root binary and its dynamic libraries) we store its table of
 * special sections. So, we build a table where for each loaded file (its path-name)
 * we store a table with special sections.
 */
typedef std::map<std::string,                       //!< Binary file path name
                 special_sections                   //!< A table of special sections of that binary file
                 >
        special_sections_table;


typedef std::shared_ptr<special_sections_table const>  special_sections_table_ptr;


}

#endif
