#include <rebours/MAL/loader/detail/mutable_sections_table.hpp>
#include <rebours/MAL/loader/sections_table.hpp>
#include <rebours/utility/buffer_io.hpp>
#include <rebours/utility/assumptions.hpp>

namespace loader { namespace detail {


std::string  write_buffer(
                    address const  start_address,
                    std::vector<uint8_t> const&  buffer,
                    bool const is_buffer_in_big_endian,
                    mutable_sections_table_ptr const  table
                    )
{
    ASSUMPTION(!buffer.empty());
    ASSUMPTION(table.operator bool());
    std::vector<uint8_t>::size_type  num_stored_bytes = 0ULL;
    address  current_address = start_address;
    while (true)
    {
        section_ptr  current_section = find_section(current_address,table);
        if (!current_section.operator bool())
            return "Attempt to write a value outside any loaded section.";
        if (is_buffer_in_big_endian != current_section->is_in_big_endian())
            return "Different endians of the stored content and of the section.";
        for ( ; current_address < current_section->end_address(); ++current_address)
        {
            std::string const  error_message =
                    write_byte(current_address,buffer.at(num_stored_bytes),current_section);
            if (!error_message.empty())
                return error_message;
            if (++num_stored_bytes == buffer.size())
                return "";
        }
    }
}

std::string  write_byte(address const key, uint8_t const value, section_ptr const  section)
{
    if (!(section->start_address() <= key && key < section->end_address()))
        return "Attempt to write a value outside a section.";
    std::const_pointer_cast<section_content>(section->content())->at(key - section->start_address()) = value;
    return "";
}


std::string  write_address(address const  start_address, address const  value, mutable_sections_table_ptr const  table)
{
    std::vector<uint8_t>  buffer;
    section_ptr const start_section = find_section(start_address,table);
    if (!start_section.operator bool())
        return "Attempt to write outside any loaded section.";
    address_to_buffer(value,buffer,start_section->is_in_big_endian());
    return write_buffer(start_address,buffer,start_section->is_in_big_endian(),table);
}

std::string  write_uint16_t(address const  start_address, uint16_t const  value, mutable_sections_table_ptr const  table)
{
    std::vector<uint8_t>  buffer;
    section_ptr const start_section = find_section(start_address,table);
    if (!start_section.operator bool())
        return "Attempt to write outside any loaded section.";
    uint16_t_to_buffer(value,buffer,start_section->is_in_big_endian());
    return write_buffer(start_address,buffer,start_section->is_in_big_endian(),table);
}

std::string  write_uint32_t(address const  start_address, uint32_t const  value, mutable_sections_table_ptr const  table)
{
    std::vector<uint8_t>  buffer;
    section_ptr const start_section = find_section(start_address,table);
    if (!start_section.operator bool())
        return "Attempt to write outside any loaded section.";
    uint32_t_to_buffer(value,buffer,start_section->is_in_big_endian());
    return write_buffer(start_address,buffer,start_section->is_in_big_endian(),table);
}

std::string  write_uint64_t(address const  start_address, uint64_t const  value, mutable_sections_table_ptr const  table)
{
    std::vector<uint8_t>  buffer;
    section_ptr const start_section = find_section(start_address,table);
    if (!start_section.operator bool())
        return "Attempt to write outside any loaded section.";
    uint64_t_to_buffer(value,buffer,start_section->is_in_big_endian());
    return write_buffer(start_address,buffer,start_section->is_in_big_endian(),table);
}


}}
