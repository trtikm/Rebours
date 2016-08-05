#ifndef REBOURS_MAL_LOADER_DETAIL_LOAD_PE_HPP_INCLUDED
#   define REBOURS_MAL_LOADER_DETAIL_LOAD_PE_HPP_INCLUDED

#   include <rebours/MAL/loader/detail/load_props_pe.hpp>
#   include <rebours/MAL/loader/file_props.hpp>
#   include <string>
#   include <utility>

namespace loader { namespace detail {


struct coff_standard_header_info
{
    coff_standard_header_info()
        : m_number_of_sections(0U)
        , m_optional_header_offset(0U)
        , m_size_of_optional_header(0U)
        , m_size_of_code(0U)
        , m_size_of_initialised_data(0U)
        , m_size_of_uninitialised_data(0U)
        , m_address_of_entry_point(0ULL)
        , m_base_of_code(0ULL)
        , m_base_of_data(0ULL)
    {}

    coff_standard_header_info(
            uint32_t  number_of_sections,
            uint32_t  optional_header_offset,
            uint32_t  size_of_optional_header,
            uint32_t  size_of_code,
            uint32_t  size_of_initialised_data,
            uint32_t  size_of_uninitialised_data,
            uint64_t  address_of_entry_point,
            uint64_t  base_of_code,
            uint64_t  base_of_data
            )
        : m_number_of_sections(number_of_sections)
        , m_optional_header_offset(optional_header_offset)
        , m_size_of_optional_header(size_of_optional_header)
        , m_size_of_code(size_of_code)
        , m_size_of_initialised_data(size_of_initialised_data)
        , m_size_of_uninitialised_data(size_of_uninitialised_data)
        , m_address_of_entry_point(address_of_entry_point)
        , m_base_of_code(base_of_code)
        , m_base_of_data(base_of_data)
    {}

    uint32_t  number_of_sections() const { return m_number_of_sections; }
    uint32_t  optional_header_offset() const { return m_optional_header_offset; }
    uint32_t  size_of_optional_header() const { return m_size_of_optional_header; }
    uint32_t  size_of_code() const { return m_size_of_code; }
    uint32_t  size_of_initialised_data() const { return m_size_of_initialised_data; }
    uint32_t  size_of_uninitialised_data() const { return m_size_of_uninitialised_data; }
    uint64_t  address_of_entry_point() const { return m_address_of_entry_point; }
    uint64_t  base_of_code() const { return m_base_of_code; }
    uint64_t  base_of_data() const { return m_base_of_data; }
private:
    uint32_t  m_number_of_sections;
    uint32_t  m_optional_header_offset;
    uint32_t  m_size_of_optional_header;
    uint32_t  m_size_of_code;
    uint32_t  m_size_of_initialised_data;
    uint32_t  m_size_of_uninitialised_data;
    uint64_t  m_address_of_entry_point;
    uint64_t  m_base_of_code;
    uint64_t  m_base_of_data;
};


inline std::string  NOT_PE_FILE() { return "Not PE file."; }

std::pair<file_props_ptr,coff_standard_header_info>
load_pe_file_props(std::string const&  pe_file, std::ifstream&  elf, load_props_pe&  load_props, std::string& error_message);

std::string  load_pe(std::string const& pe_file, load_props_pe&  load_props);

std::string  load_pe_32bit(std::ifstream&  pe, file_props_ptr  pe_props, coff_standard_header_info const&  coff_info, load_props_pe&  load_props);
std::string  load_pe_64bit(std::ifstream&  pe, file_props_ptr  pe_props, coff_standard_header_info const&  coff_info, load_props_pe&  load_props);

symbol_table_ptr  build_visible_symbols_table(load_props_pe const&  load_props);


}}

#endif
