#include <rebours/bitvectors/sat_checking.hpp>
#include <rebours/bitvectors/expression_io.hpp>
#include <rebours/bitvectors/detail/execute_shell_command_with_async_pipe.hpp>
#include <rebours/bitvectors/detail/sat_checking_interruption_function.hpp>
#include <rebours/bitvectors/detail/unique_handles.hpp>
#include <rebours/bitvectors/detail/file_utils.hpp>
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
    return "./bitvectors_sat_engine_boolector_temp_file_number_";
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

std::string  get_path_to_boolector_binary()
{
    return std::string(XSTR(BOOLECTOR_ROOT)) + "/boolector";
}

std::string  get_boolector_exec_command(uint32_t const  timeout_milliseconds, bool const  get_model,
                                        std::string const&  smtlib2_file_path_name)
{
    std::stringstream  ostr;
    ostr << get_path_to_boolector_binary()
         << " -t " << (timeout_milliseconds / 1000U) + 1U
         << (get_model ? " --model" : "")
         << " --hex --smt2 \"" << smtlib2_file_path_name << "\""
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
            *fastest_respondent_ptr = sat_engine::BOOLECTOR;
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
            *fastest_respondent_ptr = sat_engine::BOOLECTOR;
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


std::string  parse_token(std::string const& line, uint32_t&  index)
{
    while (index < line.size() && (line.at(index) == ' ' || line.at(index) == '\t'))
        ++index;
    std::string  output_token;
    while (index < line.size() && line.at(index) != ' ' && line.at(index) != '\t')
    {
        output_token.push_back(line.at(index));
        ++index;
    }
    return output_token;
}

bool  parse_model(std::stringstream&  sstr, sat_model&  output)
{
    while (!sstr.eof() && !sstr.bad())
    {
        std::string  line;
        std::getline(sstr,line);

        if (line.empty())
            continue;

        uint32_t  index = 0U;
        std::string const  var_name = parse_token(line,index);
        std::string const  value_name = parse_token(line,index);
        uint64_t  num_bits_of_value = value_name.size() * 4ULL;
        if (std::set<uint64_t>{8ULL,16ULL,32ULL,64ULL}.count(num_bits_of_value) == 0ULL)
            return false;

        symbol const  key = make_symbol_of_unintepreted_function(var_name,num_bits_of_value,{});
        expression const  value = mem(value_name);

        if (output.find(key) != output.cend())
            return false;

        output.insert(
            {key,sat_model_cases_ptr(new values_of_expression_in_model::sat_model_cases({{{},value}}))}
            );
    }
    return true;
}


}}}

namespace bv { namespace detail {


void  is_satisfiable_boolector(expression const  e, uint32_t const  timeout_milliseconds,
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
                get_boolector_exec_command(timeout_milliseconds,false,smtlib2_file_path_name),
                get_sat_checking_interruption_function(timeout_milliseconds,output,output_mutex),
                answer))
        return;
    bool is_sat;
    if (!parse_sat_result(answer,is_sat))
        return;

    write_sat_result(is_sat,output,fastest_respondent_ptr,output_mutex);
}


void  get_model_if_satisfiable_boolector(expression const  e, uint32_t const  timeout_milliseconds,
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
                get_boolector_exec_command(timeout_milliseconds,true,smtlib2_file_path_name),
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
