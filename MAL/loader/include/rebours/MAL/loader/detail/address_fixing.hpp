#ifndef REBOURS_MAL_LOADER_DETAIL_ADDRESS_FIXING_HPP_INCLUDED
#   define REBOURS_MAL_LOADER_DETAIL_ADDRESS_FIXING_HPP_INCLUDED

#   include <rebours/MAL/loader/address.hpp>
#   include <map>
#   include <memory>

namespace loader { namespace detail {


typedef  std::map<address,address>  address_fixes_map;
typedef  std::shared_ptr<address_fixes_map>  address_fixes_map_ptr;

bool  can_address_be_fixed(address const  orig_address, address_fixes_map const&  fixed_section_starts);

address  fixed_base_address(address_fixes_map const&  fixed_section_starts);

address  adjust_address_to_fixed_sections(address const  orig_address,
                                          address_fixes_map const&  fixed_section_starts);

inline bool  can_address_be_fixed(address const  orig_address, address_fixes_map_ptr const  fixed_section_starts)
{
    return can_address_be_fixed(orig_address,*fixed_section_starts);
}

inline address  fixed_base_address(address_fixes_map_ptr const fixed_section_starts)
{
    return fixed_base_address(*fixed_section_starts);
}

inline address  adjust_address_to_fixed_sections(address const  orig_address,
                                                 address_fixes_map_ptr const fixed_section_starts)
{
    return adjust_address_to_fixed_sections(orig_address,*fixed_section_starts);
}


}}

#endif
