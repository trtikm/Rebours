#ifndef ASSUMPTIONS_HPP_INCLUDED
#   define ASSUMPTIONS_HPP_INCLUDED

#   ifndef DISABLE_ASSUMPTION_CHECKING
#       include <rebours/utility/fail_message.hpp>
#       include <stdexcept>
#       include <string>
        struct assumption_failure : public std::logic_error {
            explicit assumption_failure(std::string const& msg) : std::logic_error(msg) {}
        };
#       define ASSUMPTION(C) do { if (!(C)) { throw assumption_failure(FAIL_MSG("Assumption failure.")); } } while (false)
#   else
#       define ASSUMPTION(C) {}
#   endif

#endif
