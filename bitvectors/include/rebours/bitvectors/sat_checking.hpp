#ifndef REBOURS_BITVECTORS_SAT_CHECKING_HPP_INCLUDED
#   define REBOURS_BITVECTORS_SAT_CHECKING_HPP_INCLUDED

#   include <rebours/bitvectors/expression.hpp>
#   include <set>
#   include <string>
#   include <memory>
#   include <unordered_map>
#   include <utility>
#   include <cstdint>

namespace bv {


enum struct sat_result : uint8_t {
        FAIL = 0xffU,
        NO   = 0x00U,
        YES  = 0x01U,
};


enum struct sat_engine : uint8_t {
        NONE        = 0x00U,
        Z3          = 0x01U,
        BOOLECTOR   = 0x02U,
        MATHSAT5    = 0x03U,
};


sat_result  is_satisfiable(expression const  e, uint32_t const  timeout_milliseconds, sat_engine* const  fastest_respondent_ptr = nullptr);
sat_result  is_satisfiable(expression const  e, uint32_t const  timeout_milliseconds, std::set<sat_engine> const&  engines,
                           sat_engine* const  fastest_respondent_ptr = nullptr);


/**
 * It represents a set of all possible values of a symbol in a model of some formula. The values are expressed in a form
 * of cases. A case a pair. The first component is a list of arguments to be substituted for formal parameters of the symbol.
 * The second component is a value of the symbol when the arguments are substituted. Note that an argument of a case may
 * be the invalid expression. Then it means that the argument can be any possible value. Order of cases matter. If a given
 * case has undefined arguments, then the values of the case is related to all possible values to arguments of the case (both
 * valid and invalid) which do not match arguments of any preceeding case.
 */
struct values_of_expression_in_model
{
    using  sat_model_cases = std::vector< std::pair<std::vector<expression>,expression> >;

    values_of_expression_in_model(std::shared_ptr<sat_model_cases const> const  cases);

    uint64_t  num_cases() const noexcept { return m_cases->size(); }
    std::vector<expression> const&  args_of_case(uint64_t const  case_index) const;
    expression  value_of_case(uint64_t const  case_index) const;

    std::shared_ptr<sat_model_cases const>  cases() const noexcept { return m_cases; }

private:
    std::shared_ptr<sat_model_cases const>  m_cases;
};

using  sat_model = std::unordered_map<symbol,values_of_expression_in_model,symbol::hash>;
using  sat_model_cases_ptr = std::shared_ptr<values_of_expression_in_model::sat_model_cases const>;


std::pair<sat_result,sat_model>  get_model_if_satisfiable(expression const  e, uint32_t const  timeout_milliseconds,
                                                          sat_engine* const  fastest_respondent_ptr = nullptr);
std::pair<sat_result,sat_model>  get_model_if_satisfiable(expression const  e, uint32_t const  timeout_milliseconds, std::set<sat_engine> const&  engines,
                                                          sat_engine* const  fastest_respondent_ptr = nullptr);


std::string  to_string(sat_result const  value);
std::string  to_string(sat_engine const  value);


}

#endif
