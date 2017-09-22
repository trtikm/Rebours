#include <rebours/bitvectors/symbol.hpp>
#include <rebours/utility/assumptions.hpp>
#include <rebours/utility/invariants.hpp>
#include <sstream>
#include <algorithm>
#include <functional>
#include <unordered_set>
#include <mutex>

namespace bv {


struct symbol_impl
{
    static symbol  create(std::string const&  name,
                          uint64_t const  num_bits_of_return_value,
                          std::vector<uint64_t> const&  num_bits_of_parameters,
                          bool const  is_interpreted);

    std::string const&  name() const noexcept { return m_name; }
    uint64_t  num_bits_of_return_value() const noexcept { return m_num_bits_of_return_value; }
    uint64_t  num_parameters() const noexcept { return m_num_bits_of_parameters.size(); }
    uint64_t  num_bits_of_parameter(uint64_t const  param_index) const { return m_num_bits_of_parameters.at(param_index); }
    bool  is_interpreted() const noexcept { return m_is_interpreted; }

private:

    symbol_impl(std::string const&  name,
                uint64_t const  num_bits_of_return_value,
                std::vector<uint64_t> const&  num_bits_of_parameters,
                bool const  is_interpreted);

    using symbols_dictionary = std::unordered_set<symbol_impl,decltype(&symbol_impl_hash),decltype(&symbol_impl_equal)>;
    static symbols_dictionary  symbols;
    static std::mutex  symbol_construction_mutex;

