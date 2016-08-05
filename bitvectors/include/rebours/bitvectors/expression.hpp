#ifndef REBOURS_BITVECTORS_EXPRESSION_HPP_INCLUDED
#   define REBOURS_BITVECTORS_EXPRESSION_HPP_INCLUDED

#   include <rebours/bitvectors/symbol.hpp>
#   include <rebours/bitvectors/large_types.hpp>
#   include <rebours/bitvectors/assumptions.hpp>
#   include <rebours/bitvectors/invariants.hpp>
#   include <rebours/bitvectors/endian.hpp>
#   include <limits>
#   include <vector>
#   include <string>
#   include <memory>
#   include <cstdint>

namespace bv {


/**
 * The implementation of an expression is in the file src/expression.cpp. It is not necessary for a user
 * of the library to ever directly access the implementation. Therefore, it is hidden.
 */
struct expression_impl;
bool  expression_impl_equal(expression_impl const&  e0, expression_impl const&  e1);
std::size_t  expression_impl_hash(expression_impl const&  e);


/**
 * We consider an expression as a thin wrapper over a shared pointer to its implementation 'expression_impl'.
 */
struct expression
{
    expression() : m_value{} {}
    expression(symbol const  s, std::vector<expression> const&  args);
    explicit expression(std::shared_ptr<expression_impl> const  impl) : m_value{impl} {}

    std::shared_ptr<expression_impl>  operator->() const noexcept { return m_value; }
    explicit operator bool () const noexcept { return m_value.operator bool(); }

    struct hash { std::size_t  operator()(expression const  e) const { return expression_impl_hash(*e.operator ->()); } };

private:
    std::shared_ptr<expression_impl>  m_value;
};


/**
 * It checks whether two expressions are structuraly identical, i.e. they have exactly the same AST.
 */
inline bool  operator ==(expression const  e0, expression const  e1)
{
    return e0.operator ->() == e1.operator ->() || expression_impl_equal(*e0.operator ->(),*e1.operator ->());
}
inline bool  operator !=(expression const  e0, expression const  e1) { return !(e0 == e1); }


/**
 * It allows us to use C++ operators overloading for user-friendly construction of expressions.
 */
template<typename T>
struct typed_expression
{
    static_assert(std::is_arithmetic<T>::value || std::is_same<T,uint128_t>::value || std::is_same<T,float80_t>::value,
                  "Only basic numeric types are allowed for the template parameter T.");

    static constexpr bool  is_of_integral_type() noexcept { return std::is_integral<T>::value; }
    static constexpr bool  is_of_signed_integral_type() noexcept { return is_of_integral_type() && std::is_signed<T>::value; }
    static constexpr bool  is_of_unsigned_integral_type() noexcept { return is_of_integral_type() && !std::is_signed<T>::value; }
    static constexpr bool  is_of_floating_type() noexcept { return std::is_floating_point<T>::value; }

    typed_expression(symbol const  s, std::vector<expression> const&  args) : typed_expression(expression{s,args}) {}
    typed_expression(expression const  e) : m_value{e.operator ->()} {}
    explicit typed_expression(std::shared_ptr<expression_impl> const  impl) : m_value{impl} {}

