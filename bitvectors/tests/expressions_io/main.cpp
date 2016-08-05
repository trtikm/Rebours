#include <rebours/bitvectors/test.hpp>
#include <rebours/bitvectors/expression.hpp>
#include <rebours/bitvectors/expression_io.hpp>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <fstream>


static void test_expression_io()
{
    std::cout << "Starting: test_expression_io()\n";

    bv::typed_expression<int32_t> const  i10 = bv::num(10);
    bv::typed_expression<float> const  pi = bv::num(3.1415f);
    bv::typed_expression<uint32_t> const  u5 = bv::num(5U);
    bv::typed_expression<uint8_t> const  u25 = bv::num<uint8_t>(25U);
    bv::typed_expression<int32_t> const  v0 = bv::var<int32_t>("v0");
    bv::typed_expression<int32_t> const  v1 = bv::var<int32_t>("v1");
    bv::typed_expression<uint8_t> const  v2 = bv::var<uint8_t>("v2");

    {
        std::stringstream  sstr;
        bv::expression const  original = i10 + u5;
        sstr << original;
        //std::cout << sstr.str(); std::cout.flush();
        TEST_SUCCESS(sstr.str() ==
                     "(set-logic QF_UFBV)\n\n"
                     "(declare-fun eval ((_ BitVec 32)) Bool)\n\n"
                     "(assert (eval\n"
                     "    (bvadd\n"
                     "        #x0000000a\n"
                     "        #x00000005\n"
                     "        )\n"
                     "    ))\n\n");
        bv::expression  loaded;
        sstr >> loaded;
        //std::cout << loaded; std::cout.flush();
        TEST_SUCCESS(original == loaded);
    }
    {
        std::stringstream  sstr;
        bv::expression const  original = bv::tt();
        sstr << original;
        //std::cout << sstr.str(); std::cout.flush();
        TEST_SUCCESS(sstr.str() ==
                     "(set-logic QF_UFBV)\n\n"
                     "(assert\n"
                     "    (= #b0 #b0)\n"
                     "    )\n\n");
        bv::expression  loaded;
        sstr >> loaded;
        //std::cout << loaded; std::cout.flush();
        TEST_SUCCESS(original == loaded);
    }
    {
        std::stringstream  sstr;
        bv::expression const  original = i10;
        sstr << original;
        //std::cout << sstr.str(); std::cout.flush();
        TEST_SUCCESS(sstr.str() ==
                     "(set-logic QF_UFBV)\n\n"
                     "(declare-fun eval ((_ BitVec 32)) Bool)\n\n"
                     "(assert (eval\n"
                     "    #x0000000a\n"
                     "    ))\n\n");
        bv::expression  loaded;
        sstr >> loaded;
        //std::cout << loaded; std::cout.flush();
        TEST_SUCCESS(original == loaded);
    }
    {
        std::stringstream  sstr;
        bv::expression const  original = (u5 + i10);
        sstr << original;
        //std::cout << sstr.str(); std::cout.flush();
        TEST_SUCCESS(sstr.str() ==
                     "(set-logic QF_UFBV)\n\n"
                     "(declare-fun eval ((_ BitVec 32)) Bool)\n\n"
                     "(assert (eval\n"
                     "    (bvadd\n"
                     "        #x00000005\n"
                     "        #x0000000a\n"
                     "        )\n"
                     "    ))\n\n");
        bv::expression  loaded;
        sstr >> loaded;
        //std::cout << loaded; std::cout.flush();
        TEST_SUCCESS(original == loaded);
    }
    {
        std::stringstream  sstr;
        bv::expression const  original = (v0 == u5 + i10);
        sstr << original;
        //std::cout << sstr.str(); std::cout.flush();
        TEST_SUCCESS(sstr.str() ==
                     "(set-logic QF_UFBV)\n\n"
                     "(declare-fun v0 () (_ BitVec 32))\n\n"
                     "(assert\n"
                     "    (=\n"
                     "        v0\n"
                     "        (bvadd\n"
                     "            #x00000005\n"
                     "            #x0000000a\n"
                     "            )\n"
                     "        )\n"
                     "    )\n\n");
        bv::expression  loaded;
        sstr >> loaded;
        //std::cout << loaded; std::cout.flush();
        TEST_SUCCESS(original == loaded);
    }
    {
        std::stringstream  sstr;
        bv::expression const  original = (v0 == u5 * i10 && bv::tt() && v0 < v0 + u5);
        sstr << original;
        //std::cout << sstr.str(); std::cout.flush();
        TEST_SUCCESS(sstr.str() ==
                     "(set-logic QF_UFBV)\n\n"
                     "(declare-fun v0 () (_ BitVec 32))\n\n"
                     "(assert\n"
                     "    (and\n"
                     "        (and\n"
                     "            (=\n"
                     "                v0\n"
                     "                (bvmul\n"
                     "                    #x00000005\n"
                     "                    #x0000000a\n"
                     "                    )\n"
                     "                )\n"
                     "            (= #b0 #b0)\n"
                     "            )\n"
                     "        (bvslt\n"
                     "            v0\n"
                     "            (bvadd\n"
                     "                v0\n"
                     "                #x00000005\n"
                     "                )\n"
                     "            )\n"
                     "        )\n"
                     "    )\n\n");
        bv::expression  loaded;
        sstr >> loaded;
        //std::cout << loaded; std::cout.flush();
        TEST_SUCCESS(original == loaded);
    }
    {
        std::stringstream  sstr;
        bv::expression const  original = v0 == i10;
        sstr << original;
        //std::cout << sstr.str(); std::cout.flush();
        TEST_SUCCESS(sstr.str() ==
                     "(set-logic QF_UFBV)\n\n"
                     "(declare-fun v0 () (_ BitVec 32))\n\n"
                     "(assert\n"
                     "    (=\n"
                     "        v0\n"
                     "        #x0000000a\n"
                     "        )\n"
                     "    )\n\n");
        bv::expression  loaded;
        sstr >> loaded;
        //std::cout << loaded; std::cout.flush();
        TEST_SUCCESS(original == loaded);
    }
    {
        std::stringstream  sstr;
        bv::expression const  original = v0 == pi;
        sstr << original;
        //std::cout << sstr.str(); std::cout.flush();
        TEST_SUCCESS(sstr.str() ==
                     "(set-logic QF_UFBV)\n\n"
                     "(declare-fun v0 () (_ BitVec 32))\n"
                     "(declare-fun cast_f32u32 ((_ BitVec 32)) (_ BitVec 32))\n\n"
                     "(assert\n"
                     "    (=\n"
                     "        v0\n"
                     "        (cast_f32u32\n"
                     "            #x40490e56\n"
                     "            )\n"
                     "        )\n"
                     "    )\n\n");
        bv::expression  loaded;
        sstr >> loaded;
        //std::cout << loaded; std::cout.flush();
        TEST_SUCCESS(original == loaded);
    }
    {
        std::stringstream  sstr;
        bv::expression const  original = u5 == u25;
        sstr << original;
        //std::cout << sstr.str(); std::cout.flush();
        TEST_SUCCESS(sstr.str() ==
                     "(set-logic QF_UFBV)\n\n"
                     "(assert\n"
                     "    (=\n"
                     "        #x00000005\n"
                     "        ((_ zero_extend 16)\n"
                     "            ((_ zero_extend 8)\n"
                     "                #x19\n"
                     "                )\n"
                     "            )\n"
                     "        )\n"
                     "    )\n\n");
        bv::expression  loaded;
        sstr >> loaded;
        //std::cout << loaded; std::cout.flush();
        TEST_SUCCESS(original == loaded);
    }
    {
        bv::expression const  original = v0 == i10 || v1 == u25;
        //std::cout << original; std::cout.flush();
        std::stringstream  xsstr;
        xsstr << "(set-logic QF_UFBV)\n\n"
              << "(declare-fun v0 () (_ BitVec 32))\n"
              << "(declare-fun v1 () (_ BitVec 32))\n\n"
              << "(assert\n"
              << "    (or\n"
              << "        (=\n"
              << "            v0\n"
              << "            #x0000000a\n"
              << "            )\n"
              << "        (=\n"
              << "            v1\n"
              << "            ((_ zero_extend 24)\n"
              << "                #x19\n"
              << "                )\n"
              << "            )\n"
              << "        )\n"
              << "    )\n"
              ;
        bv::expression  loaded;
        xsstr >> loaded;
        //std::cout << loaded; std::cout.flush();
        TEST_SUCCESS(original == loaded);
    }
    {
        bv::expression const  original = v0 == i10 || v1 == u25 || v2 == bv::cast<uint8_t>(i10);
        //std::cout << original; std::cout.flush();
        std::stringstream  xsstr;
        xsstr << "(set-logic QF_UFBV)\n\n"
              << "(declare-fun v0 () (_ BitVec 32))\n"
              << "(declare-fun v1 () (_ BitVec 32))\n"
              << "(declare-fun v2 () (_ BitVec 8))\n\n"
              << "(assert\n"
              << "    (or\n"
              << "        (or\n"
              << "            (=\n"
              << "                v0\n"
              << "                #x0000000a\n"
              << "                )\n"
              << "            (=\n"
              << "                v1\n"
              << "                ((_ zero_extend 24)\n"
              << "                    #x19\n"
              << "                    )\n"
              << "                )\n"
              << "            )\n"
              << "        (=\n"
              << "            v2\n"
              << "            ((_ extract 7 0)\n"
              << "                #x0000000a\n"
              << "                )\n"
              << "            )\n"
              << "        )\n"
              << "    )\n"
              ;
        bv::expression  loaded;
        xsstr >> loaded;
        //std::cout << loaded; std::cout.flush();
        TEST_SUCCESS(original == loaded);
    }

    std::cout << "SUCCESS\n";
}

static void save_crash_report(std::string const& crash_message)
{
    std::cout << crash_message << "\n";
    std::ofstream  ofile("expression_io_CRASH.txt", std::ios_base::app );
    ofile << crash_message << "\n";
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    try
    {
        test_expression_io();
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
