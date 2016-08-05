#include <import_export.hpp>
#include <dynamic_lib_four/functions.hpp>
#include <dynamic_lib_five/functions.hpp>
#include <dynamic_lib_seven/functions.hpp>

DIMPORT extern int glob_exe_var;

namespace dynamic_lib_two {


DEXPORT int glob_two_var = 112;


DEXPORT int version()
{
    return 2 + 100 * dynamic_lib_four::version() + 1000 * dynamic_lib_five::version() + 10000 * dynamic_lib_seven::version();
}

DEXPORT double value()
{
    return 2.2 + 100.0 * dynamic_lib_four::value() + 1000.0 * dynamic_lib_five::value() + 10000.0 * dynamic_lib_seven::value();
}

DEXPORT int gvar()
{
    return DLL_USE_EXE_VAR(glob_exe_var +) glob_two_var + dynamic_lib_four::glob_four_var + dynamic_lib_five::glob_five_var + dynamic_lib_seven::glob_seven_var;
}


}
