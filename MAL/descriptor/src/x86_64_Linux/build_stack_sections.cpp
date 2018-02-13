#include <rebours/MAL/descriptor/storage.hpp>
#include <rebours/utility/assumptions.hpp>

namespace mal { namespace descriptor { namespace detail { namespace x86_64_Linux {


void  build_stack_sections(std::vector<stack_section>&  sections, uint64_t const  desred_size, uint64_t const  begin_address)
{
    static uint64_t constexpr  stack_max_address = 0x7FFFFFFFFFFFULL;
    static uint64_t constexpr  stack_default_size = 0x800000ULL;

    uint64_t const  stack_size = desred_size > 0ULL ? desred_size : stack_default_size;

    ASSUMPTION(begin_address < stack_max_address - stack_size);

    sections.push_back({
            stack_max_address + 1ULL - stack_size,
            stack_max_address + 1ULL,
            true,
            true,
            false,
            false,
            true
            });
}


}}}}
