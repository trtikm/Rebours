#ifndef REBOURS_MAL_LOADER_DETAIL_LOAD_MACH_HPP_INCLUDED
#   define REBOURS_MAL_LOADER_DETAIL_LOAD_MACH_HPP_INCLUDED

#   include <rebours/MAL/loader/detail/load_props_mach.hpp>
#   include <rebours/MAL/loader/file_props.hpp>
#   include <string>
#   include <tuple>

namespace loader { namespace detail {


typedef std::tuple<uint32_t,uint32_t,bool>  mach_header_props;


inline std::string  NOT_MACH_FILE() { return "Not MACH file."; }

std::pair<file_props_ptr,mach_header_props>
load_mach_file_props(std::string const&  mach_file, std::ifstream&  mach,
                     load_props_mach&  load_props, std::string& error_message);
std::string  load_mach(std::string const& mach_file, load_props_mach&  load_props);

std::string  load_mach_32bit(std::ifstream&  mach, file_props_ptr  mach_props,
                             uint32_t const  num_load_commnads,
                             uint32_t const  num_bytes_of_load_commnads,
                             bool const  is_fixed_dynamic_library,
                             load_props_mach&  load_props);
std::string  load_mach_64bit(std::ifstream&  mach, file_props_ptr  mach_props,
                             uint32_t const  num_load_commnads,
                             uint32_t const  num_bytes_of_load_commnads,
                             bool const  is_fixed_dynamic_library,
                             load_props_mach&  load_props);


symbol_table_ptr  build_visible_symbols_table(load_props_mach const&  load_props);


}}

#endif
