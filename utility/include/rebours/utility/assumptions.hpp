#ifndef ASSUMPTIONS_HPP_INCLUDED
#   define ASSUMPTIONS_HPP_INCLUDED

#   ifndef DISABLE_ASSUMPTION_CHECKING
#       include <cassert>
#       define ASSUMPTION(C) assert(C)
#   else
#       define ASSUMPTION(C) {}
#   endif

#endif
