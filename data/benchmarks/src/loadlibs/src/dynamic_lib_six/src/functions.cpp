#include <import_export.hpp>

DIMPORT extern int glob_exe_var;

namespace dynamic_lib_six {


DEXPORT int glob_six_var = 116;


DEXPORT int version()
{
    return 6;
}

DEXPORT double value()
{
    return 6.6;
}

DEXPORT int gvar()
{
    return DLL_USE_EXE_VAR(glob_exe_var +) glob_six_var;
}


}
