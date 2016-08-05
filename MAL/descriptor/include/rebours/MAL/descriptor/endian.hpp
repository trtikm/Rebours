#ifndef ENDIAN_HPP_INCLUDED
#   define ENDIAN_HPP_INCLUDED

#   include <cstdint>

inline bool is_this_little_endian_machine() noexcept
{
    return *reinterpret_cast<uint32_t const* const>("\1\0\0\0") == 1U;
}


#endif
