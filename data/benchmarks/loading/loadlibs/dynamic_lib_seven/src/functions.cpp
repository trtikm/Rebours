#include <import_export.hpp>
#include <iostream>

DIMPORT extern int glob_exe_var;

namespace dynamic_lib_seven {


DEXPORT int glob_seven_var = 117;


DEXPORT int version()
{
    std::cout << "(version = 7)\n";
    return 7;
}


DEXPORT double value()
{
    std::cout << "(value = 7.7)\n";
    return 7.7;
}

DEXPORT int gvar()
{
    return DLL_USE_EXE_VAR(glob_exe_var +) glob_seven_var;
}


}
