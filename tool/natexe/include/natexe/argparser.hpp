#ifndef ARGPARSER_HPP_INCLUDED
#   define ARGPARSER_HPP_INCLUDED

#   include <string>
#   include <vector>
#   include <cstdint>

namespace argparser {


std::string  parse_arguments(int argc, char* argv[]);

bool  do_print_help();
std::string const&  help_text();

bool  do_print_version();
std::string const&  version_text();

bool  use_loader();
std::string const&  path_and_name_of_a_binary_file_to_be_loaded();
std::vector<std::string> const&  ignored_dynamic_link_files();
std::vector<std::string> const&  search_directories_for_dynamic_link_files();
std::string const&  path_and_name_of_a_prologue_program();
std::string const&  path_and_name_of_a_microcode_program();

bool  do_save_descriptor();
bool  do_save_sections_of_descriptor();
std::string const&  save_path_and_name_of_a_descriptor_start_file();

bool  do_save_prologue_program();
std::string const&  save_path_and_name_of_a_prologue_program();

bool  do_save_recovered_program();
std::string const&  save_path_and_name_of_a_recovered_program();

bool  do_save_encoded_program();
std::string const&  save_path_and_name_of_an_encoded_program();

uint32_t  timeout_in_seconds();

std::string const&  path_and_name_of_the_output_log_file();


}

#endif
