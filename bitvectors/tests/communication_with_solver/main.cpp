#include <rebours/bitvectors/expression.hpp>
#include <rebours/bitvectors/expression_io.hpp>
#include <rebours/bitvectors/sat_checking.hpp>
#include <rebours/utility/test.hpp>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <fstream>


static void test_is_satisfiable()
{
    std::cout << "Starting: test_is_satisfiable()\n";

    bv::typed_expression<int> const  v0 = bv::var<int>("v0");
    bv::typed_expression<int> const  i10 = bv::num(10);
    bv::typed_expression<float> const  pi = bv::num(3.1415f);
    bv::typed_expression<int> const  v1 = bv::var<int>("v1");

    {
        bv::expression const  e = v0 == i10;
        bv::sat_engine  winner;
        bv::sat_result const  result = bv::is_satisfiable(e,500U,&winner);
        TEST_SUCCESS(result == bv::sat_result::YES);
        std::cout << "  Winner = " << bv::to_string(winner) << "\n";
    }

    {
        bv::expression const  e = v0 == v0 + i10;
        bv::sat_engine  winner;
        bv::sat_result const  result = bv::is_satisfiable(e,500U,&winner);
        TEST_SUCCESS(result == bv::sat_result::NO);
        std::cout << "  Winner = " << bv::to_string(winner) << "\n";
    }

    {
        bv::expression const  e = v0 + pi == v1 + i10;
        bv::sat_engine  winner;
        bv::sat_result const  result = bv::is_satisfiable(e,500U,&winner);
        TEST_SUCCESS(result == bv::sat_result::YES);
        std::cout << "  Winner = " << bv::to_string(winner) << "\n";
    }

    std::cout << "SUCCESS\n";
}

static void test_get_model_if_satisfiable()
{
    std::cout << "Starting: test_get_model_if_satisfiable()\n";

    bv::typed_expression<int> const  v0 = bv::var<int>("v0");
    bv::typed_expression<int> const  i10 = bv::num(10);
    bv::typed_expression<int> const  i5 = bv::num(5);
    bv::typed_expression<float> const  pi = bv::num(3.1415f);
    bv::typed_expression<int> const  v1 = bv::var<int>("v1");

    {
        bv::expression const  e = i10 == i5 + i5;
        bv::sat_engine  winner;
        std::pair<bv::sat_result,bv::sat_model> const  result = bv::get_model_if_satisfiable(e,500U,&winner);
        TEST_SUCCESS(result.first == bv::sat_result::YES);
        TEST_SUCCESS(result.second.empty());
        std::cout << "  Winner = " << bv::to_string(winner) << "\n";
    }
    {
        bv::expression const  e = v0 == i10 && v1 == i10 - i5;
        bv::sat_engine  winner;
        std::pair<bv::sat_result,bv::sat_model> const  result = bv::get_model_if_satisfiable(e,500U,&winner);
        TEST_SUCCESS(result.first == bv::sat_result::YES);
        TEST_SUCCESS(result.second.size() == 2ULL);
        TEST_SUCCESS(result.second.count(bv::get_symbol(v0)) == 1ULL);
        TEST_SUCCESS(result.second.at(bv::get_symbol(v0)).num_cases() == 1ULL);
        TEST_SUCCESS(result.second.at(bv::get_symbol(v0)).args_of_case(0ULL).empty());
        TEST_SUCCESS(result.second.at(bv::get_symbol(v0)).value_of_case(0ULL) == i10);
        TEST_SUCCESS(result.second.count(bv::get_symbol(v1)) == 1ULL);
        TEST_SUCCESS(result.second.at(bv::get_symbol(v1)).num_cases() == 1ULL);
        TEST_SUCCESS(result.second.at(bv::get_symbol(v1)).args_of_case(0ULL).empty());
        TEST_SUCCESS(result.second.at(bv::get_symbol(v1)).value_of_case(0ULL) == i5);
        std::cout << "  Winner = " << bv::to_string(winner) << "\n";
    }
    {
        bv::expression const  e = v0 == i10;
        bv::sat_engine  winner;
        std::pair<bv::sat_result,bv::sat_model> const  result = bv::get_model_if_satisfiable(e,500U,&winner);
        TEST_SUCCESS(result.first == bv::sat_result::YES);
        TEST_SUCCESS(result.second.size() == 1ULL);
        TEST_SUCCESS(result.second.count(bv::get_symbol(v0)) == 1ULL);
        TEST_SUCCESS(result.second.at(bv::get_symbol(v0)).num_cases() == 1ULL);
        TEST_SUCCESS(result.second.at(bv::get_symbol(v0)).args_of_case(0ULL).empty());
        TEST_SUCCESS(result.second.at(bv::get_symbol(v0)).value_of_case(0ULL) == i10);
        std::cout << "  Winner = " << bv::to_string(winner) << "\n";
    }
    {
        bv::symbol const  f0 = bv::make_symbol_of_unintepreted_function("f0",32,{32});
        bv::expression const  e = bv::apply<int32_t>(f0,{i5}) +
                                  bv::apply<int32_t>(f0,{i10}) +
                                  bv::apply<int32_t>(f0,{i5 + i10})
                                  == i10;
        bv::sat_engine  winner;
        std::pair<bv::sat_result,bv::sat_model> const  result = bv::get_model_if_satisfiable(e,500U,&winner);
        TEST_SUCCESS(result.first == bv::sat_result::YES);
        TEST_SUCCESS(result.second.size() == 1ULL);
        TEST_SUCCESS(result.second.count(f0) == 1ULL);
        TEST_SUCCESS(result.second.at(f0).num_cases() == 3ULL);
        TEST_SUCCESS(result.second.at(f0).args_of_case(0ULL).size() == 1ULL);
        TEST_SUCCESS(result.second.at(f0).args_of_case(1ULL).size() == 1ULL);
        TEST_SUCCESS(result.second.at(f0).args_of_case(2ULL).size() == 1ULL);
        std::cout << "  Winner = " << bv::to_string(winner) << "\n";
    }
    {
        bv::symbol const  cast_f32s32 = bv::make_symbol_of_unintepreted_function("cast_f32s32",32,{32});
        bv::expression const  e = v0 + pi == v1 + i10;
        bv::sat_engine  winner;
        std::pair<bv::sat_result,bv::sat_model> const  result = bv::get_model_if_satisfiable(e,500U,&winner);
        TEST_SUCCESS(result.first == bv::sat_result::YES);
        TEST_SUCCESS(result.second.count(bv::get_symbol(v0)) == 1ULL);
        TEST_SUCCESS(result.second.at(bv::get_symbol(v0)).num_cases() == 1ULL);
        TEST_SUCCESS(result.second.at(bv::get_symbol(v0)).args_of_case(0ULL).empty());
        TEST_SUCCESS(result.second.count(bv::get_symbol(v1)) == 1ULL);
        TEST_SUCCESS(result.second.at(bv::get_symbol(v1)).num_cases() == 1ULL);
        TEST_SUCCESS(result.second.at(bv::get_symbol(v1)).args_of_case(0ULL).empty());
        TEST_SUCCESS(result.second.count(cast_f32s32) == 1ULL);
        TEST_SUCCESS(result.second.at(cast_f32s32).num_cases() == 1ULL);
        TEST_SUCCESS(result.second.at(cast_f32s32).args_of_case(0ULL).size() == 1ULL);
        std::cout << "  Winner = " << bv::to_string(winner) << "\n";
    }

    std::cout << "SUCCESS\n";
}

