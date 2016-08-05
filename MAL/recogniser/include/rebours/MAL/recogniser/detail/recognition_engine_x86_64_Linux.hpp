#ifndef REBOURS_MAL_RECOGNISER_DETAIL_RECOGNITION_ENGINE_X86_64_LINUX_HPP_INCLUDED
#   define REBOURS_MAL_RECOGNISER_DETAIL_RECOGNITION_ENGINE_X86_64_LINUX_HPP_INCLUDED

#   include <rebours/MAL/recogniser/detail/register_info.hpp>
#   include <capstone/capstone.h>
#   include <capstone/x86.h>
#   include <utility>
#   include <cstdint>

namespace mal { namespace recogniser { namespace detail { namespace x86_64_Linux {


struct  recognition_engine
{
    recognition_engine(csh const  handle, cs_insn*  instruction);
    ~recognition_engine();

    csh  handle() const noexcept { return m_handle; }
    cs_insn const&  instruction() const noexcept { return *m_instruction; }
    cs_x86 const&  details() const noexcept { return *m_details; }

    unsigned int  instruction_id() const { return instruction().id; }

    uint8_t  get_rex() const { return details().rex; }
    bool  uses_rex() const { return get_rex() != 0U; }

    uint8_t  num_bytes_of_instruction() const { return instruction().size; }
    uint8_t  get_byte_of_instruction(uint8_t const  idx) const;

    uint8_t  get_address_size() const { return details().addr_size; }

    uint8_t get_num_instruction_groups() const;
    uint8_t get_instruction_group(uint8_t const  idx) const;

    bool  is_relative_jump() const;
    bool  is_relative_call() const;

    uint8_t  num_operands() const { return details().op_count; }
    bool  is_register_operand(uint8_t const  idx) const;
    bool  is_memory_operand(uint8_t const  idx) const;
    bool  is_segment_register_of_memory_operand_valid(uint8_t const  idx) const;
    bool  is_index_register_of_memory_operand_valid(uint8_t const  idx) const;
    bool  is_base_register_of_memory_operand_valid(uint8_t const  idx) const;
    bool  is_immediate_operand(uint8_t const  idx) const;
    bool  is_floating_point_operand(uint8_t const  idx) const;
    bool  is_read_operand(uint8_t const  idx) const;
    bool  is_write_operand(uint8_t const  idx) const;
    uint8_t get_num_bytes_of_operand(uint8_t const  idx) const;
    register_info  get_register_operand(uint8_t const  idx, descriptor::storage const&  D) const;
    register_info  get_segment_register_of_memory_operand(uint8_t const  idx, descriptor::storage const&  D) const;
    register_info  get_index_register_of_memory_operand(uint8_t const  idx, descriptor::storage const&  D) const;
    register_info  get_base_register_of_memory_operand(uint8_t const  idx, descriptor::storage const&  D) const;
    int  get_scale_of_index_register_of_memory_operand(uint8_t const  idx) const;
    int64_t get_displacement_of_memory_operand(uint8_t const  idx) const;
    int64_t  get_immediate_operand(uint8_t const  idx) const;
    double  get_floating_point_operand(uint8_t const  idx) const;

private:
    csh  m_handle;
    cs_insn* m_instruction;
    cs_x86* m_details;
};


}}}}

#endif
