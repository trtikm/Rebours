#include <rebours/MAL/loader/detail/load_mach.hpp>
#include <rebours/MAL/loader/detail/abi_loaders.hpp>
#include <rebours/MAL/loader/detail/address_fixing.hpp>
#include <rebours/MAL/loader/detail/set_file_property.hpp>
#include <rebours/MAL/loader/file_utils.hpp>
#include <rebours/MAL/loader/to_string.hpp>
#include <rebours/MAL/loader/assumptions.hpp>
#include <rebours/MAL/loader/invariants.hpp>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <functional>
#include <tuple>
#include <unordered_set>
#include <iostream>

namespace loader { namespace detail {


std::string  load_mach_32bit(std::ifstream&  mach, file_props_ptr  mach_props,
                             uint32_t const  num_load_commnads,
                             uint32_t const  num_bytes_of_load_commnads,
                             bool const  is_fixed_dynamic_library,
                             load_props_mach&  load_props)
{
    ASSUMPTION( mach.is_open() );
    ASSUMPTION( mach_props->num_address_bits() == 32U );

    //bool const  is_it_load_of_root_binary = load_props.root_file() == mach_props->path();

    (void)is_fixed_dynamic_library;
    (void)load_props;

    uint64_t const  commands_begin_offset = mach.tellg();
    for (uint32_t i = 0U; i < num_load_commnads; ++i)
    {
        uint64_t const  command_offset = mach.tellg();

        uint32_t const  commnad_type = read_bytes_to_int32_t(mach,4U,mach_props->is_in_big_endian());
        uint32_t const  commnad_size = read_bytes_to_int32_t(mach,4U,mach_props->is_in_big_endian());

        if (commnad_size % 4U != 0U)
            return "Reached a load command which is not 4-byte aligned.";

        std::string  error_message = "";
        switch (commnad_type)
        {
        default:
            break;
        }
        if (!error_message.empty())
            return error_message;
        if ((uint64_t)mach.tellg() > command_offset + commnad_size)
            return "The load command went beyond its expected size.";

        mach.seekg(command_offset + commnad_size);
    }
    if ((uint64_t)mach.tellg() > commands_begin_offset + num_bytes_of_load_commnads)
        return "Load commands went beyond their expected size.";

    return "The load of Mach-O 32 bit is NOT IMPLEMENTED YET!";
}


}}
