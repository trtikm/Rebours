#ifndef REBOURS_MAL_LOADER_SECTIONS_TABLE_HPP_INCLUDED
#   define REBOURS_MAL_LOADER_SECTIONS_TABLE_HPP_INCLUDED

/**
 * Here we define data structures which describe the memory layout of a loaded process.
 * It means that all blocks of memory, called sections, loaded from a given binary file
 * (and from dynamic libraries the binary file depends on, if any) into process's virtual
 * address space are held in the data structures defined here. This module also provides
 * access functions to sections.
 */

#   include <rebours/MAL/loader/address.hpp>
#   include <rebours/MAL/loader/file_props.hpp>
#   include <cstdint>
#   include <map>
#   include <vector>
#   include <string>
#   include <memory>

namespace loader {

/**
 * It describes a part of a binary file from which a section is loaded into the memory.
 */
struct section_file_header
{
    section_file_header(
            uint64_t const  offset,         //!< Shift in the file from its begin.
            uint64_t const  virtual_address,//!< Recommended start address in the memory
                                            //!< where the section should be loaded.
            uint64_t const  size_in_file,   //!< Note that in PE file size_in_file can be bigger
                                            //!< than size_in_memory, because size_in_file is
                                            //!< aligned while size_in_memory is not.
            uint64_t const  size_in_memory, //!< Bytes not initialised from the file (i.e.
                                            //!> beyond 'size_in_file') must be zero-cleared.
            uint64_t const  in_file_align,  //!< ELF: Gives us a requirement: offset % align == virtual_address % align.
                                            //!< PE: Both offset and size_in_file must be multiples of align.
            uint64_t const  in_memory_align //!< ELF: Is the same as in_file_align
                                            //!< PE: Alignment of sections when loaded into memory.
            );
    uint64_t  offset() const { return m_offset; }
    uint64_t  virtual_address() const { return m_virtual_address; }
    uint64_t  size_in_file() const { return m_size_in_file; }
    uint64_t  size_in_memory() const { return m_size_in_memory; }
    uint64_t  in_file_align() const { return m_in_file_align; }
    uint64_t  in_memory_align() const { return m_in_memory_align; }

private:

    uint64_t  m_offset;
    uint64_t  m_virtual_address;
    uint64_t  m_size_in_file;
    uint64_t  m_size_in_memory;
    uint64_t  m_in_file_align;
    uint64_t  m_in_memory_align;
};


typedef std::shared_ptr<section_file_header const>  section_file_header_ptr;


/**
 * It represents values of all bytes of a section of a binary file
 * after the section is loaded into the memory.
 */
typedef std::vector<uint8_t>  section_content;
typedef std::shared_ptr<section_content const>  section_content_ptr;


/**
 * It represents a section of a binary file when it is loaded into the memory.
 */
struct section
{
    section(address const  start_address,       //!< Start virtual address where the section begins
            address const  end_address,         //!< End virtual address where the section ends. The byte
                                                //!< pointed to by this address does NOT belong to the
                                                //!< section. So, the section lies in an interval of addresses
                                                //!< [start_address,end_address).
            section_content_ptr const  content, //!< Array of bytes of the section in the vitrual memory.
                                                //!< Byte at the index 0 represents a value at start_address
                                                //!< in the virtual memory.
            bool const  has_read_access,
            bool const  has_write_access,
            bool const  has_execute_access,
            bool const  is_in_big_endian,
            bool const  has_const_endian,
            file_props_ptr const  file_props,   //!< Reference to basic properties of a binary file from which
                                                //!< the section was loaded.
            section_file_header_ptr const  section_file_header  //!< Reference into the binary file (denoted by file_props)
                                                                //!< where are stored data from which the section was loaded.
            );

    address  start_address() const { return m_start_address; }
    address  end_address() const { return m_end_address; }

    section_content_ptr  content() const { return m_content; }

    bool has_read_access() const { return m_has_read_access; }
    bool has_write_access() const { return m_has_write_access; }
    bool has_execute_access() const { return m_has_execute_access; }

    bool is_in_big_endian() const { return m_is_in_big_endian; }
    bool has_const_endian() const { return m_has_const_endian; }

    file_props_ptr  file_props() const { return m_file_props; }
    section_file_header_ptr  section_file_header() const { return m_section_file_header; }

private:

    address  m_start_address;
    address  m_end_address;
    section_content_ptr  m_content;
    bool  m_has_read_access;
    bool  m_has_write_access;
    bool  m_has_execute_access;
    bool  m_is_in_big_endian;
    bool  m_has_const_endian;
    file_props_ptr  m_file_props;
    section_file_header_ptr  m_section_file_header;
};


typedef std::shared_ptr<section const>  section_ptr;


/**
 * A section table holds all loaded sections from a binary file and dynamic
 * libraries which were loaded into the memory together with the binary.
 * Sections are ordered in the table according their start addresses.
 * Sections in the table may NOT overlap (i.e. intersections of intervals
 * [start_address,end_address) of any two sections is empty).
 */
typedef std::map<address,section_ptr>  sections_table;
typedef std::shared_ptr<sections_table const> sections_table_ptr;


}

namespace loader {

/**
 * These two functions compute 'alignment'-aligned address from address 'adr'.
 * Both functions returns closest aligned address ('aligned_low' from bellow, and
 * 'aligned_high' from above).
 */
address  aligned_low(address const  adr, uint64_t const  alignment = 0x1000ULL);
address  aligned_high(address const  adr, uint64_t const  alignment = 0x1000ULL);

/**
 * Returns a pointer to that section in the passed sections table, which contains
 * the address 'key' in the interval [start_address,end_address) of that section.
 * The function returns empty reference, if there is no such section in the table.
 */
section_ptr  find_section(address const key, sections_table_ptr const  table);

uint8_t  read_byte(address const key, //!< It must be inside the interval [start_address,end_address) of the passed section
                   section_ptr const  section);

/**
 * The remaining functions simplify access to the memory accross all loaded sections. They
 * allow for reading a continuous block of memory.
 *
 * They use (directly or indirectly) functions find_section and read_byte declared above
 * to access the memory of sections.
 */

/**
 * Next three functions returns true if reading of the memory was successful and false otherwise.
 * They can only be called with the empty 'output_buffer'
 */
bool  read_bytes(address const  start_address, uint64_t  count, sections_table_ptr const  table,
                 std::vector<uint8_t>& output_buffer, std::vector<section_ptr>& output_sections);
bool  read_bytes(address const  start_address, uint64_t  count, sections_table_ptr const  table,
                 std::vector<uint8_t>& output_buffer, bool& is_in_big_endian);

bool  read_zero_terminated_string(address const start_address, sections_table_ptr const  table,
                                  std::string& output);

/**
 * The following functions read data of a particular type T. It is assumed that each address
 * in the interval [start_address,start_address+sizeof(T)) lies in the memory of some section
 * in the passed sections table.
 */
address  read_address_64bit(address const start_address, sections_table_ptr const  table);
address  read_address_32bit(address const start_address, sections_table_ptr const  table);
uint8_t   read_uint8_t(address const start_address, sections_table_ptr const  table);
uint16_t  read_uint16_t(address const start_address, sections_table_ptr const  table);
uint32_t  read_uint32_t(address const start_address, sections_table_ptr const  table);
uint64_t  read_uint64_t(address const start_address, sections_table_ptr const  table);


}

#endif
