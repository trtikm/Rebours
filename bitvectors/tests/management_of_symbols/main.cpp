#include <rebours/bitvectors/test.hpp>
#include <rebours/bitvectors/symbol.hpp>
#include <stdexcept>
#include <iostream>
#include <fstream>


static void test_symbol_construction()
{
    std::cout << "Starting: test_symbol_construction()\n";

    bv::symbol const  num10 = bv::make_symbol_of_interpreted_constant(int16_t(10));
    TEST_SUCCESS(num10.operator bool());
    TEST_SUCCESS(bv::symbol_name(num10) == "0x000a");
    TEST_SUCCESS(bv::symbol_num_bits_of_return_value(num10) == 16ULL);
    TEST_SUCCESS(bv::symbol_num_parameters(num10) == 0ULL);
    TEST_SUCCESS(bv::symbol_is_interpreted(num10));
    TEST_SUCCESS(bv::symbol_is_interpreted_constant(num10));

    bv::symbol const  F = bv::make_symbol_of_unintepreted_function("F",8ULL,{});
    TEST_SUCCESS(F.operator bool());
    TEST_SUCCESS(bv::symbol_name(F) == "F");
    TEST_SUCCESS(bv::symbol_num_bits_of_return_value(F) == 8ULL);
    TEST_SUCCESS(bv::symbol_num_parameters(F) == 0ULL);
    TEST_SUCCESS(!bv::symbol_is_interpreted(F));
    TEST_SUCCESS(!bv::symbol_is_interpreted_constant(F));

    bv::symbol const  F2 = bv::make_symbol_of_unintepreted_function("F",8ULL,{});
    TEST_SUCCESS(F2.operator bool());
    TEST_SUCCESS(bv::symbol_name(F2) == "F");
    TEST_SUCCESS(bv::symbol_num_bits_of_return_value(F2) == 8ULL);
    TEST_SUCCESS(bv::symbol_num_parameters(F2) == 0ULL);
    TEST_SUCCESS(!bv::symbol_is_interpreted(F2));
    TEST_SUCCESS(!bv::symbol_is_interpreted_constant(F2));

    TEST_FAILURE(num10 == F);
    TEST_SUCCESS(F == F2);

    std::cout << "SUCCESS\n";
}

static void save_crash_report(std::string const& crash_message)
{
    std::cout << "ERROR: " << crash_message << "\n";
    std::ofstream  ofile("management_of_symbols_CRASH.txt", std::ios_base::app );
    ofile << crash_message << "\n";
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    try
    {
        test_symbol_construction();
    }
    catch(std::exception const& e)
    {
        try { save_crash_report(e.what()); } catch (...) {}
        return -1;
    }
    catch(...)
    {
        try { save_crash_report("Unknown exception was thrown."); } catch (...) {}
        return -2;
    }
    return 0;
}
