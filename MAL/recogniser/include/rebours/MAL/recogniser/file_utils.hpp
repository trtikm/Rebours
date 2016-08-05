#ifndef FILE_UTILS_HPP_INCLUDED
#   define FILE_UTILS_HPP_INCLUDED

#   include <string>
#   include <vector>
#   include <cstdint>
#   include <iosfwd>


/**
 * This module implements utility functions which provide us disc-related functionality,
 * like manipulation with disc paths (concatenation/splitting), checking for existence
 * of files, etc.
 */

namespace fileutl {


bool  file_exists(std::string const&  pathname);
bool  is_directory(std::string const&  pathname);
uint64_t  file_size(std::string const&  file_pathname);
std::string  parse_name_in_pathname(std::string const&  file_pathname);
std::string  parse_path_in_pathname(std::string const&  file_pathname);
void  create_directory(std::string const&  pathname);
std::string  concatenate_file_paths(std::string const&  left_path, std::string const&  right_path);
std::string  absolute_path(std::string const&  path);
std::string  normalise_path(std::string const&  path);
void  split_pathname(std::string const&  pathname, std::vector<std::string>& output);
std::string  join_path_parts(std::vector<std::string> const&  parts);
std::string  get_common_preffix(std::string const&  pathname1, std::string const&  pathname2);
std::string  get_relative_path(std::string const&  pathname, std::string const&  directory);


}

#endif
