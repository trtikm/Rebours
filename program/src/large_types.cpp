#include <rebours/program/large_types.hpp>
#include <rebours/program/assumptions.hpp>
#include <rebours/program/invariants.hpp>
#include <algorithm>
#include <iostream>


using  uint128_impl = __uint128_t;  static_assert(sizeof(uint128_impl) == 16ULL,"Well, just to be sure.");


uint128_t::uint128_t(uint64_t const  lo, uint64_t const  hi)
    : m_data()
{
    this->lo() = lo;
    this->hi() = hi;
}


uint128_t  operator+(uint128_t const&  a, uint128_t const&  b)
{
    uint128_t  result;
    *reinterpret_cast<uint128_impl*>(result.data()) =
            *reinterpret_cast<uint128_impl const*>(a.data()) +
            *reinterpret_cast<uint128_impl const*>(b.data()) ;
    return result;
}

uint128_t  operator*(uint128_t const&  a, uint128_t const&  b)
{
    uint128_t  result;
    *reinterpret_cast<uint128_impl*>(result.data()) =
            *reinterpret_cast<uint128_impl const*>(a.data()) *
            *reinterpret_cast<uint128_impl const*>(b.data()) ;
    return result;
}

uint128_t  operator%(uint128_t const&  a, uint128_t const&  b)
{
    uint128_t  result;
    *reinterpret_cast<uint128_impl*>(result.data()) =
            *reinterpret_cast<uint128_impl const*>(a.data()) %
            *reinterpret_cast<uint128_impl const*>(b.data()) ;
    return result;
}

uint128_t  operator/(uint128_t const&  a, uint128_t const&  b)
{
    uint128_t  result;
    *reinterpret_cast<uint128_impl*>(result.data()) =
            *reinterpret_cast<uint128_impl const*>(a.data()) /
            *reinterpret_cast<uint128_impl const*>(b.data()) ;
    return result;
}

bool  operator==(uint128_t const&  a, uint128_t const&  b)
{
    return *reinterpret_cast<uint128_impl const*>(a.data()) ==
           *reinterpret_cast<uint128_impl const*>(b.data()) ;
}

bool  operator<(uint128_t const&  a, uint128_t const&  b)
{
    return *reinterpret_cast<uint128_impl const*>(a.data()) <
           *reinterpret_cast<uint128_impl const*>(b.data()) ;
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
    uint128_impl value = *reinterpret_cast<uint128_impl const*>(a.data());
    std::string  result;
    do
    {
        result.push_back(digits.at(value % base));
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
