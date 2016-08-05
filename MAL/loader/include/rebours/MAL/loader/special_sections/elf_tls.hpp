#ifndef REBOURS_MAL_LOADER_SPECIAL_SECTION_ELF_TLS_HPP_INCLUDED
#   define REBOURS_MAL_LOADER_SPECIAL_SECTION_ELF_TLS_HPP_INCLUDED

#   include <rebours/MAL/loader/special_sections/special_section_properties.hpp>

namespace loader { namespace special_section {


struct elf_tls : public special_section_properties
{
    elf_tls(address const  start_address,
            address const  image_end_address,
            address const  end_address,
            uint64_t const  file_offset,
            uint64_t const  alignment,
            uint64_t const  flags,
            bool const  use_static_scheme
            );

    address  image_end_address() const noexcept { return m_image_end_address; }
    uint64_t  file_offset() const noexcept { return m_file_offset; }
    uint64_t  alignment() const noexcept { return m_alignment; }
    uint64_t  flags() const noexcept { return m_flags; }
    bool use_static_scheme() const noexcept { return m_use_static_scheme; }

private:
    address  m_image_end_address;
    uint64_t  m_file_offset;
    uint64_t  m_alignment;
    uint64_t  m_flags;
    bool  m_use_static_scheme;
};

typedef std::shared_ptr<elf_tls const>  elf_tls_ptr;

}}

#endif
