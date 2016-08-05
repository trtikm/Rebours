#ifndef REBOURS_MAL_LOADER_SPECIAL_SECTION_LOAD_CONFIGURATION_STRUCTURE_HPP_INCLUDED
#   define REBOURS_MAL_LOADER_SPECIAL_SECTION_LOAD_CONFIGURATION_STRUCTURE_HPP_INCLUDED

#   include <rebours/MAL/loader/special_sections/special_section_properties.hpp>
#   include <ctime>
#   include <vector>

namespace loader { namespace special_section {


struct load_configuration_structure : public special_section_properties
{
    load_configuration_structure(
            address const  start_address,
            address const  end_address
            );
    load_configuration_structure(
            address const  start_address,
            address const  end_address,
            std::time_t const  timestamp,
            uint16_t const  major_version,
            uint16_t const  minor_version,
            uint32_t const  global_clear_flags,
            uint32_t const  global_set_flags,
            uint32_t const  critical_section_timeout,
            uint64_t const  decommit_block_thresold,
            uint64_t const  decommit_free_thresold,
            address const  lock_prefix_table,
            uint64_t const  max_alloc_size,
            uint64_t const  memory_thresold,
            uint64_t const  process_affinity_mask,
            uint32_t const  process_heap_flags,
            uint16_t const  service_pack_version,
            address const  security_cookie,
            std::vector<address> const&  structured_exception_handlers
            );

    std::time_t const&  timestamp() const { return m_timestamp; }
    uint16_t  major_version() const { return m_major_version; }
    uint16_t  minor_version() const { return m_minor_version; }
    uint32_t  global_clear_flags() const { return m_global_clear_flags; }
    uint32_t  global_set_flags() const { return m_global_set_flags; }
    uint32_t  critical_section_timeout() const { return m_critical_section_timeout; }
    uint64_t  decommit_block_thresold() const { return m_decommit_block_thresold; }
    uint64_t  decommit_free_thresold() const { return m_decommit_free_thresold; }
    address  lock_prefix_table() const { return m_lock_prefix_table; }
    uint64_t  max_alloc_size() const { return m_max_alloc_size; }
    uint64_t  memory_thresold() const { return m_memory_thresold; }
    uint64_t  process_affinity_mask() const { return m_process_affinity_mask; }
    uint32_t  process_heap_flags() const { return m_process_heap_flags; }
    uint16_t  service_pack_version() const { return m_service_pack_version; }
    address  security_cookie() const { return m_security_cookie; }
    std::vector<address> const&  structured_exception_handlers() const { return m_structured_exception_handlers; }

private:
    std::time_t  m_timestamp;
    uint16_t  m_major_version;
    uint16_t  m_minor_version;
    uint32_t  m_global_clear_flags;
    uint32_t  m_global_set_flags;
    uint32_t  m_critical_section_timeout;
    uint64_t  m_decommit_block_thresold;
    uint64_t  m_decommit_free_thresold;
    address  m_lock_prefix_table;
    uint64_t  m_max_alloc_size;
    uint64_t  m_memory_thresold;
    uint64_t  m_process_affinity_mask;
    uint32_t  m_process_heap_flags;
    uint16_t  m_service_pack_version;
    address  m_security_cookie;
    std::vector<address>  m_structured_exception_handlers;
};

typedef std::shared_ptr<load_configuration_structure const>  load_configuration_structure_ptr;

}}

#endif
