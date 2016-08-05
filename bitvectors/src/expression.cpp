#include <rebours/bitvectors/expression.hpp>

namespace bv {


struct expression_impl
{
    static expression  create(symbol const  s, std::vector<expression> const&  arguments);

    symbol  get_symbol() const noexcept { return m_symbol; }
    uint64_t  num_arguments() const noexcept { return m_arguments.size(); }
    expression  argument(uint64_t const  index) const { return m_arguments.at(index); }

private:
    expression_impl(symbol const  s, std::vector<expression> const&  arguments);

    symbol  m_symbol;
    std::vector<expression>  m_arguments;
};


expression  expression_impl::create(symbol const  s, std::vector<expression> const&  arguments)
{
    return expression{ std::shared_ptr<expression_impl>{new expression_impl(s,arguments) }};
}

expression_impl::expression_impl(symbol const  s, std::vector<expression> const&  arguments)
    : m_symbol(s)
    , m_arguments(arguments)
{
    ASSUMPTION(symbol_num_parameters(m_symbol) == m_arguments.size());
    ASSUMPTION(
            [](symbol const  s, std::vector<expression> const&  arguments) {
                for (uint64_t i = 0ULL; i < arguments.size(); ++i)
                    if (symbol_num_bits_of_parameter(s,i) != num_bits_of_return_value(arguments.at(i)))
                        return false;
                return true;
            }(m_symbol,m_arguments)
            );
}


expression::expression(symbol const  s, std::vector<expression> const&  args)
    : expression(expression_impl::create(s,args))
{}


bool  expression_impl_equal(expression_impl const&  e0, expression_impl const&  e1)
{
    if (e0.get_symbol() != e1.get_symbol())
        return false;
    for (uint64_t i = 0ULL; i < e0.num_arguments(); ++i)
        if (e0.argument(i) != e1.argument(i))
            return false;
    return true;
}

std::size_t  expression_impl_hash(expression_impl const&  e)
{
    std::size_t  result = symbol::hash()(e.get_symbol());
    for (uint64_t i = 0ULL; i < e.num_arguments(); ++i)
        result += (i + 1ULL) * 12101ULL * expression::hash()(e.argument(i));
    return result;
}


/**
 * Queries for properties of an expression.
 */

symbol  get_symbol(expression const  e)
{
    return e->get_symbol();
}

uint64_t  num_arguments(expression const  e)
{
    return e->num_arguments();
}

expression  argument(expression const  e, uint64_t const  index)
{
    return e->argument(index);
}

bool  is_number(expression const  e)
{
    switch (num_bits_of_return_value(e))
    {
    case 8ULL:
    case 16ULL:
    case 32ULL:
    case 64ULL:
        return is_interpreted_constant(e);
    default:
        return false;
    }
}


/**
 * Construction of uninterpreted constants and functions
 */

expression  ufun(std::string const&  var_name, uint64_t const  num_bits_of_return_value, std::vector<expression> const&  args)
{
    std::vector<uint64_t> arg_sizes;
    for (auto e : args)
        arg_sizes.push_back(bv::num_bits_of_return_value(e));
    return {make_symbol_of_unintepreted_function(var_name,num_bits_of_return_value,arg_sizes),args};
}


/**
 * Construction of logical constants, conectives, and the universal quantifier
 */

expression  tt()
{
    static expression  value{make_symbol_of_true(),{}};
    return value;
}

expression  ff()
{
    static expression  value{make_symbol_of_false(),{}};
    return value;
}

expression  operator &&(expression const  arg0, expression const  arg1)
{
    ASSUMPTION(num_bits_of_return_value(arg0) == 1ULL);
    ASSUMPTION(num_bits_of_return_value(arg1) == 1ULL);
    return {make_symbol_of_logical_conjunction(),{arg0,arg1}};
}

expression  operator !(expression const  arg0)
{
    ASSUMPTION(num_bits_of_return_value(arg0) == 1ULL);
    return {make_symbol_of_logical_negation(),{arg0}};
}

expression  forall(expression const  var, expression const  body)
{
    ASSUMPTION(!is_interpreted(var));
    ASSUMPTION(!num_arguments(var) == 0ULL);
    ASSUMPTION(num_bits_of_return_value(body) == 1ULL);
    return {make_symbol_of_quantifier_forall(num_bits_of_return_value(var)),{var,body}};
}


/**
 * Construction of cast operations
 */

expression  make_cast_signed_int(expression const  arg0, uint64_t const  num_dst_bits)
{
    return {make_symbol_of_interpreted_cast_signed_int(num_bits_of_return_value(arg0),num_dst_bits),{arg0}};
}

expression  make_cast_unsigned_int(expression const  arg0, uint64_t const  num_dst_bits)
{
    return {make_symbol_of_interpreted_cast_unsigned_int(num_bits_of_return_value(arg0),num_dst_bits),{arg0}};
}

expression  make_cast_float(expression const  arg0, uint64_t const  num_dst_bits)
{
    return {make_symbol_of_interpreted_cast_float(num_bits_of_return_value(arg0),num_dst_bits),{arg0}};
}

expression  make_cast_signed_int_to_float(expression const  arg0, uint64_t const  num_bits)
{
    return {make_symbol_of_interpreted_cast_signed_int_to_float(num_bits),{arg0}};
}

expression  make_cast_unsigned_int_to_float(expression const  arg0, uint64_t const  num_bits)
{
    return {make_symbol_of_interpreted_cast_unsigned_int_to_float(num_bits),{arg0}};
}

expression  make_cast_float_to_signed_int(expression const  arg0, uint64_t const  num_bits)
{
    return {make_symbol_of_interpreted_cast_float_to_signed_int(num_bits),{arg0}};
}

expression  make_cast_float_to_unsigned_int(expression const  arg0, uint64_t const  num_bits)
{
    return {make_symbol_of_interpreted_cast_float_to_unsigned_int(num_bits),{arg0}};
}


/**
 * Construction of arithmetic operations
 */

expression  make_addition_int(expression const  arg0, expression const  arg1)
{
    return {make_symbol_of_interpreted_binary_add_int(num_bits_of_return_value(arg0)),{arg0,arg1}};
}

expression  make_addition_float(expression const  arg0, expression const  arg1)
{
    return {make_symbol_of_interpreted_binary_add_float(num_bits_of_return_value(arg0)),{arg0,arg1}};
}

expression  make_subtraction_int(expression const  arg0, expression const  arg1)
{
    return {make_symbol_of_interpreted_binary_subtract_int(num_bits_of_return_value(arg0)),{arg0,arg1}};
}

expression  make_subtraction_float(expression const  arg0, expression const  arg1)
{
    return {make_symbol_of_interpreted_binary_subtract_float(num_bits_of_return_value(arg0)),{arg0,arg1}};
}

expression  make_multiply_int(expression const  arg0, expression const  arg1)
{
    return {make_symbol_of_interpreted_binary_multiply_int(num_bits_of_return_value(arg0)),{arg0,arg1}};
}

expression  make_multiply_float(expression const  arg0, expression const  arg1)
{
    return {make_symbol_of_interpreted_binary_multiply_float(num_bits_of_return_value(arg0)),{arg0,arg1}};
}

expression  make_divide_signed_int(expression const  arg0, expression const  arg1)
{
    return {make_symbol_of_interpreted_binary_divide_signed_int(num_bits_of_return_value(arg0)),{arg0,arg1}};
}

expression  make_divide_unsigned_int(expression const  arg0, expression const  arg1)
{
    return {make_symbol_of_interpreted_binary_divide_unsigned_int(num_bits_of_return_value(arg0)),{arg0,arg1}};
}

expression  make_divide_float(expression const  arg0, expression const  arg1)
{
    return {make_symbol_of_interpreted_binary_divide_float(num_bits_of_return_value(arg0)),{arg0,arg1}};
}

expression  make_remainder_signed_int(expression const  arg0, expression const  arg1)
{
    return {make_symbol_of_interpreted_binary_remainder_signed_int(num_bits_of_return_value(arg0)),{arg0,arg1}};
}

expression  make_remainder_unsigned_int(expression const  arg0, expression const  arg1)
{
    return {make_symbol_of_interpreted_binary_remainder_unsigned_int(num_bits_of_return_value(arg0)),{arg0,arg1}};
}

expression  make_remainder_float(expression const  arg0, expression const  arg1)
{
    return {make_symbol_of_interpreted_binary_remainder_float(num_bits_of_return_value(arg0)),{arg0,arg1}};
}

expression  make_shift_left_int(expression const  arg0, expression const  arg1)
{
    return {make_symbol_of_interpreted_binary_shift_left_int(num_bits_of_return_value(arg0)),{arg0,arg1}};
}

expression  make_shift_right_signed_int(expression const  arg0, expression const  arg1)
{
    return {make_symbol_of_interpreted_binary_shift_right_signed_int(num_bits_of_return_value(arg0)),{arg0,arg1}};
}

expression  make_shift_right_unsigned_int(expression const  arg0, expression const  arg1)
{
    return {make_symbol_of_interpreted_binary_shift_right_unsigned_int(num_bits_of_return_value(arg0)),{arg0,arg1}};
}

expression  make_rotation_left(expression const  arg0, expression const  arg1)
{
    return {make_symbol_of_interpreted_binary_rotate_left_int(num_bits_of_return_value(arg0)),{arg0,arg1}};
}

expression  make_rotation_right(expression const  arg0, expression const  arg1)
{
    return {make_symbol_of_interpreted_binary_rotate_right_int(num_bits_of_return_value(arg0)),{arg0,arg1}};
}

expression  make_bitwise_and_int(expression const  arg0, expression const  arg1)
{
    return {make_symbol_of_interpreted_binary_bitwise_and_int(num_bits_of_return_value(arg0)),{arg0,arg1}};
}

expression  make_bitwise_or_int(expression const  arg0, expression const  arg1)
{
    return {make_symbol_of_interpreted_binary_bitwise_or_int(num_bits_of_return_value(arg0)),{arg0,arg1}};
}

expression  make_bitwise_xor_int(expression const  arg0, expression const  arg1)
{
    return {make_symbol_of_interpreted_binary_bitwise_xor_int(num_bits_of_return_value(arg0)),{arg0,arg1}};
}

expression  make_concatenation(expression const  arg0, expression const  arg1)
{
    return {make_symbol_of_interpreted_concatenation(num_bits_of_return_value(arg0),num_bits_of_return_value(arg1)),{arg0,arg1}};
}


/**
 * Construction of comparisons
 */

expression  make_less_than_signed_int(expression const  arg0, expression const  arg1)
{
    return {make_symbol_of_interpreted_less_than_signed_int(num_bits_of_return_value(arg0)),{arg0,arg1}};
}

expression  make_less_than_unsigned_int(expression const  arg0, expression const  arg1)
{
    return {make_symbol_of_interpreted_less_than_unsigned_int(num_bits_of_return_value(arg0)),{arg0,arg1}};
}

expression  make_less_than_float(expression const  arg0, expression const  arg1)
{
    return {make_symbol_of_interpreted_less_than_float(num_bits_of_return_value(arg0)),{arg0,arg1}};
}

expression  make_equal_int(expression const  arg0, expression const  arg1)
{
    return {make_symbol_of_interpreted_equal_int(num_bits_of_return_value(arg0)),{arg0,arg1}};
}

expression  make_equal_float(expression const  arg0, expression const  arg1)
{
    return {make_symbol_of_interpreted_equal_float(num_bits_of_return_value(arg0)),{arg0,arg1}};
}


}
