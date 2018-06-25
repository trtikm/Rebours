#include <import_export.hpp>
#include <dynamic_lib_seven/functions.hpp>

DIMPORT extern int glob_exe_var;

namespace dynamic_lib_three {


DEXPORT int glob_three_var = 113;


DEXPORT int version()
{
    return 3 + 100 * dynamic_lib_seven::version();
}

DEXPORT double value()
{
    return 3.3 + 100.0 * dynamic_lib_seven::value();
}

DEXPORT int gvar()
{
    return DLL_USE_EXE_VAR(glob_exe_var +) glob_three_var + dynamic_lib_seven::glob_seven_var;
}


}
