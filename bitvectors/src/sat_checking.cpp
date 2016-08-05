#include <rebours/bitvectors/sat_checking.hpp>
#include <vector>
#include <map>
#include <mutex>
#include <thread>

namespace bv { namespace detail {


extern void  is_satisfiable_z3(expression const  e, uint32_t const  timeout_milliseconds,
                               sat_result&  output, sat_engine* const  fastest_respondent_ptr,
                               std::mutex& output_mutex);
extern void  is_satisfiable_boolector(expression const  e, uint32_t const  timeout_milliseconds,
                                      sat_result&  output, sat_engine* const  fastest_respondent_ptr,
                                      std::mutex& output_mutex);
extern void  is_satisfiable_mathsat5(expression const  e, uint32_t const  timeout_milliseconds,
                                     sat_result&  output, sat_engine* const  fastest_respondent_ptr,
                                     std::mutex& output_mutex);


extern void  get_model_if_satisfiable_z3(expression const  e, uint32_t const  timeout_milliseconds,
                                         sat_result&  output_state, sat_model&  output_model,
                                         sat_engine* const  fastest_respondent_ptr, std::mutex&  output_mutex);
extern void  get_model_if_satisfiable_boolector(expression const  e, uint32_t const  timeout_milliseconds,
                                                sat_result&  output_state, sat_model&  output_model,
                                                sat_engine* const  fastest_respondent_ptr, std::mutex&  output_mutex);
extern void  get_model_if_satisfiable_mathsat5(expression const  e, uint32_t const  timeout_milliseconds,
                                               sat_result&  output_state, sat_model&  output_model,
                                               sat_engine* const  fastest_respondent_ptr, std::mutex&  output_mutex);

}}

namespace bv {


sat_result  is_satisfiable(expression const  e, uint32_t const  timeout_milliseconds, sat_engine* const  fastest_respondent_ptr)
{
    return is_satisfiable(e,timeout_milliseconds,{sat_engine::Z3,sat_engine::BOOLECTOR,sat_engine::MATHSAT5},fastest_respondent_ptr);
}

sat_result  is_satisfiable(expression const  e, uint32_t const  timeout_milliseconds, std::set<sat_engine> const&  engines,
                           sat_engine* const  fastest_respondent_ptr)
{
    static std::map<sat_engine,void(*)(expression,uint32_t,sat_result&,sat_engine*,std::mutex&)> engines_map {
            { sat_engine::Z3, &detail::is_satisfiable_z3 },
            { sat_engine::BOOLECTOR, &detail::is_satisfiable_boolector },
            { sat_engine::MATHSAT5, &detail::is_satisfiable_mathsat5 },
            };

    ASSUMPTION(e.operator bool());
    ASSUMPTION(!engines.empty());

    sat_result  result = sat_result::FAIL;
    std::mutex  result_mutex;

    uint32_t  thread_counter = 1U;
    std::vector<std::thread>  threads;
    for (auto engine : engines)
    {
        auto const  it = engines_map.find(engine);
        ASSUMPTION(it != engines_map.cend());
        ASSUMPTION(thread_counter <= engines.size());
        if (thread_counter < engines.size())
            threads.push_back(std::thread(it->second,e,timeout_milliseconds,std::ref(result),fastest_respondent_ptr,std::ref(result_mutex)));
        else
            it->second(e,timeout_milliseconds,result,fastest_respondent_ptr,result_mutex);
    }
    for (std::thread& thread : threads)
        thread.join();

    return result;
}


values_of_expression_in_model::values_of_expression_in_model(std::shared_ptr<sat_model_cases const> const  cases)
    : m_cases(cases)
{
    ASSUMPTION(!cases->empty());
    ASSUMPTION(
            [](sat_model_cases const&  cases) {
                    for (auto const&  elem : cases)
                        if (!elem.second.operator bool())
                            return false;
                    return true;
                    }(*cases)
            );
}

std::vector<expression> const&  values_of_expression_in_model::args_of_case(uint64_t const  case_index) const
{
    ASSUMPTION(case_index < num_cases());
    return m_cases->at(case_index).first;
}

expression  values_of_expression_in_model::value_of_case(uint64_t const  case_index) const
{
    ASSUMPTION(case_index < num_cases());
    return m_cases->at(case_index).second;
}


std::pair<sat_result,sat_model>  get_model_if_satisfiable(expression const  e, uint32_t const  timeout_milliseconds,
                                                          sat_engine* const  fastest_respondent_ptr)
{
    return get_model_if_satisfiable(e,timeout_milliseconds,{sat_engine::Z3,sat_engine::BOOLECTOR,sat_engine::MATHSAT5},fastest_respondent_ptr);
}

std::pair<sat_result,sat_model>  get_model_if_satisfiable(expression const  e, uint32_t const  timeout_milliseconds,
                                                          std::set<sat_engine> const&  engines, sat_engine* const  fastest_respondent_ptr)
{
    static std::map<sat_engine,void(*)(expression,uint32_t,sat_result&,sat_model&,sat_engine*,std::mutex&)> engines_map {
            { sat_engine::Z3, &detail::get_model_if_satisfiable_z3 },
            { sat_engine::BOOLECTOR, &detail::get_model_if_satisfiable_boolector },
            { sat_engine::MATHSAT5, &detail::get_model_if_satisfiable_mathsat5 },
            };

    ASSUMPTION(e.operator bool());
    ASSUMPTION(!engines.empty());

    sat_result  state = sat_result::FAIL;
    sat_model   model;
    std::mutex  result_mutex;

    uint32_t  thread_counter = 1U;
    std::vector<std::thread>  threads;
    for (auto engine : engines)
    {
        auto const  it = engines_map.find(engine);
        ASSUMPTION(it != engines_map.cend());
        ASSUMPTION(thread_counter <= engines.size());
        if (thread_counter < engines.size())
            threads.push_back(std::thread(it->second,e,timeout_milliseconds,std::ref(state),std::ref(model),fastest_respondent_ptr,std::ref(result_mutex)));
        else
            it->second(e,timeout_milliseconds,state,model,fastest_respondent_ptr,result_mutex);
    }
    for (std::thread& thread : threads)
        thread.join();

    return {state,model};
}

std::string  to_string(sat_result const  value)
{
    switch (value)
    {
    case sat_result::FAIL: return "FAIL";
    case sat_result::YES: return "YES";
    case sat_result::NO: return "NO";
    default: UNREACHABLE();
    }
}

std::string  to_string(sat_engine const  value)
{
    switch (value)
    {
    case sat_engine::Z3: return "Z3";
    case sat_engine::BOOLECTOR: return "BOOLECTOR";
    case sat_engine::MATHSAT5: return "MATHSAT5";
    default: UNREACHABLE();
    }
}


}
