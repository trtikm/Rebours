#include <rebours/MAL/loader/sections_table.hpp>
#include <rebours/utility/buffer_io.hpp>
#include <rebours/utility/assumptions.hpp>
#include <rebours/utility/invariants.hpp>

namespace loader {


section_file_header::section_file_header(
        uint64_t const  offset,
        uint64_t const  virtual_address,
        uint64_t const  size_in_file,
        uint64_t const  size_in_memory,
        uint64_t const  in_file_align,
        uint64_t const  in_memory_align)
    : m_offset(offset)
    , m_virtual_address(virtual_address)
    , m_size_in_file(size_in_file)
    , m_size_in_memory(size_in_memory)
    , m_in_file_align(in_file_align)
    , m_in_memory_align(in_memory_align)
{}



section::section(address const  start_address,
                 address const  end_address,
                 section_content_ptr const  content,
                 bool const  has_read_access,
                 bool const  has_write_access,
                 bool const  has_execute_access,
                 bool const  is_in_big_endian,
                 bool const  has_const_endian,
                 file_props_ptr const  file_props,
                 section_file_header_ptr const  section_file_header
                 )
    : m_start_address(start_address)
    , m_end_address(end_address)
    , m_content(content)
    , m_has_read_access(has_read_access)
    , m_has_write_access(has_write_access)
    , m_has_execute_access(has_execute_access)
    , m_is_in_big_endian(is_in_big_endian)
    , m_has_const_endian(has_const_endian)
    , m_file_props(file_props)
    , m_section_file_header(section_file_header)
{}


}