    operator expression () const { return expression{m_value}; }
    explicit operator bool () const { return m_value.operator bool(); }

private:
    std::shared_ptr<expression_impl>  m_value;
};


/**
 * Queries for properties of an expression.
 */

symbol  get_symbol(expression const  e);
uint64_t  num_arguments(expression const  e); //!< equal to 'num_parameters', see bellow.
expression  argument(expression const  e, uint64_t const  index);

inline std::string const&  symbol_name(expression const  e) { return symbol_name(get_symbol(e)); }
inline uint64_t  num_bits_of_return_value(expression const  e) { return symbol_num_bits_of_return_value(get_symbol(e)); }
inline uint64_t  num_parameters(expression const  e) { return symbol_num_parameters(get_symbol(e)); }
inline uint64_t  num_bits_of_parameter(expression const  e, uint64_t const  param_index) { return symbol_num_bits_of_parameter(get_symbol(e),param_index); }

inline bool  is_interpreted(expression const  e) { return symbol_is_interpreted(get_symbol(e)); }

inline bool  is_formula(expression const  e) { return num_bits_of_return_value(e) == 1ULL; }
inline bool  is_term(expression const  e) { return !is_formula(e); }

inline bool  is_tt(expression const  e) { return get_symbol(e) == make_symbol_of_true(); }
inline bool  is_ff(expression const  e) { return get_symbol(e) == make_symbol_of_false(); }
inline bool  is_conjunction(expression const  e) { return get_symbol(e) == make_symbol_of_logical_conjunction(); }
inline bool  is_negation(expression const  e) { return get_symbol(e) == make_symbol_of_logical_negation(); }
inline bool  is_forall(expression const  e) { return num_arguments(e) == 2ULL && get_symbol(e) == make_symbol_of_quantifier_forall(num_bits_of_return_value(argument(e,0ULL))); }

inline bool  is_interpreted_constant(expression const  e) { return symbol_is_interpreted_constant(get_symbol(e)); }
bool  is_number(expression const  e);

inline bool  is_ufun(expression const  e) { return !is_interpreted(e); }
inline bool  is_var(expression const  e) { return is_ufun(e) && num_parameters(e) == 0ULL; }

inline bool  is_cast(expression const  e) { return symbol_is_interpreted_cast(get_symbol(e)); }

inline bool  is_operation(expression const  e) { return symbol_is_interpreted_operation(get_symbol(e)); }

inline bool  is_comparison(expression const  e) { return symbol_is_interpreted_comparison(get_symbol(e)); }
inline bool  is_less_than(expression const  e) { return symbol_is_interpreted_less_than(get_symbol(e)); }
inline bool  is_equality(expression const  e) { return symbol_is_interpreted_equal(get_symbol(e)); }


/**
 * Construction of an expression by applying a symbol to arguments
 */


inline expression  apply(symbol const  s,  std::vector<expression> const&  args) { return {s,args}; }

template<typename T>
inline typed_expression<T>  apply(symbol const  s,  std::vector<expression> const&  args) { return {s,args}; }


/**
 * Construction of interpreted constants
 */

inline expression  mem(std::string const&  values_of_bytes_in_hexadecimal_format /** pass 2 hexadecimal lower-case digits per byte */)
{ return {make_symbol_of_interpreted_constant(values_of_bytes_in_hexadecimal_format),{}}; }

template<typename T>
inline expression  mem(T const* const  begin, T const* const  end, bool const  revert_bytes_order = false)
{ return {make_symbol_of_interpreted_constant(reinterpret_cast<uint8_t const*>(begin),reinterpret_cast<uint8_t const*>(end),revert_bytes_order),{}}; }

template<typename T>
inline typed_expression<T>  num(T const&  value, bool const  in_little_endian = is_this_little_endian_machine())
{
    static_assert(std::is_arithmetic<T>::value,"The passed type T is a non-numeric type.");
    return {make_symbol_of_interpreted_constant<T>(value,in_little_endian),{}};
}

inline typed_expression<uint128_t>  num(uint128_t const&  value, bool const  in_little_endian = is_this_little_endian_machine())
{
    return {make_symbol_of_interpreted_constant<uint128_t>(value,in_little_endian),{}};
}

inline typed_expression<float80_t>  num(float80_t const&  value, bool const  in_little_endian = is_this_little_endian_machine())
{
    return {make_symbol_of_interpreted_constant<float80_t>(value,in_little_endian),{}};
}


/**
 * Construction of uninterpreted constants and functions
 */

expression  ufun(std::string const&  var_name, uint64_t const  num_bits_of_return_value, std::vector<expression> const&  args);
inline expression  var(std::string const&  var_name, uint64_t const  num_bits_of_return_value) { return ufun(var_name,num_bits_of_return_value,{}); }

template<typename T>
inline typed_expression<T>  ufun(std::string const&  var_name, std::vector<expression> const&  args) { return {ufun(var_name,sizeof(T)*8ULL,args)}; }

template<typename T>
inline typed_expression<T>  var(std::string const&  var_name) { return ufun<T>(var_name,{}); }


/**
 * Construction of logical constants, conectives, and the universal quantifier
 */

expression  tt(); //!< Logical value 'true'
expression  ff(); //!< Logical value 'false'
expression  operator &&(expression const  arg0, expression const  arg1);
expression  operator !(expression const  arg0);
expression  forall(expression const  var, expression const  body);

inline expression  operator ||(expression const  arg0, expression const  arg1) { return !(!arg0 && !arg1); }
inline expression  implies(expression const  antecedent, expression const  consequent) { return !antecedent || consequent; }
inline expression  equivalence(expression const  arg0, expression const  arg1) { return (arg0 && arg1) || (!arg0 && !arg1); }
inline expression  exists(expression const  var, expression const  body) { return !forall(var,!body);}


/**
 * Construction of cast operations
 */

expression  make_cast_signed_int(expression const  arg0, uint64_t const  num_dst_bits);
expression  make_cast_unsigned_int(expression const  arg0, uint64_t const  num_dst_bits);
expression  make_cast_float(expression const  arg0, uint64_t const  num_dst_bits);
expression  make_cast_signed_int_to_float(expression const  arg0, uint64_t const  num_bits);
expression  make_cast_unsigned_int_to_float(expression const  arg0, uint64_t const  num_bits);
expression  make_cast_float_to_signed_int(expression const  arg0, uint64_t const  num_bits);
expression  make_cast_float_to_unsigned_int(expression const  arg0, uint64_t const  num_bits);

template<typename T, typename S>
typed_expression<T>  cast(typed_expression<S> const  arg0)
{
    static_assert(std::is_arithmetic<S>::value || std::is_same<S,uint128_t>::value || std::is_same<S,float80_t>::value,
                  "The cast operator was applied to a non-numeric expression type.");
    if (arg0.is_of_integral_type())
        if (typed_expression<T>::is_of_integral_type())
            switch ((sizeof(T)*8ULL) + ((sizeof(S)*8ULL) << 8ULL))
            {
            case 0x0810ULL: // 8 -> 16
                return { arg0.is_of_signed_integral_type() ?
                                make_cast_signed_int(arg0,16ULL) :
                                make_cast_unsigned_int(arg0,16ULL) };
            case 0x0820ULL: // 8 -> 32
                return { arg0.is_of_signed_integral_type() ?
                                make_cast_signed_int(make_cast_signed_int(arg0,16ULL),32ULL) :
                                make_cast_unsigned_int(make_cast_unsigned_int(arg0,16ULL),32ULL) };
            case 0x0840ULL: // 8 -> 64
                return { arg0.is_of_signed_integral_type() ?
                                make_cast_signed_int(make_cast_signed_int(make_cast_signed_int(arg0,16ULL),32ULL),64ULL) :
                                make_cast_unsigned_int(make_cast_unsigned_int(make_cast_unsigned_int(arg0,16ULL),32ULL),64ULL) };
            case 0x0880ULL: // 8 -> 128
                return { arg0.is_of_signed_integral_type() ?
                                make_cast_signed_int(make_cast_signed_int(make_cast_signed_int(make_cast_signed_int(arg0,16ULL),32ULL),64ULL),128ULL) :
                                make_cast_unsigned_int(make_cast_unsigned_int(make_cast_unsigned_int(make_cast_unsigned_int(arg0,16ULL),32ULL),64ULL),128ULL) };
            case 0x1020ULL: // 16 -> 32
                return { arg0.is_of_signed_integral_type() ?
                                make_cast_signed_int(arg0,32ULL) :
                                make_cast_unsigned_int(arg0,32ULL) };
            case 0x1040ULL: // 16 -> 64
                return { arg0.is_of_signed_integral_type() ?
                                make_cast_signed_int(make_cast_signed_int(arg0,32ULL),64ULL) :
                                make_cast_unsigned_int(make_cast_unsigned_int(arg0,32ULL),64ULL) };
            case 0x1080ULL: // 16 -> 128
                return { arg0.is_of_signed_integral_type() ?
                                make_cast_signed_int(make_cast_signed_int(make_cast_signed_int(arg0,32ULL),64ULL),128ULL) :
                                make_cast_unsigned_int(make_cast_unsigned_int(make_cast_unsigned_int(arg0,32ULL),64ULL),128ULL) };
            case 0x2040ULL: // 32 -> 64
                return { arg0.is_of_signed_integral_type() ?
                                make_cast_signed_int(arg0,64ULL) :
                                make_cast_unsigned_int(arg0,64ULL) };
            case 0x2080ULL: // 32 -> 128
                return { arg0.is_of_signed_integral_type() ?
                                make_cast_signed_int(make_cast_signed_int(arg0,64ULL),128ULL) :
                                make_cast_unsigned_int(make_cast_unsigned_int(arg0,64ULL),128ULL) };
            case 0x4080ULL: // 64 -> 128
                return { arg0.is_of_signed_integral_type() ?
                                make_cast_signed_int(arg0,128ULL) :
                                make_cast_unsigned_int(arg0,128ULL) };
            case 0x8040ULL: // 128 -> 64
                return { arg0.is_of_signed_integral_type() ?
                                make_cast_signed_int(arg0,64ULL) :
                                make_cast_unsigned_int(arg0,64ULL) };
            case 0x8020ULL: // 128 -> 32
                return { arg0.is_of_signed_integral_type() ?
                                make_cast_signed_int(make_cast_signed_int(arg0,64ULL),32ULL) :
                                make_cast_unsigned_int(make_cast_unsigned_int(arg0,64ULL),32ULL) };
            case 0x8010ULL: // 128 -> 16
                return { arg0.is_of_signed_integral_type() ?
                                make_cast_signed_int(make_cast_signed_int(make_cast_signed_int(arg0,64ULL),32ULL),16ULL) :
                                make_cast_unsigned_int(make_cast_unsigned_int(make_cast_unsigned_int(arg0,64ULL),32ULL),16ULL) };
            case 0x8008ULL: // 128 -> 8
                return { arg0.is_of_signed_integral_type() ?
                                make_cast_signed_int(make_cast_signed_int(make_cast_signed_int(make_cast_signed_int(arg0,64ULL),32ULL),16ULL),8ULL) :
                                make_cast_unsigned_int(make_cast_unsigned_int(make_cast_unsigned_int(make_cast_unsigned_int(arg0,64ULL),32ULL),16ULL),8ULL) };
            case 0x4020ULL: // 64 -> 32
                return { arg0.is_of_signed_integral_type() ?
                                make_cast_signed_int(arg0,32ULL) :
                                make_cast_unsigned_int(arg0,32ULL) };
            case 0x4010ULL: // 64 -> 16
                return { arg0.is_of_signed_integral_type() ?
                                make_cast_signed_int(make_cast_signed_int(arg0,32ULL),16ULL) :
                                make_cast_unsigned_int(make_cast_unsigned_int(arg0,32ULL),16ULL) };
            case 0x4008ULL: // 64 -> 8
                return { arg0.is_of_signed_integral_type() ?
                                make_cast_signed_int(make_cast_signed_int(make_cast_signed_int(arg0,32ULL),16ULL),8ULL) :
                                make_cast_unsigned_int(make_cast_unsigned_int(make_cast_unsigned_int(arg0,32ULL),16ULL),8ULL) };
            case 0x2010ULL: // 32 -> 16
                return { arg0.is_of_signed_integral_type() ?
                                make_cast_signed_int(arg0,16ULL) :
                                make_cast_unsigned_int(arg0,16ULL) };
            case 0x2008ULL: // 32 -> 8
                return { arg0.is_of_signed_integral_type() ?
                                make_cast_signed_int(make_cast_signed_int(arg0,16ULL),8ULL) :
                                make_cast_unsigned_int(make_cast_unsigned_int(arg0,16ULL),8ULL) };
            case 0x1008ULL: // 16 -> 8
                return { arg0.is_of_signed_integral_type() ?
                                make_cast_signed_int(arg0,8ULL) :
                                make_cast_unsigned_int(arg0,8ULL) };
            case 0x0808ULL: // 8 -> 8
            case 0x1010ULL: // 16 -> 16
            case 0x2020ULL: // 32 -> 32
            case 0x4040ULL: // 64 -> 64
            case 0x8080ULL: // 128 -> 128
                return {arg0};
            default:
                UNREACHABLE();
            }
        else
            switch ((sizeof(T)*8ULL) + ((sizeof(S)*8ULL) << 8ULL))
            {
            case 0x0820ULL: // 8 -> 32
                return { arg0.is_of_signed_integral_type() ?
                                make_cast_signed_int_to_float(make_cast_signed_int(make_cast_signed_int(arg0,16ULL),32ULL),32ULL) :
                                make_cast_unsigned_int_to_float(make_cast_unsigned_int(make_cast_unsigned_int(arg0,16ULL),32ULL),32ULL) };
            case 0x0840ULL: // 8 -> 64
                return { arg0.is_of_signed_integral_type() ?
                                make_cast_signed_int_to_float(make_cast_signed_int(make_cast_signed_int(make_cast_signed_int(arg0,16ULL),32ULL),64ULL),64ULL) :
                                make_cast_unsigned_int_to_float(make_cast_unsigned_int(make_cast_unsigned_int(make_cast_unsigned_int(arg0,16ULL),32ULL),64ULL),64ULL) };
            case 0x0850ULL: // 8 -> 80
                return { arg0.is_of_signed_integral_type() ?
                                make_cast_float(make_cast_signed_int_to_float(make_cast_signed_int(make_cast_signed_int(make_cast_signed_int(arg0,16ULL),32ULL),64ULL),64ULL),80ULL) :
                                make_cast_float(make_cast_unsigned_int_to_float(make_cast_unsigned_int(make_cast_unsigned_int(make_cast_unsigned_int(arg0,16ULL),32ULL),64ULL),64ULL),80ULL) };
            case 0x1020ULL: // 16 -> 32
                return { arg0.is_of_signed_integral_type() ?
                                make_cast_signed_int_to_float(make_cast_signed_int(arg0,32ULL),32ULL) :
                                make_cast_unsigned_int_to_float(make_cast_unsigned_int(arg0,32ULL),32ULL) };
            case 0x1040ULL: // 16 -> 64
                return { arg0.is_of_signed_integral_type() ?
                                make_cast_signed_int_to_float(make_cast_signed_int(make_cast_signed_int(arg0,32ULL),64ULL),64ULL) :
                                make_cast_unsigned_int_to_float(make_cast_unsigned_int(make_cast_unsigned_int(arg0,32ULL),64ULL),64ULL) };
            case 0x1050ULL: // 16 -> 80
                return { arg0.is_of_signed_integral_type() ?
                                make_cast_float(make_cast_signed_int_to_float(make_cast_signed_int(make_cast_signed_int(arg0,32ULL),64ULL),64ULL),80ULL) :
                                make_cast_float(make_cast_unsigned_int_to_float(make_cast_unsigned_int(make_cast_unsigned_int(arg0,32ULL),64ULL),64ULL),80ULL) };
            case 0x2020ULL: // 32 -> 32
                return { arg0.is_of_signed_integral_type() ?
                                make_cast_signed_int_to_float(arg0,32ULL) :
                                make_cast_unsigned_int_to_float(arg0,32ULL) };
            case 0x2040ULL: // 32 -> 64
                return { arg0.is_of_signed_integral_type() ?
                                make_cast_signed_int_to_float(make_cast_signed_int(arg0,64ULL),64ULL) :
                                make_cast_unsigned_int_to_float(make_cast_unsigned_int(arg0,64ULL),64ULL) };
            case 0x2050ULL: // 32 -> 80
                return { arg0.is_of_signed_integral_type() ?
                                make_cast_float(make_cast_signed_int_to_float(make_cast_signed_int(arg0,64ULL),64ULL),80ULL) :
                                make_cast_float(make_cast_unsigned_int_to_float(make_cast_unsigned_int(arg0,64ULL),64ULL),80ULL) };
            case 0x4020ULL: // 64 -> 32
                return { arg0.is_of_signed_integral_type() ?
                                make_cast_signed_int_to_float(make_cast_signed_int(arg0,32ULL),32ULL) :
                                make_cast_unsigned_int_to_float(make_cast_unsigned_int(arg0,32ULL),32ULL) };
            case 0x4040ULL: // 64 -> 64
                return { arg0.is_of_signed_integral_type() ?
                                make_cast_signed_int_to_float(arg0,64ULL) :
                                make_cast_unsigned_int_to_float(arg0,64ULL) };
            case 0x4050ULL: // 64 -> 80
                return { arg0.is_of_signed_integral_type() ?
                                make_cast_float(make_cast_signed_int_to_float(arg0,64ULL),80ULL) :
                                make_cast_float(make_cast_unsigned_int_to_float(arg0,64ULL),80ULL) };
            case 0x8020ULL: // 128 -> 32
                return { arg0.is_of_signed_integral_type() ?
                                make_cast_signed_int_to_float(make_cast_signed_int(make_cast_signed_int(arg0,64ULL),32ULL),32ULL) :
                                make_cast_unsigned_int_to_float(make_cast_unsigned_int(make_cast_unsigned_int(arg0,64ULL),32ULL),32ULL) };
            case 0x8040ULL: // 128 -> 64
                return { arg0.is_of_signed_integral_type() ?
                                make_cast_signed_int_to_float(make_cast_signed_int(arg0,64ULL),64ULL) :
                                make_cast_unsigned_int_to_float(make_cast_unsigned_int(arg0,64ULL),64ULL) };
            case 0x8050ULL: // 128 -> 80
                return { arg0.is_of_signed_integral_type() ?
                                make_cast_float(make_cast_signed_int_to_float(make_cast_signed_int(arg0,64ULL),64ULL),80ULL) :
                                make_cast_float(make_cast_unsigned_int_to_float(make_cast_unsigned_int(arg0,64ULL),64ULL),80ULL) };
            default:
                UNREACHABLE();
            }
    else
        if (typed_expression<T>::is_of_integral_type())
            switch ((sizeof(T)*8ULL) + ((sizeof(S)*8ULL) << 8ULL))
            {
            case 0x2008ULL: // 32 -> 8
                return { typed_expression<T>::is_of_signed_integral_type() ?
                                make_cast_signed_int(make_cast_signed_int(make_cast_float_to_signed_int(arg0,32ULL),16ULL),8ULL) :
                                make_cast_unsigned_int(make_cast_unsigned_int(make_cast_float_to_unsigned_int(arg0,32ULL),16ULL),8ULL) };
            case 0x2010ULL: // 32 -> 16
                return { typed_expression<T>::is_of_signed_integral_type() ?
                                make_cast_signed_int(make_cast_float_to_signed_int(arg0,32ULL),16ULL) :
                                make_cast_unsigned_int(make_cast_float_to_unsigned_int(arg0,32ULL),16ULL) };
            case 0x2020ULL: // 32 -> 32
                return { typed_expression<T>::is_of_signed_integral_type() ?
                                make_cast_float_to_signed_int(arg0,32ULL) :
                                make_cast_float_to_unsigned_int(arg0,32ULL) };
            case 0x2040ULL: // 32 -> 64
                return { typed_expression<T>::is_of_signed_integral_type() ?
                                make_cast_signed_int(make_cast_float_to_signed_int(arg0,32ULL),64ULL) :
                                make_cast_unsigned_int(make_cast_float_to_unsigned_int(arg0,32ULL),64ULL) };
            case 0x2080ULL: // 32 -> 128
                return { typed_expression<T>::is_of_signed_integral_type() ?
                                make_cast_signed_int(make_cast_signed_int(make_cast_float_to_signed_int(arg0,32ULL),64ULL),128ULL) :
                                make_cast_unsigned_int(make_cast_unsigned_int(make_cast_float_to_unsigned_int(arg0,32ULL),64ULL),128ULL) };
            case 0x4008ULL: // 64 -> 8
                return { typed_expression<T>::is_of_signed_integral_type() ?
                                make_cast_signed_int(make_cast_signed_int(make_cast_signed_int(make_cast_float_to_signed_int(arg0,64ULL),32ULL),16ULL),8ULL) :
                                make_cast_unsigned_int(make_cast_unsigned_int(make_cast_unsigned_int(make_cast_float_to_unsigned_int(arg0,64ULL),32ULL),16ULL),8ULL) };
            case 0x4010ULL: // 64 -> 16
                return { typed_expression<T>::is_of_signed_integral_type() ?
                                make_cast_signed_int(make_cast_signed_int(make_cast_float_to_signed_int(arg0,64ULL),32ULL),16ULL) :
                                make_cast_unsigned_int(make_cast_unsigned_int(make_cast_float_to_unsigned_int(arg0,64ULL),32ULL),16ULL) };
            case 0x4020ULL: // 64 -> 32
                return { typed_expression<T>::is_of_signed_integral_type() ?
                                make_cast_signed_int(make_cast_float_to_signed_int(arg0,64ULL),32ULL) :
                                make_cast_unsigned_int(make_cast_float_to_unsigned_int(arg0,64ULL),32ULL) };
            case 0x4040ULL: // 64 -> 64
                return { typed_expression<T>::is_of_signed_integral_type() ?
                                make_cast_float_to_signed_int(arg0,64ULL) :
                                make_cast_float_to_unsigned_int(arg0,64ULL) };
            case 0x4080ULL: // 64 -> 128
                return { typed_expression<T>::is_of_signed_integral_type() ?
                                make_cast_signed_int(make_cast_float_to_signed_int(arg0,64ULL),128ULL) :
                                make_cast_unsigned_int(make_cast_float_to_unsigned_int(arg0,64ULL),128ULL) };
            case 0x5008ULL: // 80 -> 8
                return { typed_expression<T>::is_of_signed_integral_type() ?
                                make_cast_signed_int(make_cast_signed_int(make_cast_signed_int(make_cast_float_to_signed_int(make_cast_float(arg0,64ULL),64ULL),32ULL),16ULL),8ULL) :
                                make_cast_unsigned_int(make_cast_unsigned_int(make_cast_unsigned_int(make_cast_float_to_unsigned_int(make_cast_float(arg0,64ULL),64ULL),32ULL),16ULL),8ULL) };
            case 0x5010ULL: // 80 -> 16
                return { typed_expression<T>::is_of_signed_integral_type() ?
                                make_cast_signed_int(make_cast_signed_int(make_cast_float_to_signed_int(make_cast_float(arg0,64ULL),64ULL),32ULL),16ULL) :
                                make_cast_unsigned_int(make_cast_unsigned_int(make_cast_float_to_unsigned_int(make_cast_float(arg0,64ULL),64ULL),32ULL),16ULL) };
            case 0x5020ULL: // 80 -> 32
                return { typed_expression<T>::is_of_signed_integral_type() ?
                                make_cast_signed_int(make_cast_float_to_signed_int(make_cast_float(arg0,64ULL),64ULL),32ULL) :
                                make_cast_unsigned_int(make_cast_float_to_unsigned_int(make_cast_float(arg0,64ULL),64ULL),32ULL) };
            case 0x5064ULL: // 80 -> 64
                return { typed_expression<T>::is_of_signed_integral_type() ?
                                make_cast_float_to_signed_int(make_cast_float(arg0,64ULL),64ULL) :
                                make_cast_float_to_unsigned_int(make_cast_float(arg0,64ULL),64ULL) };
            case 0x5080ULL: // 80 -> 128
                return { typed_expression<T>::is_of_signed_integral_type() ?
                                make_cast_signed_int(make_cast_float_to_signed_int(make_cast_float(arg0,64ULL),64ULL),128ULL) :
                                make_cast_unsigned_int(make_cast_float_to_unsigned_int(make_cast_float(arg0,64ULL),64ULL),128ULL) };
            default:
                UNREACHABLE();
            }
        else
        {
            uint64_t const  num_tgt_bits = std::is_same<T,float80_t>::value ? 80ULL : sizeof(T)*8ULL;
            if (num_bits_of_return_value(arg0) == num_tgt_bits)
                return {arg0};
            else
                return make_cast_float(arg0,num_tgt_bits);
        }
}


template<typename base_type, bool integral_type, bool produce_signed_type>
struct to_coerced_type
{
    using  type = typename std::conditional<produce_signed_type,typename std::make_signed<base_type>::type,base_type>::type;
};

template<typename base_type, bool produce_signed_type>
struct to_coerced_type<base_type,false,produce_signed_type>
{
    using  type = base_type;
};

template<typename S, typename T>
struct coerce
{
    static_assert((std::is_arithmetic<S>::value || std::is_same<S,uint128_t>::value || std::is_same<S,float80_t>::value) &&
                  (std::is_arithmetic<T>::value || std::is_same<T,uint128_t>::value || std::is_same<T,float80_t>::value),
                  "The operator was applied to a non-numeric expression type.");
    using  base_type = typename std::conditional<sizeof(S) >= sizeof(T),S,T>::type;
    using  type = typename to_coerced_type<base_type,std::is_integral<base_type>::value,std::is_signed<S>::value || std::is_signed<T>::value>::type;
};


/**
 * Construction of arithmetic operations
 */

expression  make_addition_int(expression const  arg0, expression const  arg1);
expression  make_addition_float(expression const  arg0, expression const  arg1);

template<typename L, typename R>
inline typed_expression<typename coerce<L,R>::type>  operator +(typed_expression<L> const  arg0, typed_expression<R> const  arg1)
{
    using  rettype = typename coerce<L,R>::type;
    return {arg0.is_of_integral_type() ? make_addition_int(cast<rettype>(arg0),cast<rettype>(arg1)) :
                                         make_addition_float(cast<rettype>(arg0),cast<rettype>(arg1))};
}

template<typename T>
inline typed_expression<T>  operator +(typed_expression<T> const  arg0)
{
    static_assert(std::is_arithmetic<T>::value || std::is_same<T,uint128_t>::value || std::is_same<T,float80_t>::value,
                  "The operator was applied to a non-numeric expression type.");
    return arg0;
}

expression  make_subtraction_int(expression const  arg0, expression const  arg1);
expression  make_subtraction_float(expression const  arg0, expression const  arg1);

template<typename L, typename R>
inline typed_expression<typename coerce<L,R>::type>  operator -(typed_expression<L> const  arg0, typed_expression<R> const  arg1)
{
    using  rettype = typename coerce<L,R>::type;
    return {arg0.is_of_integral_type() ? make_subtraction_int(cast<rettype>(arg0),cast<rettype>(arg1)) :
                                         make_subtraction_float(cast<rettype>(arg0),cast<rettype>(arg1))};
}

template<typename T>
inline typed_expression<T>  operator -(typed_expression<T> const  arg0)
{
    static_assert(std::is_arithmetic<T>::value || std::is_same<T,uint128_t>::value || std::is_same<T,float80_t>::value,
                  "The operator was applied to a non-numeric expression type.");
    return {arg0.is_of_integral_type() ? make_subtraction_int(num<T>(0),arg0) : make_subtraction_float(num<T>(0.0),arg0)};
}

expression  make_multiply_int(expression const  arg0, expression const  arg1);
expression  make_multiply_float(expression const  arg0, expression const  arg1);

template<typename L, typename R>
inline typed_expression<typename coerce<L,R>::type>  operator *(typed_expression<L> const  arg0, typed_expression<R> const  arg1)
{
    using  rettype = typename coerce<L,R>::type;
    return {arg0.is_of_integral_type() ? make_multiply_int(cast<rettype>(arg0),cast<rettype>(arg1)) :
                                         make_multiply_float(cast<rettype>(arg0),cast<rettype>(arg1))};
}

expression  make_divide_signed_int(expression const  arg0, expression const  arg1);
expression  make_divide_unsigned_int(expression const  arg0, expression const  arg1);
expression  make_divide_float(expression const  arg0, expression const  arg1);

template<typename L, typename R>
inline typed_expression<typename coerce<L,R>::type>  operator /(typed_expression<L> const  arg0, typed_expression<R> const  arg1)
{
    using  rettype = typename coerce<L,R>::type;
    return {std::is_floating_point<rettype>::value ? make_divide_float(cast<rettype>(arg0),cast<rettype>(arg1)) :
            std::is_signed<rettype>::value ? make_divide_signed_int(cast<rettype>(arg0),cast<rettype>(arg1)) :
                                             make_divide_unsigned_int(cast<rettype>(arg0),cast<rettype>(arg1)) };
}

expression  make_remainder_signed_int(expression const  arg0, expression const  arg1);
expression  make_remainder_unsigned_int(expression const  arg0, expression const  arg1);
expression  make_remainder_float(expression const  arg0, expression const  arg1);

template<typename L, typename R>
inline typed_expression<typename coerce<L,R>::type>  operator %(typed_expression<L> const  arg0, typed_expression<R> const  arg1)
{
    using  rettype = typename coerce<L,R>::type;
    return {std::is_floating_point<rettype>::value ? make_remainder_float(cast<rettype>(arg0),cast<rettype>(arg1)) :
            std::is_signed<rettype>::value ? make_remainder_signed_int(cast<rettype>(arg0),cast<rettype>(arg1)) :
                                             make_remainder_unsigned_int(cast<rettype>(arg0),cast<rettype>(arg1)) };
}

expression  make_shift_left_int(expression const  arg0, expression const  arg1);

template<typename T, typename S>
inline typed_expression<T>  operator <<(typed_expression<T> const  arg0, typed_expression<S> const  arg1)
{
    static_assert(std::is_integral<T>::value || std::is_same<T,uint128_t>::value,"The operator was applied to a non-integer expression type.");
    static_assert(std::is_integral<S>::value || std::is_same<S,uint128_t>::value,"The operator was applied to a non-integer expression type.");
    return {make_shift_left_int(arg0,cast<typename std::make_unsigned<T>::type>(arg1))};
}

expression  make_shift_right_signed_int(expression const  arg0, expression const  arg1);
expression  make_shift_right_unsigned_int(expression const  arg0, expression const  arg1);

template<typename T, typename S>
inline typed_expression<T>  operator >>(typed_expression<T> const  arg0, typed_expression<S> const  arg1)
{
    static_assert(std::is_integral<T>::value || std::is_same<T,uint128_t>::value,"The operator was applied to a non-integer expression type.");
    static_assert(std::is_integral<S>::value || std::is_same<S,uint128_t>::value,"The operator was applied to a non-integer expression type.");
    return {arg0.is_of_signed_integral_type() ? make_shift_right_signed_int(arg0,cast<typename std::make_unsigned<T>::type>(arg1)) :
                                                make_shift_right_unsigned_int(arg0,cast<typename std::make_unsigned<T>::type>(arg1)) };
}

expression  make_rotation_left(expression const  arg0, expression const  arg1);

template<typename T, typename S>
inline typed_expression<T>  rotate_left(typed_expression<T> const  arg0, typed_expression<S> const  arg1)
{
    static_assert(std::is_integral<T>::value || std::is_same<T,uint128_t>::value,"The operator was applied to a non-integer expression type.");
    static_assert(std::is_integral<S>::value || std::is_same<S,uint128_t>::value,"The operator was applied to a non-integer expression type.");
    return { make_rotation_left(arg0,cast<typename std::make_unsigned<T>::type>(arg1)) };
}

expression  make_rotation_right(expression const  arg0, expression const  arg1);

template<typename T, typename S>
inline typed_expression<T>  rotate_right(typed_expression<T> const  arg0, typed_expression<S> const  arg1)
{
    static_assert(std::is_integral<T>::value || std::is_same<T,uint128_t>::value,"The operator was applied to a non-integer expression type.");
    static_assert(std::is_integral<S>::value || std::is_same<S,uint128_t>::value,"The operator was applied to a non-integer expression type.");
    return { make_rotation_right(arg0,cast<typename std::make_unsigned<T>::type>(arg1)) };
}

expression  make_bitwise_and_int(expression const  arg0, expression const  arg1);

template<typename L, typename R>
inline typed_expression<typename coerce<L,R>::type>  operator &(typed_expression<L> const  arg0, typed_expression<R> const  arg1)
{
    static_assert(std::is_integral<L>::value || std::is_same<L,uint128_t>::value,"The operator was applied to a non-integer expression type.");
    static_assert(std::is_integral<R>::value || std::is_same<R,uint128_t>::value,"The operator was applied to a non-integer expression type.");
    using  rettype = typename coerce<L,R>::type;
    return {make_bitwise_and_int(cast<rettype>(arg0),cast<rettype>(arg1))};
}

expression  make_bitwise_or_int(expression const  arg0, expression const  arg1);

template<typename L, typename R>
inline typed_expression<typename coerce<L,R>::type>  operator |(typed_expression<L> const  arg0, typed_expression<R> const  arg1)
{
    static_assert(std::is_integral<L>::value || std::is_same<L,uint128_t>::value,"The operator was applied to a non-integer expression type.");
    static_assert(std::is_integral<R>::value || std::is_same<R,uint128_t>::value,"The operator was applied to a non-integer expression type.");
    using  rettype = typename coerce<L,R>::type;
    return {make_bitwise_or_int(cast<rettype>(arg0),cast<rettype>(arg1))};
}

expression  make_bitwise_xor_int(expression const  arg0, expression const  arg1);

template<typename L, typename R>
inline typed_expression<typename coerce<L,R>::type>  operator ^(typed_expression<L> const  arg0, typed_expression<R> const  arg1)
{
    static_assert(std::is_integral<L>::value || std::is_same<L,uint128_t>::value,"The operator was applied to a non-integer expression type.");
    static_assert(std::is_integral<R>::value || std::is_same<R,uint128_t>::value,"The operator was applied to a non-integer expression type.");
    using  rettype = typename coerce<L,R>::type;
    return {make_bitwise_xor_int(cast<rettype>(arg0),cast<rettype>(arg1))};
}

template<typename T>
inline typed_expression<T>  operator ~(typed_expression<T> const  arg0)
{
    static_assert(std::is_integral<T>::value || std::is_same<T,uint128_t>::value,"The operator was applied to a non-integer expression type.");
    return {make_bitwise_xor_int(arg0,num<typename std::make_unsigned<T>::type>(std::numeric_limits<typename std::make_unsigned<T>::type>::max()))};
}

expression  make_concatenation(expression const  arg0, expression const  arg1);


/**
 * Construction of comparisons
 */

expression  make_less_than_signed_int(expression const  arg0, expression const  arg1);
expression  make_less_than_unsigned_int(expression const  arg0, expression const  arg1);
expression  make_less_than_float(expression const  arg0, expression const  arg1);

template<typename L, typename R>
inline expression  operator <(typed_expression<L> const  arg0, typed_expression<R> const  arg1)
{
    static_assert((std::is_arithmetic<L>::value || std::is_same<L,uint128_t>::value || std::is_same<L,float80_t>::value) &&
                  (std::is_arithmetic<R>::value || std::is_same<R,uint128_t>::value || std::is_same<R,float80_t>::value),
                  "The operator was applied to a non-numeric expression type.");
    static_assert(!arg0.is_of_integral_type() || arg0.is_of_signed_integral_type() == arg1.is_of_signed_integral_type(),
                  "The operator was passed the mixed signed/unsigned expression types as arguments.");
    using  rettype = typename coerce<L,R>::type;
    return { std::is_floating_point<rettype>::value ? make_less_than_float(cast<rettype>(arg0),cast<rettype>(arg1)) :
             std::is_signed<rettype>::value ?  make_less_than_signed_int(cast<rettype>(arg0),cast<rettype>(arg1)) :
                                               make_less_than_unsigned_int(cast<rettype>(arg0),cast<rettype>(arg1)) };
}

expression  make_equal_int(expression const  arg0, expression const  arg1);
expression  make_equal_float(expression const  arg0, expression const  arg1);

template<typename L, typename R>
inline expression  operator ==(typed_expression<L> const  arg0, typed_expression<R> const  arg1)
{
    static_assert((std::is_arithmetic<L>::value || std::is_same<L,uint128_t>::value || std::is_same<L,float80_t>::value) &&
                  (std::is_arithmetic<R>::value || std::is_same<R,uint128_t>::value || std::is_same<R,float80_t>::value),
                  "The operator was applied to a non-numeric expression type.");
    using  rettype = typename coerce<L,R>::type;
    return { std::is_floating_point<rettype>::value ? make_equal_float(cast<rettype>(arg0),cast<rettype>(arg1)) :
                                                      make_equal_int(cast<rettype>(arg0),cast<rettype>(arg1)) };
}

template<typename L, typename R>
inline expression  operator >(typed_expression<L> const  arg0, typed_expression<R> const  arg1)
{ return arg1 < arg0; }

template<typename L, typename R>
inline expression  operator <=(typed_expression<L> const  arg0, typed_expression<R> const  arg1)
{ return arg0 < arg1 || arg0 == arg1; }

template<typename L, typename R>
inline expression  operator >=(typed_expression<L> const  arg0, typed_expression<R> const  arg1)
{ return arg0 > arg1 || arg0 == arg1; }

template<typename L, typename R>
inline expression  operator !=(typed_expression<L> const  arg0, typed_expression<R> const  arg1)
{ return !(arg0 == arg1); }


}

#endif
