#include <rebours/bitvectors/expression_io.hpp>
#include <rebours/bitvectors/expression_algo.hpp>
#include <rebours/bitvectors/invariants.hpp>
#include <rebours/bitvectors/msgstream.hpp>
#include <unordered_set>
#include <unordered_map>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <iostream>
#include <set>

namespace bv { namespace detail {


QF_UFBV_operator_props::QF_UFBV_operator_props(std::string const&  name,
                                               std::vector<uint64_t> const&  num_bits_of_parameters)
    : m_name{name}
    , m_num_bits_of_parameters{num_bits_of_parameters}
{
    ASSUMPTION(!m_name.empty());
    ASSUMPTION(m_name.find_first_of(" \t\r\n") == std::string::npos);
}

std::size_t  QF_UFBV_operator_props::hasher(QF_UFBV_operator_props const&  props)
{
    std::size_t  result = std::hash<std::string>()(props.name());
    for (uint64_t i = 0ULL; i < props.num_parameters(); ++i)
        result += (i + 1ULL) * 101ULL * std::hash<uint64_t>()(props.num_bits_of_parameter(i));
    return result;
}

bool  QF_UFBV_operator_props::equal(QF_UFBV_operator_props const&  p1, QF_UFBV_operator_props const&  p2)
{
    return p1.name() == p2.name() &&
           p1.num_parameters() == p2.num_parameters() &&
           [](QF_UFBV_operator_props const&  p1, QF_UFBV_operator_props const&  p2) {
                for (uint64_t i = 0ULL; i < p1.num_parameters(); ++i)
                    if (p1.num_bits_of_parameter(i) != p2.num_bits_of_parameter(i))
                        return false;
                return true;
                }(p1,p2)
           ;
}

std::unordered_map<QF_UFBV_operator_props,
                   bv::symbol,
                   decltype(&bv::detail::QF_UFBV_operator_props::hasher),
                   decltype(&bv::detail::QF_UFBV_operator_props::equal)> const&
QF_UFBV_names_to_symbols()
{
    static std::unordered_map<QF_UFBV_operator_props,
                              bv::symbol,
                              decltype(&bv::detail::QF_UFBV_operator_props::hasher),
                              decltype(&bv::detail::QF_UFBV_operator_props::equal)> const  table({

            // Logical connectives and the quantifier

            { {"and", {1ULL,1ULL}}, make_symbol_of_logical_conjunction() },
            { {"not", {1ULL}}, make_symbol_of_logical_negation() },

            { {"forall", {8ULL,1ULL}}, make_symbol_of_quantifier_forall(8ULL) },
            { {"forall", {16ULL,1ULL}}, make_symbol_of_quantifier_forall(16ULL) },
            { {"forall", {32ULL,1ULL}}, make_symbol_of_quantifier_forall(32ULL) },
            { {"forall", {64ULL,1ULL}}, make_symbol_of_quantifier_forall(64ULL) },

            // Normal operators in smtlib2

            { {"bvadd", {8ULL,8ULL}}, make_symbol_of_interpreted_binary_add_int(8ULL) },
            { {"bvadd", {16ULL,16ULL}}, make_symbol_of_interpreted_binary_add_int(16ULL) },
            { {"bvadd", {32ULL,32ULL}}, make_symbol_of_interpreted_binary_add_int(32ULL) },
            { {"bvadd", {64ULL,64ULL}}, make_symbol_of_interpreted_binary_add_int(64ULL) },

            { {"bvsub", {8ULL,8ULL}}, make_symbol_of_interpreted_binary_subtract_int(8ULL) },
            { {"bvsub", {16ULL,16ULL}}, make_symbol_of_interpreted_binary_subtract_int(16ULL) },
            { {"bvsub", {32ULL,32ULL}}, make_symbol_of_interpreted_binary_subtract_int(32ULL) },
            { {"bvsub", {64ULL,64ULL}}, make_symbol_of_interpreted_binary_subtract_int(64ULL) },

            { {"bvmul", {8ULL,8ULL}}, make_symbol_of_interpreted_binary_multiply_int(8ULL) },
            { {"bvmul", {16ULL,16ULL}}, make_symbol_of_interpreted_binary_multiply_int(16ULL) },
            { {"bvmul", {32ULL,32ULL}}, make_symbol_of_interpreted_binary_multiply_int(32ULL) },
            { {"bvmul", {64ULL,64ULL}}, make_symbol_of_interpreted_binary_multiply_int(64ULL) },

            { {"bvsdiv", {8ULL,8ULL}}, make_symbol_of_interpreted_binary_divide_signed_int(8ULL) },
            { {"bvsdiv", {16ULL,16ULL}}, make_symbol_of_interpreted_binary_divide_signed_int(16ULL) },
            { {"bvsdiv", {32ULL,32ULL}}, make_symbol_of_interpreted_binary_divide_signed_int(32ULL) },
            { {"bvsdiv", {64ULL,64ULL}}, make_symbol_of_interpreted_binary_divide_signed_int(64ULL) },

            { {"bvudiv", {8ULL,8ULL}}, make_symbol_of_interpreted_binary_divide_unsigned_int(8ULL) },
            { {"bvudiv", {16ULL,16ULL}}, make_symbol_of_interpreted_binary_divide_unsigned_int(16ULL) },
            { {"bvudiv", {32ULL,32ULL}}, make_symbol_of_interpreted_binary_divide_unsigned_int(32ULL) },
            { {"bvudiv", {64ULL,64ULL}}, make_symbol_of_interpreted_binary_divide_unsigned_int(64ULL) },

            { {"bvsrem", {8ULL,8ULL}}, make_symbol_of_interpreted_binary_remainder_signed_int(8ULL) },
            { {"bvsrem", {16ULL,16ULL}}, make_symbol_of_interpreted_binary_remainder_signed_int(16ULL) },
            { {"bvsrem", {32ULL,32ULL}}, make_symbol_of_interpreted_binary_remainder_signed_int(32ULL) },
            { {"bvsrem", {64ULL,64ULL}}, make_symbol_of_interpreted_binary_remainder_signed_int(64ULL) },

            { {"bvurem", {8ULL,8ULL}}, make_symbol_of_interpreted_binary_remainder_unsigned_int(8ULL) },
            { {"bvurem", {16ULL,16ULL}}, make_symbol_of_interpreted_binary_remainder_unsigned_int(16ULL) },
            { {"bvurem", {32ULL,32ULL}}, make_symbol_of_interpreted_binary_remainder_unsigned_int(32ULL) },
            { {"bvurem", {64ULL,64ULL}}, make_symbol_of_interpreted_binary_remainder_unsigned_int(64ULL) },

            { {"bvshl", {8ULL}}, make_symbol_of_interpreted_binary_shift_left_int(8ULL) },
            { {"bvshl", {16ULL}}, make_symbol_of_interpreted_binary_shift_left_int(16ULL) },
            { {"bvshl", {32ULL}}, make_symbol_of_interpreted_binary_shift_left_int(32ULL) },
            { {"bvshl", {64ULL}}, make_symbol_of_interpreted_binary_shift_left_int(64ULL) },

            { {"bvashr", {8ULL}}, make_symbol_of_interpreted_binary_shift_right_signed_int(8ULL) },
            { {"bvashr", {16ULL}}, make_symbol_of_interpreted_binary_shift_right_signed_int(16ULL) },
            { {"bvashr", {32ULL}}, make_symbol_of_interpreted_binary_shift_right_signed_int(32ULL) },
            { {"bvashr", {64ULL}}, make_symbol_of_interpreted_binary_shift_right_signed_int(64ULL) },

            { {"bvlshr", {8ULL}}, make_symbol_of_interpreted_binary_shift_right_unsigned_int(8ULL) },
            { {"bvlshr", {16ULL}}, make_symbol_of_interpreted_binary_shift_right_unsigned_int(16ULL) },
            { {"bvlshr", {32ULL}}, make_symbol_of_interpreted_binary_shift_right_unsigned_int(32ULL) },
            { {"bvlshr", {64ULL}}, make_symbol_of_interpreted_binary_shift_right_unsigned_int(64ULL) },

            { {"bvand", {8ULL}}, make_symbol_of_interpreted_binary_bitwise_and_int(8ULL) },
            { {"bvand", {16ULL}}, make_symbol_of_interpreted_binary_bitwise_and_int(16ULL) },
            { {"bvand", {32ULL}}, make_symbol_of_interpreted_binary_bitwise_and_int(32ULL) },
            { {"bvand", {64ULL}}, make_symbol_of_interpreted_binary_bitwise_and_int(64ULL) },

            { {"bvor", {8ULL}}, make_symbol_of_interpreted_binary_bitwise_or_int(8ULL) },
            { {"bvor", {16ULL}}, make_symbol_of_interpreted_binary_bitwise_or_int(16ULL) },
            { {"bvor", {32ULL}}, make_symbol_of_interpreted_binary_bitwise_or_int(32ULL) },
            { {"bvor", {64ULL}}, make_symbol_of_interpreted_binary_bitwise_or_int(64ULL) },

            { {"bvxor", {8ULL}}, make_symbol_of_interpreted_binary_bitwise_xor_int(8ULL) },
            { {"bvxor", {16ULL}}, make_symbol_of_interpreted_binary_bitwise_xor_int(16ULL) },
            { {"bvxor", {32ULL}}, make_symbol_of_interpreted_binary_bitwise_xor_int(32ULL) },
            { {"bvxor", {64ULL}}, make_symbol_of_interpreted_binary_bitwise_xor_int(64ULL) },

            { {"concat", {8ULL,8ULL}}, make_symbol_of_interpreted_concatenation(8ULL,8ULL) },
            { {"concat", {8ULL,16ULL}}, make_symbol_of_interpreted_concatenation(8ULL,16ULL) },
            { {"concat", {8ULL,32ULL}}, make_symbol_of_interpreted_concatenation(8ULL,32ULL) },
            { {"concat", {8ULL,64ULL}}, make_symbol_of_interpreted_concatenation(8ULL,64ULL) },
            { {"concat", {16ULL,8ULL}}, make_symbol_of_interpreted_concatenation(16ULL,8ULL) },
            { {"concat", {16ULL,16ULL}}, make_symbol_of_interpreted_concatenation(16ULL,16ULL) },
            { {"concat", {16ULL,32ULL}}, make_symbol_of_interpreted_concatenation(16ULL,32ULL) },
            { {"concat", {16ULL,64ULL}}, make_symbol_of_interpreted_concatenation(16ULL,64ULL) },
            { {"concat", {32ULL,8ULL}}, make_symbol_of_interpreted_concatenation(32ULL,8ULL) },
            { {"concat", {32ULL,16ULL}}, make_symbol_of_interpreted_concatenation(32ULL,16ULL) },
            { {"concat", {32ULL,32ULL}}, make_symbol_of_interpreted_concatenation(32ULL,32ULL) },
            { {"concat", {32ULL,64ULL}}, make_symbol_of_interpreted_concatenation(32ULL,64ULL) },
            { {"concat", {64ULL,8ULL}}, make_symbol_of_interpreted_concatenation(64ULL,8ULL) },
            { {"concat", {64ULL,16ULL}}, make_symbol_of_interpreted_concatenation(64ULL,16ULL) },
            { {"concat", {64ULL,32ULL}}, make_symbol_of_interpreted_concatenation(64ULL,32ULL) },
            { {"concat", {64ULL,64ULL}}, make_symbol_of_interpreted_concatenation(64ULL,64ULL) },

            { {"bvslt", {8ULL,8ULL}}, make_symbol_of_interpreted_less_than_signed_int(8ULL) },
            { {"bvslt", {16ULL,16ULL}}, make_symbol_of_interpreted_less_than_signed_int(16ULL) },
            { {"bvslt", {32ULL,32ULL}}, make_symbol_of_interpreted_less_than_signed_int(32ULL) },
            { {"bvslt", {64ULL,64ULL}}, make_symbol_of_interpreted_less_than_signed_int(64ULL) },

            { {"bvult", {8ULL,8ULL}}, make_symbol_of_interpreted_less_than_unsigned_int(8ULL) },
            { {"bvult", {16ULL,16ULL}}, make_symbol_of_interpreted_less_than_unsigned_int(16ULL) },
            { {"bvult", {32ULL,32ULL}}, make_symbol_of_interpreted_less_than_unsigned_int(32ULL) },
            { {"bvult", {64ULL,64ULL}}, make_symbol_of_interpreted_less_than_unsigned_int(64ULL) },

            { {"=", {8ULL,8ULL}}, make_symbol_of_interpreted_equal_int(8ULL) },
            { {"=", {16ULL,16ULL}}, make_symbol_of_interpreted_equal_int(16ULL) },
            { {"=", {32ULL,32ULL}}, make_symbol_of_interpreted_equal_int(32ULL) },
            { {"=", {64ULL,64ULL}}, make_symbol_of_interpreted_equal_int(64ULL) },

            // Composed operators in smtlib2

            { {"extract", {31ULL,0ULL,64ULL}}, make_symbol_of_interpreted_cast_unsigned_int(64ULL,32ULL) },
            { {"extract", {15ULL,0ULL,32ULL}}, make_symbol_of_interpreted_cast_unsigned_int(32ULL,16ULL) },
            { {"extract", {7ULL,0ULL,16ULL}}, make_symbol_of_interpreted_cast_unsigned_int(16ULL,8ULL) },

            { {"sign_extend", {8ULL,8ULL}}, make_symbol_of_interpreted_cast_signed_int(8ULL,16ULL) },
            { {"sign_extend", {16ULL,16ULL}}, make_symbol_of_interpreted_cast_signed_int(16ULL,32ULL) },
            { {"sign_extend", {32ULL,32ULL}}, make_symbol_of_interpreted_cast_signed_int(32ULL,64ULL) },

            { {"zero_extend", {8ULL,8ULL}}, make_symbol_of_interpreted_cast_unsigned_int(8ULL,16ULL) },
            { {"zero_extend", {16ULL,16ULL}}, make_symbol_of_interpreted_cast_unsigned_int(16ULL,32ULL) },
            { {"zero_extend", {32ULL,32ULL}}, make_symbol_of_interpreted_cast_unsigned_int(32ULL,64ULL) },

            { {"rotate_left", {1ULL,8ULL}}, make_symbol_of_interpreted_binary_rotate_left_int(8ULL) },
            { {"rotate_left", {1ULL,16ULL}}, make_symbol_of_interpreted_binary_rotate_left_int(16ULL) },
            { {"rotate_left", {1ULL,32ULL}}, make_symbol_of_interpreted_binary_rotate_left_int(32ULL) },
            { {"rotate_left", {1ULL,64ULL}}, make_symbol_of_interpreted_binary_rotate_left_int(64ULL) },

            { {"rotate_right", {1ULL,8ULL}}, make_symbol_of_interpreted_binary_rotate_right_int(8ULL) },
            { {"rotate_right", {1ULL,16ULL}}, make_symbol_of_interpreted_binary_rotate_right_int(16ULL) },
            { {"rotate_right", {1ULL,32ULL}}, make_symbol_of_interpreted_binary_rotate_right_int(32ULL) },
            { {"rotate_right", {1ULL,64ULL}}, make_symbol_of_interpreted_binary_rotate_right_int(64ULL) },

            // Unsupported operators in smtlib2 (in QF_UFBV)

            { {"cast_f64f32", {64ULL}}, make_symbol_of_interpreted_cast_float(64ULL,32ULL) },

            { {"cast_s32f32", {32ULL}}, make_symbol_of_interpreted_cast_signed_int_to_float(32ULL) },
            { {"cast_s64f64", {64ULL}}, make_symbol_of_interpreted_cast_signed_int_to_float(64ULL) },

            { {"cast_u32f32", {32ULL}}, make_symbol_of_interpreted_cast_unsigned_int_to_float(32ULL) },
            { {"cast_u64f64", {64ULL}}, make_symbol_of_interpreted_cast_unsigned_int_to_float(64ULL) },

            { {"cast_f32s32", {32ULL}}, make_symbol_of_interpreted_cast_float_to_signed_int(32ULL) },
            { {"cast_f64s64", {64ULL}}, make_symbol_of_interpreted_cast_float_to_signed_int(64ULL) },

            { {"cast_f32u32", {32ULL}}, make_symbol_of_interpreted_cast_float_to_unsigned_int(32ULL) },
            { {"cast_f64u64", {64ULL}}, make_symbol_of_interpreted_cast_float_to_unsigned_int(64ULL) },

            { {"add_f32", {32ULL,32ULL}}, make_symbol_of_interpreted_binary_add_float(32ULL) },
            { {"add_f64", {64ULL,64ULL}}, make_symbol_of_interpreted_binary_add_float(64ULL) },

            { {"sub_f32", {32ULL,32ULL}}, make_symbol_of_interpreted_binary_subtract_float(32ULL) },
            { {"sub_f64", {64ULL,64ULL}}, make_symbol_of_interpreted_binary_subtract_float(64ULL) },

            { {"mul_f32", {32ULL,32ULL}}, make_symbol_of_interpreted_binary_multiply_float(32ULL) },
            { {"mul_f64", {64ULL,64ULL}}, make_symbol_of_interpreted_binary_multiply_float(64ULL) },

            { {"div_f32", {32ULL,32ULL}}, make_symbol_of_interpreted_binary_divide_float(32ULL) },
            { {"div_f64", {64ULL,64ULL}}, make_symbol_of_interpreted_binary_divide_float(64ULL) },

            { {"rem_f32", {32ULL,32ULL}}, make_symbol_of_interpreted_binary_remainder_float(32ULL) },
            { {"rem_f64", {64ULL,64ULL}}, make_symbol_of_interpreted_binary_remainder_float(64ULL) },

            { {"less_than_f32", {32ULL,32ULL}}, make_symbol_of_interpreted_less_than_float(32ULL) },
            { {"less_than_f64", {64ULL,64ULL}}, make_symbol_of_interpreted_less_than_float(64ULL) },

            { {"equal_f32", {32ULL,32ULL}}, make_symbol_of_interpreted_equal_float(32ULL) },
            { {"equal_f64", {64ULL,64ULL}}, make_symbol_of_interpreted_equal_float(64ULL) },

            },0,&QF_UFBV_operator_props::hasher,&QF_UFBV_operator_props::equal);
    return table;
}

std::unordered_map<std::string,std::string> const&  inverted_operators_outside_QF_UFBV()
{
    static std::unordered_map<std::string,std::string> const u {
            { "cast_f32f64"   , "#f32f64"},
            { "cast_f64f32"   , "#f64f32"},

            { "cast_s32f32"   , "#s32f32"},
            { "cast_s64f64"   , "#s64f64"},

            { "cast_u32f32"   , "#u32f32"},
            { "cast_u64f64"   , "#u64f64"},

            { "cast_f32s32"   , "#f32s32"},
            { "cast_f64s64"   , "#f64s64"},

            { "cast_f32u32"   , "#f32u32"},
            { "cast_f64u64"   , "#f64u64"},

            { "add_f32"       , "+f32"   },
            { "add_f64"       , "+f64"   },

            { "sub_f32"       , "-f32"   },
            { "sub_f64"       , "-f64"   },

            { "mul_f32"       , "*f32"   },
            { "mul_f64"       , "*f64"   },

            { "div_f32"       , "/f32"   },
            { "div_f64"       , "/f64"   },

            { "rem_f32"       , "%f32"   },
            { "rem_f64"       , "%f64"   },

            { "less_than_f32" , "<f32"   },
            { "less_than_f64" , "<f64"   },

            { "equal_f32"     , "=f32"   },
            { "equal_f64"     , "=f64"   },
            };
    return u;
}


}}

