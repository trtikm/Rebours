#include <rebours/program/assembly.hpp>
#include <rebours/program/assumptions.hpp>
#include <rebours/program/invariants.hpp>
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace microcode {


static std::string  SIZE2(instruction const&  I, uint64_t const  size_index)
{
    std::stringstream  ostr;
    ostr << "{" << std::dec << 2U * I.argument<uint8_t>(size_index) << "}";
    return ostr.str();
}

static std::string  SIZE(instruction const&  I, uint64_t const  size_index, bool const  fixed_size = false)
{
    std::stringstream  ostr;
    ostr << "{" << std::dec << (fixed_size ? size_index : (uint64_t)I.argument<uint8_t>(size_index)) << "}";
    return ostr.str();
}

static std::string  IDX(instruction const&  I, uint64_t const  address_index)
{
    std::stringstream  ostr;
    ostr << "[" << std::hex << I.argument<uint64_t>(address_index) << "h]";
    return ostr.str();
}

static std::string  REG(instruction const&  I, uint64_t const  address_index, uint64_t const  size_index, bool const  fixed_size = false)
{
    std::stringstream  ostr;
    ostr << "REG" << IDX(I,address_index) << SIZE(I,size_index,fixed_size);
    return ostr.str();
}

static std::string  REG2(instruction const&  I, uint64_t const  address_index, uint64_t const  size_index)
{
    std::stringstream  ostr;
    ostr << "REG" << IDX(I,address_index) << SIZE2(I,size_index);
    return ostr.str();
}

static std::string  UINT(instruction const&  I, uint64_t const  number_index, uint64_t const  size_index, bool const  fixed_size = false)
{
    std::stringstream  ostr;
    if (I.argument_size(number_index) > 8ULL)
    {
        ostr << std::hex;
        ostr << I.argument<uint128_t>(number_index);
        ostr << "h";
        ostr << SIZE(I,size_index,fixed_size);
    }
    else
        ostr << std::hex << I.argument<uint64_t>(number_index) << "h" << SIZE(I,size_index,fixed_size);
    return ostr.str();
}

static std::string  UINTraw(instruction const&  I, uint64_t const  number_index, bool  hexadecimal = false)
{
    std::stringstream  ostr;
    if (hexadecimal)
        ostr << std::hex;
    else
        ostr << std::dec;
    ostr << I.argument<uint64_t>(number_index);
    if (hexadecimal)
        ostr << "h";
    return ostr.str();
}

static void  DATA(std::stringstream&  ostr, instruction const&  I, uint64_t const  data_start_index, std::string const&  line_adjustment)
{
    ostr << "[";
    for (uint64_t  i = data_start_index, line_break = 0ULL; i < I.num_arguments(); ++i, line_break = (line_break + 1ULL) % 16ULL)
    {
        ASSUMPTION(I.argument_size(i) == 1ULL);
        if (line_break == 0ULL)
            ostr << (i == data_start_index ? "" : ",") << " \\\n" << line_adjustment << "    ";
        else
            ostr << (line_break == 8ULL ? ",   " : ", ");
        ostr << std::hex << std::setw(2) << std::setfill('0') << (uint64_t)I.argument<uint8_t>(i) << "h";
    }
    ostr << " \\\n" << line_adjustment << "]{" << std::hex << I.num_arguments() - data_start_index << "h}";
}


std::string  assembly_text(instruction const&  I, std::string const&  line_adjustment, annotations const* const  A)
{
    std::stringstream  ostr;
    ostr << line_adjustment;

    switch (I.GIK())
    {
    case GIK::GUARDS__REG_EQUAL_TO_ZERO:
        ostr << REG(I,1,0) << " ==" << SIZE(I,0) << " 0" << SIZE(I,0);
        break;
    case GIK::GUARDS__REG_NOT_EQUAL_TO_ZERO:
        ostr << REG(I,1,0) << " !=" << SIZE(I,0) << " 0" << SIZE(I,0);
        break;

    case GIK::SETANDCOPY__REG_ASGN_NUMBER:
        ostr << REG(I,1,0) << " := " << UINT(I,2,0);
        break;
    case GIK::SETANDCOPY__REG_ASGN_REG:
        ostr << REG(I,1,0) << " := " << REG(I,2,0);
        break;

    case GIK::INDIRECTCOPY__REG_ASGN_REG_REG:
        ostr << REG(I,2,0) << " := REG[" <<  REG(I,3,1) << "]" << SIZE(I,0);
        break;
    case GIK::INDIRECTCOPY__REG_REG_ASGN_REG:
        ostr << "REG[" << REG(I,2,1) << "]" << SIZE(I,0) << " := " <<  REG(I,3,0);
        break;

    case GIK::DATATRANSFER__REG_ASGN_DEREF_ADDRESS:
        ostr << REG(I,1,0) << " := * " << UINT(I,2,8,true);
        break;
    case GIK::DATATRANSFER__REG_ASGN_DEREF_INV_ADDRESS:
        ostr << REG(I,1,0) << " := *' " << UINT(I,2,8,true);
        break;
    case GIK::DATATRANSFER__REG_ASGN_DEREF_REG:
        ostr << REG(I,1,0) << " := * " << REG(I,2,8,true);
        break;
    case GIK::DATATRANSFER__REG_ASGN_DEREF_INV_REG:
        ostr << REG(I,1,0) << " := *' " << REG(I,2,8,true);
        break;
    case GIK::DATATRANSFER__DEREF_REG_ASGN_REG:
        ostr << "* "<< REG(I,1,8,true) << " := " << REG(I,2,0);
        break;
    case GIK::DATATRANSFER__DEREF_INV_REG_ASGN_REG:
        ostr << "*' "<< REG(I,1,8,true) << " := " << REG(I,2,0);
        break;
    case GIK::DATATRANSFER__DEREF_ADDRESS_ASGN_DATA:
        ostr << "* " << UINT(I,0,8,true) << " := "; DATA(ostr,I,1ULL,line_adjustment);
        break;
    case GIK::DATATRANSFER__DEREF_REG_ASGN_DATA:
        ostr << "* " << REG(I,0,8,true) << " := "; DATA(ostr,I,1ULL,line_adjustment);
        break;
    case GIK::DATATRANSFER__DEREF_REG_ASGN_NUMBER:
        ostr << "* " << REG(I,1,0) << " := " << UINT(I,2,0);
        break;
    case GIK::DATATRANSFER__DEREF_INV_REG_ASGN_NUMBER:
        ostr << "*' " << REG(I,1,0) << " := " << UINT(I,2,0);
        break;

    case GIK::TYPECASTING__REG_ASGN_ZERO_EXTEND_REG:
        ostr << REG2(I,1,0) << " := #z " << REG(I,2,0);
        break;
    case GIK::TYPECASTING__REG_ASGN_SIGN_EXTEND_REG:
        ostr << REG2(I,1,0) << " := #s " << REG(I,2,0);
        break;

    case GIK::MEMORYMANAGEMENT__REG_ASGN_MEM_STATIC:
        ostr << REG(I,0,8,true) << " := MEM STATIC " << UINT(I,2,1,true) << ", " << UINT(I,3,8,true) << ", " << UINT(I,1,8,true);
        break;
    case GIK::MEMORYMANAGEMENT__REG_ASGN_MEM_ALLOC:
        ostr << REG(I,0,8,true) << " := MEM ALLOC " << REG(I,1,8,true) << ", " << REG(I,2,8,true) << ", " << REG(I,3,8,true);
        break;
    case GIK::MEMORYMANAGEMENT__REG_ASGN_MEM_FREE:
        ostr << REG(I,0,8,true) << " := MEM FREE " << REG(I,1,8,true) << ", " << REG(I,2,8,true);
        break;
    case GIK::MEMORYMANAGEMENT__REG_ASGN_MEM_VALID_REG_NUMBER_NUMBER:
        ostr << REG(I,0,1,true) << " := MEM VALID? " << REG(I,1,8,true) << ", " << UINT(I,2,1,true) << ", " << UINT(I,3,8,true);
        break;

    case GIK::CONCURRENCY__REG_ASGN_THREAD:
        ostr << REG(I,0,1,true) << " := THREAD";
        break;

    case GIK::MODULARITY__CALL:
        {
            ostr << "CALL ";
            uint64_t const  node = I.argument<uint64_t>(0ULL);
            std::string const* const  label_name = find(A,node,"LABELNAME");
            if (label_name != nullptr)
                ostr <<  *label_name;
            else
                ostr << UINTraw(I,0);
        }
        break;

    case GIK::HAVOC__REG_ASGN_HAVOC:
        ostr << "REG" << IDX(I,1) << "{" << UINTraw(I,0,true) << "} := HAVOC";
        break;
    case GIK::HAVOC__REG_REG_ASGN_HAVOC:
        ostr << "REG[" << REG(I,2,0) << "]{" << UINTraw(I,1) << "} := HAVOC";
        break;

    case GIK::INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER:
        ostr << REG(I,1,0) << " := " << REG(I,2,0) << " +" << SIZE(I,0) << " " << UINT(I,3,0);
        break;
    case GIK::INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG:
        ostr << REG(I,1,0) << " := " << REG(I,2,0) << " +" << SIZE(I,0) << " " << REG(I,3,0);
        break;
    case GIK::INTEGERARITHMETICS__REG_ASGN_REG_TIMES_NUMBER:
        ostr << REG(I,1,0) << " := " << REG(I,2,0) << " *" << SIZE(I,0) << " " << UINT(I,3,0);
        break;
    case GIK::INTEGERARITHMETICS__REG_ASGN_REG_DIVIDE_REG:
        ostr << REG(I,1,0) << " := " << REG(I,2,0) << " /" << SIZE(I,0) << " " << REG(I,3,0);
        break;
    case GIK::INTEGERARITHMETICS__REG_ASGN_REG_MODULO_REG:
        ostr << REG(I,1,0) << " := " << REG(I,2,0) << " %" << SIZE(I,0) << " " << REG(I,3,0);
        break;

    case GIK::ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO:
        ostr << REG(I,1,1,true) << " := " << REG(I,2,0) << " ==" << SIZE(I,0) << " 0" << SIZE(I,0);
        break;

    case GIK::BITOPERATIONS__REG_ASGN_REG_AND_NUMBER:
        ostr << REG(I,1,0) << " := " << REG(I,2,0) << " &" << SIZE(I,0) << " " << UINT(I,3,0);
        break;
    case GIK::BITOPERATIONS__REG_ASGN_REG_AND_REG:
        ostr << REG(I,1,0) << " := " << REG(I,2,0) << " &" << SIZE(I,0) << " " << REG(I,3,0);
        break;
    case GIK::BITOPERATIONS__REG_ASGN_REG_OR_NUMBER:
        ostr << REG(I,1,0) << " := " << REG(I,2,0) << " |" << SIZE(I,0) << " " << UINT(I,3,0);
        break;
    case GIK::BITOPERATIONS__REG_ASGN_REG_OR_REG:
        ostr << REG(I,1,0) << " := " << REG(I,2,0) << " |" << SIZE(I,0) << " " << REG(I,3,0);
        break;

    case GIK::BITOPERATIONS__REG_ASGN_NOT_REG:
        ostr << REG(I,1,0) << " := !" << SIZE(I,0) << " " << REG(I,2,0);
        break;
    case GIK::BITOPERATIONS__REG_ASGN_REG_XOR_NUMBER:
        ostr << REG(I,1,0) << " := " << REG(I,2,0) << " XOR" << SIZE(I,0) << " " << UINT(I,3,0);
        break;
    case GIK::BITOPERATIONS__REG_ASGN_REG_XOR_REG:
        ostr << REG(I,1,0) << " := " << REG(I,2,0) << " XOR" << SIZE(I,0) << " " << REG(I,3,0);
        break;
    case GIK::BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER:
        ostr << REG(I,1,0) << " := " << REG(I,2,0) << " >>" << SIZE(I,0) << " " << UINT(I,3,1,true);
        break;
    case GIK::BITOPERATIONS__REG_ASGN_REG_RSHIFT_REG:
        ostr << REG(I,1,0) << " := " << REG(I,2,0) << " >>" << SIZE(I,0) << " " << REG(I,3,1,true);
        break;
    case GIK::BITOPERATIONS__REG_ASGN_REG_LSHIFT_NUMBER:
        ostr << REG(I,1,0) << " := " << REG(I,2,0) << " <<" << SIZE(I,0) << " " << UINT(I,3,1,true);
        break;
    case GIK::BITOPERATIONS__REG_ASGN_REG_LSHIFT_REG:
        ostr << REG(I,1,0) << " := " << REG(I,2,0) << " <<" << SIZE(I,0) << " " << REG(I,3,1,true);
        break;

    case GIK::FLOATINGPOINTCOMPARISONS__REG_ASGN_REG_EQUAL_TO_ZERO:
        ostr << REG(I,0,10,true) << " := " << REG(I,1,10,true) << " =={10} 0{10}";
        break;

    case GIK::FLOATINGPOINTARITHMETICS__REG_ASGN_REG_PLUS_NUMBER:
        ostr << REG(I,0,10,true) << " := " << REG(I,1,10,true) << " +{10} " << "!!TODO!!";
        break;

    case GIK::INTERRUPTS__REG_ASGN_INTERRUPT_SET_AT_FIXED_NUMBER:
        ostr << REG(I,1,8,true) << " := INTERRUPT SET " << UINT(I,3,0) << ", " << REG(I,2,8,true);
        break;
    case GIK::INTERRUPTS__REG_ASGN_INTERRUPT_GET_AT_FIXED_NUMBER:
        ostr << REG(I,1,8,true) << " := INTERRUPT GET " << UINT(I,2,0);
        break;

    case GIK::INPUTOUTPUT__REG_ASGN_STREAM_OPEN_REG:
        ostr << REG(I,0,8,true) << " := STREAM OPEN " << UINT(I,2,1,true) << ", " << REG(I,1,8,true);
        break;
    case GIK::INPUTOUTPUT__REG_ASGN_STREAM_OPEN_NUMBER:
        ostr << REG(I,0,8,true) << " := STREAM OPEN " << UINT(I,1,1,true) << ", " << UINT(I,2,8,true);
        break;
    case GIK::INPUTOUTPUT__REG_ASGN_STREAM_READ_NUMBER:
        ostr << REG(I,0,1,true) << " := STREAM READ " << UINT(I,1,8,true);
        break;
    case GIK::INPUTOUTPUT__REG_ASGN_STREAM_WRITE_NUMBER_REG:
        ostr << REG(I,0,1,true) << " := STREAM WRITE " << UINT(I,1,8,true) << ", " << REG(I,2,1,true);
        break;
    case GIK::INPUTOUTPUT__REG_ASGN_STREAM_WRITE_REG_REG:
        ostr << REG(I,0,1,true) << " := STREAM WRITE " << REG(I,1,8,true) << ", " << REG(I,2,1,true);
        break;

    case GIK::MISCELLANEOUS__REG_ASGN_PARITY_REG:
        ostr << REG(I,1,1,true) << " := PARITY " << REG(I,2,0);
        break;
    case GIK::MISCELLANEOUS__NOP:
        ostr << "NOP";
        break;
    case GIK::MISCELLANEOUS__STOP:
        ostr << "STOP";
        break;

    default: UNREACHABLE();
    }

    return ostr.str();
}


}
