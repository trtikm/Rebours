#include <static_lib_one/functions.hpp>
#include <dynamic_lib_one/functions.hpp>
#include <dynamic_lib_two/functions.hpp>
#include <dynamic_lib_three/functions.hpp>
#include <dynamic_lib_four/functions.hpp>
#include <dynamic_lib_five/functions.hpp>
#include <dynamic_lib_six/functions.hpp>
#include <dynamic_lib_seven/functions.hpp>
#include <iostream>

DEXPORT int glob_exe_var = 110;

int main(int argc, char* argv[])
{
    std::cout << "static_lib_one::version() = " << static_lib_one::version() << "\n";
    std::cout << "static_lib_one::value() = " << static_lib_one::value() << "\n";

    std::cout << "dynamic_lib_one::version() = " << dynamic_lib_one::version() << "\n";
    std::cout << "dynamic_lib_one::value() = " << dynamic_lib_one::value() << "\n";

    std::cout << "dynamic_lib_two::version() = " << dynamic_lib_two::version() << "\n";
    std::cout << "dynamic_lib_two::value() = " << dynamic_lib_two::value() << "\n";

    std::cout << "dynamic_lib_three::version() = " << dynamic_lib_three::version() << "\n";
    std::cout << "dynamic_lib_three::value() = " << dynamic_lib_three::value() << "\n";

    std::cout << "static_lib_one::gvar() = " << static_lib_one::gvar() << "\n";
    std::cout << "dynamic_lib_one::gvar() = " << dynamic_lib_one::gvar() << "\n";
    std::cout << "dynamic_lib_two::gvar() = " << dynamic_lib_two::gvar() << "\n";
    std::cout << "dynamic_lib_three::gvar() = " << dynamic_lib_three::gvar() << "\n";

    std::cout << "static_lib_one::glob_static_one_var = " << static_lib_one::glob_static_one_var << "\n";
    std::cout << "dynamic_lib_one::glob_one_var = " << dynamic_lib_one::glob_one_var << "\n";
    std::cout << "dynamic_lib_two::glob_two_var = " << dynamic_lib_two::glob_two_var << "\n";
    std::cout << "dynamic_lib_three::glob_three_var = " << dynamic_lib_three::glob_three_var << "\n";

    int i;
    std::cin >> i;

    return 0;
}