namespace bv { namespace {


uint64_t  build_smtlib2_ast(std::vector<detail::smtlib2_token> const&  tokens, uint64_t const start_index,
                            std::vector<detail::smtlib2_ast_node>&  output, std::string& error)
{
    INVARIANT(start_index < tokens.size());

    uint64_t  index = start_index;

    if (tokens.at(index).name() != "(")
    {
        error = msgstream() << "ERROR[" << tokens.at(index).line() << ":" << tokens.at(index).column() << "] : "
                            << "Expected token '(', but found '" << tokens.at(index).name() << "'.";
        return tokens.size();
    }

    ++index;

    std::vector<detail::smtlib2_ast_node>  children;
    while (index < tokens.size() && tokens.at(index).name() != ")" && error.empty())
        if (tokens.at(index).name() == "(")
            index = build_smtlib2_ast(tokens,index,children,error);
        else
        {
            children.push_back({tokens.at(index),{}});
            ++index;
        }

    if (!error.empty())
        return tokens.size();

    if (index == tokens.size())
    {
        error = msgstream() << "ERROR[" << tokens.at(start_index).line() << ":" << tokens.at(start_index).column() << "] : "
                            << "This bracket does not have the related closing bracked.";
        return tokens.size();
    }

    ++index;

    output.push_back({{"",tokens.at(start_index).line(),tokens.at(start_index).column()},children});

    return index;
}

bool  is_set_logic_smtlib2_ast(detail::smtlib2_ast_node const&  ast)
{
    return ast.token().name().empty() &&
           ast.children().size() == 2ULL &&
           !ast.children().at(0ULL).token().name().empty() &&
           !ast.children().at(1ULL).token().name().empty() &&
           ast.children().at(0ULL).token().name() == "set-logic";
}

bool  is_declare_fun_smtlib2_ast(detail::smtlib2_ast_node const&  ast)
{
    return ast.token().name().empty() &&
           ast.children().size() == 4ULL &&
           !ast.children().at(0ULL).token().name().empty() &&
           !ast.children().at(1ULL).token().name().empty() &&
            ast.children().at(0ULL).token().name() == "declare-fun";
}

bool  is_assert_smtlib2_ast(detail::smtlib2_ast_node const&  ast)
{
    return ast.token().name().empty() &&
           ast.children().size() == 2ULL &&
           !ast.children().at(0ULL).token().name().empty() &&
           ast.children().at(0ULL).token().name() == "assert";
}

bool  is_assert_eval_smtlib2_ast(detail::smtlib2_ast_node const&  ast)
{
    return is_assert_smtlib2_ast(ast) &&
           ast.children().at(1ULL).token().name().empty() &&
           ast.children().at(1ULL).children().size() == 2ULL &&
           ast.children().at(1ULL).children().at(0ULL).token().name() == "eval";
}

bool  is_ignored_smtlib2_ast(detail::smtlib2_ast_node const&  ast)
{
    return ast.token().name().empty() && (
           ( ast.children().size() == 1ULL &&
             ast.children().at(0ULL).token().name() == "exit"
           ) ||
           ( ast.children().size() == 1ULL &&
             ast.children().at(0ULL).token().name() == "check-sat"
           ) ||
           ( ast.children().size() == 1ULL &&
             ast.children().at(0ULL).token().name() == "get-model"
           )
           );
}

bool  is_bitvector_type_smtlib2_ast(detail::smtlib2_ast_node const&  ast)
{
    return ast.token().name().empty() &&
           ast.children().size() == 3ULL &&
           !ast.children().at(0ULL).token().name().empty() &&
           ast.children().at(0ULL).token().name() == "_" &&
           !ast.children().at(1ULL).token().name().empty() &&
           ast.children().at(1ULL).token().name() == "BitVec" &&
           !ast.children().at(2ULL).token().name().empty() &&
           [](std::string const& name) {
                for (auto c : name)
                    if (!std::isdigit(c))
                        return false;
                return true;
                }(ast.children().at(2ULL).token().name())
           ;
}

bool is_tt(detail::smtlib2_ast_node const&  ast)
{
    return ast.token().name().empty() &&
           ast.children().size() == 3ULL &&
           ast.children().at(0ULL).token().name() == "=" &&
           ast.children().at(1ULL).token().name() == "#b0" &&
           ast.children().at(2ULL).token().name() == "#b0"
           ;
}

bool is_ff(detail::smtlib2_ast_node const&  ast)
{
    return ast.token().name().empty() &&
           ast.children().size() == 3ULL &&
           ast.children().at(0ULL).token().name() == "=" &&
           ast.children().at(1ULL).token().name() == "#b0" &&
           ast.children().at(2ULL).token().name() == "#b1"
           ;
}

bool  is_hexadecimal_number_smtlib2_ast(detail::smtlib2_ast_node const&  ast)
{
    return !ast.token().name().empty() &&
           ast.token().name().find("#x") != 0ULL &&
           [](std::string const& name) {
                for (auto c : name)
                    if (!std::isxdigit(c))
                        return false;
                return true;
                }(ast.token().name())
           ;
}

symbol  build_ufunction_symbol_from_smtlib2_ast(detail::smtlib2_ast_node const&  ast, std::string&  error)
{
    std::string const  name = ast.children().at(1ULL).token().name();
    INVARIANT(!name.empty());

    std::vector<uint64_t>  params;
    for (auto const& param : ast.children().at(2ULL).children())
    {
        uint64_t const num_bits = detail::compute_numbits_from_smtlib2_type_ast(param,error);
        if (!error.empty())
            return {};
        params.push_back(num_bits);
    }

    uint64_t const num_ret_bits = detail::compute_numbits_from_smtlib2_type_ast(ast.children().at(3ULL),error);
    if (!error.empty())
        return {};

    return make_symbol_of_unintepreted_function(name,num_ret_bits,params);
}

expression  load_derived_expression(detail::smtlib2_token const&  token, std::vector<uint64_t> const&  op_args,
                                    std::vector<expression> const&  args, std::string&  error)
{
    if (token.name() == "or")
    {
        if (op_args.size() != 2ULL || op_args.at(0ULL) != 1ULL || op_args.at(1ULL) != 1ULL)
        {
            error = msgstream() << "ERROR[" << token.line() << ":" << token.column() << "] : "
                                << "Wrong parameters of '" << token.name() << "' operator.";
            return {};
        }
        return args.at(0ULL) || args.at(1ULL);
    }
    else if (token.name() == "zero_extend")
    {
        if (op_args.size() != 2ULL || op_args.at(0ULL) <= op_args.at(1ULL) || args.size() != 1ULL ||
            std::set<uint64_t>{8ULL,16ULL,32ULL}.count(op_args.at(1ULL)) == 0ULL ||
            std::set<uint64_t>{16ULL,32ULL,64ULL}.count(op_args.at(0ULL) + op_args.at(1ULL)) == 0ULL)
        {
            error = msgstream() << "ERROR[" << token.line() << ":" << token.column() << "] : "
                                << "Wrong parameters of '" << token.name() << "' operator.";
            return {};
        }
        uint32_t  toadd = op_args.at(0ULL);
        uint32_t  current = op_args.at(1ULL);
        expression  result = {make_symbol_of_interpreted_cast_unsigned_int(current,2U*current),args};
        while (true)
        {
            INVARIANT(toadd >= current);
            toadd -= current;
            current *= 2U;
            if (toadd == 0U)
                break;
            result = {make_symbol_of_interpreted_cast_unsigned_int(current,2U*current),{result}};
        }
        return result;
    }
    else if (token.name() == "extract")
    {
        if (op_args.size() != 3ULL || op_args.at(0ULL) < 7ULL || op_args.at(1ULL) != 0ULL ||
            op_args.at(0ULL) + 1ULL >= op_args.at(2ULL) || args.size() != 1ULL ||
            std::set<uint64_t>{7ULL,15ULL,31ULL}.count(op_args.at(0ULL)) == 0ULL ||
            std::set<uint64_t>{16ULL,32ULL,64ULL}.count(op_args.at(2ULL)) == 0ULL)
        {
            error = msgstream() << "ERROR[" << token.line() << ":" << token.column() << "] : "
                                << "Wrong parameters of '" << token.name() << "' operator.";
            return {};
        }
        uint32_t  current = op_args.at(2ULL);
        expression  result = {make_symbol_of_interpreted_cast_unsigned_int(current,current / 2U),args};
        while (true)
        {
            current /= 2U;
            INVARIANT(current >= op_args.at(0ULL) + 1ULL);
            if (current == op_args.at(0ULL) + 1ULL)
                break;
            result = {make_symbol_of_interpreted_cast_unsigned_int(current,current / 2U),{result}};
        }
        return result;
    }
    error = msgstream() << "ERROR[" << token.line() << ":" << token.column() << "] : "
                        << "The token '" << token.name() << "' is unknown (wrong or supported). "
                           "Check also whether count and values of arguments are correct.";
    return {};
}


}}

