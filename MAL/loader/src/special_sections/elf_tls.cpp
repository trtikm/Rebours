#include <rebours/MAL/loader/special_sections/elf_tls.hpp>

namespace loader { namespace special_section {


elf_tls::elf_tls(
        address const  start_address,
        address const  image_end_address,
        address const  end_address,
        uint64_t const  file_offset,
        uint64_t const  alignment,
        uint64_t const  flags,
        bool const  use_static_scheme
        )
    : special_section_properties(start_address,end_address,"Template of the thread-local section.")
    , m_image_end_address(image_end_address)
    , m_file_offset(file_offset)
    , m_alignment(alignment)
    , m_flags(flags)
    , m_use_static_scheme(use_static_scheme)
{}


}}
