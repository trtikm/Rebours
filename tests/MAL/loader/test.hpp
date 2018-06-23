#ifndef TEST_HPP_INCLUDED
#   define TEST_HPP_INCLUDED

#   include <cassert>

#   define TEST_SUCCESS(C) assert(C)
#   define TEST_FAILURE(C) assert(!(C))

#endif
