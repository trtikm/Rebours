#ifndef REBOURS_MAL_LOADER_DETAIL_MUTABLE_SECTIONS_TABLE_HPP_INCLUDED
#   define REBOURS_MAL_LOADER_DETAIL_MUTABLE_SECTIONS_TABLE_HPP_INCLUDED

#   include <rebours/MAL/loader/address.hpp>
#   include <rebours/MAL/loader/sections_table.hpp>
#   include <vector>
#   include <memory>
#   include <string>

namespace loader { namespace detail {


typedef std::shared_ptr<sections_table> mutable_sections_table_ptr;


std::string  write_buffer(
                    address const  start_address,
                    std::vector<uint8_t> const&  buffer,
                    bool const is_buffer_in_big_endian,
                    mutable_sections_table_ptr const  table
                    );

std::string  write_byte(address const key, uint8_t const value, section_ptr const  section);
std::string  write_address(address const  start_address, address const  value, mutable_sections_table_ptr const  table);
std::string  write_uint16_t(address const  start_address, uint16_t const  value, mutable_sections_table_ptr const  table);
std::string  write_uint32_t(address const  start_address, uint32_t const  value, mutable_sections_table_ptr const  table);
std::string  write_uint64_t(address const  start_address, uint64_t const  value, mutable_sections_table_ptr const  table);


}}

#endif
