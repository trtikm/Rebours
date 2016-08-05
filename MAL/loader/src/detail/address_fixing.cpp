#include <rebours/MAL/loader/detail/address_fixing.hpp>
#include <rebours/MAL/loader/assumptions.hpp>

namespace loader { namespace detail {


bool  can_address_be_fixed(address const  orig_address, address_fixes_map const&  fixed_section_starts)
{
    return fixed_section_starts.upper_bound(orig_address) != fixed_section_starts.cbegin();
}

address  fixed_base_address(address_fixes_map const&  fixed_section_starts)
{
    ASSUMPTION(!fixed_section_starts.empty());
    return fixed_section_starts.cbegin()->second;
}

address  adjust_address_to_fixed_sections(address const  orig_address,
                                          address_fixes_map const&  fixed_section_starts)
{
    auto const  it = fixed_section_starts.upper_bound(orig_address);
    ASSUMPTION(it != fixed_section_starts.cbegin());
    auto const  address_it = std::prev(it);
    return address_it->second + (orig_address - address_it->first);
}


}}
