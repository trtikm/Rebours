#include <rebours/bitvectors/sat_checking.hpp>
#include <rebours/bitvectors/expression_io.hpp>
#include <rebours/bitvectors/detail/execute_shell_command_with_async_pipe.hpp>
#include <rebours/bitvectors/detail/sat_checking_interruption_function.hpp>
#include <rebours/bitvectors/detail/unique_handles.hpp>
#include <rebours/bitvectors/detail/file_utils.hpp>
#include <rebours/bitvectors/endian.hpp>
#include <mutex>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#define XSTR(s) STR(s)
#define STR(s) #s

namespace bv { namespace detail { namespace {


std::string  get_expression_file_path_prefix()
{
    return "./bitvectors_sat_engine_mathsat5_temp_file_number_";
}

struct temp_files_clean_up
{
    ~temp_files_clean_up()
    {
        std::vector<std::string>  pathnames;
        enumerate_files_by_pattern(get_expression_file_path_prefix() + "*",pathnames);
        for (auto const& p : pathnames)
            delete_file(p);
    }
} instance;

std::string  get_expression_file_path(unique_handle const&  id)
{
    std::stringstream  sstr;
    sstr << get_expression_file_path_prefix() << id;
    return sstr.str();
}

std::string  get_path_to_mathsat5_binary()
{
    return std::string(XSTR(MATHSAT5_ROOT)) + "/bin/mathsat";
}

std::string  get_mathsat5_exec_command(uint32_t const  timeout_milliseconds, bool const  get_model,
                                       std::string const&  smtlib2_file_path_name)
{
    (void)timeout_milliseconds; // No command line option for specifying a timeout.
    std::stringstream  ostr;
    ostr << get_path_to_mathsat5_binary()
         << (get_model ? " -model" : "")
         << " -input=smt2 \"" << smtlib2_file_path_name << "\""
         ;
    return ostr.str();
}

void  write_sat_result(bool const state, sat_result&  output, sat_engine* const  fastest_respondent_ptr,
                       std::mutex& output_mutex)
{
    std::lock_guard<std::mutex> const  lock(output_mutex);
    if (output == sat_result::FAIL)
    {
        output = state ? sat_result::YES : sat_result::NO;
        if (fastest_respondent_ptr != nullptr)
            *fastest_respondent_ptr = sat_engine::Z3;
    }
}

void  write_sat_result(bool const state, sat_model const&  model, sat_result&  output_state,
                       sat_model&  output_model, sat_engine* const  fastest_respondent_ptr,
                       std::mutex& output_mutex)
{
    std::lock_guard<std::mutex> const  lock(output_mutex);
    if (output_state == sat_result::FAIL)
    {
        output_state = state ? sat_result::YES : sat_result::NO;
        output_model = model;
        if (fastest_respondent_ptr != nullptr)
            *fastest_respondent_ptr = sat_engine::MATHSAT5;
    }
}

bool  parse_sat_result(std::stringstream&  sstr, bool&  output)
{
    std::string  line;
    std::getline(sstr,line);
    if (line == "sat" || line == "SAT")
    {
        output = true;
        return true;
    }
    if (line == "unsat" || line == "UNSAT")
    {
        output = false;
        return true;
    }
    return false;
}

expression  parse_model_value(detail::smtlib2_ast_node const&  ast)
{
    if (ast.children().size() != 3ULL ||
        ast.children().at(0ULL).token().name() != "_" ||
        ast.children().at(1ULL).token().name().size() < 3ULL ||
        ast.children().at(1ULL).token().name().substr(0ULL,2ULL) != "bv" ||
        ast.children().at(2ULL).token().name().empty() ||
        !std::isdigit(ast.children().at(2ULL).token().name().front())
        )
    {
        std::cerr << "ERROR[" << ast.token().line() << ":" << ast.token().column() << "]: "
                     "Expected a bit-vector value in format (_ bvN M), where N and M are non-negative integers.";
        return {};
    }

    int64_t const  bit_width = std::atoi(ast.children().at(2ULL).token().name().c_str());
    int64_t const  value = std::atoi(ast.children().at(1ULL).token().name().substr(2ULL).c_str());

    switch (bit_width)
    {
    case 8ULL: return num<uint8_t>(value);
    case 16ULL: return num<uint16_t>(value);
    case 32ULL: return num<uint32_t>(value);
    case 64ULL: return num<uint64_t>(value);
    default:
        std::cerr << "ERROR[" << ast.children().at(2ULL).token().line() << ":" << ast.children().at(2ULL).token().column() << "]: "
                     "Wrong bit-with. Allowed values are 8, 16, 32, and 64.";
        return {};
    }
}

bool  parse_model_symbol_application(detail::smtlib2_ast_node const&  ast, sat_model&  output)
{
    if (ast.children().size() != 2ULL)
    {
        std::cerr << "ERROR[" << ast.token().line() << ":" << ast.token().column() << "]: "
                     "Expected a pair (<application-of-a-symbol> <result-of-the-application>).";
        return false;
    }

    expression  value = parse_model_value(ast.children().at(1ULL));
    if (!value.operator bool())
        return false;

    symbol  key;
    std::vector<expression>  args;
    if (ast.children().at(0ULL).children().empty())
        key = make_symbol_of_unintepreted_function(ast.children().at(0ULL).token().name(),num_bits_of_return_value(value),{});
    else
    {
        for (uint64_t  i = 1ULL; i < ast.children().at(0ULL).children().size(); ++i)
        {
            expression  arg = parse_model_value(ast.children().at(0ULL).children().at(i));
            if (!arg.operator bool())
                return false;
            args.push_back(arg);
        }
        key = make_symbol_of_unintepreted_function(ast.children().at(0ULL).children().at(0ULL).token().name(),num_bits_of_return_value(value),
                                                   to_ret_numbits(args));
    }

    auto const  it = output.find(key);
    if (it == output.cend())
        output.insert({key,sat_model_cases_ptr(new values_of_expression_in_model::sat_model_cases({{args,value}}))});
    else
    {
        values_of_expression_in_model::sat_model_cases&  cases =
                *std::const_pointer_cast<values_of_expression_in_model::sat_model_cases>(it->second.cases());
        cases.push_back({args,value});
    }

    return true;
}

bool  parse_model(std::stringstream&  sstr, sat_model&  output)
{
//std::cout << sstr.str() << "\n";
//std::cout.flush();
    std::string  error;
    std::vector<detail::smtlib2_token>  tokens;
    detail::tokenise_smtlib2_stream(sstr,tokens);
    std::vector<detail::smtlib2_ast_node>  asts;
    error = detail::tokenised_smtlib2_stream_to_ast(tokens,asts);
    if (!error.empty())
        return {};

    if (asts.size() != 1ULL || !asts.at(0ULL).token().name().empty())
    {
        std::cerr << "ERROR: The model does not have the format '( (...) (...) ...)'.";
        return false;
    }

    for (uint64_t  i = 0ULL; i < asts.at(0ULL).children().size(); ++i)
        if (!parse_model_symbol_application(asts.at(0ULL).children().at(i),output))
            return false;

    return true;
}


}}}

