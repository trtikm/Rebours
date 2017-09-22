#include <rebours/bitvectors/expression_io.hpp>
#include <rebours/bitvectors/expression_algo.hpp>
#include <rebours/utility/invariants.hpp>
#include <rebours/utility/msgstream.hpp>
#include <unordered_set>
#include <unordered_map>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <iostream>


namespace bv { namespace detail {


std::unordered_map<std::string,std::string> const&  operators_outside_QF_UFBV()
{
    static std::unordered_map<std::string,std::string> const u {
            { "#f32f64", "cast_f32f64" },
            { "#f64f32", "cast_f64f32" },

            { "#s32f32", "cast_s32f32" },
            { "#s64f64", "cast_s64f64" },

            { "#u32f32", "cast_u32f32" },
            { "#u64f64", "cast_u64f64" },

            { "#f32s32", "cast_f32s32" },
            { "#f64s64", "cast_f64s64" },

            { "#f32u32", "cast_f32u32" },
            { "#f64u64", "cast_f64u64" },

            { "+f32", "add_f32" },
            { "+f64", "add_f64" },

            { "-f32", "sub_f32" },
            { "-f64", "sub_f64" },

            { "*f32", "mul_f32" },
            { "*f64", "mul_f64" },

            { "/f32", "div_f32" },
            { "/f64", "div_f64" },

            { "%f32", "rem_f32" },
            { "%f64", "rem_f64" },

            { "<f32", "less_than_f32" },
            { "<f64", "less_than_f64" },

            { "=f32", "equal_f32" },
            { "=f64", "equal_f64" },
            };
    return u;
}

std::unordered_map<std::string,std::string> const&  operators_to_QF_UFBV_names()
{
    static std::unordered_map<std::string,std::string> const  names_table {
            { "&&", "and" },
            { "||", "or" },
            { "!", "not" },
            { "A", "forall" },

            { "#i64i32", "(_ extract 31 0)" },
            { "#i32i16", "(_ extract 15 0)" },
            { "#i16i8", "(_ extract 7 0)" },

            { "#s8s16", "(_ sign_extend 8)" },
            { "#s16s32", "(_ sign_extend 16)" },
            { "#s32s64", "(_ sign_extend 32)" },

            { "#u8u16", "(_ zero_extend 8)" },
            { "#u16u32", "(_ zero_extend 16)" },
            { "#u32u64", "(_ zero_extend 32)" },

            { "+i8", "bvadd" },
            { "+i16", "bvadd" },
            { "+i32", "bvadd" },
            { "+i64", "bvadd" },

            { "-i8", "bvsub" },
            { "-i16", "bvsub" },
            { "-i32", "bvsub" },
            { "-i64", "bvsub" },

            { "*i8", "bvmul" },
            { "*i16", "bvmul" },
            { "*i32", "bvmul" },
            { "*i64", "bvmul" },

            { "/s8", "bvsdiv" },
            { "/s16", "bvsdiv" },
            { "/s32", "bvsdiv" },
            { "/s64", "bvsdiv" },

            { "/u8", "bvudiv" },
            { "/u16", "bvudiv" },
            { "/u32", "bvudiv" },
            { "/u64", "bvudiv" },

            { "%s8", "bvsrem" },
            { "%s16", "bvsrem" },
            { "%s32", "bvsrem" },
            { "%s64", "bvsrem" },

            { "%u8", "bvurem" },
            { "%u16", "bvurem" },
            { "%u32", "bvurem" },
            { "%u64", "bvurem" },

            { "<<i8", "bvshl" },
            { "<<i16", "bvshl" },
            { "<<i32", "bvshl" },
            { "<<i64", "bvshl" },

            { ">>s8", "bvashr" },
            { ">>s16", "bvashr" },
            { ">>s32", "bvashr" },
            { ">>s64", "bvashr" },

            { ">>u8", "bvlshr" },
            { ">>u16", "bvlshr" },
            { ">>u32", "bvlshr" },
            { ">>u64", "bvlshr" },

            { "<<<i8", "(_ rotate_left 1)" },
            { "<<<i16", "(_ rotate_left 1)" },
            { "<<<i32", "(_ rotate_left 1)" },
            { "<<<i64", "(_ rotate_left 1)" },

            { ">>>i8", "(_ rotate_right 1)" },
            { ">>>i16", "(_ rotate_right 1)" },
            { ">>>i32", "(_ rotate_right 1)" },
            { ">>>i64", "(_ rotate_right 1)" },

            { "&i8", "bvand" },
            { "&i16", "bvand" },
            { "&i32", "bvand" },
            { "&i64", "bvand" },

            { "|i8", "bvor" },
            { "|i16", "bvor" },
            { "|i32", "bvor" },
            { "|i64", "bvor" },

            { "^i8", "bvxor" },
            { "^i16", "bvxor" },
            { "^i32", "bvxor" },
            { "^i64", "bvxor" },

            { "+++", "concat" },

            { "<s8", "bvslt" },
            { "<s16", "bvslt" },
            { "<s32", "bvslt" },
            { "<s64", "bvslt" },

            { "<s8", "bvslt" },
            { "<s16", "bvslt" },
            { "<s32", "bvslt" },
            { "<s64", "bvslt" },

            { "us8", "bvult" },
            { "us16", "bvult" },
            { "us32", "bvult" },
            { "us64", "bvult" },

            { "=i8", "=" },
            { "=i16", "=" },
            { "=i32", "=" },
            { "=i64", "=" },
            };
    return names_table;
}


}}

