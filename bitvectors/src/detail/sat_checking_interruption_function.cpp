#include <rebours/bitvectors/detail/sat_checking_interruption_function.hpp>

namespace bv { namespace detail { namespace {


bool  sat_checking_interruption_function(std::chrono::system_clock::time_point const  start_time,
                                         uint32_t const timeout_in_milliseconds,
                                         sat_result const&  output,  std::mutex& output_mutex)
{
    std::chrono::system_clock::time_point const  current_time = std::chrono::high_resolution_clock::now();
    double const duration = std::chrono::duration<double, std::milli>(current_time - start_time).count();
    if (duration >= double(timeout_in_milliseconds))
        return true;

    {
        std::lock_guard<std::mutex> const  lock(output_mutex);
        if (output != sat_result::FAIL)
            return true;
    }

    return false;
}


}}}

namespace bv { namespace detail {


std::function<bool()>  get_sat_checking_interruption_function(uint32_t const timeout_in_milliseconds,
                                                              sat_result const&  output,  std::mutex& output_mutex)
{
    return std::bind(&sat_checking_interruption_function,
                     std::chrono::high_resolution_clock::now(),
                     timeout_in_milliseconds,
                     std::cref(output),
                     std::ref(output_mutex)
                     );
}


}}
