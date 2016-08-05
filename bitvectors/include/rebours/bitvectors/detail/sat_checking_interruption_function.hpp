#ifndef REBOURS_BITVECTORS_DETAIL_SAT_CHECKING_INTERRUPTION_FUNCTION_HPP_INCLUDED
#   define REBOURS_BITVECTORS_DETAIL_SAT_CHECKING_INTERRUPTION_FUNCTION_HPP_INCLUDED

#   include <rebours/bitvectors/sat_checking.hpp>
#   include <functional>
#   include <mutex>

namespace bv { namespace detail {


std::function<bool()>  get_sat_checking_interruption_function(uint32_t const timeout_in_milliseconds,
                                                              sat_result const&  output,  std::mutex& output_mutex);


}}

#endif
