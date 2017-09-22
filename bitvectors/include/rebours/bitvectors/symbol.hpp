#ifndef REBOURS_BITVECTORS_SYMBOL_HPP_INCLUDED
#   define REBOURS_BITVECTORS_SYMBOL_HPP_INCLUDED

#   include <rebours/utility/large_types.hpp>
#   include <rebours/utility/endian.hpp>
#   include <string>
#   include <vector>
#   include <cstdint>
#   include <type_traits>

namespace bv {


/**
 * The implementation of a symbol is in the file src/symbol.cpp. It is not necessary for a user
 * of the library to ever directly access the implementation. Therefore, it is hidden.
 *
 * We also provide declarations of hash and equal functions in order to allow a proper use of
 * symbols in dictionaries.
 */
struct symbol_impl;
std::size_t  symbol_impl_hash(symbol_impl const&  s);
bool  symbol_impl_equal(symbol_impl const&  s1, symbol_impl const&  s2);


/**
 * We consider a symbol as a thin wrapper over a pointer to its implementation 'symbol_impl'.
 */
struct symbol
{
    symbol() : m_value{nullptr} {}
    explicit symbol(symbol_impl const*  s) : m_value{s} {}
    symbol_impl const*  operator->() const noexcept { return m_value; }
    operator bool () const { return m_value != nullptr; }
    struct hash { std::size_t  operator()(symbol const  s) const { return symbol_impl_hash(*s.operator ->()); } };
private:
    symbol_impl const*  m_value;
};

inline bool  operator ==(symbol const  s0, symbol const  s1)
{
    return s0.operator ->() == s1.operator ->() || symbol_impl_equal(*s0.operator ->(),*s1.operator ->());
}
inline bool  operator !=(symbol const  s0, symbol const  s1) { return !(s0 == s1); }


/**
 * Queries for properties of a symbol.
 */

std::string const&  symbol_name(symbol const  s);
uint64_t  symbol_num_bits_of_return_value(symbol const  s);
uint64_t  symbol_num_parameters(symbol const  s);
uint64_t  symbol_num_bits_of_parameter(symbol const  s, uint64_t const  param_index);
bool  symbol_is_interpreted(symbol const  s);

bool  symbol_is_interpreted_constant(symbol const  s);
bool  symbol_is_interpreted_cast(symbol const s);
bool  symbol_is_interpreted_operation(symbol const  s);

bool  symbol_is_interpreted_comparison(symbol const  s);
bool  symbol_is_interpreted_less_than(symbol const  s);
bool  symbol_is_interpreted_equal(symbol const  s);
bool  symbol_is_interpreted_equal_int(symbol const  s);
bool  symbol_is_interpreted_equal_float(symbol const  s);


/**
 * Construction of symbols for interpreted constants
 */

symbol  make_symbol_of_interpreted_constant(std::string const&  values_of_bytes_in_hexadecimal_format /** pass 2 hexadecimal lower-case digits per byte */);

symbol  make_symbol_of_interpreted_constant(uint8_t const*  begin, uint8_t const* const  end,
                                            bool const  revert_bytes_order = false);

template<typename T>
symbol  make_symbol_of_interpreted_constant(T const&  value, bool const  in_little_endian = is_this_little_endian_machine())
{
    return make_symbol_of_interpreted_constant(reinterpret_cast<uint8_t const*>(&value),
                                               reinterpret_cast<uint8_t const*>(&value) + sizeof(T),
                                               std::is_arithmetic<T>::value && in_little_endian == is_this_little_endian_machine());
}

inline symbol  make_symbol_of_interpreted_constant(uint128_t const&  value, bool const  in_little_endian = is_this_little_endian_machine())
{
    return make_symbol_of_interpreted_constant(value.data(),value.data() + value.size(),in_little_endian == is_this_little_endian_machine());
}


/**
 * Construction of symbols for uninterpreted constants and functions
 */

symbol  make_symbol_of_unintepreted_function(
                        std::string const&  name,
                        uint64_t const  num_bits_of_return_value,
                        std::vector<uint64_t> const&  num_bits_of_parameters = {}
                        );


/**
 * Construction of symbols for logical constants, conectives, and universal quantifier
 */

symbol  make_symbol_of_true();
symbol  make_symbol_of_false();

symbol  make_symbol_of_logical_conjunction();
symbol  make_symbol_of_logical_negation();

symbol  make_symbol_of_quantifier_forall(uint64_t const  num_var_bits);


/**
 * Construction of symbols for interpreted cast operations
 */

symbol  make_symbol_of_interpreted_truncate(uint64_t const  num_src_bits, uint64_t const  num_dst_bits);
symbol  make_symbol_of_interpreted_cast_signed_int(uint64_t const  num_src_bits, uint64_t const  num_dst_bits);
symbol  make_symbol_of_interpreted_cast_unsigned_int(uint64_t const  num_src_bits, uint64_t const  num_dst_bits);
symbol  make_symbol_of_interpreted_cast_float(uint64_t const  num_src_bits, uint64_t const  num_dst_bits);
symbol  make_symbol_of_interpreted_cast_signed_int_to_float(uint64_t const  num_bits);
symbol  make_symbol_of_interpreted_cast_unsigned_int_to_float(uint64_t const  num_bits);
symbol  make_symbol_of_interpreted_cast_float_to_signed_int(uint64_t const  num_bits);
symbol  make_symbol_of_interpreted_cast_float_to_unsigned_int(uint64_t const  num_bits);


/**
 * Construction of symbols for interpreted arithmetic operations
 */

symbol  make_symbol_of_interpreted_binary_add_int(uint64_t const  num_bits);
symbol  make_symbol_of_interpreted_binary_add_float(uint64_t const  num_bits);

symbol  make_symbol_of_interpreted_binary_subtract_int(uint64_t const  num_bits);
symbol  make_symbol_of_interpreted_binary_subtract_float(uint64_t const  num_bits);

symbol  make_symbol_of_interpreted_binary_multiply_int(uint64_t const  num_bits);
symbol  make_symbol_of_interpreted_binary_multiply_float(uint64_t const  num_bits);

symbol  make_symbol_of_interpreted_binary_divide_signed_int(uint64_t const  num_bits);
symbol  make_symbol_of_interpreted_binary_divide_unsigned_int(uint64_t const  num_bits);
symbol  make_symbol_of_interpreted_binary_divide_float(uint64_t const  num_bits);

symbol  make_symbol_of_interpreted_binary_remainder_signed_int(uint64_t const  num_bits);
symbol  make_symbol_of_interpreted_binary_remainder_unsigned_int(uint64_t const  num_bits);
symbol  make_symbol_of_interpreted_binary_remainder_float(uint64_t const  num_bits);

symbol  make_symbol_of_interpreted_binary_shift_left_int(uint64_t const  num_bits);

symbol  make_symbol_of_interpreted_binary_shift_right_signed_int(uint64_t const  num_bits);
symbol  make_symbol_of_interpreted_binary_shift_right_unsigned_int(uint64_t const  num_bits);

symbol  make_symbol_of_interpreted_binary_rotate_left_int(uint64_t const  num_bits);
symbol  make_symbol_of_interpreted_binary_rotate_right_int(uint64_t const  num_bits);

symbol  make_symbol_of_interpreted_binary_bitwise_and_int(uint64_t const  num_bits);
symbol  make_symbol_of_interpreted_binary_bitwise_or_int(uint64_t const  num_bits);
symbol  make_symbol_of_interpreted_binary_bitwise_xor_int(uint64_t const  num_bits);

symbol  make_symbol_of_interpreted_concatenation(uint64_t const  num_bits_left, uint64_t const  num_bits_right);


/**
 * Construction of symbols for interpreted comparisons
 */

symbol  make_symbol_of_interpreted_less_than_signed_int(uint64_t const  num_bits);
symbol  make_symbol_of_interpreted_less_than_unsigned_int(uint64_t const  num_bits);
symbol  make_symbol_of_interpreted_less_than_float(uint64_t const  num_bits);
symbol  make_symbol_of_interpreted_equal_int(uint64_t const  num_bits);
symbol  make_symbol_of_interpreted_equal_float(uint64_t const  num_bits);


}


#endif
