#ifndef REBOURS_MAL_LOADER_DETAIL_LOAD_ELF_HPP_INCLUDED
#   define REBOURS_MAL_LOADER_DETAIL_LOAD_ELF_HPP_INCLUDED

#   include <rebours/MAL/loader/detail/load_props_elf.hpp>
#   include <rebours/MAL/loader/file_props.hpp>
#   include <string>

namespace loader { namespace detail {


inline std::string  NOT_ELF_FILE() { return "Not ELF file."; }

file_props_ptr load_elf_file_props(std::string const&  elf_file, std::ifstream&  elf,
                                   load_props_elf&  load_props, std::string& error_message);
std::string  load_elf(std::string const& elf_file, load_props_elf&  load_props);

std::string  load_elf_32bit(std::ifstream&  elf, file_props_ptr  elf_props, load_props_elf&  load_props);
std::string  load_elf_64bit(std::ifstream&  elf, file_props_ptr  elf_props, load_props_elf&  load_props);


}}

#endif
