#ifndef DYNAMIC_LIB_THREE_FUNCTIONS_HPP_INCLUDED
#   define DYNAMIC_LIB_THREE_FUNCTIONS_HPP_INCLUDED

#   include <import_export.hpp>

namespace dynamic_lib_three {


DIMPORT extern int glob_three_var;

DIMPORT extern int version();
DIMPORT extern double value();
DIMPORT extern int gvar();


}

#endif
