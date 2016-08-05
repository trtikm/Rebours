#include <rebours/MAL/loader/special_sections/load_configuration_structure.hpp>

namespace loader { namespace special_section {

load_configuration_structure::load_configuration_structure(
        address const  start_address,
        address const  end_address
        )
    : special_section_properties(start_address,end_address,
                                 "Holds additional info for the loader, including SEH list (x86 only).")
    , m_timestamp(0)
    , m_major_version(0U)
    , m_minor_version(0U)
    , m_global_clear_flags(0U)
    , m_global_set_flags(0U)
    , m_critical_section_timeout(0U)
    , m_decommit_block_thresold(0ULL)
    , m_decommit_free_thresold(0ULL)
    , m_lock_prefix_table(0ULL)
    , m_max_alloc_size(0ULL)
    , m_memory_thresold(0ULL)
    , m_process_affinity_mask(0ULL)
    , m_process_heap_flags(0U)
    , m_service_pack_version(0U)
    , m_security_cookie(0ULL)
    , m_structured_exception_handlers()
{}

load_configuration_structure::load_configuration_structure(
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
        )
    : special_section_properties(start_address,end_address,
                                 "Holds additional info for the loader, including SEH list (x86 only).")
    , m_timestamp(timestamp)
    , m_major_version(major_version)
    , m_minor_version(minor_version)
    , m_global_clear_flags(global_clear_flags)
    , m_global_set_flags(global_set_flags)
    , m_critical_section_timeout(critical_section_timeout)
    , m_decommit_block_thresold(decommit_block_thresold)
    , m_decommit_free_thresold(decommit_free_thresold)
    , m_lock_prefix_table(lock_prefix_table)
    , m_max_alloc_size(max_alloc_size)
    , m_memory_thresold(memory_thresold)
    , m_process_affinity_mask(process_affinity_mask)
    , m_process_heap_flags(process_heap_flags)
    , m_service_pack_version(service_pack_version)
    , m_security_cookie(security_cookie)
    , m_structured_exception_handlers(structured_exception_handlers)
{}


}}
