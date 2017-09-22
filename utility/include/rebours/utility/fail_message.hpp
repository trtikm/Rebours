#ifndef UTILITY_FAIL_MESSAGE_HPP_INCLUDED
#   define UTILITY_FAIL_MESSAGE_HPP_INCLUDED

#   include <string>

#   define FAIL_MSG(M) buildFailureMessage(__FILE__,__LINE__,__FUNCTION__,M)

std::string buildFailureMessage(char const* const file, int const line,
                                char const* const func, std::string const& msg);


#endif
