#include <import_export.hpp>
#include <dynamic_lib_five/functions.hpp>
#include <dynamic_lib_six/functions.hpp>
#include <iostream>

DIMPORT extern int glob_exe_var;

namespace dynamic_lib_four {


DEXPORT int glob_four_var = 114;


DEXPORT int version()
{
    std::cout << "(version = 4)\n";
    return 4 + 100 * dynamic_lib_five::version() + 1000 * dynamic_lib_six::version();
}

DEXPORT double value()
{
    std::cout << "(value = 4.4)\n";
    return 4.4 + 100.0 * dynamic_lib_five::value() + 1000.0 * dynamic_lib_six::value();
}

DEXPORT int gvar()
{
    return DLL_USE_EXE_VAR(glob_exe_var +) glob_four_var + dynamic_lib_five::glob_five_var + dynamic_lib_six::glob_six_var;
}


}
