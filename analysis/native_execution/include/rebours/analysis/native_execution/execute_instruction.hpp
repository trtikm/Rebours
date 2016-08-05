#ifndef REBOURS_ANALYSIS_NATIVE_EXECUTION_EXECUTE_INSTRUCTION_HPP_INCLUDED
#   define REBOURS_ANALYSIS_NATIVE_EXECUTION_EXECUTE_INSTRUCTION_HPP_INCLUDED

#   include <rebours/analysis/native_execution/execution_properties.hpp>
#   include <rebours/program/instruction.hpp>
#   include <string>
#   include <cstdint>

/**
 * Each of the functions bellow is responsible for performing an effect of one Microcode instruction. Each function name
 * starts with a prefix 'execute_' followed by a capital group- and instruction- name of a related Microcode instruction.
 * Instruction groups and names are separated by '__'. Parameters of these instructions are nothing but paramenters of
 * the corresponding instructions. However, the last additional parameter captures an execution context (holding additional
 * data also necessary for executing instructions).
 *
 * Not all Microcode instruction are handled here. These are excluded (intentionally):
 *      microcode::GIK::MODULARITY__CALL,
 *      microcode::GIK::CONCURRENCY__REG_ASGN_THREAD,
 *      microcode::GIK::GUARDS__REG_EQUAL_TO_ZERO,
 *      microcode::GIK::GUARDS__REG_NOT_EQUAL_TO_ZERO.
 * Due to their special nature they are handled directly inside the function 'execution_step', see 'execute_program.cpp'.
 */

namespace analysis { namespace natexe {


std::string  execute_SETANDCOPY__REG_ASGN_NUMBER(uint8_t const  n, uint64_t const  a, uint64_t const  v, execution_context&  ctx);
std::string  execute_SETANDCOPY__REG_ASGN_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, execution_context&  ctx);

std::string  execute_INDIRECTCOPY__REG_ASGN_REG_REG(uint8_t const  n0, uint8_t const  n1, uint64_t const  a0, uint64_t const  a1, execution_context&  ctx);
std::string  execute_INDIRECTCOPY__REG_REG_ASGN_REG(uint8_t const  n0, uint8_t const  n1, uint64_t const  a0, uint64_t const  a1, execution_context&  ctx);

std::string  execute_DATATRANSFER__REG_ASGN_DEREF_INV_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, execution_context&  ctx);
std::string  execute_DATATRANSFER__REG_ASGN_DEREF_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, execution_context&  ctx);
std::string  execute_DATATRANSFER__DEREF_REG_ASGN_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, execution_context&  ctx);
std::string  execute_DATATRANSFER__DEREF_INV_REG_ASGN_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, execution_context&  ctx);
std::string  execute_DATATRANSFER__DEREF_ADDRESS_ASGN_DATA(uint64_t const  a, uint8_t const*  begin, uint64_t const  num_bytes, execution_context&  ctx);
std::string  execute_DATATRANSFER__DEREF_REG_ASGN_NUMBER(uint8_t const  n, uint64_t const  a, uint64_t const  v, execution_context&  ctx);
std::string  execute_DATATRANSFER__DEREF_INV_REG_ASGN_NUMBER(uint8_t const  n, uint64_t const  a, uint64_t const  v, execution_context&  ctx);

std::string  execute_TYPECASTING__REG_ASGN_ZERO_EXTEND_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, execution_context&  ctx);
std::string  execute_TYPECASTING__REG_ASGN_SIGN_EXTEND_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, execution_context&  ctx);

std::string  execute_MEMORYMANAGEMENT__REG_ASGN_MEM_STATIC(uint64_t const  a0, uint64_t const  a1, uint8_t const  v0, uint64_t const  v1, execution_context&  ctx);
std::string  execute_MEMORYMANAGEMENT__REG_ASGN_MEM_ALLOC(uint64_t const  a0, uint64_t const  a1, uint64_t const  a2, uint64_t const  a3, execution_context&  ctx);
std::string  execute_MEMORYMANAGEMENT__REG_ASGN_MEM_FREE(uint64_t const  a0, uint64_t const  a1, uint64_t const  a2, execution_context&  ctx);
std::string  execute_MEMORYMANAGEMENT__REG_ASGN_MEM_VALID_REG_NUMBER_NUMBER(uint64_t const  a0, uint64_t const  a1, uint8_t const  v0, uint64_t const  v1, execution_context&  ctx);

