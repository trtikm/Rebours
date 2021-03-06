#ifndef REBOURS_MAL_LOADER_FILE_UTILS_HPP_INCLUDED
#   define REBOURS_MAL_LOADER_FILE_UTILS_HPP_INCLUDED

#   include <string>
#   include <vector>
#   include <cstdint>
#   include <iosfwd>


/**
 * This module implements utility functions which provide us disc-related functionality,
 * like manipulation with disc paths (concatenation/splitting), checking for existence
 * of files, read from and write to files, etc.
 */


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

void  skip_bytes(std::ifstream&  stream, uint64_t const count);

uint8_t  read_byte(std::ifstream&  stream);
void  read_bytes(std::ifstream&  stream, uint64_t const  count,  std::vector<uint8_t>& output);
void  read_bytes(std::ifstream&  stream, uint64_t const  count,  std::string& output);
std::string  read_bytes_as_null_terminated_string(std::ifstream&  stream);
uint16_t  read_bytes_to_uint16_t(std::ifstream&  stream, uint8_t const  num_bytes, bool const  is_in_big_endian);
int16_t  read_bytes_to_int16_t(std::ifstream&  stream, uint8_t const  num_bytes, bool const  is_in_big_endian);
uint32_t  read_bytes_to_uint32_t(std::ifstream&  stream, uint8_t const  num_bytes, bool const  is_in_big_endian);
int32_t  read_bytes_to_int32_t(std::ifstream&  stream, uint8_t const  num_bytes, bool const  is_in_big_endian);
uint64_t  read_bytes_to_uint64_t(std::ifstream&  stream, uint8_t const  num_bytes, bool const  is_in_big_endian);
int64_t  read_bytes_to_int64_t(std::ifstream&  stream, uint8_t const  num_bytes, bool const  is_in_big_endian);


#endif
