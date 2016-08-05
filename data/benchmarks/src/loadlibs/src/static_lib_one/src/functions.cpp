#include <static_lib_one/functions.hpp>

extern int glob_exe_var;

namespace static_lib_one {


int glob_static_one_var = 333;


int version()
{
    return 6789;
}

double value()
{
    return 2.8;
}

int gvar()
{
    return glob_exe_var + glob_static_one_var;
}


}