namespace bv { namespace detail {


std::string  to_string(smtlib2_token const&  token)
{
    std::ostringstream  ostr;
    ostr << token.name() << '{' << token.line() << ':' << token.column() << '}';
    return ostr.str();
}

std::vector<uint64_t>  to_ret_numbits(std::vector<expression> const&  parameters)
{
    std::vector<uint64_t> result;
    for (auto const& p : parameters)
    {
        ASSUMPTION(bv::num_bits_of_return_value(p) > 0ULL);
        result.push_back(bv::num_bits_of_return_value(p));
    }
    return result;
}

void  tokenise_smtlib2_stream(std::istream&  istr, std::vector<smtlib2_token>&  output)
{
    std::string  new_line;
    std::string  token;
    uint32_t  line = 1U;
    uint32_t  column = 1U;
    while (!istr.eof() && !istr.bad())
    {
        struct local {
            static void  apply_new_line(std::string&  new_line, uint32_t&  line, uint32_t&  column) {
                if (new_line.empty())
                    return;
                std::size_t const  cnt_n = std::count(new_line.cbegin(),new_line.cend(),'\n');
                std::size_t const  cnt_r = std::count(new_line.cbegin(),new_line.cend(),'\r');
                new_line.clear();
                line += cnt_n == 0ULL ? cnt_r : cnt_n;
                column = 1U;
            }
            static void  save_token_if_present(std::string&  token, uint32_t const  line, uint32_t const  column,
                                               std::vector<smtlib2_token>&  output) {
                if (!token.empty())
                {
                    output.push_back({token,line,column-(uint32_t)token.size()});
                    token.clear();
                }
            }
        };

        std::istream::int_type const  c = istr.get();
        switch (c)
        {
        case ' ':
            local::apply_new_line(new_line,line,column);
            local::save_token_if_present(token,line,column,output);
            ++column;
            break;
        case '\t':
            local::apply_new_line(new_line,line,column);
            local::save_token_if_present(token,line,column,output);
            column += 4U;
            break;
        case '(':
            local::apply_new_line(new_line,line,column);
            local::save_token_if_present(token,line,column,output);
            output.push_back({"(",line,column});
            ++column;
            break;
        case ')':
            local::apply_new_line(new_line,line,column);
            local::save_token_if_present(token,line,column,output);
            output.push_back({")",line,column});
            ++column;
            break;
        case '\n':
        case '\r':
            local::save_token_if_present(token,line,column,output);
            new_line.push_back(c);
            break;
        default:
            local::apply_new_line(new_line,line,column);
            token.push_back(c);
            ++column;
            break;
        }
    }
}

std::string  to_string(smtlib2_ast_node const&  node)
{
    std::ostringstream  ostr;

    if (node.token().name().empty())
    {
        ostr << "( ";
        for (auto const& child : node.children())
            ostr << to_string(child) << ' ';
        ostr << ')';
    }
    else
        ostr << node.token();

    return ostr.str();
}

std::string  tokenised_smtlib2_stream_to_ast(std::vector<smtlib2_token> const&  tokens,
                                             std::vector<smtlib2_ast_node>&  output)
{
    std::string  error;
    uint64_t  index = 0ULL;
    while (index < tokens.size() && error.empty())
        index = build_smtlib2_ast(tokens,index,output,error);
    return error;
}


uint64_t  compute_numbits_from_smtlib2_type_ast(smtlib2_ast_node const&  ast, std::string&  error)
{
    if (!is_bitvector_type_smtlib2_ast(ast))
    {
        error = msgstream() << "ERROR[" << ast.token().line() << ":" << ast.token().column() << "] : "
                            << "'" << to_string(ast) << "' is not a bit-vector type.";
        return 0ULL;
    }

    long long const  num_bits = std::stoll(ast.children().at(2ULL).token().name());
    if (num_bits < 1LL)
    {
        error = msgstream() << "ERROR[" << ast.children().at(2ULL).token().line() << ":" << ast.children().at(2ULL).token().column() << "] : "
                            << "The number of bits must be a positive integer.";
        return 0ULL;
    }

    return num_bits;
}

expression  build_expression_from_smtlib2_ast(detail::smtlib2_ast_node const&  ast,
                                              std::unordered_map<std::string,symbol> const&  ufunctions,
                                              std::string&  error)
{
    auto const uit = ufunctions.find(ast.token().name());
    if (is_tt(ast))
        return tt();
    else if (is_ff(ast))
        return ff();
    else if (uit != ufunctions.cend())
    {
        if (ast.children().size() != symbol_num_parameters(uit->second))
        {
            error = msgstream() << "ERROR[" << ast.token().line() << ":" << ast.token().column() << "] : "
                                << "The uninterpreted function '" << ast.token().name() << "' is passed a wrong number of arguments.";
            return {};
        }

        std::vector<expression> args;
        for (uint64_t  i = 0ULL; i < ast.children().size(); ++i)
        {
            expression const  e = build_expression_from_smtlib2_ast(ast.children().at(i),ufunctions,error);
            if (num_bits_of_return_value(e) != symbol_num_bits_of_parameter(uit->second,i))
            {
                error = msgstream() << "ERROR[" << ast.children().at(i).token().line() << ":" << ast.children().at(i).token().column() << "] : "
                                    << "The number of bits of the argument " << i << " does not match the expected number of bith for that parameter.";
                return {};
            }
            args.push_back(e);
        }

        if (detail::inverted_operators_outside_QF_UFBV().count(symbol_name(uit->second)) == 0ULL)
            return ufun(ast.token().name(),symbol_num_bits_of_return_value(uit->second),args);
        //std::string const  fixed_name = inverted_operators_outside_QF_UFBV().at(ast.token().name());

        auto const  it = detail::QF_UFBV_names_to_symbols().find({ast.token().name(),to_ret_numbits(args)});
        if (it == detail::QF_UFBV_names_to_symbols().cend())
            return load_derived_expression(ast.token(),to_ret_numbits(args),args,error);

        return {it->second,args};
    }
    else if (ast.children().empty())
    {
        INVARIANT(!ast.token().name().empty());
        if (ast.token().name().find("#x") != 0ULL || ast.token().name().size() < 3ULL)
        {
            error = msgstream() << "ERROR[" << ast.token().line() << ":" << ast.token().column() << "] : "
                                << "Expected a numeric constant in the hexadecimal format (we do not support other formats yet).";
            return {};
        }
        std::string  value_name = ast.token().name();
        std::transform(value_name.begin(), value_name.end(), value_name.begin(), ::tolower);
        uint64_t  start = 2ULL;
        if ((value_name.size()-2ULL) % 2ULL != 0ULL)
        {
            value_name[1] = '0';
            start = 1ULL;
        }
        return mem(value_name.substr(start));
    }
    else
    {
        INVARIANT(ast.token().name().empty());

        std::vector<expression> args;
        for (uint64_t  i = 1ULL; i < ast.children().size(); ++i)
        {
            args.push_back(build_expression_from_smtlib2_ast(ast.children().at(i),ufunctions,error));
            if (!error.empty())
                return {};
        }

        if (ast.children().at(0ULL).token().name().empty())
        {
            detail::smtlib2_ast_node const&  op_ast = ast.children().at(0ULL);
            if (op_ast.children().size() < 2ULL)
            {
                error = msgstream() << "ERROR[" << op_ast.token().line() << ":" << op_ast.token().column() << "] : "
                                    << "Wrong number of arguments inside the bit-vector operator.";
                return {};
            }
            if (op_ast.children().at(0ULL).token().name() != "_")
            {
                error = msgstream() << "ERROR[" << op_ast.token().line() << ":" << op_ast.token().column() << "] : "
                                    << "The first argument of the bit-vector operator must be '_'.";
                return {};
            }

            std::vector<uint64_t> op_args;
            for (uint64_t  i = 2ULL; i < op_ast.children().size(); ++i)
            {
                int64_t const  value = std::atoll(op_ast.children().at(i).token().name().c_str());
                if (op_ast.children().at(i).token().name().empty() || value < 0LL || (value == 0LL && op_ast.children().at(i).token().name() != "0"))
                {
                    error = msgstream() << "ERROR[" << op_ast.children().at(i).token().line() << ":" << op_ast.children().at(i).token().column() << "] : "
                                        << "The argument is not a decimal non-negative integer.";
                    return {};
                }

                op_args.push_back(value);
            }
            for (auto const& a : args)
                op_args.push_back(num_bits_of_return_value(a));

            auto const  it = detail::QF_UFBV_names_to_symbols().find({op_ast.children().at(1ULL).token().name(),op_args});
            if (it == detail::QF_UFBV_names_to_symbols().cend())
                return load_derived_expression(op_ast.children().at(1ULL).token(),op_args,args,error);

            return {it->second,args};
        }
        else
        {
            auto const  it = detail::QF_UFBV_names_to_symbols().find({ast.children().at(0ULL).token().name(),to_ret_numbits(args)});
            if (it == detail::QF_UFBV_names_to_symbols().cend())
                return load_derived_expression(ast.children().at(0ULL).token(),to_ret_numbits(args),args,error);

            return {it->second,args};
        }
    }

    return {};
}


}}