namespace bv { namespace detail {


void  is_satisfiable_mathsat5(expression const  e, uint32_t const  timeout_milliseconds,
                              sat_result&  output, sat_engine* const  fastest_respondent_ptr,
                              std::mutex& output_mutex)
{
    unique_handle const  smtlib2_file_id;
    std::string const  smtlib2_file_path_name = get_expression_file_path(smtlib2_file_id);
    {
        std::ofstream  expr_stream(smtlib2_file_path_name,std::ofstream::out);
        expr_stream << e;
        expr_stream << "\n(check-sat)\n(exit)\n";
    }

    std::stringstream  answer;
    if (!execute_shell_command_with_async_pipe(
                get_mathsat5_exec_command(timeout_milliseconds,false,smtlib2_file_path_name),
                get_sat_checking_interruption_function(timeout_milliseconds,output,output_mutex),
                answer))
        return;
    bool is_sat;
    if (!parse_sat_result(answer,is_sat))
        return;

    write_sat_result(is_sat,output,fastest_respondent_ptr,output_mutex);
}


void  get_model_if_satisfiable_mathsat5(expression const  e, uint32_t const  timeout_milliseconds,
                                        sat_result&  output_state, sat_model&  output_model,
                                        sat_engine* const  fastest_respondent_ptr, std::mutex&  output_mutex)
{
    unique_handle const  smtlib2_file_id;
    std::string const  smtlib2_file_path_name = get_expression_file_path(smtlib2_file_id);
    {
        std::ofstream  expr_stream(smtlib2_file_path_name,std::ofstream::out);
        expr_stream << e;
        expr_stream << "\n(check-sat)\n(exit)\n";
    }

    std::stringstream  answer;
    if (!execute_shell_command_with_async_pipe(
                get_mathsat5_exec_command(timeout_milliseconds,true,smtlib2_file_path_name),
                get_sat_checking_interruption_function(timeout_milliseconds,output_state,output_mutex),
                answer))
        return;
    bool is_sat;
    if (!parse_sat_result(answer,is_sat))
        return;
    sat_model  model;
    if (!parse_model(answer,model))
        return;

    write_sat_result(is_sat,model,output_state,output_model,fastest_respondent_ptr,output_mutex);
}


}}
