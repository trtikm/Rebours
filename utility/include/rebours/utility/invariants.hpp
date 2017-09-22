#ifndef INVARIANTS_HPP_INCLUDED
#   define INVARIANTS_HPP_INCLUDED

#   ifndef DISABLE_INVARIANT_CHECKING
#       include <rebours/utility/fail_message.hpp>
#       include <stdexcept>
#       include <string>
        struct invariant_failure : public std::logic_error {
            explicit invariant_failure(std::string const& msg) : std::logic_error(msg) {}
        };
#       define INVARIANT(C) do { if (!(C)) { throw invariant_failure(FAIL_MSG("Invariant failure.")); } } while (false)
#       define UNREACHABLE() do { throw invariant_failure(FAIL_MSG("Unreachable location reached.")); } while (false)
#   else
#       define INVARIANT(C) {}
#       define UNREACHABLE() throw 0
#   endif

#endif