namespace bv {


expression  load_in_smtlib2_format(std::istream&  istr, std::string&  error)
{
    std::vector<detail::smtlib2_token>  tokens;
    detail::tokenise_smtlib2_stream(istr,tokens);
    std::vector<detail::smtlib2_ast_node>  asts;
    error = detail::tokenised_smtlib2_stream_to_ast(tokens,asts);
    if (!error.empty())
        return {};

    std::unordered_map<std::string,symbol>  ufunctions;
    std::vector<expression>  conjuncts;
    for (auto const&  ast : asts)
    {
        if (is_set_logic_smtlib2_ast(ast))
        {
            if (ast.children().at(1ULL).token().name() != "QF_UFBV" && ast.children().at(1ULL).token().name()
                    != "QF_BV")
            {
                error = msgstream() << "ERROR[" << ast.children().at(1ULL).token().line() << ":" << ast.children().at(1ULL).token().column() << "] : "
                                    << "Unsupported logic '" << ast.children().at(1ULL).token().name() << "'.";
                return {};
            }
        }
        else if (is_declare_fun_smtlib2_ast(ast))
        {
            if (ast.children().at(1ULL).token().name() != "eval")
            {
                symbol const  ufs = build_ufunction_symbol_from_smtlib2_ast(ast,error);
                if (!error.empty())
                    return {};
                if (ufunctions.count(symbol_name(ufs)) != 0ULL)
                {
                    error = msgstream() << "ERROR[" << ast.children().at(1ULL).token().line() << ":" << ast.children().at(1ULL).token().column() << "] : "
                                        << "The unsupported function symbol was already declared.";
                    return {};
                }
                ufunctions.insert({symbol_name(ufs),ufs});
            }
        }
        else if (is_assert_smtlib2_ast(ast))
        {
            conjuncts.push_back(
                    build_expression_from_smtlib2_ast(
                                is_assert_eval_smtlib2_ast(ast) ?
                                        ast.children().at(1ULL).children().at(1ULL) :
                                        ast.children().at(1ULL),
                                ufunctions,
                                error
                                )
                    );
            if (!error.empty())
                return {};
        }
        else if (!is_ignored_smtlib2_ast(ast))
        {
            error = msgstream() << "ERROR[" << ast.token().line() << ":" << ast.token().column() << "] : "
                                << "Unsupported expression '" << detail::to_string(ast) << "'.";
            return {};
        }
    }
    if (conjuncts.empty())
    {
        error = msgstream() << "ERROR : There is no expression to load.";
        return {};
    }

    expression const  result = to_conjunction(conjuncts);

    return result;
}

std::istream&  operator >>(std::istream&  istr, expression&  e)
{
    std::string  error;
    e = load_in_smtlib2_format(istr,error);
    if (!error.empty())
        throw std::runtime_error(error);
    return istr;
}


}
