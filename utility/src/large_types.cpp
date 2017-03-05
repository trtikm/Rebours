#include <rebours/utility/large_types.hpp>
#include <rebours/utility/config.hpp>
#include <rebours/utility/assumptions.hpp>
#include <rebours/utility/invariants.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <algorithm>
#include <iostream>
#include <cstdint>


uint128_t::uint128_t(uint64_t const  lo, uint64_t const  hi)
    : m_data()
{
    this->lo() = lo;
    this->hi() = hi;
}

uint64_t  uint128_t::lo() const
{
    return *reinterpret_cast<uint64_t const *>(data() + (is_this_little_endian_machine() ? 0ULL : 8ULL));
}
uint64_t  uint128_t::hi() const
{
    return *reinterpret_cast<uint64_t const *>(data() + (is_this_little_endian_machine() ? 8ULL : 0ULL));
}

uint64_t&  uint128_t::lo()
{
    return *reinterpret_cast<uint64_t*>(data() + (is_this_little_endian_machine() ? 0ULL : 8ULL));
}
uint64_t&  uint128_t::hi()
{
    return *reinterpret_cast<uint64_t*>(data() + (is_this_little_endian_machine() ? 8ULL : 0ULL));
}


using  uint128_impl = boost::multiprecision::uint128_t;

//#if COMPILER() == COMPILER_GCC()
//using  uint128_impl = __uint128_t;
//#endif

static uint128_impl  as_impl(uint128_t const&  a)
{
    return uint128_impl(a.lo()) | (uint128_impl(a.hi()) << 64U);
}

static uint128_t  from_impl(uint128_impl const&  I)
{
    return uint128_t((uint64_t)I,(uint64_t)(I << 64U));
}


uint128_t  operator+(uint128_t const&  a, uint128_t const&  b)
{
    return from_impl(as_impl(a) + as_impl(b)); 
}

uint128_t  operator*(uint128_t const&  a, uint128_t const&  b)
{
    return from_impl(as_impl(a) * as_impl(b)); 
}

uint128_t  operator%(uint128_t const&  a, uint128_t const&  b)
{
    return from_impl(as_impl(a) % as_impl(b)); 
}

uint128_t  operator/(uint128_t const&  a, uint128_t const&  b)
{
    return from_impl(as_impl(a) / as_impl(b)); 
}

bool  operator==(uint128_t const&  a, uint128_t const&  b)
{
    return as_impl(a) == as_impl(b); 
}

bool  operator<(uint128_t const&  a, uint128_t const&  b)
{
    return as_impl(a) < as_impl(b); 
}


std::string  to_string(uint128_t const&  a, std::ios_base::fmtflags const  flags)
{
    std::string  digits;
    uint32_t  base;
    {
        if ((flags & std::ios::dec) != 0U)
        {
            digits = "0123456789";
            base = 10U;
        }
        else if ((flags & std::ios::hex) != 0U)
        {
            digits = "0123456789abcdef";
            base = 16U;
        }
        else
        {
            ASSUMPTION((flags & std::ios::oct) != 0U);
            digits = "01234567";
            base = 8U;
        }
    }
    uint128_impl value(as_impl(a));
    std::string  result;
    do
    {
        result.push_back(digits.at((std::size_t)(value % base)));
        value /= base;
    }
    while (value != 0U);
    std::reverse(result.begin(),result.end());

    return result;
}


msgstream&  operator<<(msgstream&  ostr, uint128_t const&  a)
{
    ostr << to_string(a,ostr.flags());
    return ostr;
}

std::ostream&  operator<<(std::ostream&  ostr, uint128_t const&  a)
{
    ostr << to_string(a,ostr.flags());
    return ostr;
}
