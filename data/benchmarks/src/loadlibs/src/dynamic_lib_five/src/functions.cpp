#include <import_export.hpp>

DIMPORT extern int glob_exe_var;

namespace dynamic_lib_five {


DEXPORT int glob_five_var = 115;


DEXPORT int version()
{
    return 5;
}


DEXPORT double value()
{
    return 5.5;
}


DEXPORT int gvar()
{
    return DLL_USE_EXE_VAR(glob_exe_var +) glob_five_var;
}


}