std::string  execute_HAVOC__REG_ASGN_HAVOC(uint64_t const  v, uint64_t const  a, execution_context&  ctx);
std::string  execute_HAVOC__REG_REG_ASGN_HAVOC(uint8_t const  n, uint64_t const  v, uint64_t const  a, execution_context&  ctx);

std::string  execute_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  v, execution_context&  ctx);
std::string  execute_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint128_t const  v, execution_context&  ctx);
std::string  execute_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  a2, execution_context&  ctx);
std::string  execute_INTEGERARITHMETICS__REG_ASGN_REG_TIMES_NUMBER(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  v, execution_context&  ctx);
std::string  execute_INTEGERARITHMETICS__REG_ASGN_REG_TIMES_NUMBER(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint128_t const  v, execution_context&  ctx);
std::string  execute_INTEGERARITHMETICS__REG_ASGN_REG_DIVIDE_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  a2, execution_context&  ctx);
std::string  execute_INTEGERARITHMETICS__REG_ASGN_REG_MODULO_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  a2, execution_context&  ctx);

std::string  execute_ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, execution_context&  ctx);

std::string  execute_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  v, execution_context&  ctx);
std::string  execute_BITOPERATIONS__REG_ASGN_REG_AND_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  a2, execution_context&  ctx);
std::string  execute_BITOPERATIONS__REG_ASGN_REG_OR_NUMBER(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  v, execution_context&  ctx);
std::string  execute_BITOPERATIONS__REG_ASGN_REG_OR_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  a2, execution_context&  ctx);
std::string  execute_BITOPERATIONS__REG_ASGN_NOT_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, execution_context&  ctx);
std::string  execute_BITOPERATIONS__REG_ASGN_REG_XOR_NUMBER(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  v, execution_context&  ctx);
std::string  execute_BITOPERATIONS__REG_ASGN_REG_XOR_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  a2, execution_context&  ctx);
std::string  execute_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint8_t const  v, execution_context&  ctx);
std::string  execute_BITOPERATIONS__REG_ASGN_REG_RSHIFT_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  a2, execution_context&  ctx);
std::string  execute_BITOPERATIONS__REG_ASGN_REG_LSHIFT_NUMBER(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint8_t const  v, execution_context&  ctx);
std::string  execute_BITOPERATIONS__REG_ASGN_REG_LSHIFT_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  a2, execution_context&  ctx);

std::string  execute_INPUTOUTPUT__REG_ASGN_STREAM_OPEN_NUMBER(uint64_t const  a, uint8_t const  v0, uint64_t const  v1, execution_context&  ctx);
std::string  execute_INPUTOUTPUT__REG_ASGN_STREAM_READ_NUMBER(uint64_t const  a, uint64_t const  v, execution_context&  ctx);
std::string  execute_INPUTOUTPUT__REG_ASGN_STREAM_WRITE_NUMBER_REG(uint64_t const  a0, uint64_t const  v, uint64_t const  a1, execution_context&  ctx);
std::string  execute_INPUTOUTPUT__REG_ASGN_STREAM_WRITE_REG_REG(uint64_t const  a0, uint64_t const  a1, uint64_t const  a2, execution_context&  ctx);

std::string  execute_MISCELLANEOUS__REG_ASGN_PARITY_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, execution_context&  ctx);


/**
 * This is the 'root' function whose job is to call one particular function from those above, according to the type of the passed instruction.
 */
std::string  execution_instruction(microcode::instruction const& I, execution_context&  ctx, bool const  is_sequential);


}}

#endif
