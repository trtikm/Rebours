#ifndef REBOURS_MAL_LOADER_LOAD_HPP_INCLUDED
#   define REBOURS_MAL_LOADER_LOAD_HPP_INCLUDED

#   include <rebours/MAL/loader/descriptor.hpp>
#   include <vector>
#   include <string>

namespace loader {


/**
 * It loads a given binary file from the hard drive into the memory.
 * The file is loaded into a virtual address space such that sections
 * in the file reside on desired addresses (as specified in the file).
 * If the load fails the parameter 'error_message' is filled in with
 * a textual description of reasons of the failure. Otherwise the
 * load succeeds and the parameter 'error_message' remains empty.
 * The function has to be called with empty 'error_message'.
 *
 * The parameter 'ignored_dynamic_link_files' allows you to specify
 * names of dynamic library files which will NOT be loaded with the
 * binary file 'path_and_name_of_a_binary_file_to_be_loaded' (even if
 * any of the library files is specified in the binary file for the load).
 *
 * The parameter 'search_directories_for_dynamic_link_files' allows you
 * to specify directories in which the function will serach for dynamic
 * libraries which will be loaded into the memory with the binary file.
 * Subdirectories of the directories are not searched. It means that
 * you have to specify all directorectories (even nested) explicitly.
 *
 * The return valueof the function, i.e. a descriptor, holds all information
 * about the loaded process. See the header file Loader/descriptor.hpp for
 * details about the structure and meaning of the loaded information.
 */
descriptor_ptr  load(std::string const&  path_and_name_of_a_binary_file_to_be_loaded,
                     std::vector<std::string> const&  ignored_dynamic_link_files,
                     std::vector<std::string> const&  search_directories_for_dynamic_link_files,
                     std::string& error_message);


/**
 * The following functions are responsible for loading of binary files in a particular format. They
 * are automatically called from the 'load' function above. Nevertheless, you can also call these
 * functions directly (if you know the format in advance). Meanings of parameters are the same
 * as the corresponding parameters of the function 'load' above.
 */
descriptor_ptr  load_elf(std::string const&  elf_file,
                         std::vector<std::string> const&  ignored_dynamic_link_files,
                         std::vector<std::string> const&  search_directories_for_dynamic_link_files,
                         std::string& error_message);
descriptor_ptr  load_pe(std::string const&  pe_file,
                        std::vector<std::string> const&  ignored_dynamic_link_files,
                        std::vector<std::string> const&  search_directories_for_dynamic_link_files,
                        std::string& error_message);
descriptor_ptr  load_mach(std::string const&  mach_file,
                          std::vector<std::string> const&  ignored_dynamic_link_files,
                          std::vector<std::string> const&  search_directories_for_dynamic_link_files,
                          std::string& error_message);


}

#endif