namespace bv { namespace {


bool  is_unsupported_operator(symbol const  s)
{
    return detail::operators_outside_QF_UFBV().count(symbol_name(s)) != 0ULL;
}


void  save_uninterpreted_symbol_decl_in_smtlib2_format(std::ostream&  ostr, symbol const  s)
{
    ASSUMPTION(!symbol_is_interpreted(s));
    ostr << "(declare-fun " << symbol_name(s) << " (";
    for (uint64_t  i = 0ULL; i < symbol_num_parameters(s); ++i)
        ostr << "(_ BitVec " << symbol_num_bits_of_parameter(s,i) << ")" << (i+1ULL < symbol_num_parameters(s) ? " " : "");
    ostr << ") (_ BitVec " << symbol_num_bits_of_return_value(s) << "))\n";
}

void  save_unsupported_symbol_decl_in_smtlib2_format(std::ostream&  ostr, symbol const  s)
{
    auto const  it = detail::operators_outside_QF_UFBV().find(symbol_name(s));
    ASSUMPTION(it != detail::operators_outside_QF_UFBV().cend());
    ostr << "(declare-fun " << it->second << " (";
    for (uint64_t  i = 0ULL; i < symbol_num_parameters(s); ++i)
        ostr << "(_ BitVec " << symbol_num_bits_of_parameter(s,i) << ")" << (i+1ULL < symbol_num_parameters(s) ? " " : "");
    ostr << ") (_ BitVec " << symbol_num_bits_of_return_value(s) << "))\n";
}

void  save_symbol_in_smtlib2_format(std::ostream&  ostr, symbol const  s)
{
    if (symbol_num_parameters(s) == 0ULL)
    {
        if (symbol_is_interpreted_constant(s))
            ostr << "#" << symbol_name(s).substr(1ULL);
        else if (symbol_num_bits_of_return_value(s) == 1ULL)
        {
            if (symbol_name(s) == "tt")
                ostr << "(= #b0 #b0)";
            else if (symbol_name(s) == "ff")
                ostr << "(= #b0 #b1)";
            else
                UNREACHABLE();
        }
        else
            ostr << symbol_name(s);
    }
    else
    {
        auto  it = detail::operators_to_QF_UFBV_names().find(symbol_name(s));
        if (it != detail::operators_to_QF_UFBV_names().cend())
        {
            ostr << it->second;
            return;
        }
        it = detail::operators_outside_QF_UFBV().find(symbol_name(s));
        if (it != detail::operators_outside_QF_UFBV().cend())
        {
            ostr << it->second;
            return;
        }
        ostr << symbol_name(s);
    }
}

void  save_in_smtlib2_format(std::ostream&  ostr, expression const  e, std::string const&  shift, std::string const&  tab)
{
    std::string const  shift_ex = shift + tab;
    if (num_arguments(e) == 0ULL)
    {
        ostr << shift;
        save_symbol_in_smtlib2_format(ostr,get_symbol(e));
        ostr << "\n";
    }
    else
    {
        ostr << shift << "(";
        save_symbol_in_smtlib2_format(ostr,get_symbol(e));
        ostr << "\n";
        for (uint64_t  i = 0ULL; i < num_arguments(e); ++i)
            save_in_smtlib2_format(ostr,argument(e,i),shift_ex,tab);
        ostr << shift_ex << ")\n";
    }
}


}}

namespace bv {


void  save_in_smtlib2_format(std::ostream&  ostr, expression const  e)
{
    ostr << "(set-logic QF_UFBV)\n\n";

    std::unordered_set<symbol,symbol::hash> uninterpreted;
    find_unintepreted_symbols(e,uninterpreted);
    for (symbol s : uninterpreted)
        save_uninterpreted_symbol_decl_in_smtlib2_format(ostr,s);

    std::unordered_set<symbol,symbol::hash>  unsupported;
    find_symbols(e,&is_unsupported_operator,unsupported);
    for (symbol s : unsupported)
        save_unsupported_symbol_decl_in_smtlib2_format(ostr,s);

    if (is_formula(e))
    {
        ostr << (uninterpreted.empty() && unsupported.empty() ? "" : "\n") << "(assert\n";
        save_in_smtlib2_format(ostr,e,"    ","    ");
        ostr << "    )\n\n";
    }
    else
    {
        ostr << "(declare-fun eval ((_ BitVec " << num_bits_of_return_value(e) << ")) Bool)\n\n";
        ostr << "(assert (eval\n";
        save_in_smtlib2_format(ostr,e,"    ","    ");
        ostr << "    ))\n\n";
    }
    //ostr << "\n(exit)\n";
}


}