    std::string  m_name;
    uint64_t  m_num_bits_of_return_value;
    std::vector<uint64_t>  m_num_bits_of_parameters;
    bool  m_is_interpreted;
};


symbol_impl::symbols_dictionary  symbol_impl::symbols(
        0,
        &symbol_impl_hash,
        &symbol_impl_equal
        );

std::mutex  symbol_impl::symbol_construction_mutex;

symbol  symbol_impl::create(std::string const&  name,
                            uint64_t const  num_bits_of_return_value,
                            std::vector<uint64_t> const&  num_bits_of_parameters,
                            bool const  is_interpreted)
{
    std::lock_guard<std::mutex> const  lock(symbol_construction_mutex);
    return symbol{&*symbols.insert({name,num_bits_of_return_value,num_bits_of_parameters,is_interpreted}).first};
}


symbol_impl::symbol_impl(std::string const&  name,
                         uint64_t const  num_bits_of_return_value,
                         std::vector<uint64_t> const&  num_bits_of_parameters,
                         bool const  is_interpreted)
    : m_name{name}
    , m_num_bits_of_return_value{num_bits_of_return_value}
    , m_num_bits_of_parameters{num_bits_of_parameters}
    , m_is_interpreted{is_interpreted}
{
    ASSUMPTION(!m_name.empty());
    ASSUMPTION(m_name.find_first_of(" \t\r\n") == std::string::npos);
    ASSUMPTION(m_num_bits_of_return_value > 0ULL);
    ASSUMPTION(
            [](std::vector<uint64_t> const& v) {
                for (auto num : v)
                    if (num == 0ULL)
                        return false;
                return true;
            }(m_num_bits_of_parameters)
            );
}


std::size_t  symbol_impl_hash(symbol_impl const&  s)
{
    std::size_t  result = std::hash<std::string>()(s.name());
    result += 31ULL * std::hash<uint64_t>()(s.num_bits_of_return_value());
    for (uint64_t i = 0ULL; i < s.num_parameters(); ++i)
        result += (i + 1ULL) * 101ULL * std::hash<uint64_t>()(s.num_bits_of_parameter(i));
    result += s.is_interpreted() ? 1ULL : 71ULL;
    return result;
}

bool  symbol_impl_equal(symbol_impl const&  s1, symbol_impl const&  s2)
{
    return s1.name() == s2.name() &&
           s1.num_bits_of_return_value() == s2.num_bits_of_return_value() &&
           s1.is_interpreted() == s2.is_interpreted() &&
           s1.num_parameters() == s2.num_parameters() &&
           [](symbol_impl const& s1, symbol_impl const& s2) {
                for (uint64_t i = 0ULL; i < s1.num_parameters(); ++i)
                    if (s1.num_bits_of_parameter(i) != s2.num_bits_of_parameter(i))
                        return false;
                return true;
                }(s1,s2)
           ;
}


/**
 * Queries for properties of a symbol.
 */

std::string const&  symbol_name(symbol const  s) { return s->name(); }
uint64_t  symbol_num_bits_of_return_value(symbol const  s) { return s->num_bits_of_return_value(); }
uint64_t  symbol_num_parameters(symbol const  s) { return s->num_parameters(); }
uint64_t  symbol_num_bits_of_parameter(symbol const  s, uint64_t const  param_index) { return s->num_bits_of_parameter(param_index); }
bool  symbol_is_interpreted(symbol const  s) { return s->is_interpreted(); }

bool  symbol_is_interpreted_constant(symbol const  s)
{
    return s->is_interpreted() &&
           s->name().size() >= 4ULL &&
           s->name().at(0ULL) == '0' &&
           s->name().at(1ULL) == 'x';
}

bool  symbol_is_interpreted_cast(symbol const s)
{
    return s->is_interpreted() && *symbol_name(s).cbegin() == '#';
}

bool  symbol_is_interpreted_operation(symbol const  s)
{
    return s->is_interpreted() && s->num_bits_of_return_value() != 1ULL && *s->name().cbegin() != '0' && *s->name().cbegin() != '#';
}

bool  symbol_is_interpreted_comparison(symbol const  s)
{
    return s->is_interpreted() && s->num_bits_of_return_value() == 1ULL && (*s->name().cbegin() == '<' || *s->name().cbegin() == '=');
}

bool  symbol_is_interpreted_less_than(symbol const  s)
{
    return s->is_interpreted() && s->num_bits_of_return_value() == 1ULL && *s->name().cbegin() == '<';
}

bool  symbol_is_interpreted_equal(symbol const  s)
{
    return s->is_interpreted() && s->num_bits_of_return_value() == 1ULL && *s->name().cbegin() == '=';
}

bool  symbol_is_interpreted_equal_int(symbol const  s)
{
    return s->is_interpreted() && s->num_bits_of_return_value() == 1ULL && s->name().at(0ULL) == '=' && s->name().at(1ULL) == 'i';
}

bool  symbol_is_interpreted_equal_float(symbol const  s)
{
    return s->is_interpreted() && s->num_bits_of_return_value() == 1ULL && s->name().at(0ULL) == '=' && s->name().at(1ULL) == 'f';
}


/**
 * Construction of symbols for interpreted constants
 */

symbol  make_symbol_of_interpreted_constant(std::string const&  values_of_bytes_in_hexadecimal_format)
{
    ASSUMPTION(values_of_bytes_in_hexadecimal_format.size() > 1ULL && values_of_bytes_in_hexadecimal_format.size() % 2ULL == 0ULL);
    ASSUMPTION(
            [](std::string const&  s) {
                for (auto c : s)
                    if (!::isxdigit(c) || c != ::tolower(c))
                        return false;
                return true;
                }(values_of_bytes_in_hexadecimal_format));
    std::string  name{"0x"};
    name.append(values_of_bytes_in_hexadecimal_format);
    return symbol_impl::create(name,(name.size()-2ULL)*4ULL,{},true);
}

symbol  make_symbol_of_interpreted_constant(uint8_t const*  begin, uint8_t const* const  end,
                                            bool const  revert_bytes_order)
{
    ASSUMPTION(begin < end);

    std::string  name{"0x"};
    {
        uint8_t const*  b = revert_bytes_order ? end - 1ULL : begin;
        uint8_t const* const  e = revert_bytes_order ? begin - 1ULL : end;
        int64_t const  s = revert_bytes_order ? -1LL : 1LL;
        for ( ; b != e; b += s)
        {
            std::ostringstream  ostr;
            ostr << std::hex << (uint32_t)*b;
            INVARIANT(ostr.str().size() == 1ULL || ostr.str().size() == 2ULL);
            if (ostr.str().size() == 1ULL)
                name.push_back('0');
            name.append(ostr.str().c_str());
        }
    }
    INVARIANT(name.size() >= 4ULL);

    return symbol_impl::create(name,(name.size()-2ULL)*4ULL,{},true);
}


/**
 * Construction of symbols for uninterpreted constants and functions
 */

symbol  make_symbol_of_unintepreted_function(
                        std::string const&  name,
                        uint64_t const  num_bits_of_return_value,
                        std::vector<uint64_t> const&  num_bits_of_parameters
                        )
{
    return symbol_impl::create(name,num_bits_of_return_value,num_bits_of_parameters,false);
}


/**
 * Construction of symbols for logical constants, conectives, and universal quantifier
 */

symbol  make_symbol_of_true()
{
    static symbol const  value = symbol_impl::create("tt",1ULL,{},true);
    return value;
}

symbol  make_symbol_of_false()
{
    static symbol const  value = symbol_impl::create("ff",1ULL,{},true);
    return value;
}

symbol  make_symbol_of_logical_conjunction()
{
    static symbol const  value = symbol_impl::create("&&",1ULL,{1ULL,1ULL},true);
    return value;
}

symbol  make_symbol_of_logical_negation()
{
    static symbol const  value = symbol_impl::create("!",1ULL,{1ULL},true);
    return value;
}

symbol  make_symbol_of_quantifier_forall(uint64_t const  num_var_bits)
{
    switch (num_var_bits)
    {
    case 8ULL:
    case 16ULL:
    case 32ULL:
    case 64ULL:
    case 128ULL:
        break;
    default:
        UNREACHABLE();
    }
    static symbol const  value = symbol_impl::create("A",num_var_bits,{1ULL},true);
    return value;
}


/**
 * Construction of symbols for interpreted cast operations
 */


symbol  make_symbol_of_interpreted_truncate(uint64_t const  num_src_bits, uint64_t const  num_dst_bits)
{
    uint64_t const  key = num_dst_bits + (num_src_bits << 8ULL);
    switch (key)
    {
    case 0x8040ULL: // 128 -> 64
        {
            static symbol const  value = symbol_impl::create("#i128i64",64ULL,{128ULL},true);
            return value;
        }
        break;
    case 0x4020ULL: // 64 -> 32
        {
            static symbol const  value = symbol_impl::create("#i64i32",32ULL,{64ULL},true);
            return value;
        }
        break;
    case 0x2010ULL: // 32 -> 16
        {
            static symbol const  value = symbol_impl::create("#i32i16",16ULL,{32ULL},true);
            return value;
        }
        break;
    case 0x1008ULL: // 16 -> 8
        {
            static symbol const  value = symbol_impl::create("#i16i8",8ULL,{16ULL},true);
            return value;
        }
        break;
    default:
        UNREACHABLE();
    }
}

symbol  make_symbol_of_interpreted_cast_signed_int(uint64_t const  num_src_bits, uint64_t const  num_dst_bits)
{
    uint64_t const  key = num_dst_bits + (num_src_bits << 8ULL);
    switch (key)
    {
    case 0x0810ULL: // 8 -> 16
        {
            static symbol const  value = symbol_impl::create("#s8s16",16ULL,{8ULL},true);
            return value;
        }
        break;
    case 0x1020ULL: // 16 -> 32
        {
            static symbol const  value = symbol_impl::create("#s16s32",32ULL,{16ULL},true);
            return value;
        }
        break;
    case 0x2040ULL: // 32 -> 64
        {
            static symbol const  value = symbol_impl::create("#s32s64",64ULL,{32ULL},true);
            return value;
        }
        break;
    case 0x4080ULL: // 64 -> 128
        {
            static symbol const  value = symbol_impl::create("#s64s128",128ULL,{64ULL},true);
            return value;
        }
        break;
    default:
        return make_symbol_of_interpreted_truncate(num_src_bits,num_dst_bits);
    }
}

symbol  make_symbol_of_interpreted_cast_unsigned_int(uint64_t const  num_src_bits, uint64_t const  num_dst_bits)
{
    uint64_t const  key = num_dst_bits + (num_src_bits << 8ULL);
    switch (key)
    {
    case 0x0810ULL: // 8 -> 16
        {
            static symbol const  value = symbol_impl::create("#u8u16",16ULL,{8ULL},true);
            return value;
        }
        break;
    case 0x1020ULL: // 16 -> 32
        {
            static symbol const  value = symbol_impl::create("#u16u32",32ULL,{16ULL},true);
            return value;
        }
        break;
    case 0x2040ULL: // 32 -> 64
        {
            static symbol const  value = symbol_impl::create("#u32u64",64ULL,{32ULL},true);
            return value;
        }
        break;
    case 0x4080ULL: // 64 -> 128
        {
            static symbol const  value = symbol_impl::create("#u64u128",128ULL,{64ULL},true);
            return value;
        }
        break;
    default:
        return make_symbol_of_interpreted_truncate(num_src_bits,num_dst_bits);
    }
}

symbol  make_symbol_of_interpreted_cast_float(uint64_t const  num_src_bits, uint64_t const  num_dst_bits)
{
    uint64_t const  key = num_dst_bits + (num_src_bits << 8ULL);
    switch (key)
    {
    case 0x2040ULL: // 32 -> 64
        {
            static symbol const  value = symbol_impl::create("#f32f64",64ULL,{32ULL},true);
            return value;
        }
        break;
    case 0x4050ULL: // 64 -> 80
        {
            static symbol const  value = symbol_impl::create("#f64f80",80ULL,{64ULL},true);
            return value;
        }
        break;
    case 0x5040ULL: // 80 -> 64
        {
            static symbol const  value = symbol_impl::create("#f80f64",64ULL,{80ULL},true);
            return value;
        }
        break;
    case 0x4020ULL: // 64 -> 32
        {
            static symbol const  value = symbol_impl::create("#f64f32",32ULL,{64ULL},true);
            return value;
        }
        break;
    default:
        UNREACHABLE();
    }
}

symbol  make_symbol_of_interpreted_cast_signed_int_to_float(uint64_t const  num_bits)
{
    switch (num_bits)
    {
    case 32ULL:
        {
            static symbol const  value = symbol_impl::create("#s32f32",32ULL,{32ULL},true);
            return value;
        }
        break;
    case 64ULL:
        {
            static symbol const  value = symbol_impl::create("#s64f64",64ULL,{64ULL},true);
            return value;
        }
        break;
    default:
        UNREACHABLE();
    }
}

symbol  make_symbol_of_interpreted_cast_unsigned_int_to_float(uint64_t const  num_bits)
{
    switch (num_bits)
    {
    case 32ULL:
        {
            static symbol const  value = symbol_impl::create("#u32f32",32ULL,{32ULL},true);
            return value;
        }
        break;
    case 64ULL:
        {
            static symbol const  value = symbol_impl::create("#u64f64",64ULL,{64ULL},true);
            return value;
        }
        break;
    default:
        UNREACHABLE();
    }
}

symbol  make_symbol_of_interpreted_cast_float_to_signed_int(uint64_t const  num_bits)
{
    switch (num_bits)
    {
    case 32ULL:
        {
            static symbol const  value = symbol_impl::create("#f32s32",32ULL,{32ULL},true);
            return value;
        }
        break;
    case 64ULL:
        {
            static symbol const  value = symbol_impl::create("#f64s64",64ULL,{64ULL},true);
            return value;
        }
        break;
    default:
        UNREACHABLE();
    }
}

symbol  make_symbol_of_interpreted_cast_float_to_unsigned_int(uint64_t const  num_bits)
{
    switch (num_bits)
    {
    case 32ULL:
        {
            static symbol const  value = symbol_impl::create("#f32u32",32ULL,{32ULL},true);
            return value;
        }
        break;
    case 64ULL:
        {
            static symbol const  value = symbol_impl::create("#f64u64",64ULL,{64ULL},true);
            return value;
        }
        break;
    default:
        UNREACHABLE();
    }
}


/**
 * Construction of symbols for interpreted arithmetic operations
 */

symbol  make_symbol_of_interpreted_binary_add_int(uint64_t const  num_bits)
{
    switch (num_bits)
    {
    case 8ULL:
        {
            static symbol const  value = symbol_impl::create("+i8",8ULL,{8ULL,8ULL},true);
            return value;
        }
        break;
    case 16ULL:
        {
            static symbol const  value = symbol_impl::create("+i16",16ULL,{16ULL,16ULL},true);
            return value;
        }
        break;
    case 32ULL:
        {
            static symbol const  value = symbol_impl::create("+i32",32ULL,{32ULL,32ULL},true);
            return value;
        }
        break;
    case 64ULL:
        {
            static symbol const  value = symbol_impl::create("+i64",64ULL,{64ULL,64ULL},true);
            return value;
        }
        break;
    case 128ULL:
        {
            static symbol const  value = symbol_impl::create("+i128",128ULL,{128ULL,128ULL},true);
            return value;
        }
        break;
    default:
        UNREACHABLE();
    }
}

symbol  make_symbol_of_interpreted_binary_add_float(uint64_t const  num_bits)
{
    switch (num_bits)
    {
    case 32ULL:
        {
            static symbol const  value = symbol_impl::create("+f32",32ULL,{32ULL,32ULL},true);
            return value;
        }
        break;
    case 64ULL:
        {
            static symbol const  value = symbol_impl::create("+f64",64ULL,{64ULL,64ULL},true);
            return value;
        }
        break;
    case 80ULL:
        {
            static symbol const  value = symbol_impl::create("+f80",80ULL,{80ULL,80ULL},true);
            return value;
        }
        break;
    default:
        UNREACHABLE();
    }
}


symbol  make_symbol_of_interpreted_binary_subtract_int(uint64_t const  num_bits)
{
    switch (num_bits)
    {
    case 8ULL:
        {
            static symbol const  value = symbol_impl::create("-i8",8ULL,{8ULL,8ULL},true);
            return value;
        }
        break;
    case 16ULL:
        {
            static symbol const  value = symbol_impl::create("-i16",16ULL,{16ULL,16ULL},true);
            return value;
        }
        break;
    case 32ULL:
        {
            static symbol const  value = symbol_impl::create("-i32",32ULL,{32ULL,32ULL},true);
            return value;
        }
        break;
    case 64ULL:
        {
            static symbol const  value = symbol_impl::create("-i64",64ULL,{64ULL,64ULL},true);
            return value;
        }
        break;
    case 128ULL:
        {
            static symbol const  value = symbol_impl::create("-i128",128ULL,{128ULL,128ULL},true);
            return value;
        }
        break;
    default:
        UNREACHABLE();
    }
}

symbol  make_symbol_of_interpreted_binary_subtract_float(uint64_t const  num_bits)
{
    switch (num_bits)
    {
    case 32ULL:
        {
            static symbol const  value = symbol_impl::create("-f32",32ULL,{32ULL,32ULL},true);
            return value;
        }
        break;
    case 64ULL:
        {
            static symbol const  value = symbol_impl::create("-f64",64ULL,{64ULL,64ULL},true);
            return value;
        }
        break;
    case 80ULL:
        {
            static symbol const  value = symbol_impl::create("-f80",80ULL,{80ULL,80ULL},true);
            return value;
        }
        break;
    default:
        UNREACHABLE();
    }
}


symbol  make_symbol_of_interpreted_binary_multiply_int(uint64_t const  num_bits)
{
    switch (num_bits)
    {
    case 8ULL:
        {
            static symbol const  value = symbol_impl::create("*i8",8ULL,{8ULL,8ULL},true);
            return value;
        }
        break;
    case 16ULL:
        {
            static symbol const  value = symbol_impl::create("*i16",16ULL,{16ULL,16ULL},true);
            return value;
        }
        break;
    case 32ULL:
        {
            static symbol const  value = symbol_impl::create("*i32",32ULL,{32ULL,32ULL},true);
            return value;
        }
        break;
    case 64ULL:
        {
            static symbol const  value = symbol_impl::create("*i64",64ULL,{64ULL,64ULL},true);
            return value;
        }
        break;
    case 128ULL:
        {
            static symbol const  value = symbol_impl::create("*i128",128ULL,{128ULL,128ULL},true);
            return value;
        }
        break;
    default:
        UNREACHABLE();
    }
}

symbol  make_symbol_of_interpreted_binary_multiply_float(uint64_t const  num_bits)
{
    switch (num_bits)
    {
    case 32ULL:
        {
            static symbol const  value = symbol_impl::create("*f32",32ULL,{32ULL,32ULL},true);
            return value;
        }
        break;
    case 64ULL:
        {
            static symbol const  value = symbol_impl::create("*f64",64ULL,{64ULL,64ULL},true);
            return value;
        }
        break;
    case 80ULL:
        {
            static symbol const  value = symbol_impl::create("*f80",80ULL,{80ULL,80ULL},true);
            return value;
        }
        break;
    default:
        UNREACHABLE();
    }
}


symbol  make_symbol_of_interpreted_binary_divide_signed_int(uint64_t const  num_bits)
{
    switch (num_bits)
    {
    case 8ULL:
        {
            static symbol const  value = symbol_impl::create("/s8",8ULL,{8ULL,8ULL},true);
            return value;
        }
        break;
    case 16ULL:
        {
            static symbol const  value = symbol_impl::create("/s16",16ULL,{16ULL,16ULL},true);
            return value;
        }
        break;
    case 32ULL:
        {
            static symbol const  value = symbol_impl::create("/s32",32ULL,{32ULL,32ULL},true);
            return value;
        }
        break;
    case 64ULL:
        {
            static symbol const  value = symbol_impl::create("/s64",64ULL,{64ULL,64ULL},true);
            return value;
        }
        break;
    case 128ULL:
        {
            static symbol const  value = symbol_impl::create("/s128",128ULL,{128ULL,128ULL},true);
            return value;
        }
        break;
    default:
        UNREACHABLE();
    }
}

symbol  make_symbol_of_interpreted_binary_divide_unsigned_int(uint64_t const  num_bits)
{
    switch (num_bits)
    {
    case 8ULL:
        {
            static symbol const  value = symbol_impl::create("/u8",8ULL,{8ULL,8ULL},true);
            return value;
        }
        break;
    case 16ULL:
        {
            static symbol const  value = symbol_impl::create("/u16",16ULL,{16ULL,16ULL},true);
            return value;
        }
        break;
    case 32ULL:
        {
            static symbol const  value = symbol_impl::create("/u32",32ULL,{32ULL,32ULL},true);
            return value;
        }
        break;
    case 64ULL:
        {
            static symbol const  value = symbol_impl::create("/u64",64ULL,{64ULL,64ULL},true);
            return value;
        }
        break;
    case 128ULL:
        {
            static symbol const  value = symbol_impl::create("/u128",128ULL,{128ULL,128ULL},true);
            return value;
        }
        break;
    default:
        UNREACHABLE();
    }
}

symbol  make_symbol_of_interpreted_binary_divide_float(uint64_t const  num_bits)
{
    switch (num_bits)
    {
    case 32ULL:
        {
            static symbol const  value = symbol_impl::create("/f32",32ULL,{32ULL,32ULL},true);
            return value;
        }
        break;
    case 64ULL:
        {
            static symbol const  value = symbol_impl::create("/f64",64ULL,{64ULL,64ULL},true);
            return value;
        }
        break;
    case 80ULL:
        {
            static symbol const  value = symbol_impl::create("/f80",80ULL,{80ULL,80ULL},true);
            return value;
        }
        break;
    default:
        UNREACHABLE();
    }
}


symbol  make_symbol_of_interpreted_binary_remainder_signed_int(uint64_t const  num_bits)
{
    switch (num_bits)
    {
    case 8ULL:
        {
            static symbol const  value = symbol_impl::create("%s8",8ULL,{8ULL,8ULL},true);
            return value;
        }
        break;
    case 16ULL:
        {
            static symbol const  value = symbol_impl::create("%s16",16ULL,{16ULL,16ULL},true);
            return value;
        }
        break;
    case 32ULL:
        {
            static symbol const  value = symbol_impl::create("%s32",32ULL,{32ULL,32ULL},true);
            return value;
        }
        break;
    case 64ULL:
        {
            static symbol const  value = symbol_impl::create("%s64",64ULL,{64ULL,64ULL},true);
            return value;
        }
        break;
    case 128ULL:
        {
            static symbol const  value = symbol_impl::create("%s128",128ULL,{128ULL,128ULL},true);
            return value;
        }
        break;
    default:
        UNREACHABLE();
    }
}

symbol  make_symbol_of_interpreted_binary_remainder_unsigned_int(uint64_t const  num_bits)
{
    switch (num_bits)
    {
    case 8ULL:
        {
            static symbol const  value = symbol_impl::create("%u8",8ULL,{8ULL,8ULL},true);
            return value;
        }
        break;
    case 16ULL:
        {
            static symbol const  value = symbol_impl::create("%u16",16ULL,{16ULL,16ULL},true);
            return value;
        }
        break;
    case 32ULL:
        {
            static symbol const  value = symbol_impl::create("%u32",32ULL,{32ULL,32ULL},true);
            return value;
        }
        break;
    case 64ULL:
        {
            static symbol const  value = symbol_impl::create("%u64",64ULL,{64ULL,64ULL},true);
            return value;
        }
        break;
    case 128ULL:
        {
            static symbol const  value = symbol_impl::create("%u128",128ULL,{128ULL,128ULL},true);
            return value;
        }
        break;
    default:
        UNREACHABLE();
    }
}

symbol  make_symbol_of_interpreted_binary_remainder_float(uint64_t const  num_bits)
{
    switch (num_bits)
    {
    case 32ULL:
        {
            static symbol const  value = symbol_impl::create("%f32",32ULL,{32ULL,32ULL},true);
            return value;
        }
        break;
    case 64ULL:
        {
            static symbol const  value = symbol_impl::create("%f64",64ULL,{64ULL,64ULL},true);
            return value;
        }
        break;
    case 80ULL:
        {
            static symbol const  value = symbol_impl::create("%f80",80ULL,{80ULL,80ULL},true);
            return value;
        }
        break;
    default:
        UNREACHABLE();
    }
}


symbol  make_symbol_of_interpreted_binary_shift_left_int(uint64_t const  num_bits)
{
    switch (num_bits)
    {
    case 8ULL:
        {
            static symbol const  value = symbol_impl::create("<<i8",8ULL,{8ULL,8ULL},true);
            return value;
        }
        break;
    case 16ULL:
        {
            static symbol const  value = symbol_impl::create("<<i16",16ULL,{16ULL,16ULL},true);
            return value;
        }
        break;
    case 32ULL:
        {
            static symbol const  value = symbol_impl::create("<<i32",32ULL,{32ULL,32ULL},true);
            return value;
        }
        break;
    case 64ULL:
        {
            static symbol const  value = symbol_impl::create("<<i64",64ULL,{64ULL,64ULL},true);
            return value;
        }
        break;
    case 128ULL:
        {
            static symbol const  value = symbol_impl::create("<<i128",128ULL,{128ULL,128ULL},true);
            return value;
        }
        break;
    default:
        UNREACHABLE();
    }
}


symbol  make_symbol_of_interpreted_binary_shift_right_signed_int(uint64_t const  num_bits)
{
    switch (num_bits)
    {
    case 8ULL:
        {
            static symbol const  value = symbol_impl::create(">>s8",8ULL,{8ULL,8ULL},true);
            return value;
        }
        break;
    case 16ULL:
        {
            static symbol const  value = symbol_impl::create(">>s16",16ULL,{16ULL,16ULL},true);
            return value;
        }
        break;
    case 32ULL:
        {
            static symbol const  value = symbol_impl::create(">>s32",32ULL,{32ULL,32ULL},true);
            return value;
        }
        break;
    case 64ULL:
        {
            static symbol const  value = symbol_impl::create(">>s64",64ULL,{64ULL,64ULL},true);
            return value;
        }
        break;
    case 128ULL:
        {
            static symbol const  value = symbol_impl::create(">>s128",128ULL,{128ULL,128ULL},true);
            return value;
        }
        break;
    default:
        UNREACHABLE();
    }
}

symbol  make_symbol_of_interpreted_binary_shift_right_unsigned_int(uint64_t const  num_bits)
{
    switch (num_bits)
    {
    case 8ULL:
        {
            static symbol const  value = symbol_impl::create(">>u8",8ULL,{8ULL,8ULL},true);
            return value;
        }
        break;
    case 16ULL:
        {
            static symbol const  value = symbol_impl::create(">>u16",16ULL,{16ULL,16ULL},true);
            return value;
        }
        break;
    case 32ULL:
        {
            static symbol const  value = symbol_impl::create(">>u32",32ULL,{32ULL,32ULL},true);
            return value;
        }
        break;
    case 64ULL:
        {
            static symbol const  value = symbol_impl::create(">>u64",64ULL,{64ULL,64ULL},true);
            return value;
        }
        break;
    case 128ULL:
        {
            static symbol const  value = symbol_impl::create(">>u128",128ULL,{128ULL,128ULL},true);
            return value;
        }
        break;
    default:
        UNREACHABLE();
    }
}

symbol  make_symbol_of_interpreted_binary_rotate_left_int(uint64_t const  num_bits)
{
    switch (num_bits)
    {
    case 8ULL:
        {
            static symbol const  value = symbol_impl::create("<<<i8",8ULL,{8ULL,8ULL},true);
            return value;
        }
        break;
    case 16ULL:
        {
            static symbol const  value = symbol_impl::create("<<<i16",16ULL,{16ULL,16ULL},true);
            return value;
        }
        break;
    case 32ULL:
        {
            static symbol const  value = symbol_impl::create("<<<i32",32ULL,{32ULL,32ULL},true);
            return value;
        }
        break;
    case 64ULL:
        {
            static symbol const  value = symbol_impl::create("<<<i64",64ULL,{64ULL,64ULL},true);
            return value;
        }
        break;
    case 128ULL:
        {
            static symbol const  value = symbol_impl::create("<<<i128",128ULL,{128ULL,128ULL},true);
            return value;
        }
        break;
    default:
        UNREACHABLE();
    }
}

symbol  make_symbol_of_interpreted_binary_rotate_right_int(uint64_t const  num_bits)
{
    switch (num_bits)
    {
    case 8ULL:
        {
            static symbol const  value = symbol_impl::create(">>>i8",8ULL,{8ULL,8ULL},true);
            return value;
        }
        break;
    case 16ULL:
        {
            static symbol const  value = symbol_impl::create(">>>i16",16ULL,{16ULL,16ULL},true);
            return value;
        }
        break;
    case 32ULL:
        {
            static symbol const  value = symbol_impl::create(">>>i32",32ULL,{32ULL,32ULL},true);
            return value;
        }
        break;
    case 64ULL:
        {
            static symbol const  value = symbol_impl::create(">>>i64",64ULL,{64ULL,64ULL},true);
            return value;
        }
        break;
    case 128ULL:
        {
            static symbol const  value = symbol_impl::create(">>>i128",128ULL,{128ULL,128ULL},true);
            return value;
        }
        break;
    default:
        UNREACHABLE();
    }
}

symbol  make_symbol_of_interpreted_binary_bitwise_and_int(uint64_t const  num_bits)
{
    switch (num_bits)
    {
    case 8ULL:
        {
            static symbol const  value = symbol_impl::create("&i8",8ULL,{8ULL,8ULL},true);
            return value;
        }
        break;
    case 16ULL:
        {
            static symbol const  value = symbol_impl::create("&i16",16ULL,{16ULL,16ULL},true);
            return value;
        }
        break;
    case 32ULL:
        {
            static symbol const  value = symbol_impl::create("&i32",32ULL,{32ULL,32ULL},true);
            return value;
        }
        break;
    case 64ULL:
        {
            static symbol const  value = symbol_impl::create("&i64",64ULL,{64ULL,64ULL},true);
            return value;
        }
        break;
    case 128ULL:
        {
            static symbol const  value = symbol_impl::create("&i128",128ULL,{128ULL,128ULL},true);
            return value;
        }
        break;
    default:
        UNREACHABLE();
    }
}

symbol  make_symbol_of_interpreted_binary_bitwise_or_int(uint64_t const  num_bits)
{
    switch (num_bits)
    {
    case 8ULL:
        {
            static symbol const  value = symbol_impl::create("|i8",8ULL,{8ULL,8ULL},true);
            return value;
        }
        break;
    case 16ULL:
        {
            static symbol const  value = symbol_impl::create("|i16",16ULL,{16ULL,16ULL},true);
            return value;
        }
        break;
    case 32ULL:
        {
            static symbol const  value = symbol_impl::create("|i32",32ULL,{32ULL,32ULL},true);
            return value;
        }
        break;
    case 64ULL:
        {
            static symbol const  value = symbol_impl::create("|i64",64ULL,{64ULL,64ULL},true);
            return value;
        }
        break;
    case 128ULL:
        {
            static symbol const  value = symbol_impl::create("|i128",128ULL,{128ULL,128ULL},true);
            return value;
        }
        break;
    default:
        UNREACHABLE();
    }
}

symbol  make_symbol_of_interpreted_binary_bitwise_xor_int(uint64_t const  num_bits)
{
    switch (num_bits)
    {
    case 8ULL:
        {
            static symbol const  value = symbol_impl::create("^i8",8ULL,{8ULL,8ULL},true);
            return value;
        }
        break;
    case 16ULL:
        {
            static symbol const  value = symbol_impl::create("^i16",16ULL,{16ULL,16ULL},true);
            return value;
        }
        break;
    case 32ULL:
        {
            static symbol const  value = symbol_impl::create("^i32",32ULL,{32ULL,32ULL},true);
            return value;
        }
        break;
    case 64ULL:
        {
            static symbol const  value = symbol_impl::create("^i64",64ULL,{64ULL,64ULL},true);
            return value;
        }
        break;
    case 128ULL:
        {
            static symbol const  value = symbol_impl::create("^i128",128ULL,{128ULL,128ULL},true);
            return value;
        }
        break;
    default:
        UNREACHABLE();
    }
}

symbol  make_symbol_of_interpreted_concatenation(uint64_t const  num_bits_left, uint64_t const  num_bits_right)
{
    return symbol_impl::create("+++",num_bits_left + num_bits_right,{num_bits_left,num_bits_right},true);
}


/**
 * Construction of symbols for interpreted comparisons
 */

symbol  make_symbol_of_interpreted_less_than_signed_int(uint64_t const  num_bits)
{
    switch (num_bits)
    {
    case 8ULL:
        {
            static symbol const  value = symbol_impl::create("<s8",1ULL,{8ULL,8ULL},true);
            return value;
        }
        break;
    case 16ULL:
        {
            static symbol const  value = symbol_impl::create("<s16",1ULL,{16ULL,16ULL},true);
            return value;
        }
        break;
    case 32ULL:
        {
            static symbol const  value = symbol_impl::create("<s32",1ULL,{32ULL,32ULL},true);
            return value;
        }
        break;
    case 64ULL:
        {
            static symbol const  value = symbol_impl::create("<s64",1ULL,{64ULL,64ULL},true);
            return value;
        }
        break;
    case 128ULL:
        {
            static symbol const  value = symbol_impl::create("<s128",1ULL,{128ULL,128ULL},true);
            return value;
        }
        break;
    default:
        UNREACHABLE();
    }
}

symbol  make_symbol_of_interpreted_less_than_unsigned_int(uint64_t const  num_bits)
{
    switch (num_bits)
    {
    case 8ULL:
        {
            static symbol const  value = symbol_impl::create("<u8",1ULL,{8ULL,8ULL},true);
            return value;
        }
        break;
    case 16ULL:
        {
            static symbol const  value = symbol_impl::create("<u16",1ULL,{16ULL,16ULL},true);
            return value;
        }
        break;
    case 32ULL:
        {
            static symbol const  value = symbol_impl::create("<u32",1ULL,{32ULL,32ULL},true);
            return value;
        }
        break;
    case 64ULL:
        {
            static symbol const  value = symbol_impl::create("<u64",1ULL,{64ULL,64ULL},true);
            return value;
        }
        break;
    case 128ULL:
        {
            static symbol const  value = symbol_impl::create("<u128",1ULL,{128ULL,128ULL},true);
            return value;
        }
        break;
    default:
        UNREACHABLE();
    }
}

symbol  make_symbol_of_interpreted_less_than_float(uint64_t const  num_bits)
{
    switch (num_bits)
    {
    case 32ULL:
        {
            static symbol const  value = symbol_impl::create("<f32",1ULL,{32ULL,32ULL},true);
            return value;
        }
        break;
    case 64ULL:
        {
            static symbol const  value = symbol_impl::create("<f64",1ULL,{64ULL,64ULL},true);
            return value;
        }
        break;
    case 80ULL:
        {
            static symbol const  value = symbol_impl::create("<f80",1ULL,{80ULL,80ULL},true);
            return value;
        }
        break;
    default:
        UNREACHABLE();
    }
}

symbol  make_symbol_of_interpreted_equal_int(uint64_t const  num_bits)
{
    switch (num_bits)
    {
    case 8ULL:
        {
            static symbol const  value = symbol_impl::create("=i8",1ULL,{8ULL,8ULL},true);
            return value;
        }
        break;
    case 16ULL:
        {
            static symbol const  value = symbol_impl::create("=i16",1ULL,{16ULL,16ULL},true);
            return value;
        }
        break;
    case 32ULL:
        {
            static symbol const  value = symbol_impl::create("=i32",1ULL,{32ULL,32ULL},true);
            return value;
        }
        break;
    case 64ULL:
        {
            static symbol const  value = symbol_impl::create("=i64",1ULL,{64ULL,64ULL},true);
            return value;
        }
        break;
    case 128ULL:
        {
            static symbol const  value = symbol_impl::create("=i128",1ULL,{128ULL,128ULL},true);
            return value;
        }
        break;
    default:
        UNREACHABLE();
    }
}

symbol  make_symbol_of_interpreted_equal_float(uint64_t const  num_bits)
{
    switch (num_bits)
    {
    case 32ULL:
        {
            static symbol const  value = symbol_impl::create("=f32",1ULL,{32ULL,32ULL},true);
            return value;
        }
        break;
    case 64ULL:
        {
            static symbol const  value = symbol_impl::create("=f64",1ULL,{64ULL,64ULL},true);
            return value;
        }
        break;
    case 80ULL:
        {
            static symbol const  value = symbol_impl::create("=f80",1ULL,{80ULL,80ULL},true);
            return value;
        }
        break;
    default:
        UNREACHABLE();
    }
}


}
