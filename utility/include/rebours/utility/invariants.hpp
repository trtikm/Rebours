#ifndef INVARIANTS_HPP_INCLUDED
#   define INVARIANTS_HPP_INCLUDED

#   ifndef DISABLE_INVARIANT_CHECKING
#       include <cassert>
#       define INVARIANT(C) assert(C)
#       define UNREACHABLE() do { assert(false); throw 0; } while (false)
#   else
#       define INVARIANT(C) {}
#       define UNREACHABLE() throw 0
#   endif

#endif
