#include <import_export.hpp>
#include <dynamic_lib_two/functions.hpp>
#include <dynamic_lib_six/functions.hpp>
#include <iostream>

DIMPORT extern int glob_exe_var;

namespace dynamic_lib_one {


DEXPORT int glob_one_var = 111;


DEXPORT int version()
{
    std::cout << "(version = 1)\n";
    return 1 + 100 * dynamic_lib_two::version() + 1000 * dynamic_lib_six::version();
}

DEXPORT double value()
{
    std::cout << "(value = 1.1)\n";
    return 1.1 + 100.0 * dynamic_lib_two::value() + 1000.0 * dynamic_lib_six::value();
}

DEXPORT int gvar()
{
    return DLL_USE_EXE_VAR(glob_exe_var +) glob_one_var + dynamic_lib_two::glob_two_var + dynamic_lib_six::glob_six_var;
}


}
