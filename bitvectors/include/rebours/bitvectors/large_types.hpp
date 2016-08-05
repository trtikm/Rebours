#ifndef LARGE_TYPES_HPP_INCLUDED
#   define LARGE_TYPES_HPP_INCLUDED

#   include <rebours/bitvectors/msgstream.hpp>
#   include <array>
#   include <tuple>
#   include <string>
#   include <cstdint>
#   include <iosfwd>


struct uint128_t
{
    using data_type = std::array<uint8_t,16U>;

    uint128_t() : m_data() {}
    uint128_t(uint64_t const  lo, uint64_t const  hi = 0ULL);

    static constexpr uint64_t  size() noexcept { return std::tuple_size<data_type>::value; }
    uint8_t  at(uint64_t const  i) const { return m_data.at(i); }
    uint8_t&  at(uint64_t const  i) { return m_data.at(i); }

    uint8_t const*  data() const { return m_data.data(); }
    uint8_t*  data() { return m_data.data(); }

    uint64_t  lo() const { return *reinterpret_cast<uint64_t const *>(data()); }
    uint64_t  hi() const { return *reinterpret_cast<uint64_t const *>(data() + 8ULL); }

    uint64_t&  lo() { return *reinterpret_cast<uint64_t*>(data()); }
    uint64_t&  hi() { return *reinterpret_cast<uint64_t*>(data() + 8ULL); }

private:
    data_type  m_data;
};

uint128_t  operator+(uint128_t const&  a, uint128_t const&  b);
uint128_t  operator*(uint128_t const&  a, uint128_t const&  b);
uint128_t  operator%(uint128_t const&  a, uint128_t const&  b);
uint128_t  operator/(uint128_t const&  a, uint128_t const&  b);

bool  operator==(uint128_t const&  a, uint128_t const&  b);
bool  operator<(uint128_t const&  a, uint128_t const&  b);

inline bool  operator!=(uint128_t const&  a, uint128_t const&  b) { return !(a == b); }
inline bool  operator<=(uint128_t const&  a, uint128_t const&  b) { return a == b || a < b; }
inline bool  operator>=(uint128_t const&  a, uint128_t const&  b) { return a == b || b < a; }
inline bool  operator>(uint128_t const&  a, uint128_t const&  b) { return b < a; }


using  float80_t = std::array<uint8_t,10>;


std::string  to_string(uint128_t const&  a, std::ios_base::fmtflags const  flags = std::ios::dec);


msgstream&  operator<<(msgstream&  ostr, uint128_t const&  a);
std::ostream&  operator<<(std::ostream&  ostr, uint128_t const&  a);


#endif