namespace loader {


address  aligned_low(address const  adr, uint64_t const  alignment)
{
    uint64_t const  remainder = adr % alignment;
    return adr - remainder;
}

address  aligned_high(address const  adr, uint64_t const  alignment)
{
    uint64_t const  remainder = adr % alignment;
    return adr + (remainder == 0ULL ? 0ULL : alignment - remainder);
}

section_ptr  find_section(address const key, sections_table_ptr const  table)
{
    if (table->empty())
        return section_ptr();

    sections_table::const_iterator const  upper_it = table->upper_bound(key);
    if (upper_it == table->cbegin())
        return section_ptr();
    section_ptr const  ptr = std::prev(upper_it)->second;

    return (ptr->start_address() <= key && key < ptr->end_address()) ? ptr : section_ptr();
}

uint8_t  read_byte(address const key, section_ptr const  section)
{
    ASSUMPTION(section->start_address() <= key && key < section->end_address());
    return section->content()->at(key - section->start_address());
}

bool  read_bytes(address const  start_address, uint64_t  count, sections_table_ptr const  table,
                 std::vector<uint8_t>& output_buffer, std::vector<section_ptr>& output_sections)
{
    if (count == 0ULL)
        return true;
    address  current_address = start_address;
    while (true)
    {
        section_ptr  current_section = find_section(current_address,table);
        if (!current_section.operator bool())
            return false;
        output_sections.push_back(current_section);
        for ( ; current_address < current_section->end_address(); ++current_address)
        {
            uint8_t const  byte = read_byte(current_address,current_section);
            output_buffer.push_back(byte);
            if (--count == 0ULL)
                return true;
        }
    }
}

bool  read_bytes(address const  start_address, uint64_t  count, sections_table_ptr const  table,
                 std::vector<uint8_t>& output_buffer, bool& is_in_big_endian)
{
    ASSUMPTION(output_buffer.empty());
    std::vector<section_ptr> sections;
    if (!read_bytes(start_address,count,table,output_buffer,sections))
        return false;
    INVARIANT(output_buffer.size() == count && !sections.empty());
    is_in_big_endian = sections.at(0)->is_in_big_endian();
    for (auto const secptr : sections)
        if (secptr->is_in_big_endian() != is_in_big_endian)
            return false;
    return true;
}

bool  read_zero_terminated_string(address const start_address, sections_table_ptr const  table,
                                  std::string& output)
{
    address  current_address = start_address;
    while (true)
    {
        section_ptr  current_section = find_section(current_address,table);
        if (!current_section.operator bool())
            return false;
        for ( ; current_address < current_section->end_address(); ++current_address)
        {
            uint8_t const  byte = read_byte(current_address,current_section);
            if (byte == 0U)
                return true;
            output.push_back(byte);
        }
    }
}

address  read_address_64bit(address const start_address, sections_table_ptr const  table)
{
    std::vector<uint8_t>  buffer;
    bool  is_in_big_endian;
    bool const  read_success = read_bytes(start_address,sizeof(address),table,buffer,is_in_big_endian);
    ASSUMPTION(read_success && buffer.size() == sizeof(address));
    (void)read_success;
    uint8_t const* const  buffer_begin = (uint8_t const*)&buffer.at(0);
    return (address)buffer_to_uint64_t(buffer_begin,buffer_begin + sizeof(address),is_in_big_endian);
}

address  read_address_32bit(address const start_address, sections_table_ptr const  table)
{
    std::vector<uint8_t>  buffer;
    bool  is_in_big_endian;
    bool const  read_success = read_bytes(start_address,4ULL,table,buffer,is_in_big_endian);
    ASSUMPTION(read_success && buffer.size() == 4ULL);
    (void)read_success;
    uint8_t const* const  buffer_begin = (uint8_t const*)&buffer.at(0);
    return (address)buffer_to_uint32_t(buffer_begin,buffer_begin + 4ULL,is_in_big_endian);
}

uint8_t  read_uint8_t(address const start_address, sections_table_ptr const  table)
{
    std::vector<uint8_t>  buffer;
    bool  is_in_big_endian;
    bool const  read_success = read_bytes(start_address,sizeof(uint8_t),table,buffer,is_in_big_endian);
    ASSUMPTION(read_success && buffer.size() == sizeof(uint8_t));
    (void)read_success;
    return buffer.at(0);
}

uint16_t  read_uint16_t(address const start_address, sections_table_ptr const  table)
{
    std::vector<uint8_t>  buffer;
    bool  is_in_big_endian;
    bool const  read_success = read_bytes(start_address,sizeof(uint16_t),table,buffer,is_in_big_endian);
    ASSUMPTION(read_success && buffer.size() == sizeof(uint16_t));
    (void)read_success;
    uint8_t const* const  buffer_begin = (uint8_t const*)&buffer.at(0);
    return buffer_to_uint16_t(buffer_begin,buffer_begin + sizeof(uint16_t),is_in_big_endian);
}

uint32_t  read_uint32_t(address const start_address, sections_table_ptr const  table)
{
    std::vector<uint8_t>  buffer;
    bool  is_in_big_endian;
    bool const  read_success = read_bytes(start_address,sizeof(uint32_t),table,buffer,is_in_big_endian);
    ASSUMPTION(read_success && buffer.size() == sizeof(uint32_t));
    (void)read_success;
    uint8_t const* const  buffer_begin = (uint8_t const*)&buffer.at(0);
    return buffer_to_uint32_t(buffer_begin,buffer_begin + sizeof(uint32_t),is_in_big_endian);
}

uint64_t  read_uint64_t(address const start_address, sections_table_ptr const  table)
{
    std::vector<uint8_t>  buffer;
    bool  is_in_big_endian;
    bool const  read_success = read_bytes(start_address,sizeof(uint64_t),table,buffer,is_in_big_endian);
    ASSUMPTION(read_success && buffer.size() == sizeof(uint64_t));
    (void)read_success;
    uint8_t const* const  buffer_begin = (uint8_t const*)&buffer.at(0);
    return buffer_to_uint64_t(buffer_begin,buffer_begin + sizeof(uint64_t),is_in_big_endian);
}


}