static void test_nonlinear_terms()
{
    std::cout << "Starting: test_nonlinear_terms()\n";

    bv::typed_expression<int> const  a0 = bv::var<int>("a0");
    bv::typed_expression<int> const  a1 = bv::var<int>("a1");
    bv::typed_expression<int> const  a2 = bv::var<int>("a2");
    bv::typed_expression<int> const  c2 = bv::num(2);
    bv::typed_expression<int> const  c4 = bv::num(4);
    bv::typed_expression<int> const  c5 = bv::num(5);
    bv::typed_expression<int> const  c8827 = bv::num(8827);
    //bv::expression const  e = a0 * a1 * a2 + c2 * a0 * a1 + c4 * a0 * a2 == c10897;
    bv::expression const  e = c2 * a0 * a0 * a0 + c4 * a0 * a0 - c5 * a0 == //c10897
                              c4 * a1 * a1 - c2 * a1 + c8827
                              ;
    //std::cout << "** e **\n" << e << "\n";
    bv::sat_engine  winner;
    std::pair<bv::sat_result,bv::sat_model> const  result = bv::get_model_if_satisfiable(e,500U,&winner);
    TEST_SUCCESS(result.first == bv::sat_result::YES);
    TEST_SUCCESS(result.second.count(bv::get_symbol(a0)) == 1ULL);
    TEST_SUCCESS(result.second.at(bv::get_symbol(a0)).num_cases() == 1ULL);
    TEST_SUCCESS(result.second.at(bv::get_symbol(a0)).args_of_case(0ULL).empty());
    //std::cout << "** a0 **\n" << result.second.at(bv::get_symbol(a0)).value_of_case(0ULL) << "\n";
    TEST_SUCCESS(result.second.count(bv::get_symbol(a1)) == 1ULL);
    TEST_SUCCESS(result.second.at(bv::get_symbol(a1)).num_cases() == 1ULL);
    TEST_SUCCESS(result.second.at(bv::get_symbol(a1)).args_of_case(0ULL).empty());
    //std::cout << "** a1 **\n" << result.second.at(bv::get_symbol(a1)).value_of_case(0ULL) << "\n";
//    TEST_SUCCESS(result.second.count(bv::get_symbol(a2)) == 1ULL);
//    TEST_SUCCESS(result.second.at(bv::get_symbol(a2)).num_cases() == 1ULL);
//    TEST_SUCCESS(result.second.at(bv::get_symbol(a2)).args_of_case(0ULL).empty());
//    std::cout << "** a2 **\n" << result.second.at(bv::get_symbol(a2)).value_of_case(0ULL) << "\n";
    std::cout << "  Winner = " << bv::to_string(winner) << "\n";

    std::cout << "SUCCESS\n";
}

static void save_crash_report(std::string const& crash_message)
{
    std::cout << "ERROR: " << crash_message << "\n";
    std::ofstream  ofile("communication_with_solver_CRASH.txt", std::ios_base::app );
    ofile << crash_message << "\n";
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    try
    {
        test_is_satisfiable();
        test_get_model_if_satisfiable();
        test_nonlinear_terms();
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
