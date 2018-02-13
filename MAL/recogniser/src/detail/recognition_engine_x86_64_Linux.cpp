#include <rebours/MAL/recogniser/detail/recognition_engine_x86_64_Linux.hpp>
#include <rebours/utility/msgstream.hpp>
#include <rebours/utility/assumptions.hpp>
#include <rebours/utility/invariants.hpp>
#include <rebours/utility/development.hpp>
#include <rebours/MAL/descriptor/storage.hpp>
#include <string>

namespace mal { namespace recogniser { namespace detail { namespace x86_64_Linux { namespace {


register_info  get_register_info(std::string const&  register_name, descriptor::storage const&  D)
{
    ASSUMPTION(D.registers_to_ranges().count(register_name) != 0ULL);
    mal::descriptor::address_range const&  range = D.registers_to_ranges().at(register_name);
    ASSUMPTION(range.second > range.first && range.second - range.first < 256ULL);
    return { range.first, (uint8_t)(range.second - range.first) };
}


}}}}}


namespace mal { namespace recogniser { namespace detail { namespace x86_64_Linux {


recognition_engine::recognition_engine(csh const handle, cs_insn* instruction)
    : m_handle(handle)
    , m_instruction(instruction)
    , m_details(&instruction->detail->x86)
{}

recognition_engine::~recognition_engine()
{
    cs_free(m_instruction, 1ULL);
    cs_close(&m_handle);
}

uint8_t  recognition_engine::get_byte_of_instruction(uint8_t const  idx) const
{
    ASSUMPTION(idx < num_bytes_of_instruction());
    return instruction().bytes[idx];
}

uint8_t recognition_engine::get_num_instruction_groups() const
{
    ASSUMPTION(instruction().detail != nullptr);
    return instruction().detail->groups_count;
}

uint8_t recognition_engine::get_instruction_group(uint8_t const  idx) const
{
    ASSUMPTION(instruction().detail != nullptr);
    ASSUMPTION(idx < get_num_instruction_groups());
    return instruction().detail->groups[idx];
}

bool  recognition_engine::is_relative_jump() const
{
    ASSUMPTION(instruction_id() == X86_INS_JMP);
    ASSUMPTION(num_bytes_of_instruction() > 0U);
    uint8_t const  value = get_byte_of_instruction(0U);
    return value == 0xebU ||value == 0xe9U;
}

bool  recognition_engine::is_relative_call() const
{
    ASSUMPTION(instruction_id() == X86_INS_CALL);
    ASSUMPTION(num_bytes_of_instruction() > 0U);
    uint8_t const  value = get_byte_of_instruction(0U);
    return value == 0xe8U;
}

bool  recognition_engine::is_register_operand(uint8_t const  idx) const
{
    ASSUMPTION(idx < num_operands());
    return details().operands[idx].type == X86_OP_REG;
}

bool  recognition_engine::is_memory_operand(uint8_t const  idx) const
{
    ASSUMPTION(idx < num_operands());
    return details().operands[idx].type == X86_OP_MEM;
}

bool  recognition_engine::is_segment_register_of_memory_operand_valid(uint8_t const  idx) const
{
    ASSUMPTION(idx < num_operands() && is_memory_operand(idx));
    return details().operands[idx].mem.segment != X86_REG_INVALID;
}

bool  recognition_engine::is_index_register_of_memory_operand_valid(uint8_t const  idx) const
{
    ASSUMPTION(idx < num_operands() && is_memory_operand(idx));
    return details().operands[idx].mem.index != X86_REG_INVALID;
}

bool  recognition_engine::is_base_register_of_memory_operand_valid(uint8_t const  idx) const
{
    ASSUMPTION(idx < num_operands() && is_memory_operand(idx));
    return details().operands[idx].mem.base != X86_REG_INVALID;
}

bool  recognition_engine::is_immediate_operand(uint8_t const  idx) const
{
    ASSUMPTION(idx < num_operands());
    return details().operands[idx].type == X86_OP_IMM;
}

bool  recognition_engine::is_floating_point_operand(uint8_t const  idx) const
{
//    ASSUMPTION(idx < num_operands());
//    return details().operands[idx].type == X86_OP_FP;
    return false;   // It looks like Capstone removed this flag from 'x86_op_type' structure.
}

bool  recognition_engine::is_read_operand(uint8_t const  idx) const
{
    ASSUMPTION(idx < num_operands());
    if (is_immediate_operand(idx))
        return true;
    return (details().operands[idx].access & CS_AC_READ) != 0U;
}

bool  recognition_engine::is_write_operand(uint8_t const  idx) const
{
    ASSUMPTION(idx < num_operands());
    if (is_immediate_operand(idx))
        return false;
    return (details().operands[idx].access & CS_AC_WRITE) != 0U;
}


uint8_t recognition_engine::get_num_bytes_of_operand(uint8_t const  idx) const
{
    ASSUMPTION(idx < num_operands());
    return details().operands[idx].size;
}


register_info  recognition_engine::get_register_operand(uint8_t const  idx, descriptor::storage const&  D) const
{
    ASSUMPTION(idx < num_operands() && is_register_operand(idx));
    return get_register_info(cs_reg_name(handle(),details().operands[idx].reg),D);
}

register_info  recognition_engine::get_segment_register_of_memory_operand(uint8_t const  idx, descriptor::storage const&  D) const
{
    ASSUMPTION(idx < num_operands() && is_memory_operand(idx));
    return get_register_info(cs_reg_name(handle(),details().operands[idx].mem.segment),D);
}

register_info  recognition_engine::get_index_register_of_memory_operand(uint8_t const  idx, descriptor::storage const&  D) const
{
    ASSUMPTION(idx < num_operands() && is_memory_operand(idx));
    return get_register_info(cs_reg_name(handle(),details().operands[idx].mem.index),D);
}

register_info  recognition_engine::get_base_register_of_memory_operand(uint8_t const  idx, descriptor::storage const&  D) const
{
    ASSUMPTION(idx < num_operands() && is_memory_operand(idx));
    return get_register_info(cs_reg_name(handle(),details().operands[idx].mem.base),D);
}

int  recognition_engine::get_scale_of_index_register_of_memory_operand(uint8_t const  idx) const
{
    ASSUMPTION(idx < num_operands() && is_memory_operand(idx));
    return details().operands[idx].mem.scale;
}

int64_t recognition_engine::get_displacement_of_memory_operand(uint8_t const  idx) const
{
    ASSUMPTION(idx < num_operands() && is_memory_operand(idx));
    return details().operands[idx].mem.disp;
}

int64_t  recognition_engine::get_immediate_operand(uint8_t const  idx) const
{
    ASSUMPTION(idx < num_operands() && is_immediate_operand(idx));
    return details().operands[idx].imm;
}

double  recognition_engine::get_floating_point_operand(uint8_t const  idx) const
{
//    ASSUMPTION(idx < num_operands() && is_floating_point_operand(idx));
//    return details().operands[idx].fp;
    UNREACHABLE();  // It looks like Capstone removed this flag from 'x86_op_type' structure.
}


}}}}
