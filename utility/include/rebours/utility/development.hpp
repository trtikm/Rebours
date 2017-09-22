#ifndef DEVELOPMENT_HPP_INCLUDED
#   define DEVELOPMENT_HPP_INCLUDED

#   include <rebours/utility/fail_message.hpp>
#   include <stdexcept>
#   include <string>

#   define NOT_IMPLEMENTED_YET() do { throw under_construction(FAIL_MSG("NOT IMPLEMENTED YET !")); } while (false)

struct under_construction : public std::logic_error {
    explicit under_construction(std::string const& msg) : std::logic_error(msg) {}
};

#endif
