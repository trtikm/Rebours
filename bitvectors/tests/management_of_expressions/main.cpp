#include <rebours/bitvectors/expression.hpp>
#include <rebours/bitvectors/expression_io.hpp>
#include <rebours/utility/test.hpp>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <fstream>


static void test_expression_construction()
{
    TEST_MESSAGE("Starting: test_expression_construction()");

    bv::typed_expression<int> const  n0 = bv::num(10);
    TEST_SUCCESS(n0.operator bool());
    TEST_SUCCESS(bv::is_interpreted_constant(n0));
    TEST_SUCCESS(bv::is_number(n0));
    TEST_SUCCESS(!bv::is_var(n0));
    TEST_SUCCESS(n0.is_of_signed_integral_type());
    TEST_SUCCESS(bv::num_bits_of_return_value(n0) == 32ULL);
    TEST_SUCCESS(bv::num_parameters(n0) == 0ULL);
    TEST_SUCCESS(bv::symbol_name(n0) == "0x0000000a");
    TEST_SUCCESS(bv::is_term(n0));

    std::string const  memory{"Hello world!"};
    bv::expression const  m0 = bv::mem(memory.data(),memory.data()+memory.size());
    TEST_SUCCESS(m0.operator bool());
    TEST_SUCCESS(bv::is_interpreted_constant(m0));
    TEST_SUCCESS(!bv::is_number(m0));
    TEST_SUCCESS(!bv::is_var(m0));
    TEST_SUCCESS(bv::num_bits_of_return_value(m0) == 8ULL*memory.size());
    TEST_SUCCESS(bv::num_parameters(m0) == 0ULL);
    TEST_SUCCESS(bv::symbol_name(m0) == "0x48656c6c6f20776f726c6421");
    TEST_SUCCESS(bv::is_term(m0));

    bv::typed_expression<int> const  v0 = bv::var<int>("v0");
    TEST_SUCCESS(v0.operator bool());
    TEST_SUCCESS(bv::num_parameters(v0) == 0ULL);
    TEST_SUCCESS(!bv::is_number(v0));
    TEST_SUCCESS(bv::is_var(v0));
    TEST_SUCCESS(bv::num_bits_of_return_value(v0) == 32ULL);
    TEST_SUCCESS(!bv::is_interpreted(v0));
    TEST_SUCCESS(bv::is_term(v0));

    bv::typed_expression<uint8_t> const  e0 = bv::num<uint8_t>(10) + bv::var<uint8_t>("x");
    TEST_SUCCESS(e0.operator bool());
    TEST_SUCCESS(e0.is_of_unsigned_integral_type());
    TEST_SUCCESS(bv::num_bits_of_return_value(e0) == 8ULL);
    TEST_SUCCESS(bv::num_parameters(e0) == 2ULL);
    TEST_SUCCESS(bv::num_bits_of_parameter(e0,0ULL) == 8ULL);
    TEST_SUCCESS(bv::num_bits_of_parameter(e0,1ULL) == 8ULL);
    TEST_SUCCESS(bv::is_term(e0));

    bv::typed_expression<uint8_t> const  e1 = -(bv::num<uint8_t>(5) - bv::var<uint8_t>("x"));
    TEST_SUCCESS(e1.operator bool());
    TEST_SUCCESS(e1.is_of_unsigned_integral_type());
    TEST_SUCCESS(bv::num_bits_of_return_value(e1) == 8ULL);
    TEST_SUCCESS(bv::num_parameters(e1) == 2ULL);
    TEST_SUCCESS(bv::num_bits_of_parameter(e1,0ULL) == 8ULL);
    TEST_SUCCESS(bv::num_bits_of_parameter(e1,1ULL) == 8ULL);
    TEST_SUCCESS(bv::is_term(e1));

    bv::typed_expression<float> const  e2 = bv::cast<float>(e1);
    TEST_SUCCESS(e2.operator bool());
    TEST_SUCCESS(e2.is_of_floating_type());
    TEST_SUCCESS(bv::symbol_name(e2) == "#u32f32");
    TEST_SUCCESS(bv::num_bits_of_return_value(e2) == 32ULL);
    TEST_SUCCESS(bv::num_parameters(e2) == 1ULL);
    TEST_SUCCESS(bv::num_bits_of_parameter(e2,0ULL) == 32ULL);
    TEST_SUCCESS(bv::symbol_name(bv::argument(e2,0ULL)) == "#u16u32");
    TEST_SUCCESS(bv::symbol_name(bv::argument(bv::argument(e2,0ULL),0ULL)) == "#u8u16");

    bv::typed_expression<int> const  e3 = v0 * n0;
    TEST_SUCCESS(e3.operator bool());
    TEST_SUCCESS(e3.is_of_signed_integral_type());
    TEST_SUCCESS(bv::symbol_name(e3) == "*i32");

    bv::typed_expression<int> const  e4 = v0 / n0;
    TEST_SUCCESS(e4.operator bool());
    TEST_SUCCESS(e4.is_of_signed_integral_type());
    TEST_SUCCESS(bv::symbol_name(e4) == "/s32");

    bv::typed_expression<int> const  e5 = v0 % n0;
    TEST_SUCCESS(e5.operator bool());
    TEST_SUCCESS(e5.is_of_signed_integral_type());
    TEST_SUCCESS(bv::symbol_name(e5) == "%s32");

    bv::typed_expression<int> const  e6 = v0 << e0;
    TEST_SUCCESS(e6.operator bool());
    TEST_SUCCESS(e6.is_of_signed_integral_type());
    TEST_SUCCESS(bv::symbol_name(e6) == "<<i32");
    TEST_SUCCESS(bv::symbol_name(argument(e6,0ULL)) == "v0");
    TEST_SUCCESS(bv::symbol_name(argument(e6,1ULL)) == "#u16u32");
    TEST_SUCCESS(bv::symbol_name(argument(argument(e6,1ULL),0ULL)) == "#u8u16");
    TEST_SUCCESS(bv::symbol_name(argument(argument(argument(e6,1ULL),0ULL),0ULL)) == "+i8");

    bv::typed_expression<int> const  e7 = v0 + e0;
    TEST_SUCCESS(e7.operator bool());
    TEST_SUCCESS(e7.is_of_signed_integral_type());
    TEST_SUCCESS(bv::symbol_name(e7) == "+i32");
    TEST_SUCCESS(bv::symbol_name(argument(e7,0ULL)) == "v0");
    TEST_SUCCESS(bv::symbol_name(argument(e7,1ULL)) == "#u16u32");
    TEST_SUCCESS(bv::symbol_name(argument(argument(e7,1ULL),0ULL)) == "#u8u16");
    TEST_SUCCESS(bv::symbol_name(argument(argument(argument(e7,1ULL),0ULL),0ULL)) == "+i8");

    bv::typed_expression<int> const  e8 = v0 - e0;
    TEST_SUCCESS(e8.operator bool());
    TEST_SUCCESS(e8.is_of_signed_integral_type());
    TEST_SUCCESS(bv::symbol_name(e8) == "-i32");
    TEST_SUCCESS(bv::symbol_name(argument(e8,0ULL)) == "v0");
    TEST_SUCCESS(bv::symbol_name(argument(e8,1ULL)) == "#u16u32");
    TEST_SUCCESS(bv::symbol_name(argument(argument(e8,1ULL),0ULL)) == "#u8u16");
    TEST_SUCCESS(bv::symbol_name(argument(argument(argument(e8,1ULL),0ULL),0ULL)) == "+i8");

    bv::typed_expression<int> const  e9 = v0 * e0;
    TEST_SUCCESS(e9.operator bool());
    TEST_SUCCESS(e9.is_of_signed_integral_type());
    TEST_SUCCESS(bv::symbol_name(e9) == "*i32");
    TEST_SUCCESS(bv::symbol_name(argument(e9,0ULL)) == "v0");
    TEST_SUCCESS(bv::symbol_name(argument(e9,1ULL)) == "#u16u32");
    TEST_SUCCESS(bv::symbol_name(argument(argument(e9,1ULL),0ULL)) == "#u8u16");
    TEST_SUCCESS(bv::symbol_name(argument(argument(argument(e9,1ULL),0ULL),0ULL)) == "+i8");

    bv::typed_expression<int> const  e10 = v0 / e0;
    TEST_SUCCESS(e10.operator bool());
    TEST_SUCCESS(e10.is_of_signed_integral_type());
    TEST_SUCCESS(bv::symbol_name(e10) == "/s32");
    TEST_SUCCESS(bv::symbol_name(argument(e10,0ULL)) == "v0");
    TEST_SUCCESS(bv::symbol_name(argument(e10,1ULL)) == "#u16u32");
    TEST_SUCCESS(bv::symbol_name(argument(argument(e10,1ULL),0ULL)) == "#u8u16");
    TEST_SUCCESS(bv::symbol_name(argument(argument(argument(e10,1ULL),0ULL),0ULL)) == "+i8");

    bv::typed_expression<int> const  e11 = v0 % e0;
    TEST_SUCCESS(e11.operator bool());
    TEST_SUCCESS(e11.is_of_signed_integral_type());
    TEST_SUCCESS(bv::symbol_name(e11) == "%s32");
    TEST_SUCCESS(bv::symbol_name(argument(e11,0ULL)) == "v0");
    TEST_SUCCESS(bv::symbol_name(argument(e11,1ULL)) == "#u16u32");
    TEST_SUCCESS(bv::symbol_name(argument(argument(e11,1ULL),0ULL)) == "#u8u16");
    TEST_SUCCESS(bv::symbol_name(argument(argument(argument(e11,1ULL),0ULL),0ULL)) == "+i8");

    bv::typed_expression<int> const  e12 = v0 >> e0;
    TEST_SUCCESS(e12.operator bool());
    TEST_SUCCESS(e12.is_of_signed_integral_type());
    TEST_SUCCESS(bv::symbol_name(e12) == ">>s32");
    TEST_SUCCESS(bv::symbol_name(argument(e12,0ULL)) == "v0");
    TEST_SUCCESS(bv::symbol_name(argument(e12,1ULL)) == "#u16u32");
    TEST_SUCCESS(bv::symbol_name(argument(argument(e12,1ULL),0ULL)) == "#u8u16");
    TEST_SUCCESS(bv::symbol_name(argument(argument(argument(e12,1ULL),0ULL),0ULL)) == "+i8");

    bv::typed_expression<int> const  e13 = v0 & e0;
    TEST_SUCCESS(e13.operator bool());
    TEST_SUCCESS(e13.is_of_signed_integral_type());
    TEST_SUCCESS(bv::symbol_name(e13) == "&i32");
    TEST_SUCCESS(bv::symbol_name(argument(e13,0ULL)) == "v0");
    TEST_SUCCESS(bv::symbol_name(argument(e13,1ULL)) == "#u16u32");
    TEST_SUCCESS(bv::symbol_name(argument(argument(e13,1ULL),0ULL)) == "#u8u16");
    TEST_SUCCESS(bv::symbol_name(argument(argument(argument(e13,1ULL),0ULL),0ULL)) == "+i8");

    bv::typed_expression<int> const  e14 = v0 | e0;
    TEST_SUCCESS(e14.operator bool());
    TEST_SUCCESS(e14.is_of_signed_integral_type());
    TEST_SUCCESS(bv::symbol_name(e14) == "|i32");
    TEST_SUCCESS(bv::symbol_name(argument(e14,0ULL)) == "v0");
    TEST_SUCCESS(bv::symbol_name(argument(e14,1ULL)) == "#u16u32");
    TEST_SUCCESS(bv::symbol_name(argument(argument(e14,1ULL),0ULL)) == "#u8u16");
    TEST_SUCCESS(bv::symbol_name(argument(argument(argument(e14,1ULL),0ULL),0ULL)) == "+i8");

    bv::typed_expression<int> const  e15 = v0 ^ e0;
    TEST_SUCCESS(e15.operator bool());
    TEST_SUCCESS(e15.is_of_signed_integral_type());
    TEST_SUCCESS(bv::symbol_name(e15) == "^i32");
    TEST_SUCCESS(bv::symbol_name(argument(e15,0ULL)) == "v0");
    TEST_SUCCESS(bv::symbol_name(argument(e15,1ULL)) == "#u16u32");
    TEST_SUCCESS(bv::symbol_name(argument(argument(e15,1ULL),0ULL)) == "#u8u16");
    TEST_SUCCESS(bv::symbol_name(argument(argument(argument(e15,1ULL),0ULL),0ULL)) == "+i8");

    bv::typed_expression<int> const  e16 = ~v0;
    TEST_SUCCESS(e16.operator bool());
    TEST_SUCCESS(e16.is_of_signed_integral_type());
    TEST_SUCCESS(bv::symbol_name(e16) == "^i32");
    TEST_SUCCESS(bv::symbol_name(argument(e16,0ULL)) == "v0");
    TEST_SUCCESS(bv::symbol_name(argument(e16,1ULL)) == "0xffffffff");
    TEST_SUCCESS(bv::is_term(e16));

    bv::expression const  c0 = e0 < e1;
    TEST_SUCCESS(c0.operator bool());
    TEST_SUCCESS(bv::num_bits_of_return_value(c0) == 1ULL);
    TEST_SUCCESS(bv::num_parameters(c0) == 2ULL);
    TEST_SUCCESS(bv::num_bits_of_parameter(c0,0ULL) == 8ULL);
    TEST_SUCCESS(bv::num_bits_of_parameter(c0,1ULL) == 8ULL);
    TEST_SUCCESS(bv::symbol_name(c0) == "<u8");
    TEST_SUCCESS(bv::is_formula(c0));

    bv::expression const  c1 = (c0 && !bv::tt()) && !(n0 >= v0 || bv::num(3.14f) != e2);
    TEST_SUCCESS(c1.operator bool());
    TEST_SUCCESS(bv::symbol_name(c1) == "&&");
    TEST_SUCCESS(bv::is_conjunction(c1));
    TEST_SUCCESS(bv::num_bits_of_return_value(c1) == 1ULL);
    TEST_SUCCESS(bv::num_parameters(c1) == 2ULL);
    TEST_SUCCESS(bv::num_bits_of_parameter(c1,0ULL) == 1ULL);
    TEST_SUCCESS(bv::num_bits_of_parameter(c1,1ULL) == 1ULL);
    TEST_SUCCESS(bv::symbol_name(bv::argument(c1,0ULL)) == "&&");
    TEST_SUCCESS(bv::is_conjunction(bv::argument(c1,0ULL)));
    TEST_SUCCESS(bv::symbol_name(bv::argument(c1,1ULL)) == "!");
    TEST_SUCCESS(bv::is_negation(bv::argument(c1,1ULL)));
    TEST_SUCCESS(bv::is_formula(c1));
}

static void  management_of_expressions()
{
    test_expression_construction();
}

TEST_DEFINE_MAIN_FUNCTION_CALLING(management_of_expressions)
