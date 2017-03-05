#ifndef TEST_HPP_INCLUDED
#   define TETS_HPP_INCLUDED

#   include <cassert>
#   include <stdexcept>

#   define TEST_SUCCESS(C) do { if (!(C)) { assert(C); throw std::logic_error("TEST_SUCCESS has failed."); } } while (false)
#   define TEST_FAILURE(C) do { if (C) { assert(!(C)); throw std::logic_error("TEST_FAILURE has failed."); } } while (false)

#endif
