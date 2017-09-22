#include <rebours/bitvectors/sat_checking.hpp>
#include <rebours/bitvectors/expression_io.hpp>
#include <rebours/bitvectors/detail/execute_shell_command_with_async_pipe.hpp>
#include <rebours/bitvectors/detail/sat_checking_interruption_function.hpp>
#include <rebours/utility/unique_handle.hpp>
#include <rebours/utility/file_utils.hpp>
#include <mutex>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <algorithm>

#define XSTR(s) STR(s)
#define STR(s) #s

namespace bv { namespace detail { namespace {


std::string  get_expression_file_path_prefix()
{
    return "./bitvectors_sat_engine_z3_temp_file_number_";
}

struct temp_files_clean_up
{
    ~temp_files_clean_up()
    {
        std::vector<std::string>  pathnames;
        fileutl::enumerate_files_by_pattern(get_expression_file_path_prefix() + "*",pathnames);
        for (auto const& p : pathnames)
            fileutl::delete_file(p);
    }
} instance;

std::string  get_expression_file_path(unique_handle const&  id)
{
    std::stringstream  sstr;
    sstr << get_expression_file_path_prefix() << id;
    return sstr.str();
}

std::string  get_path_to_z3_binary()
{
    return std::string(XSTR(Z3_ROOT)) + "/bin/z3";
}

std::string  get_z3_exec_command(uint32_t const  timeout_milliseconds, std::string const&  smtlib2_file_path_name)
{
    std::stringstream  ostr;
    ostr << get_path_to_z3_binary()
         << " -t:" << timeout_milliseconds
         << " -T:" << (timeout_milliseconds / 1000U) + 1U
         << " -smt2 -file:\"" << smtlib2_file_path_name << "\""
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
            *fastest_respondent_ptr = sat_engine::Z3;
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

bool  parse_model_value(symbol const&  ufn_symbol, std::vector<std::string> const&  vars,
                        detail::smtlib2_ast_node const&  ast,
                        std::unordered_map<std::string,expression>&  bindings,
                        sat_model&  output)
{
    if (ast.children().empty())
    {
        std::vector<expression> args;
        for (auto const&  var : vars)
        {
            auto const  it = bindings.find(var);
            args.push_back(it == bindings.cend() ? expression{} : it->second);
        }

        std::string  error;
        expression const  value = detail::build_expression_from_smtlib2_ast(ast,{},error);
        if (!error.empty())
        {
            std::cerr << error;
            return false;
        }

        auto const  it = output.find(ufn_symbol);
        if (it == output.cend())
            output.insert({ufn_symbol,sat_model_cases_ptr(new values_of_expression_in_model::sat_model_cases({{args,value}}))});
        else
        {
            values_of_expression_in_model::sat_model_cases&  cases =
                    *std::const_pointer_cast<values_of_expression_in_model::sat_model_cases>(it->second.cases());
            cases.push_back({args,value});
        }

        return true;
    }
    else if (ast.children().size() == 4ULL &&
             ast.children().at(0ULL).token().name() == "ite")
    {
        if (ast.children().at(1ULL).children().size() != 3ULL ||
            ast.children().at(1ULL).children().at(0ULL).token().name() != "=" ||
            std::find(vars.cbegin(),vars.cend(),ast.children().at(1ULL).children().at(1ULL).token().name()) == vars.cend() ||
            bindings.count(ast.children().at(1ULL).children().at(1ULL).token().name()) != 0ULL)
        {
            std::cerr << "ERROR[" << ast.children().at(1ULL).token().line() << ":" << ast.children().at(1ULL).token().column() << "]: "
                         "Unexpected structure of the condition of the 'ite' expression.";
            return false;
        }

        std::string const  var_name = ast.children().at(1ULL).children().at(1ULL).token().name();
        std::string  error;
        expression const  var_binding = detail::build_expression_from_smtlib2_ast(ast.children().at(1ULL).children().at(2ULL),{},error);
        if (!error.empty())
        {
            std::cerr << error;
            return false;
        }
        bindings.insert({var_name,var_binding});
        if (!parse_model_value(ufn_symbol,vars,ast.children().at(2ULL),bindings,output))
            return false;
        bindings.erase(var_name);
        return parse_model_value(ufn_symbol,vars,ast.children().at(3ULL),bindings,output);
    }
    std::cerr << "ERROR[" << ast.token().line() << ":" << ast.token().column() << "]: "
                 "There is expected either a numeric constant or an 'ite' expression.";
    return false;
}

bool  parse_model(detail::smtlib2_ast_node const&  ast, sat_model&  output)
{
    if (ast.children().size() == 5ULL &&
        ast.children().at(0ULL).token().name() == "define-fun" &&
        !ast.children().at(1ULL).token().name().empty())
    {
        std::string const  name = ast.children().at(1ULL).token().name();

        std::vector<std::string> vars;
        std::vector<uint64_t> params_num_bits;
        for (auto const& param : ast.children().at(2ULL).children())
        {
            if (param.children().size() != 2ULL ||
                param.children().at(0ULL).token().name().empty() ||
                param.children().at(1ULL).children().size() != 3ULL)
            {
                std::cerr << "ERROR[" << param.token().line() << ":" << param.token().column() << "]: "
                             "Wrong declaration of the parameter of the function symbol.";
                return false;
            }

            std::string  error;
            uint64_t  num_bits = compute_numbits_from_smtlib2_type_ast(param.children().at(1ULL),error);
            if (!error.empty())
            {
                std::cerr << error;
                return false;
            }

            vars.push_back(param.children().at(0ULL).token().name());
            params_num_bits.push_back(num_bits);
        }

        std::string  error;
        uint64_t const num_ret_bits = detail::compute_numbits_from_smtlib2_type_ast(ast.children().at(3ULL),error);
        if (!error.empty())
        {
            std::cerr << error;
            return false;
        }

        symbol const s = make_symbol_of_unintepreted_function(name,num_ret_bits,params_num_bits);
        std::unordered_map<std::string,expression>  bindings;

        return parse_model_value(s,vars,ast.children().at(4ULL),bindings,output);
    }
    else if (ast.children().size() == 2ULL)
        return parse_model(ast.children().at(0ULL),output) && parse_model(ast.children().at(1ULL),output);
    return false;
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

    if (asts.size() != 1ULL || asts.at(0ULL).children().empty() ||
        asts.at(0ULL).children().at(0ULL).token().name() != "model")
    {
        std::cerr << "ERROR: The model does not have the format '(model ...)'.";
        return false;
    }

    for (uint64_t  i = 1ULL; i < asts.at(0ULL).children().size(); ++i)
        if (!parse_model(asts.at(0ULL).children().at(i),output))
            return false;

    return true;
}


}}}

namespace bv { namespace detail {


void  is_satisfiable_z3(expression const  e, uint32_t const  timeout_milliseconds,
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
                get_z3_exec_command(timeout_milliseconds,smtlib2_file_path_name),
                get_sat_checking_interruption_function(timeout_milliseconds,output,output_mutex),
                answer))
        return;
    bool is_sat;
    if (!parse_sat_result(answer,is_sat))
        return;

    write_sat_result(is_sat,output,fastest_respondent_ptr,output_mutex);
}


void  get_model_if_satisfiable_z3(expression const  e, uint32_t const  timeout_milliseconds,
                                  sat_result&  output_state, sat_model&  output_model,
                                  sat_engine* const  fastest_respondent_ptr, std::mutex&  output_mutex)
{
    unique_handle const  smtlib2_file_id;
    std::string const  smtlib2_file_path_name = get_expression_file_path(smtlib2_file_id);
    {
        std::ofstream  expr_stream(smtlib2_file_path_name,std::ofstream::out);
        expr_stream << e;
        expr_stream << "\n(check-sat)\n(get-model)\n(exit)\n";
    }

    std::stringstream  answer;
    if (!execute_shell_command_with_async_pipe(
                get_z3_exec_command(timeout_milliseconds,smtlib2_file_path_name),
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
