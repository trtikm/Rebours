#include <rebours/MAL/recogniser/detail/recognise_x86_64_Linux.hpp>
#include <rebours/MAL/recogniser/detail/recognise_x86_64_Linux_syscall_utils.hpp>
#include <rebours/MAL/recogniser/detail/recognise_x86_64_Linux_syscall_syscall.hpp>
#include <rebours/MAL/recogniser/detail/register_info.hpp>
#include <rebours/MAL/descriptor/storage.hpp>
#include <rebours/utility/large_types.hpp>
#include <rebours/utility/assumptions.hpp>
#include <rebours/utility/invariants.hpp>
#include <rebours/utility/msgstream.hpp>
#include <rebours/utility/development.hpp>
#include <type_traits>
#include <algorithm>
#include <iomanip>

namespace mal { namespace recogniser { namespace detail { namespace x86_64_Linux {


using  node_id = microcode::program_component::node_id;


recognition_data::recognition_data(uint64_t const  start_address, reg_fn_type const&  reg_fn, mem_fn_type const&  mem_fn)
    : detail::recognition_data(start_address,reg_fn,mem_fn)
{}

void  recognition_data::recognise(descriptor::storage const&  description)
{
    csh handle;

    cs_err const open_error = cs_open(CS_ARCH_X86, CS_MODE_64, &handle);
    ASSUMPTION(open_error == CS_ERR_OK);
    cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);

    uint64_t  address = start_address();
    uint8_t const  rights = 2U; //!< Executable

    cs_insn*  insn = nullptr;
    for (int i = 0; i < 64; ++i, ++address)
    {
        int16_t const  res_val = mem_fn()(address,rights);
        ASSUMPTION(res_val >= -3 && res_val < 256);
        if (res_val < 0)
        {
            cs_close(&handle);

            set_error_result(1U << (-res_val));
            INVARIANT(error_result() == 2U || error_result() == 4U || error_result() == 8U);
            set_error_address(address);
            set_error_rights(rights);
            return;
        }
        buffer().push_back((uint8_t)res_val);
        cs_insn*  ptr = nullptr;
        size_t const  count = cs_disasm(handle,buffer().data(),buffer().size(),0ULL,1ULL,&ptr);
        INVARIANT(count < 2ULL);
        if (count == 1ULL)
        {
            ASSUMPTION(ptr != nullptr);
            insn = ptr;
            break;
        }
    }

    if (insn == nullptr)
    {
        set_error_result(16U);
        set_error_address(start_address());
        return;
    }

    cs_insn* const  instr = &(insn[0ULL]);

    if (instr->detail == nullptr)
    {
        cs_free(insn, 1ULL);
        cs_close(&handle);
        set_error_result(254U);
        return;
    }

    set_asm_text(msgstream() << std::string(instr->mnemonic) << " " << std::string(instr->op_str));

    {
        msgstream  mstr;
        for (uint64_t  i = 0ULL; i < buffer().size(); ++i)
            mstr << std::hex << std::setw(2) << std::setfill('0') << (uint32_t)buffer().at(i) << "h" << (i + 1ULL != buffer().size() ? "," : "");
        set_asm_bytes(mstr.str());
    }

    recognition_engine const  re(handle,instr);
    bool  success;

    switch (re.instruction_id())
    {
    case X86_INS_NOP:
        success = recognise_NOP(re,description);
        break;

    case X86_INS_MOVABS:
    case X86_INS_MOV:
        success = recognise_MOV(re,description);
        break;
    case X86_INS_MOVSXD:
        success = recognise_MOVSXD(re,description);
        break;
    case X86_INS_MOVZX:
        success = recognise_MOVZX(re,description);
        break;
    case X86_INS_MOVDQU:
        success = recognise_MOVDQU(re,description);
        break;
    case X86_INS_SETE:
        success = recognise_SETE(re,description);
        break;
    case X86_INS_SETNE:
        success = recognise_SETNE(re,description);
        break;
    case X86_INS_PMOVMSKB:
        success = recognise_PMOVMSKB(re,description);
        break;
    case X86_INS_CMOVE:
        success = recognise_CMOVE(re,description);
        break;
    case X86_INS_CMOVA:
        success = recognise_CMOVA(re,description);
        break;

    case X86_INS_LEA:
        success = recognise_LEA(re,description);
        break;

    case X86_INS_TEST:
        success = recognise_TEST(re,description);
        break;
    case X86_INS_CMP:
        success = recognise_SUB(re,description,true);
        break;
    case X86_INS_PCMPEQB:
        success = recognise_PCMPEQB(re,description);
        break;
    case X86_INS_CMPXCHG:
        success = recognise_CMPXCHG(re,description);
        break;

    case X86_INS_INC:
        success = recognise_INC(re,description);
        break;
    case X86_INS_DEC:
        success = recognise_DEC(re,description);
        break;
    case X86_INS_ADD:
        success = recognise_ADD(re,description);
        break;
    case X86_INS_SUB:
        success = recognise_SUB(re,description);
        break;
    case X86_INS_NEG:
        success = recognise_NEG(re,description);
        break;
    case X86_INS_DIV:
        success = recognise_DIV(re,description);
        break;

    case X86_INS_AND:
        success = recognise_AND(re,description);
        break;
    case X86_INS_OR:
        success = recognise_OR(re,description);
        break;
    case X86_INS_XOR:
        success = recognise_XOR(re,description);
        break;
    case X86_INS_PXOR:
        success = recognise_PXOR(re,description);
        break;
    case X86_INS_SAR:
        success = recognise_SAR(re,description);
        break;
    case X86_INS_SHR:
        success = recognise_SHR(re,description);
        break;
    case X86_INS_SHL:
        success = recognise_SHL(re,description);
        break;
    case X86_INS_ROL:
        success = recognise_ROL(re,description);
        break;
    case X86_INS_BSF:
        success = recognise_BSF(re,description);
        break;

    case X86_INS_JMP:
        success = recognise_JMP(re,description);
        break;
    case X86_INS_JNE:
        success = recognise_JNE(re,description);
        break;
    case X86_INS_JE:
        success = recognise_JE(re,description);
        break;
    case X86_INS_JS:
        success = recognise_JS(re,description);
        break;
    case X86_INS_JNS:
        success = recognise_JNS(re,description);
        break;
    case X86_INS_JA:
        success = recognise_JA(re,description);
        break;
    case X86_INS_JAE:
        success = recognise_JAE(re,description);
        break;
    case X86_INS_JG:
        success = recognise_JG(re,description);
        break;
    case X86_INS_JLE:
        success = recognise_JLE(re,description);
        break;
    case X86_INS_JB:
        success = recognise_JB(re,description);
        break;
    case X86_INS_JBE:
        success = recognise_JBE(re,description);
        break;

    case X86_INS_PUSH:
        success = recognise_PUSH(re,description);
        break;
    case X86_INS_POP:
        success = recognise_POP(re,description);
        break;

    case X86_INS_CALL:
        success = recognise_CALL(re,description);
        break;
    case X86_INS_RET:
        success = recognise_RET(re,description);
        break;
    case X86_INS_LEAVE:
        success = recognise_LEAVE(re,description);
        break;

    case X86_INS_INT:
        success = recognise_INT(re,description);
        break;
    case X86_INS_SYSCALL:
        success = recognise_SYSCALL(re,description);
        break;

    default:
        success = false;
        break;
    }

    if (!success)
    {
        set_error_result(255U);
        set_error_address(start_address());
    }
}

bool  recognition_data::recognise_NOP(recognition_engine const& re, descriptor::storage const&  D)
{
    (void)D;
    component().insert_sequence(component().entry(),{
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8U,
                        0ULL,
                        0ULL,
                        re.num_bytes_of_instruction()
                        )
                });
    return true;
}

bool  recognition_data::recognise_MOV(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 2U
        && re.is_register_operand(0U)
        && re.is_write_operand(0U)
        && re.is_immediate_operand(1U)
        )
    {
        register_info const  target_reg = re.get_register_operand(0U,D);
        microcode::instruction const  zero_extend_32bit_target_register_to_64bit =
                target_reg.second == 4U ? microcode::create_SETANDCOPY__REG_ASGN_NUMBER(4U,target_reg.first - 4ULL,0ULL) : microcode::instruction();

        component().insert_sequence(component().entry(),{
                    microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                            re.get_register_operand(0U,D).second,
                            re.get_register_operand(0U,D).first,
                            re.get_immediate_operand(1U)
                            ),
                    zero_extend_32bit_target_register_to_64bit,
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

        return true;
    }

    if (re.num_operands() == 2U
        && re.is_register_operand(0U)
        && re.is_write_operand(0U)
        && re.is_register_operand(1U)
        && re.is_read_operand(1U)
        )
    {
        register_info const  target_reg = re.get_register_operand(0U,D);
        microcode::instruction const  zero_extend_32bit_target_register_to_64bit =
                target_reg.second == 4U ? microcode::create_SETANDCOPY__REG_ASGN_NUMBER(4U,target_reg.first - 4ULL,0ULL) : microcode::instruction();

        component().insert_sequence(component().entry(),{
                    microcode::create_SETANDCOPY__REG_ASGN_REG(
                            re.get_register_operand(0U,D).second,
                            re.get_register_operand(0U,D).first,
                            re.get_register_operand(1U,D).first
                            ),
                    zero_extend_32bit_target_register_to_64bit,
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

        return true;
    }

    if (re.num_operands() == 2U
        && re.is_register_operand(0U)
        && re.is_write_operand(0U)
        && re.is_memory_operand(1U)
        && re.is_read_operand(1U)
        && !re.is_write_operand(1U)
        && !re.is_segment_register_of_memory_operand_valid(1U)
        && re.is_base_register_of_memory_operand_valid(1U)
        )
    {
        register_info const  target_reg = re.get_register_operand(0U,D);
        microcode::instruction const  zero_extend_32bit_target_register_to_64bit =
                target_reg.second == 4U ? microcode::create_SETANDCOPY__REG_ASGN_NUMBER(4U,target_reg.first - 4ULL,0ULL) : microcode::instruction();

        ASSUMPTION(re.get_base_register_of_memory_operand(1U,D).second == 8ULL);

        bool const  use_index_reg = re.is_index_register_of_memory_operand_valid(1U);
        bool const  use_scale = re.get_scale_of_index_register_of_memory_operand(1U) != 1;

        ASSUMPTION(!use_scale || use_index_reg);

        bool const  use_rip = re.get_base_register_of_memory_operand(1U,D) == get_register_info("rip",D);
        bool const  use_displacement = re.get_displacement_of_memory_operand(1U) != 0ULL;

        node_id  n = component().entry();
        uint64_t  temp = re.get_base_register_of_memory_operand(1U,D).first;
        if (use_scale || use_index_reg)
        {
            ASSUMPTION(re.get_index_register_of_memory_operand(1U,D).second == 8ULL);

            if (use_scale)
            {
                n = component().insert_sequence(n,{
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_TIMES_NUMBER(
                                re.get_index_register_of_memory_operand(1U,D).second,
                                D.start_address_of_temporaries(),
                                re.get_index_register_of_memory_operand(1U,D).first,
                                re.get_scale_of_index_register_of_memory_operand(1U)
                                ),
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                                re.get_base_register_of_memory_operand(1U,D).second,
                                D.start_address_of_temporaries(),
                                D.start_address_of_temporaries(),
                                temp
                                )
                        });
            }
            else
            {
                n = component().insert_sequence(n,{
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                                re.get_base_register_of_memory_operand(1U,D).second,
                                D.start_address_of_temporaries(),
                                re.get_index_register_of_memory_operand(1U,D).first,
                                temp
                                )
                        });
            }
            temp = D.start_address_of_temporaries();
        }
        if (use_displacement || use_rip)
        {
            n = component().insert_sequence(n,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            re.get_base_register_of_memory_operand(1U,D).second,
                            D.start_address_of_temporaries(),
                            temp,
                            re.get_displacement_of_memory_operand(1U) + (use_rip ? re.num_bytes_of_instruction() : 0)
                            )
                    });
            temp = D.start_address_of_temporaries();
        }
        n = component().insert_sequence(n,{
                microcode::create_DATATRANSFER__REG_ASGN_DEREF_INV_REG(
                        re.get_register_operand(0U,D).second,
                        re.get_register_operand(0U,D).first,
                        temp
                        )
                });
        if (temp >= D.start_address_of_temporaries())
        {
            n = component().insert_sequence(n,{
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            8ULL,
                            temp
                            )
                    });
        }
        component().insert_sequence(n,{
                zero_extend_32bit_target_register_to_64bit,
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8U,
                        0ULL,
                        0ULL,
                        re.num_bytes_of_instruction()
                        )
                });

        return true;
    }

    if (re.num_operands() == 2U
        && re.is_register_operand(0U)
        && !re.is_read_operand(0U)
        && re.is_write_operand(0U)
        && re.is_memory_operand(1U)
        && re.is_read_operand(1U)
        && !re.is_write_operand(1U)
        && re.is_segment_register_of_memory_operand_valid(1U)
        && !re.is_base_register_of_memory_operand_valid(1U)
        && !re.is_index_register_of_memory_operand_valid(1U)
        && re.get_scale_of_index_register_of_memory_operand(1U) == 1U
        )
    {
        register_info const  target_reg = re.get_register_operand(0U,D);
        microcode::instruction const  zero_extend_32bit_target_register_to_64bit =
                target_reg.second == 4U ? microcode::create_SETANDCOPY__REG_ASGN_NUMBER(4U,target_reg.first - 4ULL,0ULL) : microcode::instruction();

        ASSUMPTION(re.get_segment_register_of_memory_operand(1U,D).second == 8ULL);

        bool const  use_displacement = re.get_displacement_of_memory_operand(1U) != 0ULL;

        node_id  n = component().entry();
        uint64_t  temp = re.get_segment_register_of_memory_operand(1U,D).first;
        if (use_displacement)
        {
            n = component().insert_sequence(n,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8ULL,
                            D.start_address_of_temporaries(),
                            temp,
                            re.get_displacement_of_memory_operand(1U)
                            )
                    });
            temp = D.start_address_of_temporaries();
        }
        n = component().insert_sequence(n,{
                microcode::create_DATATRANSFER__REG_ASGN_DEREF_INV_REG(
                        re.get_register_operand(0U,D).second,
                        re.get_register_operand(0U,D).first,
                        temp
                        )
                });
        if (temp >= D.start_address_of_temporaries())
        {
            n = component().insert_sequence(n,{
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            8ULL,
                            temp
                            )
                    });
        }
        component().insert_sequence(n,{
                zero_extend_32bit_target_register_to_64bit,
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8U,
                        0ULL,
                        0ULL,
                        re.num_bytes_of_instruction()
                        )
                });

        return true;
    }

    if (re.num_operands() == 2U
        && re.is_memory_operand(0U)
        && re.is_write_operand(0U)
        && !re.is_segment_register_of_memory_operand_valid(0U)
        && re.is_base_register_of_memory_operand_valid(0U)
        && re.is_register_operand(1U)
        && re.is_read_operand(1U)
        )
    {
        ASSUMPTION(re.get_base_register_of_memory_operand(0U,D).second == 8ULL);

        bool const  use_index_reg = re.is_index_register_of_memory_operand_valid(0U);
        bool const  use_scale = re.get_scale_of_index_register_of_memory_operand(0U) != 1;

        ASSUMPTION(!use_scale || use_index_reg);

        bool const  use_rip = re.get_base_register_of_memory_operand(0U,D) == get_register_info("rip",D);
        bool const  use_displacement = re.get_displacement_of_memory_operand(0U) != 0ULL;

        node_id  n = component().entry();
        uint64_t  temp = re.get_base_register_of_memory_operand(0U,D).first;
        if (use_scale || use_index_reg)
        {
            ASSUMPTION(re.get_index_register_of_memory_operand(0U,D).second == 8ULL);

            if (use_scale)
            {
                n = component().insert_sequence(n,{
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_TIMES_NUMBER(
                                re.get_index_register_of_memory_operand(0U,D).second,
                                D.start_address_of_temporaries(),
                                re.get_index_register_of_memory_operand(0U,D).first,
                                re.get_scale_of_index_register_of_memory_operand(0U)
                                ),
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                                re.get_base_register_of_memory_operand(0U,D).second,
                                D.start_address_of_temporaries(),
                                D.start_address_of_temporaries(),
                                temp
                                )
                        });
            }
            else
            {
                n = component().insert_sequence(n,{
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                                re.get_base_register_of_memory_operand(0U,D).second,
                                D.start_address_of_temporaries(),
                                re.get_index_register_of_memory_operand(0U,D).first,
                                temp
                                )
                        });
            }
            temp = D.start_address_of_temporaries();
        }
        if (use_displacement || use_rip)
        {
            n = component().insert_sequence(n,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            re.get_base_register_of_memory_operand(0U,D).second,
                            D.start_address_of_temporaries(),
                            temp,
                            re.get_displacement_of_memory_operand(0U) + (use_rip ? re.num_bytes_of_instruction() : 0)
                            )
                    });
            temp = D.start_address_of_temporaries();
        }
        n = component().insert_sequence(n,{
                microcode::create_DATATRANSFER__DEREF_INV_REG_ASGN_REG(
                        re.get_register_operand(1U,D).second,
                        temp,
                        re.get_register_operand(1U,D).first
                        )
                });
        if (use_scale || use_displacement || use_rip)
        {
            n = component().insert_sequence(n,{
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            8ULL,
                            temp
                            )
                    });
        }
        component().insert_sequence(n,{
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8U,
                        0ULL,
                        0ULL,
                        re.num_bytes_of_instruction()
                        )
                });

        return true;
    }

    if (re.num_operands() == 2U
        && re.is_memory_operand(0U)
        && !re.is_read_operand(0U)
        && re.is_write_operand(0U)
        && re.is_segment_register_of_memory_operand_valid(0U)
        && !re.is_base_register_of_memory_operand_valid(0U)
        && !re.is_index_register_of_memory_operand_valid(0U)
        && re.get_scale_of_index_register_of_memory_operand(0U) == 1
        && re.is_register_operand(1U)
        && re.is_read_operand(1U)
        && !re.is_write_operand(1U)
        )
    {
        ASSUMPTION(re.get_segment_register_of_memory_operand(0U,D).second == 8ULL);

        bool const  use_displacement = re.get_displacement_of_memory_operand(0U) != 0ULL;

        node_id  n = component().entry();
        uint64_t  temp = re.get_segment_register_of_memory_operand(0U,D).first;
        if (use_displacement)
        {
            n = component().insert_sequence(n,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8ULL,
                            D.start_address_of_temporaries(),
                            temp,
                            re.get_displacement_of_memory_operand(0U)
                            )
                    });
            temp = D.start_address_of_temporaries();
        }
        n = component().insert_sequence(n,{
                microcode::create_DATATRANSFER__DEREF_INV_REG_ASGN_REG(
                        re.get_register_operand(1U,D).second,
                        temp,
                        re.get_register_operand(1U,D).first
                        )
                });
        if (use_displacement)
        {
            n = component().insert_sequence(n,{
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            8ULL,
                            temp
                            )
                    });
        }
        component().insert_sequence(n,{
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8U,
                        0ULL,
                        0ULL,
                        re.num_bytes_of_instruction()
                        )
                });

        return true;
    }

    if (re.num_operands() == 2U
        && re.is_memory_operand(0U)
        && !re.is_read_operand(0U)
        && re.is_write_operand(0U)
        && !re.is_segment_register_of_memory_operand_valid(0U)
        && re.is_base_register_of_memory_operand_valid(0U)
        && re.is_immediate_operand(1U)
        && re.is_read_operand(1U)
        && !re.is_write_operand(1U)
        )
    {
        ASSUMPTION(re.get_base_register_of_memory_operand(0U,D).second == 8ULL);
        ASSUMPTION(re.get_num_bytes_of_operand(0U) == re.get_num_bytes_of_operand(1U));

        bool const  use_index_reg = re.is_index_register_of_memory_operand_valid(0U);
        bool const  use_scale = re.get_scale_of_index_register_of_memory_operand(0U) != 1;

        ASSUMPTION(!use_scale || use_index_reg);

        bool const  use_rip = re.get_base_register_of_memory_operand(0U,D) == get_register_info("rip",D);
        bool const  use_displacement = re.get_displacement_of_memory_operand(0U) != 0ULL;

        node_id  n = component().entry();
        uint64_t  temp = re.get_base_register_of_memory_operand(0U,D).first;
        if (use_scale || use_index_reg)
        {
            ASSUMPTION(re.get_index_register_of_memory_operand(0U,D).second == 8ULL);

            if (use_scale)
            {
                n = component().insert_sequence(n,{
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_TIMES_NUMBER(
                                re.get_index_register_of_memory_operand(0U,D).second,
                                D.start_address_of_temporaries(),
                                re.get_index_register_of_memory_operand(0U,D).first,
                                re.get_scale_of_index_register_of_memory_operand(0U)
                                ),
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                                re.get_base_register_of_memory_operand(0U,D).second,
                                D.start_address_of_temporaries(),
                                D.start_address_of_temporaries(),
                                temp
                                )
                        });
            }
            else
            {
                n = component().insert_sequence(n,{
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                                re.get_base_register_of_memory_operand(0U,D).second,
                                D.start_address_of_temporaries(),
                                re.get_index_register_of_memory_operand(0U,D).first,
                                temp
                                )
                        });
            }
            temp = D.start_address_of_temporaries();
        }
        if (use_displacement || use_rip)
        {
            n = component().insert_sequence(n,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            re.get_base_register_of_memory_operand(0U,D).second,
                            D.start_address_of_temporaries(),
                            temp,
                            re.get_displacement_of_memory_operand(0U) + (use_rip ? re.num_bytes_of_instruction() : 0)
                            )
                    });
            temp = D.start_address_of_temporaries();
        }
        n = component().insert_sequence(n,{
                microcode::create_DATATRANSFER__DEREF_INV_REG_ASGN_NUMBER(
                        re.get_num_bytes_of_operand(0U),
                        temp,
                        re.get_immediate_operand(1U)
                        )
                });
        if (use_scale || use_displacement || use_rip)
        {
            n = component().insert_sequence(n,{
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            8ULL,
                            temp
                            )
                    });
        }
        component().insert_sequence(n,{
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8U,
                        0ULL,
                        0ULL,
                        re.num_bytes_of_instruction()
                        )
                });

        return true;
    }

    return false;
}

bool  recognition_data::recognise_MOVSXD(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 2U
        && re.is_register_operand(0U)
        && re.is_write_operand(0U)
        && re.is_register_operand(1U)
        && re.is_read_operand(1U)
        )
    {
        component().insert_sequence(component().entry(),{
                    microcode::create_TYPECASTING__REG_ASGN_SIGN_EXTEND_REG(
                            re.get_register_operand(1U,D).second,
                            re.get_register_operand(0U,D).first,
                            re.get_register_operand(1U,D).first
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

        return true;
    }

    return false;
}

bool  recognition_data::recognise_MOVZX(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 2U
        && re.is_register_operand(0U)
        && re.is_write_operand(0U)
        && re.is_register_operand(1U)
        && re.is_read_operand(1U)
        )
    {
        register_info const  target_reg = re.get_register_operand(0U,D);
        uint8_t const  num_bytes = re.get_num_bytes_of_operand(1U);
        ASSUMPTION(num_bytes == re.get_register_operand(1U,D).second);

         component().insert_sequence(component().entry(),{
                     target_reg.second == 4U ?
                            microcode::create_SETANDCOPY__REG_ASGN_NUMBER(8U,target_reg.first - 4ULL,0ULL) :
                            microcode::create_SETANDCOPY__REG_ASGN_NUMBER(target_reg.second,target_reg.first,0ULL),
                    microcode::create_SETANDCOPY__REG_ASGN_REG(
                            num_bytes,
                            target_reg.first + target_reg.second - num_bytes,
                            re.get_register_operand(1U,D).first
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

        return true;
    }

    if (re.num_operands() == 2U
        && re.is_register_operand(0U)
        && !re.is_read_operand(0U)
        && re.is_write_operand(0U)
        && re.is_memory_operand(1U)
        && re.is_read_operand(1U)
        && !re.is_write_operand(1U)
        && !re.is_segment_register_of_memory_operand_valid(1U)
        && re.is_base_register_of_memory_operand_valid(1U)
        )
    {
        ASSUMPTION(re.get_base_register_of_memory_operand(1U,D).second == 8ULL);

        bool const  use_index_reg = re.is_index_register_of_memory_operand_valid(1U);
        bool const  use_scale = re.get_scale_of_index_register_of_memory_operand(1U) != 1;

        ASSUMPTION(!use_scale || use_index_reg);

        bool const  use_rip = re.get_base_register_of_memory_operand(1U,D) == get_register_info("rip",D);
        bool const  use_displacement = re.get_displacement_of_memory_operand(1U) != 0ULL;

        uint8_t const  num_bytes = re.get_num_bytes_of_operand(1U);

        node_id  n = component().entry();
        uint64_t  temp = re.get_base_register_of_memory_operand(1U,D).first;
        if (use_scale || use_index_reg)
        {
            ASSUMPTION(re.get_index_register_of_memory_operand(1U,D).second == 8ULL);

            if (use_scale)
            {
                n = component().insert_sequence(n,{
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_TIMES_NUMBER(
                                8U,
                                D.start_address_of_temporaries(),
                                re.get_index_register_of_memory_operand(1U,D).first,
                                re.get_scale_of_index_register_of_memory_operand(1U)
                                ),
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                                8U,
                                D.start_address_of_temporaries(),
                                D.start_address_of_temporaries(),
                                temp
                                )
                        });
            }
            else
            {
                n = component().insert_sequence(n,{
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                                8U,
                                D.start_address_of_temporaries(),
                                re.get_index_register_of_memory_operand(1U,D).first,
                                temp
                                )
                        });
            }
            temp = D.start_address_of_temporaries();
        }
        if (use_displacement || use_rip)
        {
            n = component().insert_sequence(n,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            D.start_address_of_temporaries(),
                            temp,
                            re.get_displacement_of_memory_operand(1U) + (use_rip ? re.num_bytes_of_instruction() : 0)
                            )
                    });
            temp = D.start_address_of_temporaries();
        }

        register_info const  target_reg = re.get_register_operand(0U,D);
        microcode::instruction const  zero_extend_32bit_target_register_to_64bit =
                target_reg.second == 4U ? microcode::create_SETANDCOPY__REG_ASGN_NUMBER(4U,target_reg.first - 4ULL,0ULL) : microcode::instruction();

        n = component().insert_sequence(n,{
                zero_extend_32bit_target_register_to_64bit,
                microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                        re.get_register_operand(0U,D).second,
                        re.get_register_operand(0U,D).first,
                        0ULL
                        ),
                microcode::create_DATATRANSFER__REG_ASGN_DEREF_INV_REG(
                        num_bytes,
                        re.get_register_operand(0U,D).first + re.get_register_operand(0U,D).second - num_bytes,
                        temp
                        ),
                (temp < D.start_address_of_temporaries() ?  microcode::instruction() :
                                                            microcode::create_HAVOC__REG_ASGN_HAVOC(
                                                                    8ULL,
                                                                    D.start_address_of_temporaries()
                                                                    )),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8U,
                        0ULL,
                        0ULL,
                        re.num_bytes_of_instruction()
                        )
                });

        return true;
    }

    return false;
}

bool  recognition_data::recognise_MOVDQU(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 2U
        && re.is_register_operand(0U)
        && !re.is_read_operand(0U)
        && re.is_write_operand(0U)
        && re.get_num_bytes_of_operand(0U) == uint128_t::size()
        && re.is_memory_operand(1U)
        && re.is_read_operand(1U)
        && !re.is_write_operand(1U)
        && !re.is_segment_register_of_memory_operand_valid(1U)
        && re.is_base_register_of_memory_operand_valid(1U)
        )
    {
        ASSUMPTION(re.get_register_operand(0U,D).second == uint128_t::size());
        ASSUMPTION(uint128_t::size() % 2ULL == 0ULL);
        ASSUMPTION(re.get_base_register_of_memory_operand(1U,D).second == 8ULL);

        bool const  use_index_reg = re.is_index_register_of_memory_operand_valid(1U);
        bool const  use_scale = re.get_scale_of_index_register_of_memory_operand(1U) != 1;

        ASSUMPTION(!use_scale || use_index_reg);

        bool const  use_rip = re.get_base_register_of_memory_operand(1U,D) == get_register_info("rip",D);
        bool const  use_displacement = re.get_displacement_of_memory_operand(1U) != 0ULL;

        node_id  n = component().entry();
        uint64_t  temp = re.get_base_register_of_memory_operand(1U,D).first;
        if (use_scale || use_index_reg)
        {
            ASSUMPTION(re.get_index_register_of_memory_operand(1U,D).second == 8ULL);

            if (use_scale)
            {
                n = component().insert_sequence(n,{
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_TIMES_NUMBER(
                                8U,
                                D.start_address_of_temporaries(),
                                re.get_index_register_of_memory_operand(1U,D).first,
                                re.get_scale_of_index_register_of_memory_operand(1U)
                                ),
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                                8U,
                                D.start_address_of_temporaries(),
                                D.start_address_of_temporaries(),
                                temp
                                )
                        });
            }
            else
            {
                n = component().insert_sequence(n,{
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                                8U,
                                D.start_address_of_temporaries(),
                                re.get_index_register_of_memory_operand(1U,D).first,
                                temp
                                )
                        });
            }
            temp = D.start_address_of_temporaries();
        }
        if (use_displacement || use_rip)
        {
            n = component().insert_sequence(n,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            D.start_address_of_temporaries(),
                            temp,
                            re.get_displacement_of_memory_operand(1U) + (use_rip ? re.num_bytes_of_instruction() : 0)
                            )
                    });
            temp = D.start_address_of_temporaries();
        }
        n = component().insert_sequence(n,{
                microcode::create_DATATRANSFER__REG_ASGN_DEREF_INV_REG(
                        (uint8_t)(uint128_t::size() / 2ULL),
                        re.get_register_operand(0U,D).first + uint128_t::size() / 2ULL,
                        temp
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8U,
                        D.start_address_of_temporaries(),
                        temp,
                        uint128_t::size() / 2ULL
                        ),
                microcode::create_DATATRANSFER__REG_ASGN_DEREF_INV_REG(
                        (uint8_t)(uint128_t::size() / 2ULL),
                        re.get_register_operand(0U,D).first,
                        D.start_address_of_temporaries()
                        ),
                microcode::create_HAVOC__REG_ASGN_HAVOC(
                        8ULL,
                        D.start_address_of_temporaries()
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8U,
                        0ULL,
                        0ULL,
                        re.num_bytes_of_instruction()
                        )
                });

        return true;
    }

    return false;
}

bool  recognition_data::recognise_SETE(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 1U
        && re.is_register_operand(0U)
        && !re.is_read_operand(0U)
        && re.is_write_operand(0U)
        && re.get_num_bytes_of_operand(0U) == 1ULL
        )
    {
        component().insert_sequence(component().entry(),{
                    microcode::create_SETANDCOPY__REG_ASGN_REG(
                            re.get_register_operand(0U,D).second,
                            re.get_register_operand(0U,D).first,
                            get_register_info("zf",D).first
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

        return true;
    }

    return false;
}

bool  recognition_data::recognise_SETNE(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 1U
        && re.is_register_operand(0U)
        && !re.is_read_operand(0U)
        && re.is_write_operand(0U)
        && re.get_num_bytes_of_operand(0U) == 1ULL
        )
    {
        component().insert_sequence(component().entry(),{
                    microcode::create_ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO(
                            re.get_register_operand(0U,D).second,
                            re.get_register_operand(0U,D).first,
                            get_register_info("zf",D).first
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

        return true;
    }

    return false;
}

bool  recognition_data::recognise_PMOVMSKB(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 2U
        && re.is_register_operand(0U)
        && !re.is_read_operand(0U)
        && re.is_write_operand(0U)
        && re.is_register_operand(1U)
        && re.is_read_operand(1U)
        && !re.is_write_operand(1U)
        && re.get_num_bytes_of_operand(0U) <= sizeof(uint64_t)
        && re.get_num_bytes_of_operand(1U) == uint128_t::size()
        )
    {
        ASSUMPTION(16ULL == uint128_t::size());
        register_info  const  reg0 = re.get_register_operand(0U,D);
        register_info  const  reg1 = re.get_register_operand(1U,D);

        uint64_t const  tgt_reg = reg0.first - (8ULL - reg0.second);

        node_id  u = component().insert_sequence(component().entry(),{
                microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                        8U,
                        tgt_reg,
                        0ULL
                        )
                });

        uint64_t const src[2] = { reg1.first, reg1.first + 8ULL };
        uint64_t const dst[2] = { tgt_reg + 6ULL, tgt_reg + 7ULL };

        for (uint8_t  j = 0U; j < 2U; ++j)
        {
            u = component().insert_sequence(u,{
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                            8U,
                            D.start_address_of_temporaries(),
                            src[j],
                            0x8080808080808080ULL
                            ),
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                            1U,
                            dst[j],
                            D.start_address_of_temporaries() + 7ULL,
                            7ULL
                            ),
                    });
            for (uint8_t  i = 1U; i < 8U; ++i)
            {
                u = component().insert_sequence(u,{
                        i != 7U ?   microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                                            1U,
                                            D.start_address_of_temporaries() + 7ULL - i,
                                            D.start_address_of_temporaries() + 7ULL - i,
                                            7ULL - i
                                            ) :
                                    microcode::instruction(
                                            ),
                        microcode::create_BITOPERATIONS__REG_ASGN_REG_OR_REG(
                                1U,
                                dst[j],
                                dst[j],
                                D.start_address_of_temporaries() + 7ULL - i
                                ),
                        });
            }
        }

        component().insert_sequence(u,{
                microcode::create_HAVOC__REG_ASGN_HAVOC(
                        8U,
                        D.start_address_of_temporaries()
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8U,
                        0ULL,
                        0ULL,
                        re.num_bytes_of_instruction()
                        )
                });

        return true;
    }

    return false;
}

bool  recognition_data::recognise_CMOVE(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 2U
        && re.is_register_operand(0U)
        && re.is_read_operand(0U)
        && re.is_write_operand(0U)
        && re.is_register_operand(1U)
        && re.is_read_operand(1U)
        && !re.is_write_operand(1U)
        )
    {
        node_id  u,v;
        std::tie(u,v) =
                component().insert_branching(microcode::GIK::GUARDS__REG_NOT_EQUAL_TO_ZERO,get_register_info("zf",D).second,get_register_info("zf",D).first,component().entry());

        register_info const  target_reg = re.get_register_operand(0U,D);
        microcode::instruction const  zero_extend_32bit_target_register_to_64bit =
                target_reg.second == 4U ? microcode::create_SETANDCOPY__REG_ASGN_NUMBER(4U,target_reg.first - 4ULL,0ULL) : microcode::instruction();

        component().insert_sequence(u,{
                    microcode::create_SETANDCOPY__REG_ASGN_REG(
                            re.get_register_operand(0U,D).second,
                            re.get_register_operand(0U,D).first,
                            re.get_register_operand(1U,D).first
                            ),
                    zero_extend_32bit_target_register_to_64bit
                    },v);

        component().insert_sequence(v,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

        return true;
    }

    return false;
}

bool  recognition_data::recognise_CMOVA(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 2U
        && re.is_register_operand(0U)
        && re.is_read_operand(0U)
        && re.is_write_operand(0U)
        && re.is_register_operand(1U)
        && re.is_read_operand(1U)
        && !re.is_write_operand(1U)
        )
    {
        node_id  u =
        component().insert_sequence(component().entry(),{
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_OR_REG(
                            1U,
                            D.start_address_of_temporaries(),
                            get_register_info("cf",D).first,
                            get_register_info("zf",D).first
                            ),
                    });

        node_id  v;
        std::tie(u,v) =
                component().insert_branching(microcode::GIK::GUARDS__REG_EQUAL_TO_ZERO,1U,D.start_address_of_temporaries(),u);

        register_info const  target_reg = re.get_register_operand(0U,D);
        microcode::instruction const  zero_extend_32bit_target_register_to_64bit =
                target_reg.second == 4U ? microcode::create_SETANDCOPY__REG_ASGN_NUMBER(4U,target_reg.first - 4ULL,0ULL) : microcode::instruction();

        component().insert_sequence(u,{
                    zero_extend_32bit_target_register_to_64bit,
                    microcode::create_SETANDCOPY__REG_ASGN_REG(
                            re.get_register_operand(0U,D).second,
                            re.get_register_operand(0U,D).first,
                            re.get_register_operand(1U,D).first
                            )
                    },v);

        component().insert_sequence(v,{
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            1ULL,
                            D.start_address_of_temporaries()
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

        return true;
    }

    return false;
}


bool  recognition_data::recognise_LEA(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 2U
        && re.is_register_operand(0U)
        && re.is_write_operand(0U)
        && re.is_memory_operand(1U)
        && re.is_read_operand(1U)
        && !re.is_write_operand(1U)
        && !re.is_segment_register_of_memory_operand_valid(1U)
        && re.is_base_register_of_memory_operand_valid(1U)
        )
    {
        ASSUMPTION(re.get_register_operand(0U,D).second == 8ULL);
        ASSUMPTION(re.get_base_register_of_memory_operand(1U,D).second == 8ULL);

        bool const  use_index_reg = re.is_index_register_of_memory_operand_valid(1U);
        bool const  use_scale = re.get_scale_of_index_register_of_memory_operand(1U) != 1;

        ASSUMPTION(!use_scale || use_index_reg);

        bool const  use_rip = re.get_base_register_of_memory_operand(1U,D) == get_register_info("rip",D);
        bool const  use_displacement = re.get_displacement_of_memory_operand(1U) != 0ULL || use_rip;

        node_id  n = component().entry();
        uint64_t  temp = re.get_base_register_of_memory_operand(1U,D).first;
        if (use_scale || use_index_reg)
        {
            ASSUMPTION(re.get_index_register_of_memory_operand(1U,D).second == 8ULL);

            if (use_scale)
            {
                n = component().insert_sequence(n,{
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_TIMES_NUMBER(
                                re.get_index_register_of_memory_operand(1U,D).second,
                                D.start_address_of_temporaries(),
                                re.get_index_register_of_memory_operand(1U,D).first,
                                re.get_scale_of_index_register_of_memory_operand(1U)
                                ),
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                                re.get_base_register_of_memory_operand(1U,D).second,
                                D.start_address_of_temporaries(),
                                D.start_address_of_temporaries(),
                                temp
                                )
                        });
            }
            else
            {
                n = component().insert_sequence(n,{
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                                re.get_base_register_of_memory_operand(1U,D).second,
                                D.start_address_of_temporaries(),
                                re.get_index_register_of_memory_operand(1U,D).first,
                                temp
                                )
                        });
            }
            temp = D.start_address_of_temporaries();
        }
        if (use_displacement || use_rip)
        {
            n = component().insert_sequence(n,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            re.get_base_register_of_memory_operand(1U,D).second,
                            D.start_address_of_temporaries(),
                            temp,
                            re.get_displacement_of_memory_operand(1U) + (use_rip ? re.num_bytes_of_instruction() : 0)
                            ),
                    });
            temp = D.start_address_of_temporaries();
        }
        n = component().insert_sequence(n,{
                microcode::create_SETANDCOPY__REG_ASGN_REG(
                        re.get_register_operand(0U,D).second,
                        re.get_register_operand(0U,D).first,
                        temp
                        )
                });
        if (use_scale || use_displacement || use_rip)
        {
            n = component().insert_sequence(n,{
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            8ULL,
                            temp
                            ),
                    });
        }
        component().insert_sequence(n,{
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8U,
                        0ULL,
                        0ULL,
                        re.num_bytes_of_instruction()
                        )
                });

        return true;
    }

    return false;
}


bool  recognition_data::recognise_TEST(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 2U
        && re.is_register_operand(0U)
        && re.is_read_operand(0U)
        && !re.is_write_operand(0U)
        && re.is_register_operand(1U)
        && re.is_read_operand(1U)
        && !re.is_write_operand(1U)
        && re.get_num_bytes_of_operand(0U) == re.get_num_bytes_of_operand(1U)
        )
    {
        component().insert_sequence(component().entry(),{
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_REG(
                            re.get_register_operand(0U,D).second,
                            D.start_address_of_temporaries(),
                            re.get_register_operand(0U,D).first,
                            re.get_register_operand(1U,D).first
                            ),
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                            1U,
                            get_register_info("sf",D).first,
                            D.start_address_of_temporaries(),
                            7U
                            ),
                    microcode::create_ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO(
                            re.get_register_operand(0U,D).second,
                            get_register_info("zf",D).first,
                            D.start_address_of_temporaries()
                            ),
                    microcode::create_MISCELLANEOUS__REG_ASGN_PARITY_REG(
                            1U,
                            get_register_info("pf",D).first,
                            D.start_address_of_temporaries() + re.get_register_operand(0U,D).second - 1ULL
                            ),
                    microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                            1U,
                            get_register_info("of",D).first,
                            0ULL
                            ),
                    microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                            1U,
                            get_register_info("cf",D).first,
                            0ULL
                            ),
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            1U,
                            get_register_info("af",D).first
                            ),
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            re.get_register_operand(0U,D).second,
                            D.start_address_of_temporaries()
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

        return true;
    }

    if (re.num_operands() == 2U
        && re.is_register_operand(0U)
        && re.is_read_operand(0U)
        //&& !re.is_write_operand(0U) //!< There is a bug in capstone (it says the operand is writeable)
        && re.is_immediate_operand(1U)
        && re.is_read_operand(1U)
        && !re.is_write_operand(1U)
        && re.get_num_bytes_of_operand(0U) == re.get_num_bytes_of_operand(1U)
        )
    {
        component().insert_sequence(component().entry(),{
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                            re.get_register_operand(0U,D).second,
                            D.start_address_of_temporaries(),
                            re.get_register_operand(0U,D).first,
                            re.get_immediate_operand(1U)
                            ),
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                            1U,
                            get_register_info("sf",D).first,
                            D.start_address_of_temporaries(),
                            7U
                            ),
                    microcode::create_ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO(
                            re.get_register_operand(0U,D).second,
                            get_register_info("zf",D).first,
                            D.start_address_of_temporaries()
                            ),
                    microcode::create_MISCELLANEOUS__REG_ASGN_PARITY_REG(
                            1U,
                            get_register_info("pf",D).first,
                            D.start_address_of_temporaries() + re.get_register_operand(0U,D).second - 1ULL
                            ),
                    microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                            1U,
                            get_register_info("of",D).first,
                            0ULL
                            ),
                    microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                            1U,
                            get_register_info("cf",D).first,
                            0ULL
                            ),
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            1U,
                            get_register_info("af",D).first
                            ),
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            re.get_register_operand(0U,D).second,
                            D.start_address_of_temporaries()
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

        return true;
    }

    if (re.num_operands() == 2U
        && re.is_memory_operand(0U)
        && re.is_read_operand(0U)
        //&& !re.is_write_operand(0U) //!< There is a bug in capstone (it says the operand is writeable)
        && !re.is_segment_register_of_memory_operand_valid(0U)
        && re.is_base_register_of_memory_operand_valid(0U)
        && re.is_immediate_operand(1U)
        && re.is_read_operand(1U)
        && re.get_num_bytes_of_operand(0U) == re.get_num_bytes_of_operand(1U)
        )
    {
        ASSUMPTION(re.get_base_register_of_memory_operand(0U,D).second == 8ULL);

        bool const  use_index_reg = re.is_index_register_of_memory_operand_valid(0U);
        bool const  use_scale = re.get_scale_of_index_register_of_memory_operand(0U) != 1;

        ASSUMPTION(!use_scale || use_index_reg);

        bool const  use_rip = re.get_base_register_of_memory_operand(0U,D) == get_register_info("rip",D);
        bool const  use_displacement = re.get_displacement_of_memory_operand(0U) != 0ULL;

        uint8_t const  num_bytes = re.get_num_bytes_of_operand(0U);

        node_id  u = component().entry();
        uint64_t  temp = re.get_base_register_of_memory_operand(0U,D).first;
        if (use_scale || use_index_reg)
        {
            ASSUMPTION(re.get_index_register_of_memory_operand(0U,D).second == 8ULL);

            if (use_scale)
            {
                u = component().insert_sequence(u,{
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_TIMES_NUMBER(
                                re.get_index_register_of_memory_operand(0U,D).second,
                                D.start_address_of_temporaries(),
                                re.get_index_register_of_memory_operand(0U,D).first,
                                re.get_scale_of_index_register_of_memory_operand(0U)
                                ),
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                                re.get_base_register_of_memory_operand(0U,D).second,
                                D.start_address_of_temporaries(),
                                D.start_address_of_temporaries(),
                                temp
                                )
                        });
            }
            else
            {
                u = component().insert_sequence(u,{
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                                re.get_base_register_of_memory_operand(0U,D).second,
                                D.start_address_of_temporaries(),
                                re.get_index_register_of_memory_operand(0U,D).first,
                                temp
                                )
                        });
            }
            temp = D.start_address_of_temporaries();
        }
        if (use_displacement || use_rip)
        {
            u = component().insert_sequence(u,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            re.get_base_register_of_memory_operand(0U,D).second,
                            D.start_address_of_temporaries(),
                            temp,
                            re.get_displacement_of_memory_operand(0U) + (use_rip ? re.num_bytes_of_instruction() : 0)
                            )
                    });
            temp = D.start_address_of_temporaries();
        }

        uint8_t const num_clear_bytes = temp < D.start_address_of_temporaries() ? num_bytes : std::max(num_bytes,(uint8_t)8);

        component().insert_sequence(u,{
                    microcode::create_DATATRANSFER__REG_ASGN_DEREF_INV_REG(
                            num_bytes,
                            D.start_address_of_temporaries(),
                            temp
                            ),
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                            num_bytes,
                            D.start_address_of_temporaries(),
                            D.start_address_of_temporaries(),
                            re.get_immediate_operand(1U)
                            ),
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                            1U,
                            get_register_info("sf",D).first,
                            D.start_address_of_temporaries(),
                            7U
                            ),
                    microcode::create_ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO(
                            num_bytes,
                            get_register_info("zf",D).first,
                            D.start_address_of_temporaries()
                            ),
                    microcode::create_MISCELLANEOUS__REG_ASGN_PARITY_REG(
                            1U,
                            get_register_info("pf",D).first,
                            D.start_address_of_temporaries() + num_bytes - 1ULL
                            ),
                    microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                            1U,
                            get_register_info("of",D).first,
                            0ULL
                            ),
                    microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                            1U,
                            get_register_info("cf",D).first,
                            0ULL
                            ),
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            1U,
                            get_register_info("af",D).first
                            ),
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            num_clear_bytes,
                            D.start_address_of_temporaries()
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

        return true;
    }

    return false;
}

bool  recognition_data::recognise_PCMPEQB(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 2U
        && re.is_register_operand(0U)
        && re.is_read_operand(0U)
        && re.is_write_operand(0U)
        && re.is_register_operand(1U)
        && re.is_read_operand(1U)
        && !re.is_write_operand(1U)
        && re.get_num_bytes_of_operand(0U) == re.get_num_bytes_of_operand(1U)
        && re.get_num_bytes_of_operand(0U) == uint128_t::size()
        )
    {
        uint8_t const n = 8ULL;
        ASSUMPTION(2ULL * n == uint128_t::size());
        register_info  const  reg0 = re.get_register_operand(0U,D);
        register_info  const  reg1 = re.get_register_operand(1U,D);
        uint64_t const  lo0 = reg0.first + n;
        uint64_t const  hi0 = reg0.first;
        uint64_t const  lo1 = reg1.first + n;
        uint64_t const  hi1 = reg1.first;

        node_id  u = component().insert_sequence(component().entry(),{
                microcode::create_BITOPERATIONS__REG_ASGN_REG_XOR_REG(
                        n,
                        lo0,
                        lo0,
                        lo1
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_XOR_REG(
                        n,
                        hi0,
                        hi0,
                        hi1
                        ),
                });

        for (uint64_t  i = hi0; i != lo0+n; ++i)
        {
            node_id  v;
            std::tie(u,v) = component().insert_branching(microcode::GIK::GUARDS__REG_EQUAL_TO_ZERO,1U,i,u);
            component().insert_sequence(v,{
                    microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                            1U,
                            i,
                            255ULL
                            )
                    },u);
        }

        component().insert_sequence(u,{
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        n,
                        lo0,
                        lo0
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        n,
                        hi0,
                        hi0
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8U,
                        0ULL,
                        0ULL,
                        re.num_bytes_of_instruction()
                        )
                });

        return true;
    }

    if (re.num_operands() == 2U
        && re.is_register_operand(0U)
        && re.is_read_operand(0U)
        && re.is_write_operand(0U)
        && re.get_num_bytes_of_operand(0U) == uint128_t::size()
        && re.is_memory_operand(1U)
        && re.is_read_operand(1U)
        && !re.is_write_operand(1U)
        && !re.is_segment_register_of_memory_operand_valid(1U)
        && re.is_base_register_of_memory_operand_valid(1U)
        )
    {
        ASSUMPTION(re.get_base_register_of_memory_operand(1U,D).second == 8ULL);

        bool const  use_index_reg = re.is_index_register_of_memory_operand_valid(1U);
        bool const  use_scale = re.get_scale_of_index_register_of_memory_operand(1U) != 1;

        ASSUMPTION(!use_scale || use_index_reg);

        bool const  use_rip = re.get_base_register_of_memory_operand(1U,D) == get_register_info("rip",D);
        bool const  use_displacement = re.get_displacement_of_memory_operand(1U) != 0ULL;

        register_info  const  reg0 = re.get_register_operand(0U,D);
        uint64_t const  lo0 = reg0.first + 8ULL;
        uint64_t const  hi0 = reg0.first;
        uint64_t const  lo1 = D.start_address_of_temporaries() + 16ULL;
        uint64_t const  hi1 = D.start_address_of_temporaries() + 8ULL;

        node_id  n = component().entry();
        uint64_t  temp = re.get_base_register_of_memory_operand(1U,D).first;
        if (use_scale || use_index_reg)
        {
            ASSUMPTION(re.get_index_register_of_memory_operand(1U,D).second == 8ULL);

            if (use_scale)
            {
                n = component().insert_sequence(n,{
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_TIMES_NUMBER(
                                8U,
                                D.start_address_of_temporaries(),
                                re.get_index_register_of_memory_operand(1U,D).first,
                                re.get_scale_of_index_register_of_memory_operand(1U)
                                ),
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                                8U,
                                D.start_address_of_temporaries(),
                                D.start_address_of_temporaries(),
                                temp
                                )
                        });
            }
            else
            {
                n = component().insert_sequence(n,{
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                                8U,
                                D.start_address_of_temporaries(),
                                re.get_index_register_of_memory_operand(1U,D).first,
                                temp
                                )
                        });
            }
            temp = D.start_address_of_temporaries();
        }
        if (use_displacement || use_rip)
        {
            n = component().insert_sequence(n,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            D.start_address_of_temporaries(),
                            temp,
                            re.get_displacement_of_memory_operand(1U) + (use_rip ? re.num_bytes_of_instruction() : 0)
                            )
                    });
            temp = D.start_address_of_temporaries();
        }
        n = component().insert_sequence(n,{
                microcode::create_DATATRANSFER__REG_ASGN_DEREF_INV_REG(
                        8U,
                        lo1,
                        temp
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8U,
                        D.start_address_of_temporaries(),
                        temp,
                        8ULL
                        ),
                microcode::create_DATATRANSFER__REG_ASGN_DEREF_INV_REG(
                        8U,
                        hi1,
                        D.start_address_of_temporaries()
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_XOR_REG(
                        8U,
                        lo0,
                        lo0,
                        lo1
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_XOR_REG(
                        8U,
                        hi0,
                        hi0,
                        hi1
                        ),
                });

        for (uint64_t  i = hi0; i != lo0+8ULL; ++i)
        {
            node_id  v;
            std::tie(n,v) = component().insert_branching(microcode::GIK::GUARDS__REG_EQUAL_TO_ZERO,1U,i,n);
            component().insert_sequence(v,{
                    microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                            1U,
                            i,
                            255ULL
                            )
                    },n);
        }

        component().insert_sequence(n,{
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        8U,
                        lo0,
                        lo0
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        8U,
                        hi0,
                        hi0
                        ),
                microcode::create_HAVOC__REG_ASGN_HAVOC(
                        0x18ULL,
                        D.start_address_of_temporaries()
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8U,
                        0ULL,
                        0ULL,
                        re.num_bytes_of_instruction()
                        )
                });

        return true;
    }

    return false;
}

bool  recognition_data::recognise_CMPXCHG(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 2U
        && re.is_memory_operand(0U)
        && re.is_read_operand(0U)
        && !re.is_write_operand(0U)
        && !re.is_segment_register_of_memory_operand_valid(0U)
        && re.is_base_register_of_memory_operand_valid(0U)
        && re.is_register_operand(1U)
        && re.is_read_operand(1U)
        && !re.is_write_operand(1U)
        )
    {
        ASSUMPTION(re.get_base_register_of_memory_operand(0U,D).second == 8ULL);

        bool const  use_index_reg = re.is_index_register_of_memory_operand_valid(0U);
        bool const  use_scale = re.get_scale_of_index_register_of_memory_operand(0U) != 1;

        ASSUMPTION(!use_scale || use_index_reg);

        bool const  use_rip = re.get_base_register_of_memory_operand(0U,D) == get_register_info("rip",D);
        bool const  use_displacement = re.get_displacement_of_memory_operand(0U) != 0ULL;

        node_id  u = component().entry();
        uint64_t  temp = re.get_base_register_of_memory_operand(0U,D).first;
        if (use_scale || use_index_reg)
        {
            ASSUMPTION(re.get_index_register_of_memory_operand(0U,D).second == 8ULL);

            if (use_scale)
            {
                u = component().insert_sequence(u,{
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_TIMES_NUMBER(
                                re.get_index_register_of_memory_operand(0U,D).second,
                                D.start_address_of_temporaries(),
                                re.get_index_register_of_memory_operand(0U,D).first,
                                re.get_scale_of_index_register_of_memory_operand(0U)
                                ),
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                                re.get_base_register_of_memory_operand(0U,D).second,
                                D.start_address_of_temporaries(),
                                D.start_address_of_temporaries(),
                                temp
                                )
                        });
            }
            else
            {
                u = component().insert_sequence(u,{
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                                re.get_base_register_of_memory_operand(0U,D).second,
                                D.start_address_of_temporaries(),
                                re.get_index_register_of_memory_operand(0U,D).first,
                                temp
                                )
                        });
            }
            temp = D.start_address_of_temporaries();
        }
        if (use_displacement || use_rip)
        {
            u = component().insert_sequence(u,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            re.get_base_register_of_memory_operand(0U,D).second,
                            D.start_address_of_temporaries(),
                            temp,
                            re.get_displacement_of_memory_operand(0U) + (use_rip ? re.num_bytes_of_instruction() : 0)
                            )
                    });
            temp = D.start_address_of_temporaries();
        }



        uint8_t const  n = re.get_num_bytes_of_operand(1U);
        uint8_t const  N = 2U * n;

        register_info  cmp_reg;
        switch (n)
        {
        case 1U: cmp_reg = get_register_info("al",D); break;
        case 2U: cmp_reg = get_register_info("ax",D); break;
        case 4U: cmp_reg = get_register_info("eax",D); break;
        case 8U: cmp_reg = get_register_info("rax",D); break;
        default: UNREACHABLE();
        }

        ASSUMPTION(cmp_reg.second == n);
        ASSUMPTION(re.get_register_operand(1U,D).second == n);

        uint64_t const  res_hi_begin = temp < D.start_address_of_temporaries() ? D.start_address_of_temporaries() : temp + 8ULL;
        uint64_t const  res_lo_begin = res_hi_begin + n;
        uint64_t const  res_end = res_lo_begin + n;

        uint64_t const  arg0_hi_begin = res_end;
        uint64_t const  arg0_lo_begin = arg0_hi_begin + n;
        uint64_t const  arg0_end = arg0_lo_begin + n;

        uint64_t const  arg1_hi_begin = arg0_end;
        uint64_t const  arg1_lo_begin = arg1_hi_begin + n;
        uint64_t const  arg1_end = arg1_lo_begin + n;

        uint64_t const  tmp0 = arg1_end;
        uint64_t const  tmp1 = tmp0 + 1ULL;
        uint64_t const  tmp2 = tmp1 + 1ULL;

        uint64_t const  num_used_bytes = tmp2 - D.start_address_of_temporaries() + 1ULL;

        u = component().insert_sequence(u,{
                microcode::create_SETANDCOPY__REG_ASGN_REG(
                        n,
                        arg0_hi_begin,
                        cmp_reg.first
                        ),
                microcode::create_TYPECASTING__REG_ASGN_ZERO_EXTEND_REG(
                        n,
                        arg0_hi_begin,
                        arg0_hi_begin
                        ),
                microcode::create_DATATRANSFER__REG_ASGN_DEREF_INV_REG(
                        n,
                        arg1_hi_begin,
                        temp
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        n,
                        arg1_hi_begin,
                        arg1_hi_begin
                        ),
                microcode::create_TYPECASTING__REG_ASGN_SIGN_EXTEND_REG(
                        n,
                        arg1_hi_begin,
                        arg1_hi_begin
                        ),
                });
        if (N > 8U)
        {
            ASSUMPTION(N == 16U);
            u = component().insert_sequence(u,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            N,
                            arg1_hi_begin,
                            arg1_hi_begin,
                            (uint128_t)1ULL
                            )
                    });
        }
        else
        {
            u = component().insert_sequence(u,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            N,
                            arg1_hi_begin,
                            arg1_hi_begin,
                            1ULL
                            )
                    });
        }
        u = component().insert_sequence(u,{
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                        N,
                        res_hi_begin,
                        arg0_hi_begin,
                        arg1_hi_begin
                        )
                });
        u = component().insert_sequence(u,{
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1ULL,
                        get_register_info("cf",D).first,
                        res_hi_begin + n - 1ULL,
                        1ULL
                        ),
                microcode::create_MISCELLANEOUS__REG_ASGN_PARITY_REG(
                        1U,
                        get_register_info("pf",D).first,
                        res_lo_begin + n - 1ULL
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        tmp1,
                        arg0_lo_begin + n - 1U,
                        15U
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        tmp2,
                        arg1_lo_begin + n - 1U,
                        15U
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        1U,
                        tmp2,
                        tmp2
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        1ULL,
                        tmp2,
                        tmp2,
                        1ULL
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                        1ULL,
                        tmp0,
                        tmp1,
                        tmp2
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                        1U,
                        tmp0,
                        tmp0,
                        4U
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        get_register_info("af",D).first,
                        tmp0,
                        1ULL
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                        1U,
                        get_register_info("sf",D).first,
                        res_lo_begin,
                        7U
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        1U,
                        tmp0,
                        arg0_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_REG(
                        1U,
                        tmp0,
                        tmp0,
                        arg1_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_REG(
                        1U,
                        tmp0,
                        tmp0,
                        res_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        1U,
                        tmp2,
                        arg1_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_REG(
                        1U,
                        tmp1,
                        arg0_lo_begin,
                        tmp2
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        1U,
                        tmp2,
                        res_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_REG(
                        1U,
                        tmp1,
                        tmp1,
                        tmp2
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_OR_REG(
                        1U,
                        tmp0,
                        tmp0,
                        tmp1
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                        1U,
                        get_register_info("of",D).first,
                        tmp0,
                        7U
                        ),
                });

        node_id  v;
        std::tie(u,v) =
                component().insert_branching(microcode::GIK::GUARDS__REG_EQUAL_TO_ZERO,n,res_lo_begin,u);

        u = component().insert_sequence(u,{
                microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                        1U,
                        get_register_info("zf",D).first,
                        1U
                        ),
                microcode::create_DATATRANSFER__DEREF_INV_REG_ASGN_REG(
                        n,
                        temp,
                        re.get_register_operand(1U,D).first
                        ),
                });

        component().insert_sequence(v,{
                microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                        1U,
                        get_register_info("zf",D).first,
                        0U
                        ),
                microcode::create_DATATRANSFER__REG_ASGN_DEREF_INV_REG(
                        n,
                        cmp_reg.first,
                        temp
                        ),
                },u);

        component().insert_sequence(u,{
                microcode::create_HAVOC__REG_ASGN_HAVOC(
                        num_used_bytes,
                        res_hi_begin
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8U,
                        0ULL,
                        0ULL,
                        re.num_bytes_of_instruction()
                        )
                });

        return true;
    }

    return false;
}


bool  recognition_data::recognise_INC(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 1U
        && re.is_register_operand(0U)
        && re.is_read_operand(0U)
        && re.is_write_operand(0U)
        )
    {
        register_info const  reg_info = re.get_register_operand(0U,D);

        component().insert_sequence(component().entry(),{
                microcode::create_SETANDCOPY__REG_ASGN_REG(
                        reg_info.second,
                        D.start_address_of_temporaries(),
                        reg_info.first
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        reg_info.second,
                        reg_info.first,
                        reg_info.first,
                        1ULL
                        ),
                microcode::create_MISCELLANEOUS__REG_ASGN_PARITY_REG(
                        1U,
                        get_register_info("pf",D).first,
                        reg_info.first + reg_info.second - 1U
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        D.start_address_of_temporaries() + reg_info.second - 1U,
                        D.start_address_of_temporaries() + reg_info.second - 1U,
                        15U
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        1U,
                        D.start_address_of_temporaries() + reg_info.second - 1U,
                        D.start_address_of_temporaries() + reg_info.second - 1U,
                        1ULL
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                        1U,
                        D.start_address_of_temporaries() + reg_info.second - 1U,
                        D.start_address_of_temporaries() + reg_info.second - 1U,
                        4U
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        get_register_info("af",D).first,
                        D.start_address_of_temporaries() + reg_info.second - 1U,
                        1ULL
                        ),
                microcode::create_ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO(
                        reg_info.second,
                        get_register_info("zf",D).first,
                        reg_info.first
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                        1U,
                        get_register_info("sf",D).first,
                        reg_info.first,
                        7U
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        1U,
                        D.start_address_of_temporaries(),
                        D.start_address_of_temporaries()
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_REG(
                        1U,
                        D.start_address_of_temporaries(),
                        D.start_address_of_temporaries(),
                        reg_info.first
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                        1U,
                        get_register_info("of",D).first,
                        D.start_address_of_temporaries(),
                        7U
                        ),
                microcode::create_HAVOC__REG_ASGN_HAVOC(
                        8ULL,
                        D.start_address_of_temporaries()
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8U,
                        0ULL,
                        0ULL,
                        re.num_bytes_of_instruction()
                        )
                });

        return true;
    }

    return false;
}

bool  recognition_data::recognise_DEC(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 1U
        && re.is_register_operand(0U)
        && re.is_read_operand(0U)
        && re.is_write_operand(0U)
        )
    {
        register_info const  reg_info = re.get_register_operand(0U,D);

        component().insert_sequence(component().entry(),{
                microcode::create_SETANDCOPY__REG_ASGN_REG(
                        reg_info.second,
                        D.start_address_of_temporaries(),
                        reg_info.first
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        reg_info.second,
                        reg_info.first,
                        reg_info.first,
                        0xffffffffffffffffULL
                        ),
                microcode::create_MISCELLANEOUS__REG_ASGN_PARITY_REG(
                        1U,
                        get_register_info("pf",D).first,
                        reg_info.first + reg_info.second - 1U
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        D.start_address_of_temporaries() + reg_info.second - 1U,
                        D.start_address_of_temporaries() + reg_info.second - 1U,
                        15U
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        1U,
                        D.start_address_of_temporaries() + reg_info.second - 1U,
                        D.start_address_of_temporaries() + reg_info.second - 1U,
                        0xfULL
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                        1U,
                        D.start_address_of_temporaries() + reg_info.second - 1U,
                        D.start_address_of_temporaries() + reg_info.second - 1U,
                        4U
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        get_register_info("af",D).first,
                        D.start_address_of_temporaries() + reg_info.second - 1U,
                        1ULL
                        ),
                microcode::create_ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO(
                        reg_info.second,
                        get_register_info("zf",D).first,
                        reg_info.first
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                        1U,
                        get_register_info("sf",D).first,
                        reg_info.first,
                        7U
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        1U,
                        D.start_address_of_temporaries() + 1ULL,
                        reg_info.first
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_REG(
                        1U,
                        D.start_address_of_temporaries(),
                        D.start_address_of_temporaries(),
                        D.start_address_of_temporaries() + 1ULL
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                        1U,
                        get_register_info("of",D).first,
                        D.start_address_of_temporaries(),
                        7U
                        ),
                microcode::create_HAVOC__REG_ASGN_HAVOC(
                        8ULL,
                        D.start_address_of_temporaries()
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8U,
                        0ULL,
                        0ULL,
                        re.num_bytes_of_instruction()
                        )
                });

        return true;
    }

    if (re.num_operands() == 1U
        && re.is_memory_operand(0U)
        && re.is_read_operand(0U)
        && re.is_write_operand(0U)
        && !re.is_segment_register_of_memory_operand_valid(0U)
        && re.is_base_register_of_memory_operand_valid(0U)
        )
    {
        ASSUMPTION(re.get_base_register_of_memory_operand(0U,D).second == 8ULL);

        bool const  use_index_reg = re.is_index_register_of_memory_operand_valid(0U);
        bool const  use_scale = re.get_scale_of_index_register_of_memory_operand(0U) != 1;

        ASSUMPTION(!use_scale || use_index_reg);

        bool const  use_rip = re.get_base_register_of_memory_operand(0U,D) == get_register_info("rip",D);
        bool const  use_displacement = re.get_displacement_of_memory_operand(0U) != 0ULL;

        node_id  u = component().entry();
        uint64_t  temp = re.get_base_register_of_memory_operand(0U,D).first;
        if (use_scale || use_index_reg)
        {
            ASSUMPTION(re.get_index_register_of_memory_operand(0U,D).second == 8ULL);

            if (use_scale)
            {
                u = component().insert_sequence(u,{
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_TIMES_NUMBER(
                                re.get_index_register_of_memory_operand(0U,D).second,
                                D.start_address_of_temporaries(),
                                re.get_index_register_of_memory_operand(0U,D).first,
                                re.get_scale_of_index_register_of_memory_operand(0U)
                                ),
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                                re.get_base_register_of_memory_operand(0U,D).second,
                                D.start_address_of_temporaries(),
                                D.start_address_of_temporaries(),
                                temp
                                )
                        });
            }
            else
            {
                u = component().insert_sequence(u,{
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                                re.get_base_register_of_memory_operand(0U,D).second,
                                D.start_address_of_temporaries(),
                                re.get_index_register_of_memory_operand(0U,D).first,
                                temp
                                )
                        });
            }

            temp = D.start_address_of_temporaries();
        }
        if (use_displacement || use_rip)
        {
            u = component().insert_sequence(u,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            re.get_base_register_of_memory_operand(0U,D).second,
                            D.start_address_of_temporaries(),
                            temp,
                            re.get_displacement_of_memory_operand(0U) + (use_rip ? re.num_bytes_of_instruction() : 0)
                            )
                    });
            temp = D.start_address_of_temporaries();
        }

        uint8_t const  n = re.get_num_bytes_of_operand(0U);
        uint64_t const  orig_begin = temp < D.start_address_of_temporaries() ? D.start_address_of_temporaries() : temp + 8ULL;
        uint64_t const  res_begin = orig_begin + n;

        u = component().insert_sequence(u,{
                microcode::create_DATATRANSFER__REG_ASGN_DEREF_INV_REG(
                        n,
                        orig_begin,
                        temp
                        ),
                microcode::create_SETANDCOPY__REG_ASGN_REG(
                        n,
                        res_begin,
                        orig_begin
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        n,
                        res_begin,
                        res_begin,
                        0xffffffffffffffffULL
                        ),
                microcode::create_DATATRANSFER__DEREF_INV_REG_ASGN_REG(
                        n,
                        temp,
                        res_begin
                        ),
                microcode::create_MISCELLANEOUS__REG_ASGN_PARITY_REG(
                        1U,
                        get_register_info("pf",D).first,
                        res_begin + n - 1U
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        orig_begin + n - 1U,
                        orig_begin + n - 1U,
                        15U
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        1U,
                        orig_begin + n - 1U,
                        orig_begin + n - 1U,
                        0xfULL
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                        1U,
                        orig_begin + n - 1U,
                        orig_begin + n - 1U,
                        4U
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        get_register_info("af",D).first,
                        orig_begin + n - 1U,
                        1ULL
                        ),
                microcode::create_ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO(
                        n,
                        get_register_info("zf",D).first,
                        res_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                        1U,
                        get_register_info("sf",D).first,
                        res_begin,
                        7U
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        1U,
                        orig_begin + 1ULL,
                        res_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_REG(
                        1U,
                        orig_begin,
                        orig_begin,
                        orig_begin + 1ULL
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                        1U,
                        get_register_info("of",D).first,
                        orig_begin,
                        7U
                        ),
                microcode::create_HAVOC__REG_ASGN_HAVOC(
                        res_begin + 1ULL - D.start_address_of_temporaries(),
                        D.start_address_of_temporaries()
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8U,
                        0ULL,
                        0ULL,
                        re.num_bytes_of_instruction()
                        )
                });

        return true;
    }

    return false;
}

bool  recognition_data::recognise_ADD(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 2U
        && re.is_register_operand(0U)
        && re.is_read_operand(0U)
        && re.is_write_operand(0U)
        && re.is_immediate_operand(1U)
        && re.is_read_operand(1U)
        )
    {
        register_info const  reg_info = re.get_register_operand(0U,D);

        uint8_t const  n = reg_info.second;
        uint8_t const  N = 2U * n;

        uint64_t const  res_hi_begin = D.start_address_of_temporaries();
        uint64_t const  res_lo_begin = res_hi_begin + n;
        uint64_t const  res_end = res_lo_begin + n;

        uint64_t const  arg0_hi_begin = res_end;
        uint64_t const  arg0_lo_begin = arg0_hi_begin + n;
        uint64_t const  arg0_end = arg0_lo_begin + n;

        uint64_t const  arg1_hi_begin = arg0_end;
        uint64_t const  arg1_lo_begin = arg1_hi_begin + n;
        uint64_t const  arg1_end = arg1_lo_begin + n;

        uint64_t const  tmp0 = arg1_end;
        uint64_t const  tmp1 = tmp0 + 1ULL;
        uint64_t const  tmp2 = tmp1 + 1ULL;

        uint64_t const  num_used_bytes = tmp2 - res_hi_begin + 1ULL;

        component().insert_sequence(component().entry(),{
                microcode::create_TYPECASTING__REG_ASGN_ZERO_EXTEND_REG(
                        n,
                        arg0_hi_begin,
                        reg_info.first
                        ),
                microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                        n,
                        arg1_hi_begin,
                        0ULL
                        ),
                microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                        n,
                        arg1_lo_begin,
                        re.get_immediate_operand(1U)
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                        N,
                        res_hi_begin,
                        arg0_hi_begin,
                        arg1_hi_begin
                        ),
                microcode::create_SETANDCOPY__REG_ASGN_REG(
                        n,
                        reg_info.first,
                        res_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1ULL,
                        get_register_info("cf",D).first,
                        res_hi_begin + n - 1ULL,
                        1ULL
                        ),
                microcode::create_MISCELLANEOUS__REG_ASGN_PARITY_REG(
                        1U,
                        get_register_info("pf",D).first,
                        reg_info.first + n - 1ULL
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        tmp1,
                        arg0_lo_begin + n - 1U,
                        15U
                        ),
                microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                        1U,
                        tmp2,
                        (uint64_t)re.get_immediate_operand(1U) & 15ULL
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                        1U,
                        tmp0,
                        tmp1,
                        tmp2
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                        1U,
                        tmp0,
                        tmp0,
                        4U
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        get_register_info("af",D).first,
                        tmp0,
                        1ULL
                        ),
                microcode::create_ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO(
                        n,
                        get_register_info("zf",D).first,
                        reg_info.first
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                        1U,
                        get_register_info("sf",D).first,
                        reg_info.first,
                        7U
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        1U,
                        tmp0,
                        reg_info.first
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        tmp1,
                        arg0_lo_begin,
                        ((uint64_t)re.get_immediate_operand(1U) >> (7ULL * 8ULL)) & 0x80ULL
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_REG(
                        1U,
                        tmp0,
                        tmp0,
                        tmp1
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        1U,
                        tmp1,
                        arg0_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        tmp1,
                        tmp1,
                        (~(uint64_t)re.get_immediate_operand(1U) >> (7ULL * 8ULL)) & 0x80ULL
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_REG(
                        1U,
                        tmp1,
                        tmp1,
                        reg_info.first
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_OR_REG(
                        1U,
                        tmp0,
                        tmp0,
                        tmp1
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                        1U,
                        get_register_info("of",D).first,
                        tmp0,
                        7U
                        ),
                microcode::create_HAVOC__REG_ASGN_HAVOC(
                        num_used_bytes,
                        res_hi_begin
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8U,
                        0ULL,
                        0ULL,
                        re.num_bytes_of_instruction()
                        )
                });

         return true;
     }

    if (re.num_operands() == 2U
        && re.is_register_operand(0U)
        && re.is_read_operand(0U)
        && re.is_write_operand(0U)
        && re.is_register_operand(1U)
        && re.is_read_operand(1U)
        && !re.is_write_operand(1U)
        )
    {
        register_info const  reg0_info = re.get_register_operand(0U,D);
        register_info const  reg1_info = re.get_register_operand(1U,D);

        ASSUMPTION(reg0_info.second == reg1_info.second);

        uint8_t const  n = reg0_info.second;
        uint8_t const  N = 2U * n;

        uint64_t const  res_hi_begin = D.start_address_of_temporaries();
        uint64_t const  res_lo_begin = res_hi_begin + n;
        uint64_t const  res_end = res_lo_begin + n;

        uint64_t const  arg0_hi_begin = res_end;
        uint64_t const  arg0_lo_begin = arg0_hi_begin + n;
        uint64_t const  arg0_end = arg0_lo_begin + n;

        uint64_t const  arg1_hi_begin = arg0_end;
        uint64_t const  arg1_lo_begin = arg1_hi_begin + n;
        uint64_t const  arg1_end = arg1_lo_begin + n;

        uint64_t const  tmp0 = arg1_end;
        uint64_t const  tmp1 = tmp0 + 1ULL;
        uint64_t const  tmp2 = tmp1 + 1ULL;

        uint64_t const  num_used_bytes = tmp2 - res_hi_begin + 1ULL;

        component().insert_sequence(component().entry(),{
                microcode::create_TYPECASTING__REG_ASGN_ZERO_EXTEND_REG(
                        n,
                        arg0_hi_begin,
                        reg0_info.first
                        ),
                 microcode::create_TYPECASTING__REG_ASGN_ZERO_EXTEND_REG(
                        n,
                        arg1_hi_begin,
                        reg1_info.first
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                        N,
                        res_hi_begin,
                        arg0_hi_begin,
                        arg1_hi_begin
                        ),
                microcode::create_SETANDCOPY__REG_ASGN_REG(
                        n,
                        reg0_info.first,
                        res_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1ULL,
                        get_register_info("cf",D).first,
                        res_hi_begin + n - 1ULL,
                        1ULL
                        ),
                microcode::create_MISCELLANEOUS__REG_ASGN_PARITY_REG(
                        1U,
                        get_register_info("pf",D).first,
                        reg0_info.first + n - 1ULL
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        tmp1,
                        arg0_lo_begin + n - 1U,
                        15U
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        tmp2,
                        arg1_lo_begin + n - 1U,
                        15U
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                        1ULL,
                        tmp0,
                        tmp1,
                        tmp2
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                        1U,
                        tmp0,
                        tmp0,
                        4U
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        get_register_info("af",D).first,
                        tmp0,
                        1ULL
                        ),
                microcode::create_ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO(
                        n,
                        get_register_info("zf",D).first,
                        reg0_info.first
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                        1U,
                        get_register_info("sf",D).first,
                        reg0_info.first,
                        7U
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        1U,
                        tmp0,
                        res_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_REG(
                        1U,
                        tmp1,
                        arg0_lo_begin,
                        arg1_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_REG(
                        1U,
                        tmp0,
                        tmp0,
                        tmp1
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        1U,
                        tmp1,
                        arg0_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        1U,
                        tmp2,
                        arg1_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_REG(
                        1U,
                        tmp1,
                        tmp1,
                        tmp2
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_REG(
                        1U,
                        tmp1,
                        tmp1,
                        res_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_OR_REG(
                        1U,
                        tmp0,
                        tmp0,
                        tmp1
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                        1U,
                        get_register_info("of",D).first,
                        tmp0,
                        7U
                        ),
                microcode::create_HAVOC__REG_ASGN_HAVOC(
                        num_used_bytes,
                        res_hi_begin
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8U,
                        0ULL,
                        0ULL,
                        re.num_bytes_of_instruction()
                        )
                });

        return true;
    }

    if (re.num_operands() == 2U
        && re.is_memory_operand(0U)
        && re.is_read_operand(0U)
        && re.is_write_operand(0U)
        && !re.is_segment_register_of_memory_operand_valid(0U)
        && re.is_base_register_of_memory_operand_valid(0U)
        && re.is_immediate_operand(1U)
        && re.is_read_operand(1U)
        && !re.is_write_operand(1U)
        )
    {
        ASSUMPTION(re.get_base_register_of_memory_operand(0U,D).second == 8ULL);

        bool const  use_index_reg = re.is_index_register_of_memory_operand_valid(0U);
        bool const  use_scale = re.get_scale_of_index_register_of_memory_operand(0U) != 1;

        ASSUMPTION(!use_scale || use_index_reg);

        bool const  use_rip = re.get_base_register_of_memory_operand(0U,D) == get_register_info("rip",D);
        bool const  use_displacement = re.get_displacement_of_memory_operand(0U) != 0ULL;

        node_id  u = component().entry();
        uint64_t  temp = re.get_base_register_of_memory_operand(0U,D).first;
        if (use_scale || use_index_reg)
        {
            ASSUMPTION(re.get_index_register_of_memory_operand(0U,D).second == 8ULL);

            if (use_scale)
            {
                u = component().insert_sequence(u,{
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_TIMES_NUMBER(
                                re.get_index_register_of_memory_operand(0U,D).second,
                                D.start_address_of_temporaries(),
                                re.get_index_register_of_memory_operand(0U,D).first,
                                re.get_scale_of_index_register_of_memory_operand(0U)
                                ),
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                                re.get_base_register_of_memory_operand(0U,D).second,
                                D.start_address_of_temporaries(),
                                D.start_address_of_temporaries(),
                                temp
                                )
                        });
            }
            else
            {
                u = component().insert_sequence(u,{
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                                re.get_base_register_of_memory_operand(0U,D).second,
                                D.start_address_of_temporaries(),
                                re.get_index_register_of_memory_operand(0U,D).first,
                                temp
                                )
                        });
            }
            temp = D.start_address_of_temporaries();
        }
        if (use_displacement || use_rip)
        {
            u = component().insert_sequence(u,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            re.get_base_register_of_memory_operand(0U,D).second,
                            D.start_address_of_temporaries(),
                            temp,
                            re.get_displacement_of_memory_operand(0U) + (use_rip ? re.num_bytes_of_instruction() : 0)
                            )
                    });
            temp = D.start_address_of_temporaries();
        }

        uint8_t const  n = re.get_num_bytes_of_operand(0U);
        uint8_t const  N = 2U * n;

        uint64_t const  res_hi_begin = temp < D.start_address_of_temporaries() ? D.start_address_of_temporaries() : temp + 8ULL;
        uint64_t const  res_lo_begin = res_hi_begin + n;
        uint64_t const  res_end = res_lo_begin + n;

        uint64_t const  arg0_hi_begin = res_end;
        uint64_t const  arg0_lo_begin = arg0_hi_begin + n;
        uint64_t const  arg0_end = arg0_lo_begin + n;

        uint64_t const  arg1_hi_begin = arg0_end;
        uint64_t const  arg1_lo_begin = arg1_hi_begin + n;
        uint64_t const  arg1_end = arg1_lo_begin + n;

        uint64_t const  tmp0 = arg1_end;
        uint64_t const  tmp1 = tmp0 + 1ULL;
        uint64_t const  tmp2 = tmp1 + 1ULL;

        uint64_t const  num_used_bytes = tmp2 - D.start_address_of_temporaries() + 1ULL;

        component().insert_sequence(u,{
                microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                        n,
                        arg0_hi_begin,
                        0ULL
                        ),
                microcode::create_DATATRANSFER__REG_ASGN_DEREF_INV_REG(
                        n,
                        arg0_lo_begin,
                        temp
                        ),
                microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                        n,
                        arg1_hi_begin,
                        0ULL
                        ),
                microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                        n,
                        arg1_lo_begin,
                        re.get_immediate_operand(1U)
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                        N,
                        res_hi_begin,
                        arg0_hi_begin,
                        arg1_hi_begin
                        ),
                microcode::create_DATATRANSFER__DEREF_INV_REG_ASGN_REG(
                        n,
                        temp,
                        res_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1ULL,
                        get_register_info("cf",D).first,
                        res_hi_begin + n - 1ULL,
                        1ULL
                        ),
                microcode::create_MISCELLANEOUS__REG_ASGN_PARITY_REG(
                        1U,
                        get_register_info("pf",D).first,
                        res_lo_begin + n - 1ULL
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        tmp1,
                        arg0_lo_begin + n - 1U,
                        15U
                        ),
                microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                        1U,
                        tmp2,
                        (uint64_t)re.get_immediate_operand(1U) & 15ULL
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                        1U,
                        tmp0,
                        tmp1,
                        tmp2
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                        1U,
                        tmp0,
                        tmp0,
                        4U
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        get_register_info("af",D).first,
                        tmp0,
                        1ULL
                        ),
                microcode::create_ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO(
                        n,
                        get_register_info("zf",D).first,
                        res_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                        1U,
                        get_register_info("sf",D).first,
                        res_lo_begin,
                        7U
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        1U,
                        tmp0,
                        res_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        tmp1,
                        arg0_lo_begin,
                        ((uint64_t)re.get_immediate_operand(1U) >> (7ULL * 8ULL)) & 0x80ULL
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_REG(
                        1U,
                        tmp0,
                        tmp0,
                        tmp1
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        1U,
                        tmp1,
                        arg0_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        tmp1,
                        tmp1,
                        (~(uint64_t)re.get_immediate_operand(1U) >> (7ULL * 8ULL)) & 0x80ULL
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_REG(
                        1U,
                        tmp1,
                        tmp1,
                        res_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_OR_REG(
                        1U,
                        tmp0,
                        tmp0,
                        tmp1
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                        1U,
                        get_register_info("of",D).first,
                        tmp0,
                        7U
                        ),
                microcode::create_HAVOC__REG_ASGN_HAVOC(
                        num_used_bytes,
                        D.start_address_of_temporaries()
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8U,
                        0ULL,
                        0ULL,
                        re.num_bytes_of_instruction()
                        )
                });

         return true;
     }

    if (re.num_operands() == 2U
        && re.is_register_operand(0U)
        && re.is_read_operand(0U)
        && re.is_write_operand(0U)
        && re.is_memory_operand(1U)
        && re.is_read_operand(1U)
        && !re.is_write_operand(1U)
        && !re.is_segment_register_of_memory_operand_valid(1U)
        && re.is_base_register_of_memory_operand_valid(1U)
        )
    {
        ASSUMPTION(re.get_base_register_of_memory_operand(1U,D).second == 8ULL);

        bool const  use_index_reg = re.is_index_register_of_memory_operand_valid(1U);
        bool const  use_scale = re.get_scale_of_index_register_of_memory_operand(1U) != 1;

        ASSUMPTION(!use_scale || use_index_reg);

        bool const  use_rip = re.get_base_register_of_memory_operand(1U,D) == get_register_info("rip",D);
        bool const  use_displacement = re.get_displacement_of_memory_operand(1U) != 0ULL;

        node_id  u = component().entry();
        uint64_t  temp = re.get_base_register_of_memory_operand(1U,D).first;
        if (use_scale || use_index_reg)
        {
            ASSUMPTION(re.get_index_register_of_memory_operand(1U,D).second == 8ULL);

            if (use_scale)
            {
                u = component().insert_sequence(u,{
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_TIMES_NUMBER(
                                re.get_index_register_of_memory_operand(1U,D).second,
                                D.start_address_of_temporaries(),
                                re.get_index_register_of_memory_operand(1U,D).first,
                                re.get_scale_of_index_register_of_memory_operand(1U)
                                ),
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                                re.get_base_register_of_memory_operand(1U,D).second,
                                D.start_address_of_temporaries(),
                                D.start_address_of_temporaries(),
                                temp
                                )
                        });
            }
            else
            {
                u = component().insert_sequence(u,{
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                                re.get_base_register_of_memory_operand(1U,D).second,
                                D.start_address_of_temporaries(),
                                re.get_index_register_of_memory_operand(1U,D).first,
                                temp
                                )
                        });
            }
            temp = D.start_address_of_temporaries();
        }
        if (use_displacement || use_rip)
        {
            u = component().insert_sequence(u,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            re.get_base_register_of_memory_operand(1U,D).second,
                            D.start_address_of_temporaries(),
                            temp,
                            re.get_displacement_of_memory_operand(1U) + (use_rip ? re.num_bytes_of_instruction() : 0)
                            )
                    });
            temp = D.start_address_of_temporaries();
        }

        register_info const  reg0_info = re.get_register_operand(0U,D);

        uint8_t const  n = reg0_info.second;
        uint8_t const  N = 2U * n;

        uint64_t const  res_hi_begin = temp < D.start_address_of_temporaries() ? D.start_address_of_temporaries() : temp + 8ULL;
        uint64_t const  res_lo_begin = res_hi_begin + n;
        uint64_t const  res_end = res_lo_begin + n;

        uint64_t const  arg0_hi_begin = res_end;
        uint64_t const  arg0_lo_begin = arg0_hi_begin + n;
        uint64_t const  arg0_end = arg0_lo_begin + n;

        uint64_t const  arg1_hi_begin = arg0_end;
        uint64_t const  arg1_lo_begin = arg1_hi_begin + n;
        uint64_t const  arg1_end = arg1_lo_begin + n;

        uint64_t const  tmp0 = arg1_end;
        uint64_t const  tmp1 = tmp0 + 1ULL;
        uint64_t const  tmp2 = tmp1 + 1ULL;

        uint64_t const  num_used_bytes = tmp2 - D.start_address_of_temporaries() + 1ULL;

        component().insert_sequence(u,{
                microcode::create_TYPECASTING__REG_ASGN_ZERO_EXTEND_REG(
                        n,
                        arg0_hi_begin,
                        reg0_info.first
                        ),
                microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                        n,
                        arg1_hi_begin,
                        0ULL
                        ),
                microcode::create_DATATRANSFER__REG_ASGN_DEREF_INV_REG(
                        n,
                        arg1_lo_begin,
                        temp
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                        N,
                        res_hi_begin,
                        arg0_hi_begin,
                        arg1_hi_begin
                        ),
                microcode::create_SETANDCOPY__REG_ASGN_REG(
                        n,
                        reg0_info.first,
                        res_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1ULL,
                        get_register_info("cf",D).first,
                        res_hi_begin + n - 1ULL,
                        1ULL
                        ),
                microcode::create_MISCELLANEOUS__REG_ASGN_PARITY_REG(
                        1U,
                        get_register_info("pf",D).first,
                        reg0_info.first + n - 1ULL
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        tmp1,
                        arg0_lo_begin + n - 1U,
                        15U
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        tmp2,
                        arg1_lo_begin + n - 1U,
                        15U
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                        1ULL,
                        tmp0,
                        tmp1,
                        tmp2
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                        1U,
                        tmp0,
                        tmp0,
                        4U
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        get_register_info("af",D).first,
                        tmp0,
                        1ULL
                        ),
                microcode::create_ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO(
                        n,
                        get_register_info("zf",D).first,
                        reg0_info.first
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                        1U,
                        get_register_info("sf",D).first,
                        reg0_info.first,
                        7U
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        1U,
                        tmp0,
                        res_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_REG(
                        1U,
                        tmp1,
                        arg0_lo_begin,
                        arg1_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_REG(
                        1U,
                        tmp0,
                        tmp0,
                        tmp1
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        1U,
                        tmp1,
                        arg0_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        1U,
                        tmp2,
                        arg1_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_REG(
                        1U,
                        tmp1,
                        tmp1,
                        tmp2
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_REG(
                        1U,
                        tmp1,
                        tmp1,
                        res_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_OR_REG(
                        1U,
                        tmp0,
                        tmp0,
                        tmp1
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                        1U,
                        get_register_info("of",D).first,
                        tmp0,
                        7U
                        ),
                microcode::create_HAVOC__REG_ASGN_HAVOC(
                        num_used_bytes,
                        res_hi_begin
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8U,
                        0ULL,
                        0ULL,
                        re.num_bytes_of_instruction()
                        )
                });

         return true;
     }

    return false;
}

bool  recognition_data::recognise_SUB(recognition_engine const& re, descriptor::storage const&  D, bool const  compute_flags_only)
{
    if (re.num_operands() == 2U
        && re.is_register_operand(0U)
        && re.is_read_operand(0U)
        && (re.is_write_operand(0U) || compute_flags_only)
        && re.is_immediate_operand(1U)
        && re.is_read_operand(1U)
        )
    {
        register_info const  reg_info = re.get_register_operand(0U,D);

        uint8_t const  n = reg_info.second;
        uint8_t const  N = 2U * n;

        uint64_t const  res_hi_begin = D.start_address_of_temporaries();
        uint64_t const  res_lo_begin = res_hi_begin + n;
        uint64_t const  res_end = res_lo_begin + n;

        uint64_t const  arg0_hi_begin = res_end;
        uint64_t const  arg0_lo_begin = arg0_hi_begin + n;
        uint64_t const  arg0_end = arg0_lo_begin + n;

        uint64_t const  arg1_hi_begin = arg0_end;
        uint64_t const  arg1_lo_begin = arg1_hi_begin + n;
        uint64_t const  arg1_end = arg1_lo_begin + n;

        uint64_t const  tmp0 = arg1_end;
        uint64_t const  tmp1 = tmp0 + 1ULL;
        uint64_t const  tmp2 = tmp1 + 1ULL;

        uint64_t const  num_used_bytes = tmp2 - res_hi_begin + 1ULL;

        node_id  u = component().insert_sequence(component().entry(),{
                microcode::create_TYPECASTING__REG_ASGN_ZERO_EXTEND_REG(
                        n,
                        arg0_hi_begin,
                        reg_info.first
                        ),
                microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                        n,
                        arg1_hi_begin,
                        0xffffffffffffffffULL
                        ),
                microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                        n,
                        arg1_lo_begin,
                        ~(uint64_t)re.get_immediate_operand(1U)
                        ),
                });
        if (N > 8U)
        {
            ASSUMPTION(N == 16U);
            u = component().insert_sequence(u,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            N,
                            arg1_hi_begin,
                            arg1_hi_begin,
                            (uint128_t)1ULL
                            )
                    });
        }
        else
        {
            u = component().insert_sequence(u,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            N,
                            arg1_hi_begin,
                            arg1_hi_begin,
                            1ULL
                            )
                    });
        }
        u = component().insert_sequence(u,{
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                        N,
                        res_hi_begin,
                        arg0_hi_begin,
                        arg1_hi_begin
                        )
                });
        if (!compute_flags_only)
        {
            u = component().insert_sequence(u,{
                    microcode::create_SETANDCOPY__REG_ASGN_REG(
                            n,
                            reg_info.first,
                            res_lo_begin
                            ),
                    });
        }
        u = component().insert_sequence(u,{
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1ULL,
                        get_register_info("cf",D).first,
                        res_hi_begin + n - 1ULL,
                        1ULL
                        ),
                microcode::create_MISCELLANEOUS__REG_ASGN_PARITY_REG(
                        1U,
                        get_register_info("pf",D).first,
                        res_lo_begin + n - 1ULL
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        tmp1,
                        arg0_lo_begin + n - 1U,
                        15U
                        ),
                microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                        1U,
                        tmp2,
                        (uint8_t)(~(uint8_t)((uint64_t)re.get_immediate_operand(1U) & 15ULL) + (uint8_t)1U)
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                        1U,
                        tmp0,
                        tmp1,
                        tmp2
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                        1U,
                        tmp0,
                        tmp0,
                        4U
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        get_register_info("af",D).first,
                        tmp0,
                        1ULL
                        ),
                microcode::create_ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO(
                        n,
                        get_register_info("zf",D).first,
                        res_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                        1U,
                        get_register_info("sf",D).first,
                        res_lo_begin,
                        7U
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        1U,
                        tmp0,
                        arg0_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        tmp0,
                        tmp0,
                        ((uint64_t)re.get_immediate_operand(1U) >> (7ULL * 8ULL)) & 0x80ULL
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_REG(
                        1U,
                        tmp0,
                        tmp0,
                        res_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        tmp1,
                        arg0_lo_begin,
                        (~(uint64_t)re.get_immediate_operand(1U) >> (7ULL * 8ULL)) & 0x80ULL
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        1U,
                        tmp2,
                        res_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_REG(
                        1U,
                        tmp1,
                        tmp1,
                        tmp2
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_OR_REG(
                        1U,
                        tmp0,
                        tmp0,
                        tmp1
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                        1U,
                        get_register_info("of",D).first,
                        tmp0,
                        7U
                        ),
                microcode::create_HAVOC__REG_ASGN_HAVOC(
                        num_used_bytes,
                        res_hi_begin
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8U,
                        0ULL,
                        0ULL,
                        re.num_bytes_of_instruction()
                        )
                });

        return true;
    }

    if (re.num_operands() == 2U
        && re.is_register_operand(0U)
        && re.is_read_operand(0U)
        && (re.is_write_operand(0U) || compute_flags_only)
        && re.is_register_operand(1U)
        && re.is_read_operand(1U)
        && !re.is_write_operand(1U)
        )
    {
        register_info const  reg0_info = re.get_register_operand(0U,D);
        register_info const  reg1_info = re.get_register_operand(1U,D);

        ASSUMPTION(reg0_info.second == reg1_info.second);

        uint8_t const  n = reg0_info.second;
        uint8_t const  N = 2U * n;

        uint64_t const  res_hi_begin = D.start_address_of_temporaries();
        uint64_t const  res_lo_begin = res_hi_begin + n;
        uint64_t const  res_end = res_lo_begin + n;

        uint64_t const  arg0_hi_begin = res_end;
        uint64_t const  arg0_lo_begin = arg0_hi_begin + n;
        uint64_t const  arg0_end = arg0_lo_begin + n;

        uint64_t const  arg1_hi_begin = arg0_end;
        uint64_t const  arg1_lo_begin = arg1_hi_begin + n;
        uint64_t const  arg1_end = arg1_lo_begin + n;

        uint64_t const  tmp0 = arg1_end;
        uint64_t const  tmp1 = tmp0 + 1ULL;
        uint64_t const  tmp2 = tmp1 + 1ULL;

        uint64_t const  num_used_bytes = tmp2 - res_hi_begin + 1ULL;

        node_id  u = component().insert_sequence(component().entry(),{
                microcode::create_TYPECASTING__REG_ASGN_ZERO_EXTEND_REG(
                        n,
                        arg0_hi_begin,
                        reg0_info.first
                        ),
                microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                        n,
                        arg1_hi_begin,
                        0xffffffffffffffffULL
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        n,
                        arg1_lo_begin,
                        reg1_info.first
                        ),
                });
        if (N > 8U)
        {
            ASSUMPTION(N == 16U);
            u = component().insert_sequence(u,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            N,
                            arg1_hi_begin,
                            arg1_hi_begin,
                            (uint128_t)1ULL
                            )
                    });
        }
        else
        {
            u = component().insert_sequence(u,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            N,
                            arg1_hi_begin,
                            arg1_hi_begin,
                            1ULL
                            )
                    });
        }
        u = component().insert_sequence(u,{
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                        N,
                        res_hi_begin,
                        arg0_hi_begin,
                        arg1_hi_begin
                        ),
                });
        if (!compute_flags_only)
        {
            u = component().insert_sequence(u,{
                    microcode::create_SETANDCOPY__REG_ASGN_REG(
                            n,
                            reg0_info.first,
                            res_lo_begin
                            ),
                    });
        }
        u = component().insert_sequence(u,{
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1ULL,
                        get_register_info("cf",D).first,
                        res_hi_begin + n - 1ULL,
                        1ULL
                        ),
                microcode::create_MISCELLANEOUS__REG_ASGN_PARITY_REG(
                        1U,
                        get_register_info("pf",D).first,
                        res_lo_begin + n - 1ULL
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        tmp1,
                        arg0_lo_begin + n - 1U,
                        15U
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        tmp2,
                        arg1_lo_begin + n - 1U,
                        15U
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        1U,
                        tmp2,
                        tmp2
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        1ULL,
                        tmp2,
                        tmp2,
                        1ULL
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                        1ULL,
                        tmp0,
                        tmp1,
                        tmp2
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                        1U,
                        tmp0,
                        tmp0,
                        4U
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        get_register_info("af",D).first,
                        tmp0,
                        1ULL
                        ),
                microcode::create_ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO(
                        n,
                        get_register_info("zf",D).first,
                        res_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                        1U,
                        get_register_info("sf",D).first,
                        res_lo_begin,
                        7U
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        1U,
                        tmp0,
                        arg0_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_REG(
                        1U,
                        tmp0,
                        tmp0,
                        arg1_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_REG(
                        1U,
                        tmp0,
                        tmp0,
                        res_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        1U,
                        tmp2,
                        arg1_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_REG(
                        1U,
                        tmp1,
                        arg0_lo_begin,
                        tmp2
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        1U,
                        tmp2,
                        res_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_REG(
                        1U,
                        tmp1,
                        tmp1,
                        tmp2
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_OR_REG(
                        1U,
                        tmp0,
                        tmp0,
                        tmp1
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                        1U,
                        get_register_info("of",D).first,
                        tmp0,
                        7U
                        ),
                microcode::create_HAVOC__REG_ASGN_HAVOC(
                        num_used_bytes,
                        res_hi_begin
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8U,
                        0ULL,
                        0ULL,
                        re.num_bytes_of_instruction()
                        )
                });

        return true;
    }

    if (re.num_operands() == 2U
        && re.is_memory_operand(0U)
        && (re.is_write_operand(0U) || compute_flags_only)
        && !re.is_segment_register_of_memory_operand_valid(0U)
        && re.is_base_register_of_memory_operand_valid(0U)
        && re.is_immediate_operand(1U)
        )
    {
        ASSUMPTION(re.get_base_register_of_memory_operand(0U,D).second == 8ULL);

        bool const  use_index_reg = re.is_index_register_of_memory_operand_valid(0U);
        bool const  use_scale = re.get_scale_of_index_register_of_memory_operand(0U) != 1;

        ASSUMPTION(!use_scale || use_index_reg);

        bool const  use_rip = re.get_base_register_of_memory_operand(0U,D) == get_register_info("rip",D);
        bool const  use_displacement = re.get_displacement_of_memory_operand(0U) != 0ULL;

        node_id  u = component().entry();
        uint64_t  temp = re.get_base_register_of_memory_operand(0U,D).first;
        if (use_scale || use_index_reg)
        {
            ASSUMPTION(re.get_index_register_of_memory_operand(0U,D).second == 8ULL);

            if (use_scale)
            {
                u = component().insert_sequence(u,{
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_TIMES_NUMBER(
                                re.get_index_register_of_memory_operand(0U,D).second,
                                D.start_address_of_temporaries(),
                                re.get_index_register_of_memory_operand(0U,D).first,
                                re.get_scale_of_index_register_of_memory_operand(0U)
                                ),
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                                re.get_base_register_of_memory_operand(0U,D).second,
                                D.start_address_of_temporaries(),
                                D.start_address_of_temporaries(),
                                temp
                                )
                        });
            }
            else
            {
                u = component().insert_sequence(u,{
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                                re.get_base_register_of_memory_operand(0U,D).second,
                                D.start_address_of_temporaries(),
                                re.get_index_register_of_memory_operand(0U,D).first,
                                temp
                                )
                        });
            }

            temp = D.start_address_of_temporaries();
        }
        if (use_displacement || use_rip)
        {
            u = component().insert_sequence(u,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            re.get_base_register_of_memory_operand(0U,D).second,
                            D.start_address_of_temporaries(),
                            temp,
                            re.get_displacement_of_memory_operand(0U) + (use_rip ? re.num_bytes_of_instruction() : 0)
                            )
                    });
            temp = D.start_address_of_temporaries();
        }

        uint8_t const  n = re.get_num_bytes_of_operand(0U);
        uint8_t const  N = 2U * n;

        uint64_t const  res_hi_begin = temp < D.start_address_of_temporaries() ? D.start_address_of_temporaries() : temp + 8ULL;
        uint64_t const  res_lo_begin = res_hi_begin + n;
        uint64_t const  res_end = res_lo_begin + n;

        uint64_t const  arg0_hi_begin = res_end;
        uint64_t const  arg0_lo_begin = arg0_hi_begin + n;
        uint64_t const  arg0_end = arg0_lo_begin + n;

        uint64_t const  arg1_hi_begin = arg0_end;
        uint64_t const  arg1_lo_begin = arg1_hi_begin + n;
        uint64_t const  arg1_end = arg1_lo_begin + n;

        uint64_t const  tmp0 = arg1_end;
        uint64_t const  tmp1 = tmp0 + 1ULL;
        uint64_t const  tmp2 = tmp1 + 1ULL;

        uint64_t const  num_used_bytes = tmp2 - D.start_address_of_temporaries() + 1ULL;

        u = component().insert_sequence(u,{
                microcode::create_DATATRANSFER__REG_ASGN_DEREF_INV_REG(
                        n,
                        arg0_hi_begin,
                        temp
                        ),
                microcode::create_TYPECASTING__REG_ASGN_ZERO_EXTEND_REG(
                        n,
                        arg0_hi_begin,
                        arg0_hi_begin
                        ),
                microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                        n,
                        arg1_hi_begin,
                        0xffffffffffffffffULL
                        ),
                microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                        n,
                        arg1_lo_begin,
                        ~(uint64_t)re.get_immediate_operand(1U)
                        ),
                });
        if (N > 8U)
        {
            ASSUMPTION(N == 16U);
            u = component().insert_sequence(u,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            N,
                            arg1_hi_begin,
                            arg1_hi_begin,
                            (uint128_t)1ULL
                            )
                    });
        }
        else
        {
            u = component().insert_sequence(u,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            N,
                            arg1_hi_begin,
                            arg1_hi_begin,
                            1ULL
                            )
                    });
        }
        u = component().insert_sequence(u,{
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                        N,
                        res_hi_begin,
                        arg0_hi_begin,
                        arg1_hi_begin
                        )
                });
        if (!compute_flags_only)
        {
            u = component().insert_sequence(u,{
                    microcode::create_DATATRANSFER__DEREF_INV_REG_ASGN_REG(
                            n,
                            temp,
                            res_lo_begin
                            )
                    });
        }
        u = component().insert_sequence(u,{
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1ULL,
                        get_register_info("cf",D).first,
                        res_hi_begin + n - 1ULL,
                        1ULL
                        ),
                microcode::create_MISCELLANEOUS__REG_ASGN_PARITY_REG(
                        1U,
                        get_register_info("pf",D).first,
                        res_lo_begin + n - 1ULL
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        tmp1,
                        arg0_lo_begin + n - 1U,
                        15U
                        ),
                microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                        1U,
                        tmp2,
                        (uint8_t)(~(uint8_t)((uint64_t)re.get_immediate_operand(1U) & 15ULL) + (uint8_t)1U)
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                        1U,
                        tmp0,
                        tmp1,
                        tmp2
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                        1U,
                        tmp0,
                        tmp0,
                        4U
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        get_register_info("af",D).first,
                        tmp0,
                        1ULL
                        ),
                microcode::create_ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO(
                        n,
                        get_register_info("zf",D).first,
                        res_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                        1U,
                        get_register_info("sf",D).first,
                        res_lo_begin,
                        7U
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        1U,
                        tmp0,
                        arg0_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        tmp0,
                        tmp0,
                        ((uint64_t)re.get_immediate_operand(1U) >> (7ULL * 8ULL)) & 0x80ULL
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_REG(
                        1U,
                        tmp0,
                        tmp0,
                        res_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        tmp1,
                        arg0_lo_begin,
                        (~(uint64_t)re.get_immediate_operand(1U) >> (7ULL * 8ULL)) & 0x80ULL
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        1U,
                        tmp2,
                        res_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_REG(
                        1U,
                        tmp1,
                        tmp1,
                        tmp2
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_OR_REG(
                        1U,
                        tmp0,
                        tmp0,
                        tmp1
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                        1U,
                        get_register_info("of",D).first,
                        tmp0,
                        7U
                        ),
                microcode::create_HAVOC__REG_ASGN_HAVOC(
                        num_used_bytes,
                        D.start_address_of_temporaries()
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8U,
                        0ULL,
                        0ULL,
                        re.num_bytes_of_instruction()
                        )
                });

        return true;
    }

    if (re.num_operands() == 2U
        && re.is_register_operand(0U)
        && re.is_read_operand(0U)
        && (re.is_write_operand(0U) || compute_flags_only)
        && re.is_memory_operand(1U)
        && !re.is_write_operand(1U)
        && !re.is_segment_register_of_memory_operand_valid(1U)
        && re.is_base_register_of_memory_operand_valid(1U)
        )
    {
        ASSUMPTION(re.get_base_register_of_memory_operand(1U,D).second == 8ULL);

        bool const  use_index_reg = re.is_index_register_of_memory_operand_valid(1U);
        bool const  use_scale = re.get_scale_of_index_register_of_memory_operand(1U) != 1;

        ASSUMPTION(!use_scale || use_index_reg);

        bool const  use_rip = re.get_base_register_of_memory_operand(1U,D) == get_register_info("rip",D);
        bool const  use_displacement = re.get_displacement_of_memory_operand(1U) != 0ULL;

        node_id  u = component().entry();
        uint64_t  temp = re.get_base_register_of_memory_operand(1U,D).first;
        if (use_scale || use_index_reg)
        {
            ASSUMPTION(re.get_index_register_of_memory_operand(1U,D).second == 8ULL);

            if (use_scale)
            {
                u = component().insert_sequence(u,{
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_TIMES_NUMBER(
                                re.get_index_register_of_memory_operand(1U,D).second,
                                D.start_address_of_temporaries(),
                                re.get_index_register_of_memory_operand(1U,D).first,
                                re.get_scale_of_index_register_of_memory_operand(1U)
                                ),
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                                re.get_base_register_of_memory_operand(1U,D).second,
                                D.start_address_of_temporaries(),
                                D.start_address_of_temporaries(),
                                temp
                                )
                        });
            }
            else
            {
                u = component().insert_sequence(u,{
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                                re.get_base_register_of_memory_operand(1U,D).second,
                                D.start_address_of_temporaries(),
                                re.get_index_register_of_memory_operand(1U,D).first,
                                temp
                                )
                        });
            }
            temp = D.start_address_of_temporaries();
        }
        if (use_displacement || use_rip)
        {
            u = component().insert_sequence(u,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            re.get_base_register_of_memory_operand(1U,D).second,
                            D.start_address_of_temporaries(),
                            temp,
                            re.get_displacement_of_memory_operand(1U) + (use_rip ? re.num_bytes_of_instruction() : 0)
                            )
                    });
            temp = D.start_address_of_temporaries();
        }

        uint8_t const  n = re.get_num_bytes_of_operand(1U);
        uint8_t const  N = 2U * n;

        ASSUMPTION(re.get_register_operand(0U,D).second == n);

        uint64_t const  res_hi_begin = temp < D.start_address_of_temporaries() ? D.start_address_of_temporaries() : temp + 8ULL;
        uint64_t const  res_lo_begin = res_hi_begin + n;
        uint64_t const  res_end = res_lo_begin + n;

        uint64_t const  arg0_hi_begin = res_end;
        uint64_t const  arg0_lo_begin = arg0_hi_begin + n;
        uint64_t const  arg0_end = arg0_lo_begin + n;

        uint64_t const  arg1_hi_begin = arg0_end;
        uint64_t const  arg1_lo_begin = arg1_hi_begin + n;
        uint64_t const  arg1_end = arg1_lo_begin + n;

        uint64_t const  tmp0 = arg1_end;
        uint64_t const  tmp1 = tmp0 + 1ULL;
        uint64_t const  tmp2 = tmp1 + 1ULL;

        uint64_t const  num_used_bytes = tmp2 - D.start_address_of_temporaries() + 1ULL;

        u = component().insert_sequence(u,{
                microcode::create_SETANDCOPY__REG_ASGN_REG(
                        n,
                        arg0_hi_begin,
                        re.get_register_operand(0U,D).first
                        ),
                microcode::create_TYPECASTING__REG_ASGN_ZERO_EXTEND_REG(
                        n,
                        arg0_hi_begin,
                        arg0_hi_begin
                        ),
                microcode::create_DATATRANSFER__REG_ASGN_DEREF_INV_REG(
                        n,
                        arg1_hi_begin,
                        temp
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        n,
                        arg1_hi_begin,
                        arg1_hi_begin
                        ),
                microcode::create_TYPECASTING__REG_ASGN_SIGN_EXTEND_REG(
                        n,
                        arg1_hi_begin,
                        arg1_hi_begin
                        ),
                });
        if (N > 8U)
        {
            ASSUMPTION(N == 16U);
            u = component().insert_sequence(u,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            N,
                            arg1_hi_begin,
                            arg1_hi_begin,
                            (uint128_t)1ULL
                            )
                    });
        }
        else
        {
            u = component().insert_sequence(u,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            N,
                            arg1_hi_begin,
                            arg1_hi_begin,
                            1ULL
                            )
                    });
        }
        u = component().insert_sequence(u,{
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                        N,
                        res_hi_begin,
                        arg0_hi_begin,
                        arg1_hi_begin
                        )
                });
        if (!compute_flags_only)
        {
            u = component().insert_sequence(u,{
                    microcode::create_SETANDCOPY__REG_ASGN_REG(
                            n,
                            re.get_register_operand(0U,D).first,
                            res_lo_begin
                            )
                    });
        }
        u = component().insert_sequence(u,{
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1ULL,
                        get_register_info("cf",D).first,
                        res_hi_begin + n - 1ULL,
                        1ULL
                        ),
                microcode::create_MISCELLANEOUS__REG_ASGN_PARITY_REG(
                        1U,
                        get_register_info("pf",D).first,
                        res_lo_begin + n - 1ULL
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        tmp1,
                        arg0_lo_begin + n - 1U,
                        15U
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        tmp2,
                        arg1_lo_begin + n - 1U,
                        15U
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        1U,
                        tmp2,
                        tmp2
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        1ULL,
                        tmp2,
                        tmp2,
                        1ULL
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                        1ULL,
                        tmp0,
                        tmp1,
                        tmp2
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                        1U,
                        tmp0,
                        tmp0,
                        4U
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        get_register_info("af",D).first,
                        tmp0,
                        1ULL
                        ),
                microcode::create_ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO(
                        n,
                        get_register_info("zf",D).first,
                        res_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                        1U,
                        get_register_info("sf",D).first,
                        res_lo_begin,
                        7U
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        1U,
                        tmp0,
                        arg0_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_REG(
                        1U,
                        tmp0,
                        tmp0,
                        arg1_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_REG(
                        1U,
                        tmp0,
                        tmp0,
                        res_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        1U,
                        tmp2,
                        arg1_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_REG(
                        1U,
                        tmp1,
                        arg0_lo_begin,
                        tmp2
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        1U,
                        tmp2,
                        res_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_REG(
                        1U,
                        tmp1,
                        tmp1,
                        tmp2
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_OR_REG(
                        1U,
                        tmp0,
                        tmp0,
                        tmp1
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                        1U,
                        get_register_info("of",D).first,
                        tmp0,
                        7U
                        ),
                microcode::create_HAVOC__REG_ASGN_HAVOC(
                        num_used_bytes,
                        res_hi_begin
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8U,
                        0ULL,
                        0ULL,
                        re.num_bytes_of_instruction()
                        )
                });

        return true;
    }

    if (re.num_operands() == 2U
        && re.is_memory_operand(0U)
        && (re.is_write_operand(0U) || compute_flags_only)
        && !re.is_segment_register_of_memory_operand_valid(0U)
        && re.is_base_register_of_memory_operand_valid(0U)
        && re.is_register_operand(1U)
        && re.is_read_operand(1U)
        && !re.is_write_operand(1U)
        )
    {
        ASSUMPTION(re.get_base_register_of_memory_operand(0U,D).second == 8ULL);

        bool const  use_index_reg = re.is_index_register_of_memory_operand_valid(0U);
        bool const  use_scale = re.get_scale_of_index_register_of_memory_operand(0U) != 1;

        ASSUMPTION(!use_scale || use_index_reg);

        bool const  use_rip = re.get_base_register_of_memory_operand(0U,D) == get_register_info("rip",D);
        bool const  use_displacement = re.get_displacement_of_memory_operand(0U) != 0ULL;

        node_id  u = component().entry();
        uint64_t  temp = re.get_base_register_of_memory_operand(0U,D).first;
        if (use_scale || use_index_reg)
        {
            ASSUMPTION(re.get_index_register_of_memory_operand(0U,D).second == 8ULL);

            if (use_scale)
            {
                u = component().insert_sequence(u,{
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_TIMES_NUMBER(
                                re.get_index_register_of_memory_operand(0U,D).second,
                                D.start_address_of_temporaries(),
                                re.get_index_register_of_memory_operand(0U,D).first,
                                re.get_scale_of_index_register_of_memory_operand(0U)
                                ),
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                                re.get_base_register_of_memory_operand(0U,D).second,
                                D.start_address_of_temporaries(),
                                D.start_address_of_temporaries(),
                                temp
                                )
                        });
            }
            else
            {
                u = component().insert_sequence(u,{
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                                re.get_base_register_of_memory_operand(0U,D).second,
                                D.start_address_of_temporaries(),
                                re.get_index_register_of_memory_operand(0U,D).first,
                                temp
                                )
                        });
            }

            temp = D.start_address_of_temporaries();
        }
        if (use_displacement || use_rip)
        {
            u = component().insert_sequence(u,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            re.get_base_register_of_memory_operand(0U,D).second,
                            D.start_address_of_temporaries(),
                            temp,
                            re.get_displacement_of_memory_operand(0U) + (use_rip ? re.num_bytes_of_instruction() : 0)
                            )
                    });
            temp = D.start_address_of_temporaries();
        }

        uint8_t const  n = re.get_num_bytes_of_operand(0U);
        uint8_t const  N = 2U * n;

        uint64_t const  res_hi_begin = temp < D.start_address_of_temporaries() ? D.start_address_of_temporaries() : temp + 8ULL;
        uint64_t const  res_lo_begin = res_hi_begin + n;
        uint64_t const  res_end = res_lo_begin + n;

        uint64_t const  arg0_hi_begin = res_end;
        uint64_t const  arg0_lo_begin = arg0_hi_begin + n;
        uint64_t const  arg0_end = arg0_lo_begin + n;

        uint64_t const  arg1_hi_begin = arg0_end;
        uint64_t const  arg1_lo_begin = arg1_hi_begin + n;
        uint64_t const  arg1_end = arg1_lo_begin + n;

        uint64_t const  tmp0 = arg1_end;
        uint64_t const  tmp1 = tmp0 + 1ULL;
        uint64_t const  tmp2 = tmp1 + 1ULL;

        uint64_t const  num_used_bytes = tmp2 - D.start_address_of_temporaries() + 1ULL;

        u = component().insert_sequence(u,{
                microcode::create_DATATRANSFER__REG_ASGN_DEREF_INV_REG(
                        n,
                        arg0_hi_begin,
                        temp
                        ),
                microcode::create_TYPECASTING__REG_ASGN_ZERO_EXTEND_REG(
                        n,
                        arg0_hi_begin,
                        arg0_hi_begin
                        ),
                microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                        n,
                        arg1_hi_begin,
                        0xffffffffffffffffULL
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        n,
                        arg1_lo_begin,
                        re.get_register_operand(1U,D).first
                        ),
                });
        if (N > 8U)
        {
            ASSUMPTION(N == 16U);
            u = component().insert_sequence(u,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            N,
                            arg1_hi_begin,
                            arg1_hi_begin,
                            (uint128_t)1ULL
                            )
                    });
        }
        else
        {
            u = component().insert_sequence(u,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            N,
                            arg1_hi_begin,
                            arg1_hi_begin,
                            1ULL
                            )
                    });
        }
        u = component().insert_sequence(u,{
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                        N,
                        res_hi_begin,
                        arg0_hi_begin,
                        arg1_hi_begin
                        )
                });
        if (!compute_flags_only)
        {
            u = component().insert_sequence(u,{
                    microcode::create_DATATRANSFER__DEREF_INV_REG_ASGN_REG(
                            n,
                            temp,
                            res_lo_begin
                            )
                    });
        }
        u = component().insert_sequence(u,{
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1ULL,
                        get_register_info("cf",D).first,
                        res_hi_begin + n - 1ULL,
                        1ULL
                        ),
                microcode::create_MISCELLANEOUS__REG_ASGN_PARITY_REG(
                        1U,
                        get_register_info("pf",D).first,
                        res_lo_begin + n - 1ULL
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        tmp1,
                        arg0_lo_begin + n - 1U,
                        15U
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        tmp2,
                        arg1_lo_begin + n - 1U,
                        15U
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        1U,
                        tmp2,
                        tmp2
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        1ULL,
                        tmp2,
                        tmp2,
                        1ULL
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                        1ULL,
                        tmp0,
                        tmp1,
                        tmp2
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                        1U,
                        tmp0,
                        tmp0,
                        4U
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1U,
                        get_register_info("af",D).first,
                        tmp0,
                        1ULL
                        ),
                microcode::create_ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO(
                        n,
                        get_register_info("zf",D).first,
                        res_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                        1U,
                        get_register_info("sf",D).first,
                        res_lo_begin,
                        7U
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        1U,
                        tmp0,
                        arg0_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_REG(
                        1U,
                        tmp0,
                        tmp0,
                        arg1_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_REG(
                        1U,
                        tmp0,
                        tmp0,
                        res_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        1U,
                        tmp2,
                        arg1_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_REG(
                        1U,
                        tmp1,
                        arg0_lo_begin,
                        tmp2
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        1U,
                        tmp2,
                        res_lo_begin
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_REG(
                        1U,
                        tmp1,
                        tmp1,
                        tmp2
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_OR_REG(
                        1U,
                        tmp0,
                        tmp0,
                        tmp1
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                        1U,
                        get_register_info("of",D).first,
                        tmp0,
                        7U
                        ),
                microcode::create_HAVOC__REG_ASGN_HAVOC(
                        num_used_bytes,
                        res_hi_begin
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8U,
                        0ULL,
                        0ULL,
                        re.num_bytes_of_instruction()
                        )
                });

        return true;
    }

    return false;
}

bool  recognition_data::recognise_NEG(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 1U
        && re.is_register_operand(0U)
        && re.is_read_operand(0U)
        && re.is_write_operand(0U)
        )
    {
        register_info const  reg = re.get_register_operand(0U,D);
        uint8_t const  n = reg.second;

        uint64_t const  orig_val = D.start_address_of_temporaries();
        uint64_t const  tmp = orig_val + n;

        component().insert_sequence(component().entry(),{
                    microcode::create_SETANDCOPY__REG_ASGN_REG(
                            n,
                            orig_val,
                            reg.first
                            ),
                    microcode::create_ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO(
                            n,
                            get_register_info("cf",D).first,
                            reg.first
                            ),
                    n == 4U ?   microcode::create_SETANDCOPY__REG_ASGN_NUMBER(4U,reg.first - 4ULL,0ULL) :
                                microcode::instruction(),
                    microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                            n,
                            reg.first,
                            reg.first
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            n,
                            reg.first,
                            reg.first,
                            1ULL
                            ),
                    microcode::create_MISCELLANEOUS__REG_ASGN_PARITY_REG(
                            1U,
                            get_register_info("pf",D).first,
                            reg.first + n - 1ULL
                            ),
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                            1U,
                            tmp,
                            orig_val + n - 1U,
                            15ULL
                            ),
                    microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                            1U,
                            tmp,
                            tmp
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            1ULL,
                            tmp,
                            tmp,
                            1ULL
                            ),
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                            1U,
                            tmp,
                            tmp,
                            4U
                            ),
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                            1U,
                            get_register_info("af",D).first,
                            tmp,
                            1ULL
                            ),
                    microcode::create_ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO(
                            n,
                            get_register_info("zf",D).first,
                            reg.first
                            ),
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                            1U,
                            get_register_info("sf",D).first,
                            reg.first,
                            7U
                            ),
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_REG(
                            1U,
                            tmp,
                            orig_val,
                            reg.first
                            ),
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                            1U,
                            get_register_info("of",D).first,
                            tmp,
                            7U
                            ),
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            tmp + 1ULL - D.start_address_of_temporaries(),
                            D.start_address_of_temporaries()
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

        return true;
    }

    return false;
}

bool  recognition_data::recognise_DIV(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 1U
        && re.is_register_operand(0U)
        && re.is_read_operand(0U)
        && !re.is_write_operand(0U)
        )
    {
        register_info const  reg1_lo = re.get_register_operand(0U,D);
        uint8_t const  n = reg1_lo.second;

        register_info  reg0_hi;
        register_info  reg0_lo;
        {
            switch (n)
            {
            case 1U:
                reg0_hi = get_register_info("ah",D);
                reg0_lo = get_register_info("al",D);
                break;
            case 2U:
                reg0_hi = get_register_info("dx",D);
                reg0_lo = get_register_info("ax",D);
                break;
            case 4U:
                reg0_hi = get_register_info("edx",D);
                reg0_lo = get_register_info("eax",D);
                break;
            case 8U:
                reg0_hi = get_register_info("rdx",D);
                reg0_lo = get_register_info("rax",D);
                break;
            default:
                UNREACHABLE();
            }
            ASSUMPTION(n == reg0_hi.second);
            ASSUMPTION(n == reg0_lo.second);
        }

        uint64_t const  arg0_hi_begin = D.start_address_of_temporaries();
        uint64_t const  arg0_lo_begin = arg0_hi_begin + n;
        uint64_t const  arg1_hi_begin = arg0_lo_begin + n;
        uint64_t const  arg1_lo_begin = arg1_hi_begin + n;
        uint64_t const  res_hi_begin = arg1_lo_begin + n;
        uint64_t const  res_lo_begin = res_hi_begin + n;

        node_id  u =
        component().insert_sequence(component().entry(),{
                microcode::create_SETANDCOPY__REG_ASGN_REG(
                        n,
                        arg0_hi_begin,
                        reg0_hi.first
                        ),
                microcode::create_SETANDCOPY__REG_ASGN_REG(
                        n,
                        arg0_lo_begin,
                        reg0_lo.first
                        ),
                microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                        n,
                        arg1_hi_begin,
                        0ULL
                        ),
                microcode::create_SETANDCOPY__REG_ASGN_REG(
                        n,
                        arg1_lo_begin,
                        reg1_lo.first
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_DIVIDE_REG(
                        2U * n,
                        res_hi_begin,
                        arg0_hi_begin,
                        arg1_hi_begin
                        ),
                });

        node_id  v;
        std::tie(u,v) =
                component().insert_branching(microcode::GIK::GUARDS__REG_EQUAL_TO_ZERO,n,res_hi_begin,u);

        component().insert_sequence(v,{
                microcode::create_INTERRUPTS__REG_ASGN_INTERRUPT_GET_AT_FIXED_NUMBER(
                        8U,
                        D.start_address_of_temporaries(),
                        0ULL
                        ),
                microcode::create_SETANDCOPY__REG_ASGN_REG(
                        8U,
                        0ULL,
                        D.start_address_of_temporaries()
                        ),
                microcode::create_HAVOC__REG_ASGN_HAVOC(
                        std::max((uint64_t)8ULL,res_lo_begin + n) - D.start_address_of_temporaries(),
                        D.start_address_of_temporaries()
                        )
                });

        component().insert_sequence(u,{
                microcode::create_SETANDCOPY__REG_ASGN_REG(
                        n,
                        reg0_lo.first,
                        res_lo_begin
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_MODULO_REG(
                        2U * n,
                        res_hi_begin,
                        arg0_hi_begin,
                        arg1_hi_begin
                        ),
                microcode::create_SETANDCOPY__REG_ASGN_REG(
                        n,
                        reg0_hi.first,
                        res_lo_begin
                        ),
                microcode::create_HAVOC__REG_ASGN_HAVOC(
                        res_lo_begin + n - D.start_address_of_temporaries(),
                        D.start_address_of_temporaries()
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8U,
                        0ULL,
                        0ULL,
                        re.num_bytes_of_instruction()
                        )
                });

        return true;
    }

    return false;
}


bool  recognition_data::recognise_AND(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 2U
        && re.is_register_operand(0U)
        && re.is_read_operand(0U)
        && re.is_write_operand(0U)
        && re.is_immediate_operand(1U)
        && re.is_read_operand(1U)
        && re.get_num_bytes_of_operand(0U) == re.get_num_bytes_of_operand(1U)
        )
    {
        register_info const  reg_info = re.get_register_operand(0U,D);

        component().insert_sequence(component().entry(),{
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                            reg_info.second,
                            reg_info.first,
                            reg_info.first,
                            re.get_immediate_operand(1U)
                            ),
                    microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                            1U,
                            get_register_info("cf",D).first,
                            0ULL
                            ),
                    microcode::create_MISCELLANEOUS__REG_ASGN_PARITY_REG(
                            1U,
                            get_register_info("pf",D).first,
                            reg_info.first + reg_info.second - 1U
                            ),
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            1ULL,
                            get_register_info("af",D).first
                            ),
                    microcode::create_ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO(
                            reg_info.second,
                            get_register_info("zf",D).first,
                            reg_info.first
                            ),
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                            1U,
                            get_register_info("sf",D).first,
                            reg_info.first,
                            7U
                            ),
                    microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                            1U,
                            get_register_info("of",D).first,
                            0ULL
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

        return true;
    }

    return false;
}

bool  recognition_data::recognise_OR(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 2U
        && re.is_register_operand(0U)
        && re.is_read_operand(0U)
        && re.is_write_operand(0U)
        && re.is_immediate_operand(1U)
        && re.is_read_operand(1U)
        && re.get_num_bytes_of_operand(0U) == re.get_num_bytes_of_operand(1U)
        )
    {
        register_info const  reg_info = re.get_register_operand(0U,D);

        component().insert_sequence(component().entry(),{
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_OR_NUMBER(
                            reg_info.second,
                            reg_info.first,
                            reg_info.first,
                            re.get_immediate_operand(1U)
                            ),
                    microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                            1U,
                            get_register_info("cf",D).first,
                            0ULL
                            ),
                    microcode::create_MISCELLANEOUS__REG_ASGN_PARITY_REG(
                            1U,
                            get_register_info("pf",D).first,
                            reg_info.first + reg_info.second - 1U
                            ),
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            1ULL,
                            get_register_info("af",D).first
                            ),
                    microcode::create_ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO(
                            reg_info.second,
                            get_register_info("zf",D).first,
                            reg_info.first
                            ),
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                            1U,
                            get_register_info("sf",D).first,
                            reg_info.first,
                            7U
                            ),
                    microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                            1U,
                            get_register_info("of",D).first,
                            0ULL
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

        return true;
    }

    if (re.num_operands() == 2U
        && re.is_register_operand(0U)
        && re.is_read_operand(0U)
        && re.is_write_operand(0U)
        && re.is_register_operand(1U)
        && re.is_read_operand(1U)
        && !re.is_write_operand(1U)
        && re.get_num_bytes_of_operand(0U) == re.get_num_bytes_of_operand(1U)
        )
    {
        register_info const  reg0_info = re.get_register_operand(0U,D);

        component().insert_sequence(component().entry(),{
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_OR_REG(
                            reg0_info.second,
                            reg0_info.first,
                            reg0_info.first,
                            re.get_register_operand(1U,D).first
                            ),
                    microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                            1U,
                            get_register_info("cf",D).first,
                            0ULL
                            ),
                    microcode::create_MISCELLANEOUS__REG_ASGN_PARITY_REG(
                            1U,
                            get_register_info("pf",D).first,
                            reg0_info.first + reg0_info.second - 1U
                            ),
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            1ULL,
                            get_register_info("af",D).first
                            ),
                    microcode::create_ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO(
                            reg0_info.second,
                            get_register_info("zf",D).first,
                            reg0_info.first
                            ),
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                            1U,
                            get_register_info("sf",D).first,
                            reg0_info.first,
                            7U
                            ),
                    microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                            1U,
                            get_register_info("of",D).first,
                            0ULL
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

        return true;
    }

    if (re.num_operands() == 2U
        && re.is_memory_operand(0U)
        && re.is_read_operand(0U)
        && re.is_write_operand(0U)
        && !re.is_segment_register_of_memory_operand_valid(0U)
        && re.is_base_register_of_memory_operand_valid(0U)
        && re.is_immediate_operand(1U)
        && re.is_read_operand(1U)
        && re.get_num_bytes_of_operand(0U) == re.get_num_bytes_of_operand(1U)
        )
    {
        ASSUMPTION(re.get_base_register_of_memory_operand(0U,D).second == 8ULL);

        bool const  use_index_reg = re.is_index_register_of_memory_operand_valid(0U);
        bool const  use_scale = re.get_scale_of_index_register_of_memory_operand(0U) != 1;

        ASSUMPTION(!use_scale || use_index_reg);

        bool const  use_rip = re.get_base_register_of_memory_operand(0U,D) == get_register_info("rip",D);
        bool const  use_displacement = re.get_displacement_of_memory_operand(0U) != 0ULL;

        node_id  n = component().entry();
        uint64_t  temp = re.get_base_register_of_memory_operand(0U,D).first;

        if (use_scale || use_index_reg)
        {
            ASSUMPTION(re.get_index_register_of_memory_operand(0U,D).second == 8ULL);

            if (use_scale)
            {
                n = component().insert_sequence(n,{
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_TIMES_NUMBER(
                                re.get_index_register_of_memory_operand(0U,D).second,
                                D.start_address_of_temporaries(),
                                re.get_index_register_of_memory_operand(0U,D).first,
                                re.get_scale_of_index_register_of_memory_operand(0U)
                                ),
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                                re.get_base_register_of_memory_operand(0U,D).second,
                                D.start_address_of_temporaries(),
                                D.start_address_of_temporaries(),
                                temp
                                )
                        });
            }
            else
            {
                n = component().insert_sequence(n,{
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                                re.get_base_register_of_memory_operand(0U,D).second,
                                D.start_address_of_temporaries(),
                                re.get_index_register_of_memory_operand(0U,D).first,
                                temp
                                )
                        });
            }
            temp = D.start_address_of_temporaries();
        }
        if (use_displacement || use_rip)
        {
            n = component().insert_sequence(n,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            re.get_base_register_of_memory_operand(0U,D).second,
                            D.start_address_of_temporaries(),
                            temp,
                            re.get_displacement_of_memory_operand(0U) + (use_rip ? re.num_bytes_of_instruction() : 0)
                            )
                    });
            temp = D.start_address_of_temporaries();
        }

        uint8_t const  num_bytes = re.get_num_bytes_of_operand(0U);
        uint64_t const  arg0_begin = temp < D.start_address_of_temporaries() ? D.start_address_of_temporaries() : temp + 8ULL;
        uint64_t const  res_begin = arg0_begin + num_bytes;

        component().insert_sequence(component().entry(),{
                    microcode::create_DATATRANSFER__REG_ASGN_DEREF_INV_REG(
                            num_bytes,
                            arg0_begin,
                            temp
                            ),
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_OR_NUMBER(
                            num_bytes,
                            res_begin,
                            arg0_begin,
                            re.get_immediate_operand(1U)
                            ),
                    microcode::create_DATATRANSFER__DEREF_INV_REG_ASGN_REG(
                            num_bytes,
                            temp,
                            res_begin
                            ),
                    microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                            1U,
                            get_register_info("cf",D).first,
                            0ULL
                            ),
                    microcode::create_MISCELLANEOUS__REG_ASGN_PARITY_REG(
                            1U,
                            get_register_info("pf",D).first,
                            res_begin + num_bytes - 1U
                            ),
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            1ULL,
                            get_register_info("af",D).first
                            ),
                    microcode::create_ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO(
                            num_bytes,
                            get_register_info("zf",D).first,
                            res_begin
                            ),
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                            1U,
                            get_register_info("sf",D).first,
                            res_begin,
                            7U
                            ),
                    microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                            1U,
                            get_register_info("of",D).first,
                            0ULL
                            ),
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            res_begin + num_bytes - D.start_address_of_temporaries(),
                            D.start_address_of_temporaries()
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

        return true;
    }

    return false;
}

bool  recognition_data::recognise_XOR(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 2U
        && re.is_register_operand(0U)
        && re.is_read_operand(0U)
        && re.is_write_operand(0U)
        && re.is_register_operand(1U)
        && re.is_read_operand(1U)
        && re.get_num_bytes_of_operand(0U) == re.get_num_bytes_of_operand(1U)
        )
    {
        register_info const  reg0_info = re.get_register_operand(0U,D);
        register_info const  reg1_info = re.get_register_operand(1U,D);

        component().insert_sequence(component().entry(),{
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_XOR_REG(
                            reg0_info.second,
                            reg0_info.first,
                            reg0_info.first,
                            reg1_info.first
                            ),
                    microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                            1U,
                            get_register_info("cf",D).first,
                            0ULL
                            ),
                    microcode::create_MISCELLANEOUS__REG_ASGN_PARITY_REG(
                            1U,
                            get_register_info("pf",D).first,
                            reg0_info.first + reg0_info.second - 1U
                            ),
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            1ULL,
                            get_register_info("af",D).first
                            ),
                    microcode::create_ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO(
                            reg0_info.second,
                            get_register_info("zf",D).first,
                            reg0_info.first
                            ),
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                            1U,
                            get_register_info("sf",D).first,
                            reg0_info.first,
                            7U
                            ),
                    microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                            1U,
                            get_register_info("of",D).first,
                            0ULL
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

        return true;
    }

    if (re.num_operands() == 2U
        && re.is_register_operand(0U)
        && re.is_read_operand(0U)
        && re.is_write_operand(0U)
        && re.is_memory_operand(1U)
        && re.is_read_operand(1U)
        && !re.is_write_operand(1U)
        && re.is_segment_register_of_memory_operand_valid(1U)
        && re.get_segment_register_of_memory_operand(1U,D).second == 8U
        && !re.is_base_register_of_memory_operand_valid(1U)
        && !re.is_index_register_of_memory_operand_valid(1U)
        && re.get_scale_of_index_register_of_memory_operand(1U) == 1
        && re.get_num_bytes_of_operand(1U) == 8U
        )
    {
        register_info const  reg_info = re.get_register_operand(0U,D);

        component().insert_sequence(component().entry(),{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8ULL,
                            D.start_address_of_temporaries(),
                            re.get_segment_register_of_memory_operand(1U,D).first,
                            re.get_displacement_of_memory_operand(1U)
                            ),
                    microcode::create_DATATRANSFER__REG_ASGN_DEREF_INV_REG(
                            reg_info.second,
                            D.start_address_of_temporaries(),
                            D.start_address_of_temporaries()
                            ),
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_XOR_REG(
                            reg_info.second,
                            reg_info.first,
                            reg_info.first,
                            D.start_address_of_temporaries()
                            ),
                    microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                            1U,
                            get_register_info("cf",D).first,
                            0ULL
                            ),
                    microcode::create_MISCELLANEOUS__REG_ASGN_PARITY_REG(
                            1U,
                            get_register_info("pf",D).first,
                            reg_info.first + reg_info.second - 1U
                            ),
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            1ULL,
                            get_register_info("af",D).first
                            ),
                    microcode::create_ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO(
                            reg_info.second,
                            get_register_info("zf",D).first,
                            reg_info.first
                            ),
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                            1U,
                            get_register_info("sf",D).first,
                            reg_info.first,
                            7U
                            ),
                    microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                            1U,
                            get_register_info("of",D).first,
                            0ULL
                            ),
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            8ULL,
                            D.start_address_of_temporaries()
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

        return true;
    }

    return false;
}

bool  recognition_data::recognise_PXOR(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 2U
        && re.is_register_operand(0U)
        && re.is_read_operand(0U)
        && re.is_write_operand(0U)
        && re.is_register_operand(1U)
        && re.is_read_operand(1U)
        && re.get_num_bytes_of_operand(0U) == re.get_num_bytes_of_operand(1U)
        && re.get_num_bytes_of_operand(0U) == uint128_t::size()
        )
    {
        ASSUMPTION(uint128_t::size() == 0x10ULL);

        register_info const  reg0_info = re.get_register_operand(0U,D);
        register_info const  reg1_info = re.get_register_operand(1U,D);

        component().insert_sequence(component().entry(),{
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_XOR_REG(
                            8U,
                            reg0_info.first,
                            reg0_info.first,
                            reg1_info.first
                            ),
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_XOR_REG(
                            8U,
                            reg0_info.first + 8ULL,
                            reg0_info.first + 8ULL,
                            reg1_info.first + 8ULL
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

        return true;
    }

    return false;
}

bool  recognition_data::recognise_SAR(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 2U
        && re.is_register_operand(0U)
        && re.is_read_operand(0U)
        && re.is_write_operand(0U)
        && re.is_immediate_operand(1U)
        )
    {
        uint64_t const  raw_count = re.get_immediate_operand(1U);
        if (raw_count == 0ULL)
        {
            // In this case, the instruction only moves the IP register.
            component().insert_sequence(component().entry(),{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

            return true;
        }
        uint64_t const  count = raw_count & (re.uses_rex() ? 0x3fULL : 0x1fULL);

        register_info const  reg_info = re.get_register_operand(0U,D);

        node_id  n = component().insert_sequence(component().entry(),{
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                            reg_info.second,
                            D.start_address_of_temporaries(),
                            reg_info.first,
                            1ULL << (count - 1ULL)
                            ),
                    microcode::create_ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO(
                            reg_info.second,
                            get_register_info("cf",D).first,
                            D.start_address_of_temporaries()
                            ),
                     microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                             reg_info.second,
                             D.start_address_of_temporaries(),
                             reg_info.first,
                             1ULL << (8ULL * reg_info.second - 1ULL)
                             ),
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                            reg_info.second,
                            D.start_address_of_temporaries(),
                            D.start_address_of_temporaries(),
                            (uint8_t)count
                            ),
                     microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                             reg_info.second,
                             D.start_address_of_temporaries(),
                             D.start_address_of_temporaries(),
                             0xffffffffffffffffULL
                             ),
                     microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                             reg_info.second,
                             D.start_address_of_temporaries(),
                             D.start_address_of_temporaries()
                             ),
                     microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                             reg_info.second,
                             reg_info.first,
                             reg_info.first,
                             (uint8_t)count
                             ),
                     microcode::create_BITOPERATIONS__REG_ASGN_REG_OR_REG(
                             reg_info.second,
                             reg_info.first,
                             reg_info.first,
                             D.start_address_of_temporaries()
                             ),
                     microcode::create_HAVOC__REG_ASGN_HAVOC(
                             reg_info.second,
                             D.start_address_of_temporaries()
                             ),
                     microcode::create_MISCELLANEOUS__REG_ASGN_PARITY_REG(
                             1U,
                             get_register_info("pf",D).first,
                             reg_info.first + reg_info.second - 1ULL
                             ),
                     microcode::create_HAVOC__REG_ASGN_HAVOC(
                             1U,
                             get_register_info("af",D).first
                             ),
                     microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                             1U,
                             get_register_info("sf",D).first,
                             reg_info.first,
                             7U
                             ),
                     microcode::create_ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO(
                             reg_info.second,
                             get_register_info("zf",D).first,
                             reg_info.first
                             )
                    });
        if ((count & raw_count) == 1ULL)
        {
            n = component().insert_sequence(n,{
                    microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                            1U,
                            get_register_info("of",D).first,
                            0ULL
                            )
                    });
        }
        else
        {
            n = component().insert_sequence(n,{
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            1U,
                            get_register_info("of",D).first
                            )
                    });
        }

        component().insert_sequence(n,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

        return true;
    }

    if (re.num_operands() == 2U
        && re.is_register_operand(0U)
        && re.is_read_operand(0U)
        && re.is_write_operand(0U)
        && re.is_register_operand(1U)
        && re.is_read_operand(1U)
        && !re.is_write_operand(1U)
        && re.get_num_bytes_of_operand(1U) == 1U
        )
    {
        //uint64_t const  count = raw_count & (re.uses_rex() ? 0x3fULL : 0x1fULL);

        register_info const  reg0_info = re.get_register_operand(0U,D);
        register_info const  reg1_info = re.get_register_operand(1U,D);

        node_id  u, v;
        std::tie(u,v) =
                component().insert_branching(microcode::GIK::GUARDS__REG_NOT_EQUAL_TO_ZERO,reg1_info.second,reg1_info.first,component().entry());

        component().insert_sequence(v,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

        uint64_t const  count_mask = re.uses_rex() ? 0x3fULL : 0x1fULL;
        uint64_t const  count_begin = D.start_address_of_temporaries();

        uint64_t const  temp0 = count_begin + reg1_info.second;

        u = component().insert_sequence(u,{
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                            reg1_info.second,
                            count_begin,
                            reg1_info.first,
                            count_mask
                            ),

                    microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                            reg0_info.second,
                            temp0,
                            1ULL
                            ),
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_LSHIFT_REG(
                            reg0_info.second,
                            temp0,
                            temp0,
                            count_begin
                            ),
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                            reg0_info.second,
                            temp0,
                            temp0,
                            1ULL
                            ),
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_REG(
                            reg0_info.second,
                            temp0,
                            reg0_info.first,
                            temp0
                            ),
                    microcode::create_ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO(
                            reg0_info.second,
                            get_register_info("cf",D).first,
                            temp0
                            ),

                     microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                             reg0_info.second,
                             temp0,
                             reg0_info.first,
                             1ULL << (8ULL * reg0_info.second - 1ULL)
                             ),
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_REG(
                            reg0_info.second,
                            temp0,
                            temp0,
                            count_begin
                            ),
                     microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                             reg0_info.second,
                             temp0,
                             temp0,
                             0xffffffffffffffffULL
                             ),
                     microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                             reg0_info.second,
                             temp0,
                             temp0
                             ),
                     microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_REG(
                             reg0_info.second,
                             reg0_info.first,
                             reg0_info.first,
                             count_begin
                             ),
                     microcode::create_BITOPERATIONS__REG_ASGN_REG_OR_REG(
                             reg0_info.second,
                             reg0_info.first,
                             reg0_info.first,
                             temp0
                             ),

                     microcode::create_MISCELLANEOUS__REG_ASGN_PARITY_REG(
                             1U,
                             get_register_info("pf",D).first,
                             reg0_info.first + reg0_info.second - 1ULL
                             ),
                     microcode::create_HAVOC__REG_ASGN_HAVOC(
                             1U,
                             get_register_info("af",D).first
                             ),
                     microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                             1U,
                             get_register_info("sf",D).first,
                             reg0_info.first,
                             7U
                             ),
                     microcode::create_ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO(
                             reg0_info.second,
                             get_register_info("zf",D).first,
                             reg0_info.first
                             ),

                     microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                             reg1_info.second,
                             count_begin,
                             count_begin,
                             0xffffffffffffffffULL
                             ),
                });

        std::tie(u,v) =
                component().insert_branching(microcode::GIK::GUARDS__REG_EQUAL_TO_ZERO,reg1_info.second,count_begin,u);

        u = component().insert_sequence(u,{
                microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                        1U,
                        get_register_info("of",D).first,
                        0ULL
                        )
                });

        component().insert_sequence(v,{
                microcode::create_HAVOC__REG_ASGN_HAVOC(
                        1U,
                        get_register_info("of",D).first
                        )
                },u);

        component().insert_sequence(u,{
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            reg1_info.second + reg0_info.second,
                            D.start_address_of_temporaries()
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

        return true;
    }

    return false;
}

bool  recognition_data::recognise_SHR(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 2U
        && re.is_register_operand(0U)
        && re.is_read_operand(0U)
        && re.is_write_operand(0U)
        && re.is_immediate_operand(1U)
        )
    {
        uint64_t const  raw_count = re.get_immediate_operand(1U);
        if (raw_count == 0ULL)
        {
            // In this case, the instruction only moves the IP register.
            component().insert_sequence(component().entry(),{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

            return true;
        }
        uint64_t const  count = raw_count & (re.uses_rex() ? 0x3fULL : 0x1fULL);

        register_info const  reg_info = re.get_register_operand(0U,D);

        component().insert_sequence(component().entry(),{
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                            reg_info.second,
                            D.start_address_of_temporaries(),
                            reg_info.first,
                            1ULL << (count - 1ULL)
                            ),
                    microcode::create_ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO(
                            reg_info.second,
                            get_register_info("cf",D).first,
                            D.start_address_of_temporaries()
                            ),
                    (count & raw_count) == 1ULL ?
                                microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                                        1U,
                                        get_register_info("of",D).first,
                                        reg_info.first,
                                        7U
                                        ) :
                                 microcode::create_HAVOC__REG_ASGN_HAVOC(
                                        1U,
                                        get_register_info("of",D).first
                                        ),
                     microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                            reg_info.second,
                            reg_info.first,
                            reg_info.first,
                            (uint8_t)count
                            ),
                     microcode::create_MISCELLANEOUS__REG_ASGN_PARITY_REG(
                            1U,
                            get_register_info("pf",D).first,
                            reg_info.first + reg_info.second - 1ULL
                            ),
                     microcode::create_HAVOC__REG_ASGN_HAVOC(
                            1U,
                            get_register_info("af",D).first
                            ),
                     microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                            1U,
                            get_register_info("sf",D).first,
                            reg_info.first,
                            7U
                            ),
                     microcode::create_ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO(
                            reg_info.second,
                            get_register_info("zf",D).first,
                            reg_info.first
                            ),
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            reg_info.second,
                            D.start_address_of_temporaries()
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

        return true;
    }

    return false;
}

bool  recognition_data::recognise_SHL(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 2U
        && re.is_register_operand(0U)
        && re.is_read_operand(0U)
        && re.is_write_operand(0U)
        && re.is_immediate_operand(1U)
        )
    {
        uint64_t const  raw_count = re.get_immediate_operand(1U);
        if (raw_count == 0ULL)
        {
            // In this case, the instruction only moves the IP register.
            component().insert_sequence(component().entry(),{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

            return true;
        }

        uint64_t const  count = raw_count & (re.uses_rex() ? 0x3fULL : 0x1fULL);
        register_info const  reg_info = re.get_register_operand(0U,D);

        node_id  n = component().entry();

        if (count < 8ULL * reg_info.second)
        {
            n = component().insert_sequence(n,{
                        microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                                reg_info.second,
                                D.start_address_of_temporaries(),
                                reg_info.first,
                                1ULL << (8ULL * reg_info.second - count)
                                ),
                        microcode::create_ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO(
                                reg_info.second,
                                get_register_info("cf",D).first,
                                D.start_address_of_temporaries()
                                )
                        });
        }
        else
        {
            n = component().insert_sequence(n,{
                        microcode::create_HAVOC__REG_ASGN_HAVOC(
                                1U,
                                get_register_info("cf",D).first
                                )
                        });
        }

        n = component().insert_sequence(n,{
                     microcode::create_BITOPERATIONS__REG_ASGN_REG_LSHIFT_NUMBER(
                             reg_info.second,
                             reg_info.first,
                             reg_info.first,
                             (uint8_t)count
                             ),
                     microcode::create_MISCELLANEOUS__REG_ASGN_PARITY_REG(
                             1U,
                             get_register_info("pf",D).first,
                             reg_info.first + reg_info.second - 1ULL
                             ),
                     microcode::create_HAVOC__REG_ASGN_HAVOC(
                             1U,
                             get_register_info("af",D).first
                             ),
                     microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                             1U,
                             get_register_info("sf",D).first,
                             reg_info.first,
                             7U
                             ),
                     microcode::create_ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO(
                             reg_info.second,
                             get_register_info("zf",D).first,
                             reg_info.first
                             )
                    });
        if ((count & raw_count) == 1ULL)
        {
            n = component().insert_sequence(n,{
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                            1U,
                            D.start_address_of_temporaries(),
                            reg_info.first,
                            7U
                            ),
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_XOR_REG(
                            1U,
                            get_register_info("of",D).first,
                            D.start_address_of_temporaries(),
                            get_register_info("cf",D).first
                            )
                    });
        }
        else
        {
            n = component().insert_sequence(n,{
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            1U,
                            get_register_info("of",D).first
                            )
                    });
        }

        component().insert_sequence(n,{
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            reg_info.second,
                            D.start_address_of_temporaries()
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

        return true;
    }

    return false;
}

bool  recognition_data::recognise_ROL(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 2U
        && re.is_register_operand(0U)
        && re.is_read_operand(0U)
        && re.is_write_operand(0U)
        && re.is_immediate_operand(1U)
        )
    {
        uint64_t const  raw_count = re.get_immediate_operand(1U);
        if (raw_count == 0ULL)
        {
            // In this case, the instruction only moves the IP register.
            component().insert_sequence(component().entry(),{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

            return true;
        }
        uint64_t const  count = raw_count & (re.uses_rex() ? 0x3fULL : 0x1fULL);

        register_info const  reg_info = re.get_register_operand(0U,D);

        ASSUMPTION(8ULL * reg_info.second >= count);

        node_id  n = component().insert_sequence(component().entry(),{
                     microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                             reg_info.second,
                             D.start_address_of_temporaries(),
                             reg_info.first,
                             (uint8_t)(8ULL * reg_info.second - count)
                             ),
                     microcode::create_BITOPERATIONS__REG_ASGN_REG_LSHIFT_NUMBER(
                             reg_info.second,
                             reg_info.first,
                             reg_info.first,
                             (uint8_t)count
                             ),
                     microcode::create_BITOPERATIONS__REG_ASGN_REG_OR_REG(
                             reg_info.second,
                             reg_info.first,
                             reg_info.first,
                             D.start_address_of_temporaries()
                             ),
                     microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                             1U,
                             get_register_info("cf",D).first,
                             reg_info.first + reg_info.second - 1U,
                             1ULL
                             ),
                    });
        if ((count & raw_count) == 1ULL)
        {
            n = component().insert_sequence(n,{
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(
                            1U,
                            D.start_address_of_temporaries(),
                            reg_info.first,
                            7U
                            ),
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_XOR_REG(
                            1U,
                            get_register_info("of",D).first,
                            get_register_info("cf",D).first,
                            D.start_address_of_temporaries()
                            )
                    });
        }
        else
        {
            n = component().insert_sequence(n,{
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            1U,
                            get_register_info("of",D).first
                            )
                    });
        }

        component().insert_sequence(n,{
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            reg_info.second,
                            D.start_address_of_temporaries()
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

        return true;
    }

    return false;
}

bool  recognition_data::recognise_BSF(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 2U
        && re.is_register_operand(0U)
        && !re.is_read_operand(0U)
        && re.is_write_operand(0U)
        && re.is_register_operand(1U)
        && re.is_read_operand(1U)
        && !re.is_write_operand(1U)
        )
    {
        ASSUMPTION(re.get_num_bytes_of_operand(0U) == re.get_num_bytes_of_operand(1U));

        register_info const  reg0_info = re.get_register_operand(0U,D);
        register_info const  reg1_info = re.get_register_operand(1U,D);

        node_id  u, src_reg_is_zero;
        std::tie(u,src_reg_is_zero) =
                component().insert_branching(microcode::GIK::GUARDS__REG_NOT_EQUAL_TO_ZERO,reg1_info.second,reg1_info.first,component().entry());

        node_id const  loop_head =
            component().insert_sequence(u,{
                microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                        reg0_info.second,
                        reg0_info.first,
                        0ULL
                        ),
                microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                        reg0_info.second,
                        D.start_address_of_temporaries(),
                        1ULL
                        ),
                });

        u = component().insert_sequence(loop_head,{
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_REG(
                        reg0_info.second,
                        D.start_address_of_temporaries() + reg0_info.second,
                        D.start_address_of_temporaries(),
                        reg1_info.first
                        ),
                });

        node_id  loop_exit;
        std::tie(u,loop_exit) =
                component().insert_branching(microcode::GIK::GUARDS__REG_EQUAL_TO_ZERO,reg0_info.second,D.start_address_of_temporaries() + reg0_info.second,u);

        component().insert_sequence(u,{
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        reg0_info.second,
                        reg0_info.first,
                        reg0_info.first,
                        1ULL
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_LSHIFT_NUMBER(
                        reg0_info.second,
                        D.start_address_of_temporaries(),
                        D.start_address_of_temporaries(),
                        1ULL
                        ),
                },loop_head);

        u = component().insert_sequence(loop_exit,{
                microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                        1U,
                        get_register_info("zf",D).first,
                        0ULL
                        ),
                });

        component().insert_sequence(src_reg_is_zero,{
                microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                        1U,
                        get_register_info("zf",D).first,
                        1ULL
                        ),
                microcode::create_HAVOC__REG_ASGN_HAVOC(
                        reg0_info.second,
                        reg0_info.first
                        )
                },u);

        component().insert_sequence(u,{
                microcode::create_HAVOC__REG_ASGN_HAVOC(
                        1U,
                        get_register_info("of",D).first
                        ),
                microcode::create_HAVOC__REG_ASGN_HAVOC(
                        1U,
                        get_register_info("sf",D).first
                        ),
                microcode::create_HAVOC__REG_ASGN_HAVOC(
                        1U,
                        get_register_info("pf",D).first
                        ),
                microcode::create_HAVOC__REG_ASGN_HAVOC(
                        1U,
                        get_register_info("af",D).first
                        ),
                microcode::create_HAVOC__REG_ASGN_HAVOC(
                        1U,
                        get_register_info("cf",D).first
                        ),
                microcode::create_HAVOC__REG_ASGN_HAVOC(
                        2ULL * reg0_info.second,
                        D.start_address_of_temporaries()
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8U,
                        0ULL,
                        0ULL,
                        re.num_bytes_of_instruction()
                        )
                });
        return true;
    }

    return false;
}


bool  recognition_data::recognise_JMP(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 1U
        && re.is_immediate_operand(0U)
        && re.is_relative_jump()
        )
    {
        component().insert_sequence(component().entry(),{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.get_immediate_operand(0U)
                            )
                    });

        return true;
    }

    if (re.num_operands() == 1U
        && re.is_register_operand(0U)
        && re.is_read_operand(0U)
        && !re.is_write_operand(0U)
        && !re.is_relative_jump()
        && re.get_num_bytes_of_operand(0U) == 8ULL
        )
    {
        component().insert_sequence(component().entry(),{
                    microcode::create_SETANDCOPY__REG_ASGN_REG(
                            8U,
                            0ULL,
                            re.get_register_operand(0U,D).first
                            )
                    });

        return true;
    }

    if (re.num_operands() == 1U
        && re.is_memory_operand(0U)
        && !re.is_read_operand(0U)
        && !re.is_write_operand(0U)
        && !re.is_segment_register_of_memory_operand_valid(0U)
        && re.is_base_register_of_memory_operand_valid(0U)
        && !re.is_relative_jump()
        )
    {
        ASSUMPTION(re.get_base_register_of_memory_operand(0U,D).second == 8ULL);

        bool const  use_index_reg = re.is_index_register_of_memory_operand_valid(0U);
        bool const  use_scale = re.get_scale_of_index_register_of_memory_operand(0U) != 1;

        ASSUMPTION(!use_scale || use_index_reg);

        bool const  use_rip = re.get_base_register_of_memory_operand(0U,D) == get_register_info("rip",D);
        bool const  use_displacement = re.get_displacement_of_memory_operand(0U) != 0ULL;

        node_id  n = component().entry();
        uint64_t  temp = re.get_base_register_of_memory_operand(0U,D).first;
        if (use_scale || use_index_reg)
        {
            ASSUMPTION(re.get_index_register_of_memory_operand(0U,D).second == 8ULL);

            if (use_scale)
            {
                n = component().insert_sequence(n,{
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_TIMES_NUMBER(
                                re.get_index_register_of_memory_operand(0U,D).second,
                                D.start_address_of_temporaries(),
                                re.get_index_register_of_memory_operand(0U,D).first,
                                re.get_scale_of_index_register_of_memory_operand(0U)
                                ),
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                                re.get_base_register_of_memory_operand(0U,D).second,
                                D.start_address_of_temporaries(),
                                D.start_address_of_temporaries(),
                                temp
                                )
                        });
            }
            else
            {
                n = component().insert_sequence(n,{
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                                re.get_base_register_of_memory_operand(0U,D).second,
                                D.start_address_of_temporaries(),
                                re.get_index_register_of_memory_operand(0U,D).first,
                                temp
                                )
                        });
            }
            temp = D.start_address_of_temporaries();
        }
        if (use_displacement || use_rip)
        {
            n = component().insert_sequence(n,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            re.get_base_register_of_memory_operand(0U,D).second,
                            D.start_address_of_temporaries(),
                            temp,
                            re.get_displacement_of_memory_operand(0U) + (use_rip ? re.num_bytes_of_instruction() : 0)
                            )
                    });
            temp = D.start_address_of_temporaries();
        }

        n = component().insert_sequence(n,{
                microcode::create_DATATRANSFER__REG_ASGN_DEREF_INV_REG(
                        8U,
                        D.start_address_of_temporaries(),
                        temp
                        ),
                microcode::create_SETANDCOPY__REG_ASGN_REG(
                        8U,
                        0ULL,
                        D.start_address_of_temporaries()
                        ),
                microcode::create_HAVOC__REG_ASGN_HAVOC(
                        8ULL,
                        D.start_address_of_temporaries()
                        )
                });

        return true;
    }

    return false;
}

bool  recognition_data::recognise_JNE(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 1U
        && re.is_immediate_operand(0U)
        )
    {
        register_info const  zf = get_register_info("zf",D);
        INVARIANT(zf.second == 1U);

        node_id  zf_is_0, zf_is_1;
        std::tie(zf_is_0,zf_is_1) =
                component().insert_branching(microcode::GIK::GUARDS__REG_EQUAL_TO_ZERO,zf.second,zf.first,component().entry());

        component().insert_sequence(zf_is_0,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.get_immediate_operand(0U)
                            )
                    });

        component().insert_sequence(zf_is_1,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

        return true;
    }

    return false;
}

bool  recognition_data::recognise_JE(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 1U
        && re.is_immediate_operand(0U)
        )
    {
        register_info const  zf = get_register_info("zf",D);
        INVARIANT(zf.second == 1U);

        node_id  zf_is_1, zf_is_0;
        std::tie(zf_is_1,zf_is_0) =
                component().insert_branching(microcode::GIK::GUARDS__REG_NOT_EQUAL_TO_ZERO,zf.second,zf.first,component().entry());

        component().insert_sequence(zf_is_1,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.get_immediate_operand(0U)
                            )
                    });

        component().insert_sequence(zf_is_0,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

        return true;
    }

    return false;
}

bool  recognition_data::recognise_JS(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 1U
        && re.is_immediate_operand(0U)
        )
    {
        register_info const  sf = get_register_info("sf",D);
        INVARIANT(sf.second == 1U);

        node_id  sf_is_1, sf_is_0;
        std::tie(sf_is_1,sf_is_0) =
                component().insert_branching(microcode::GIK::GUARDS__REG_NOT_EQUAL_TO_ZERO,sf.second,sf.first,component().entry());

        component().insert_sequence(sf_is_1,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.get_immediate_operand(0U)
                            )
                    });

        component().insert_sequence(sf_is_0,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

        return true;
    }

    return false;
}

bool  recognition_data::recognise_JNS(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 1U
        && re.is_immediate_operand(0U)
        )
    {
        register_info const  sf = get_register_info("sf",D);
        INVARIANT(sf.second == 1U);

        node_id  cond_is_1, cond_is_0;
        std::tie(cond_is_1,cond_is_0) =
                component().insert_branching(microcode::GIK::GUARDS__REG_EQUAL_TO_ZERO,sf.second,sf.first,component().entry());

        component().insert_sequence(cond_is_1,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.get_immediate_operand(0U)
                            )
                    });

        component().insert_sequence(cond_is_0,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

        return true;
    }

    return false;
}

bool  recognition_data::recognise_JA(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 1U
        && re.is_immediate_operand(0U)
        )
    {
        node_id const  u =
            component().insert_sequence(component().entry(),{
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_OR_REG(
                            1U,
                            D.start_address_of_temporaries(),
                            get_register_info("zf",D).first,
                            get_register_info("cf",D).first
                            )
                    });

        node_id  cond_is_1, cond_is_0;
        std::tie(cond_is_1,cond_is_0) =
                component().insert_branching(microcode::GIK::GUARDS__REG_EQUAL_TO_ZERO,1U,D.start_address_of_temporaries(),u);

        component().insert_sequence(cond_is_1,{
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            1U,
                            D.start_address_of_temporaries()
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.get_immediate_operand(0U)
                            )
                    });

        component().insert_sequence(cond_is_0,{
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            1U,
                            D.start_address_of_temporaries()
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

        return true;
    }

    return false;
}

bool  recognition_data::recognise_JAE(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 1U
        && re.is_immediate_operand(0U)
        )
    {
        node_id  cond_is_1, cond_is_0;
        std::tie(cond_is_1,cond_is_0) =
                component().insert_branching(microcode::GIK::GUARDS__REG_EQUAL_TO_ZERO,get_register_info("cf",D).second,get_register_info("cf",D).first,component().entry());

        component().insert_sequence(cond_is_1,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.get_immediate_operand(0U)
                            )
                    });

        component().insert_sequence(cond_is_0,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

        return true;
    }

    return false;
}

bool  recognition_data::recognise_JG(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 1U
        && re.is_immediate_operand(0U)
        )
    {
        node_id const  u =
            component().insert_sequence(component().entry(),{
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_XOR_REG(
                            1U,
                            D.start_address_of_temporaries(),
                            get_register_info("sf",D).first,
                            get_register_info("of",D).first
                            ),
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_OR_REG(
                            1U,
                            D.start_address_of_temporaries(),
                            D.start_address_of_temporaries(),
                            get_register_info("zf",D).first
                            )
                    });

        node_id  cond_is_1, cond_is_0;
        std::tie(cond_is_1,cond_is_0) =
                component().insert_branching(microcode::GIK::GUARDS__REG_EQUAL_TO_ZERO,1U,D.start_address_of_temporaries(),u);

        component().insert_sequence(cond_is_1,{
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            1U,
                            D.start_address_of_temporaries()
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.get_immediate_operand(0U)
                            )
                    });

        component().insert_sequence(cond_is_0,{
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            1U,
                            D.start_address_of_temporaries()
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

        return true;
    }

    return false;
}

bool  recognition_data::recognise_JLE(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 1U
        && re.is_immediate_operand(0U)
        )
    {
        node_id const  u =
            component().insert_sequence(component().entry(),{
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_XOR_REG(
                            1U,
                            D.start_address_of_temporaries(),
                            get_register_info("sf",D).first,
                            get_register_info("of",D).first
                            ),
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_OR_REG(
                            1U,
                            D.start_address_of_temporaries(),
                            D.start_address_of_temporaries(),
                            get_register_info("zf",D).first
                            )
                    });

        node_id  cond_is_1, cond_is_0;
        std::tie(cond_is_1,cond_is_0) =
                component().insert_branching(microcode::GIK::GUARDS__REG_NOT_EQUAL_TO_ZERO,1U,D.start_address_of_temporaries(),u);

        component().insert_sequence(cond_is_1,{
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            1U,
                            D.start_address_of_temporaries()
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.get_immediate_operand(0U)
                            )
                    });

        component().insert_sequence(cond_is_0,{
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            1U,
                            D.start_address_of_temporaries()
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

        return true;
    }

    return false;
}

bool  recognition_data::recognise_JB(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 1U
        && re.is_immediate_operand(0U)
        )
    {
        node_id  cond_is_1, cond_is_0;
        std::tie(cond_is_1,cond_is_0) =
                component().insert_branching(microcode::GIK::GUARDS__REG_NOT_EQUAL_TO_ZERO,1U,get_register_info("cf",D).first,component().entry());

        component().insert_sequence(cond_is_1,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.get_immediate_operand(0U)
                            )
                    });

        component().insert_sequence(cond_is_0,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

        return true;
    }

    return false;
}

bool  recognition_data::recognise_JBE(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 1U
        && re.is_immediate_operand(0U)
        )
    {
        node_id const  u =
            component().insert_sequence(component().entry(),{
                    microcode::create_BITOPERATIONS__REG_ASGN_REG_OR_REG(
                            1U,
                            D.start_address_of_temporaries(),
                            get_register_info("zf",D).first,
                            get_register_info("cf",D).first
                            )
                    });

        node_id  cond_is_1, cond_is_0;
        std::tie(cond_is_1,cond_is_0) =
                component().insert_branching(microcode::GIK::GUARDS__REG_NOT_EQUAL_TO_ZERO,1U,D.start_address_of_temporaries(),u);

        component().insert_sequence(cond_is_1,{
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            1U,
                            D.start_address_of_temporaries()
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.get_immediate_operand(0U)
                            )
                    });

        component().insert_sequence(cond_is_0,{
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            1U,
                            D.start_address_of_temporaries()
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

        return true;
    }

    return false;
}


bool  recognition_data::recognise_PUSH(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 1U
        && re.is_register_operand(0U)
        && re.is_read_operand(0U)
        )
    {
        register_info const  reg_info = re.get_register_operand(0U,D);

        component().insert_sequence(component().entry(),{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            get_register_info("rsp",D).first,
                            get_register_info("rsp",D).first,
                            ~(uint64_t)reg_info.second + 1ULL
                            ),
                    microcode::create_DATATRANSFER__DEREF_INV_REG_ASGN_REG(
                            reg_info.second,
                            get_register_info("rsp",D).first,
                            reg_info.first
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

        return true;
    }

    if (re.num_operands() == 1U
        && re.is_immediate_operand(0U)
        && (re.num_bytes_of_instruction() == 5U || re.num_bytes_of_instruction() == 3U || re.num_bytes_of_instruction() == 2U)
        )
    {
        uint64_t const  num_bytes = re.num_bytes_of_instruction() - 1ULL;

        component().insert_sequence(component().entry(),{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            get_register_info("rsp",D).first,
                            get_register_info("rsp",D).first,
                            ~num_bytes + 1ULL
                            ),
                    microcode::create_DATATRANSFER__DEREF_INV_REG_ASGN_NUMBER(
                            (uint8_t)num_bytes,
                            get_register_info("rsp",D).first,
                            re.get_immediate_operand(0U)
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

        return true;
    }

    if (re.num_operands() == 1U
        && re.is_memory_operand(0U)
        && re.is_read_operand(0U)
        && !re.is_write_operand(0U)
        && !re.is_segment_register_of_memory_operand_valid(0U)
        && re.is_base_register_of_memory_operand_valid(0U)
        && re.num_bytes_of_instruction() == 6U
        )
    {
        ASSUMPTION(re.get_base_register_of_memory_operand(0U,D).second == 8ULL);

        bool const  use_index_reg = re.is_index_register_of_memory_operand_valid(0U);
        bool const  use_scale = re.get_scale_of_index_register_of_memory_operand(0U) != 1;

        ASSUMPTION(!use_scale || use_index_reg);

        bool const  use_rip = re.get_base_register_of_memory_operand(0U,D) == get_register_info("rip",D);
        bool const  use_displacement = re.get_displacement_of_memory_operand(0U) != 0ULL;

        node_id  n = component().entry();
        uint64_t  temp = re.get_base_register_of_memory_operand(0U,D).first;
        if (use_scale || use_index_reg)
        {
            ASSUMPTION(re.get_index_register_of_memory_operand(0U,D).second == 8ULL);

            if (use_scale)
            {
                n = component().insert_sequence(n,{
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_TIMES_NUMBER(
                                re.get_index_register_of_memory_operand(0U,D).second,
                                D.start_address_of_temporaries(),
                                re.get_index_register_of_memory_operand(0U,D).first,
                                re.get_scale_of_index_register_of_memory_operand(0U)
                                ),
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                                re.get_base_register_of_memory_operand(0U,D).second,
                                D.start_address_of_temporaries(),
                                D.start_address_of_temporaries(),
                                temp
                                )
                        });
            }
            else
            {
                n = component().insert_sequence(n,{
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                                re.get_base_register_of_memory_operand(0U,D).second,
                                D.start_address_of_temporaries(),
                                re.get_index_register_of_memory_operand(0U,D).first,
                                temp
                                )
                        });
            }
            temp = D.start_address_of_temporaries();
        }
        if (use_displacement || use_rip)
        {
            n = component().insert_sequence(n,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            re.get_base_register_of_memory_operand(0U,D).second,
                            D.start_address_of_temporaries(),
                            temp,
                            re.get_displacement_of_memory_operand(0U) + (use_rip ? re.num_bytes_of_instruction() : 0)
                            )
                    });
            temp = D.start_address_of_temporaries();
        }
        uint64_t const  num_bytes = 8ULL;
        uint64_t const  deref_value_begin = temp < D.start_address_of_temporaries() ? D.start_address_of_temporaries() : temp + 8ULL;
        n = component().insert_sequence(n,{
                    microcode::create_DATATRANSFER__REG_ASGN_DEREF_INV_REG(
                            8ULL,
                            deref_value_begin,
                            temp
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8ULL,
                            get_register_info("rsp",D).first,
                            get_register_info("rsp",D).first,
                            ~num_bytes + 1ULL
                            ),
                    microcode::create_DATATRANSFER__DEREF_INV_REG_ASGN_NUMBER(
                            8ULL,
                            get_register_info("rsp",D).first,
                            deref_value_begin
                            ),
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            deref_value_begin + 8ULL - D.start_address_of_temporaries(),
                            D.start_address_of_temporaries()
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                });

        return true;
    }

    return false;
}

bool  recognition_data::recognise_POP(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 1U
        && re.is_register_operand(0U)
        && re.is_write_operand(0U)
        )
    {
        register_info const  reg_info = re.get_register_operand(0U,D);

        component().insert_sequence(component().entry(),{
                    microcode::create_DATATRANSFER__REG_ASGN_DEREF_INV_REG(
                            reg_info.second,
                            reg_info.first,
                            get_register_info("rsp",D).first
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            get_register_info("rsp",D).first,
                            get_register_info("rsp",D).first,
                            reg_info.second
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.num_bytes_of_instruction()
                            )
                    });

        return true;
    }

    return false;
}


bool  recognition_data::recognise_CALL(recognition_engine const& re, descriptor::storage const&  D)
{
    if (re.num_operands() == 1U
        && re.is_immediate_operand(0U)
        && re.is_relative_call()
        )
    {
        component().insert_sequence(component().entry(),{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            D.start_address_of_temporaries(),
                            0ULL,
                            re.num_bytes_of_instruction()
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            get_register_info("rsp",D).first,
                            get_register_info("rsp",D).first,
                            0xfffffffffffffff8ULL
                            ),
                    microcode::create_DATATRANSFER__DEREF_INV_REG_ASGN_REG(
                            8U,
                            get_register_info("rsp",D).first,
                            D.start_address_of_temporaries()
                            ),
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            8ULL,
                            D.start_address_of_temporaries()
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            0ULL,
                            0ULL,
                            re.get_immediate_operand(0U)
                            )
                    });

        return true;
    }

    if (re.num_operands() == 1U
        && re.is_register_operand(0U)
        && re.is_read_operand(0U)
        && !re.is_relative_call()
        )
    {
        ASSUMPTION(re.get_register_operand(0U,D).second == 8ULL);
        component().insert_sequence(component().entry(),{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            D.start_address_of_temporaries(),
                            0ULL,
                            re.num_bytes_of_instruction()
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            get_register_info("rsp",D).first,
                            get_register_info("rsp",D).first,
                            0xfffffffffffffff8ULL
                            ),
                    microcode::create_DATATRANSFER__DEREF_INV_REG_ASGN_REG(
                            8U,
                            get_register_info("rsp",D).first,
                            D.start_address_of_temporaries()
                            ),
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                            8ULL,
                            D.start_address_of_temporaries()
                            ),
                    microcode::create_SETANDCOPY__REG_ASGN_REG(
                            8U,
                            0ULL,
                            re.get_register_operand(0U,D).first
                            )
                    });

        return true;
    }

    if (re.num_operands() == 1U
        && re.is_memory_operand(0U)
        && !re.is_write_operand(0U)
        && !re.is_segment_register_of_memory_operand_valid(0U)
        && !re.is_relative_call()
        )
    {
        ASSUMPTION(re.get_base_register_of_memory_operand(0U,D).second == 8ULL);

        bool const  use_index_reg = re.is_index_register_of_memory_operand_valid(0U);
        bool const  use_scale = re.get_scale_of_index_register_of_memory_operand(0U) != 1;

        ASSUMPTION(!use_scale || use_index_reg);

        bool const  use_rip = re.get_base_register_of_memory_operand(0U,D) == get_register_info("rip",D);
        bool const  use_displacement = re.get_displacement_of_memory_operand(0U) != 0ULL;

        node_id  n = component().insert_sequence(component().entry(),{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            D.start_address_of_temporaries(),
                            0ULL,
                            re.num_bytes_of_instruction()
                            ),
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            8U,
                            get_register_info("rsp",D).first,
                            get_register_info("rsp",D).first,
                            0xfffffffffffffff8ULL
                            ),
                    microcode::create_DATATRANSFER__DEREF_INV_REG_ASGN_REG(
                            8U,
                            get_register_info("rsp",D).first,
                            D.start_address_of_temporaries()
                            ),
                    });

        uint64_t  temp = re.get_base_register_of_memory_operand(0U,D).first;
        if (use_scale || use_index_reg)
        {
            ASSUMPTION(re.get_index_register_of_memory_operand(0U,D).second == 8ULL);

            if (use_scale)
            {
                n = component().insert_sequence(n,{
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_TIMES_NUMBER(
                                re.get_index_register_of_memory_operand(0U,D).second,
                                D.start_address_of_temporaries(),
                                re.get_index_register_of_memory_operand(0U,D).first,
                                re.get_scale_of_index_register_of_memory_operand(0U)
                                ),
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                                re.get_base_register_of_memory_operand(0U,D).second,
                                D.start_address_of_temporaries(),
                                D.start_address_of_temporaries(),
                                temp
                                )
                        });
            }
            else
            {
                n = component().insert_sequence(n,{
                        microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                                re.get_base_register_of_memory_operand(0U,D).second,
                                D.start_address_of_temporaries(),
                                re.get_index_register_of_memory_operand(0U,D).first,
                                temp
                                )
                        });
            }

            temp = D.start_address_of_temporaries();
        }
        if (use_displacement || use_rip)
        {
            n = component().insert_sequence(n,{
                    microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                            re.get_base_register_of_memory_operand(0U,D).second,
                            D.start_address_of_temporaries(),
                            temp,
                            re.get_displacement_of_memory_operand(0U) + (use_rip ? re.num_bytes_of_instruction() : 0)
                            )
                    });
            temp = D.start_address_of_temporaries();
        }

        uint64_t const  IP_value_begin = temp < D.start_address_of_temporaries() ? D.start_address_of_temporaries() : temp + 8ULL;

        n = component().insert_sequence(n,{
                microcode::create_DATATRANSFER__REG_ASGN_DEREF_INV_REG(
                        8U,
                        IP_value_begin,
                        temp
                        ),
                microcode::create_SETANDCOPY__REG_ASGN_REG(
                        8U,
                        0ULL,
                        IP_value_begin
                        ),
                microcode::create_HAVOC__REG_ASGN_HAVOC(
                        IP_value_begin + 8ULL - D.start_address_of_temporaries(),
                        D.start_address_of_temporaries()
                        )
                });

        return true;
    }

    return false;
}

bool  recognition_data::recognise_RET(recognition_engine const& re, descriptor::storage const&  D)
{
    ASSUMPTION(re.num_operands() == 0U || (re.num_operands() == 1U && re.is_immediate_operand(0U)));

    uint64_t const  num_bytes_to_pop = 8ULL + (re.num_operands() != 0U ? re.is_immediate_operand(0U) : 0ULL);

    component().insert_sequence(component().entry(),{
                microcode::create_DATATRANSFER__REG_ASGN_DEREF_INV_REG(
                        8U,
                        D.start_address_of_temporaries(),
                        get_register_info("rsp",D).first
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8U,
                        get_register_info("rsp",D).first,
                        get_register_info("rsp",D).first,
                        num_bytes_to_pop
                        ),
                microcode::create_SETANDCOPY__REG_ASGN_REG(
                        8U,
                        0ULL,
                        D.start_address_of_temporaries()
                        ),
                microcode::create_HAVOC__REG_ASGN_HAVOC(
                        8ULL,
                        D.start_address_of_temporaries()
                        ),
                });

    return true;
}

bool  recognition_data::recognise_LEAVE(recognition_engine const& re, descriptor::storage const&  D)
{
    ASSUMPTION(re.num_operands() == 0U);

    component().insert_sequence(component().entry(),{
                microcode::create_SETANDCOPY__REG_ASGN_REG(
                        8U,
                        get_register_info("rsp",D).first,
                        get_register_info("rbp",D).first
                        ),
                microcode::create_DATATRANSFER__REG_ASGN_DEREF_INV_REG(
                        8U,
                        get_register_info("rbp",D).first,
                        get_register_info("rsp",D).first
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8U,
                        get_register_info("rsp",D).first,
                        get_register_info("rsp",D).first,
                        8ULL
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8U,
                        0ULL,
                        0ULL,
                        re.num_bytes_of_instruction()
                        ),
                });

    return true;
}


bool  recognition_data::recognise_INT(recognition_engine const& re, descriptor::storage const&  D)
{
//  int 0x80
//  ~~~~~~~~
//  On both Linux x86 and Linux x86_64 systems you can make a syscall by calling interrupt 0x80 using
//  the int $0x80 command. Parameters are passed by setting the general purpose registers as following:
//      Syscall no. |   Param 1 |   Param 2 |   Param 3 |   Param 4 |   Param 5 |   Param 6
//      eax         |   ebx     |   ecx     |   edx     |   esi     |   edi     |   ebp
//  Return value: eax
//  The syscall numbers are described in the Linux source file arch/x86/include/asm/unistd_32.h.
//  All registers are preserved during the syscall.

    syscall::ret_type  ret_state = syscall::make_failure();

    if (re.num_operands() == 1U
        && re.is_immediate_operand(0U)
        && re.get_immediate_operand(0U) == 0x80LL
        )
    {
        uint32_t const  eax = syscall::read_register<uint32_t>("eax",D,reg_fn(),ret_state);
        if (syscall::error_code(ret_state) == 0U)
            switch (eax)
            {
            case 1U:
                component().insert_sequence(component().entry(),{
                            microcode::create_MISCELLANEOUS__STOP()
                            });
                ret_state = syscall::make_succes();
                break;
            default:
                ret_state = syscall::make_failure();
                break;
            }

    }

    if (syscall::succeeded(ret_state))
    {
        INVARIANT( syscall::error_code(ret_state) == 0U ||
                   syscall::error_code(ret_state) == 1U ||
                   syscall::error_code(ret_state) == 2U ||
                   syscall::error_code(ret_state) == 4U ||
                   syscall::error_code(ret_state) == 8U );

        if (syscall::error_code(ret_state) != 0U)
        {
            INVARIANT(syscall::error_code(ret_state) == 1U || syscall::address(ret_state) != 0ULL);
            INVARIANT(syscall::error_code(ret_state) == 1U || syscall::rights(ret_state) == 1U || syscall::rights(ret_state) == 2U);

            set_error_result(syscall::error_code(ret_state));
            set_error_address(syscall::address(ret_state));
            if (syscall::error_code(ret_state) != 1U)
                set_error_rights(syscall::rights(ret_state));
        }

        return true;
    }

    return false;
}

bool  recognition_data::recognise_SYSCALL(recognition_engine const& re, descriptor::storage const&  D)
{
//  syscall
//  ~~~~~~~
//  The x86_64 architecture introduced a dedicated instruction to make a syscall. It does not access the interrupt
//  descriptor table and is faster. Parameters are passed by setting the general purpose registers as following:
//      Syscall no. |   Param 1 |   Param 2 |   Param 3 |   Param 4 |   Param 5 |   Param 6
//      rax         |   rdi     |   rsi     |   rdx     |   r10     |   r8      |   r9
//  Return value: rax
//  The syscall numbers are described in the Linux source file arch/x86/include/asm/unistd_64.h.
//  All registers, except rcx and r11, are preserved during the syscall.
//  Mapping of syscall numbers to names:
//  %rax 	Name                Entry point                 Implementation
//  0       read                sys_read                    fs/read_write.c
//  1       write               sys_write                   fs/read_write.c
//  2       open                sys_open                    fs/open.c
//  3       close               sys_close                   fs/open.c
//  4       stat                sys_newstat                 fs/stat.c
//  5       fstat               sys_newfstat                fs/stat.c
//  6       lstat               sys_newlstat                fs/stat.c
//  7       poll                sys_poll                    fs/select.c
//  8       lseek               sys_lseek                   fs/read_write.c
//  9       mmap                sys_mmap                    arch/x86/kernel/sys_x86_64.c
//  10      mprotect            sys_mprotect                mm/mprotect.c
//  11      munmap              sys_munmap                  mm/mmap.c
//  12      brk                 sys_brk                     mm/mmap.c
//  13      rt_sigaction        sys_rt_sigaction            kernel/signal.c
//  14      rt_sigprocmask      sys_rt_sigprocmask          kernel/signal.c
//  15      rt_sigreturn        stub_rt_sigreturn           arch/x86/kernel/signal.c
//  16      ioctl               sys_ioctl                   fs/ioctl.c
//  17      pread64	            sys_pread64                 fs/read_write.c
//  18      pwrite64	        sys_pwrite64                fs/read_write.c
//  19      readv	            sys_readv                   fs/read_write.c
//  20      writev	            sys_writev                  fs/read_write.c
//  21      access	            sys_access                  fs/open.c
//  22      pipe	            sys_pipe                    fs/pipe.c
//  23      select	            sys_select                  fs/select.c
//  24      sched_yield	        sys_sched_yield             kernel/sched/core.c
//  25      mremap	            sys_mremap                  mm/mmap.c
//  26      msync	            sys_msync                   mm/msync.c
//  27      mincore	            sys_mincore                 mm/mincore.c
//  28      madvise	            sys_madvise                 mm/madvise.c
//  29      shmget	            sys_shmget                  ipc/shm.c
//  30      shmat	            sys_shmat                   ipc/shm.c
//  31      shmctl	            sys_shmctl                  ipc/shm.c
//  32      dup	                sys_dup                     fs/file.c
//  33      dup2	            sys_dup2                    fs/file.c
//  34      pause	            sys_pause                   kernel/signal.c
//  35      nanosleep	        sys_nanosleep               kernel/hrtimer.c
//  36      getitimer	        sys_getitimer               kernel/itimer.c
//  37      alarm	            sys_alarm                   kernel/timer.c
//  38      setitimer	        sys_setitimer               kernel/itimer.c
//  39      getpid	            sys_getpid                  kernel/sys.c
//  40      sendfile	        sys_sendfile64              fs/read_write.c
//  41      socket	            sys_socket                  net/socket.c
//  42      connect	            sys_connect                 net/socket.c
//  43      accept	            sys_accept                  net/socket.c
//  44      sendto	            sys_sendto                  net/socket.c
//  45      recvfrom	        sys_recvfrom                net/socket.c
//  46      sendmsg	            sys_sendmsg                 net/socket.c
//  47      recvmsg	            sys_recvmsg                 net/socket.c
//  48      shutdown	        sys_shutdown                net/socket.c
//  49      bind	            sys_bind                    net/socket.c
//  50      listen	            sys_listen                  net/socket.c
//  51      getsockname	        sys_getsockname	            net/socket.c
//  52      getpeername	        sys_getpeername	            net/socket.c
//  53      socketpair	        sys_socketpair	            net/socket.c
//  54      setsockopt	        sys_setsockopt	            net/socket.c
//  55      getsockopt	        sys_getsockopt	            net/socket.c
//  56      clone               stub_clone	                kernel/fork.c
//  57      fork                stub_fork	                kernel/fork.c
//  58      vfork               stub_vfork	                kernel/fork.c
//  59      execve              stub_execve	                fs/exec.c
//  60      exit	            sys_exit	                kernel/exit.c
//  61      wait4	            sys_wait4	                kernel/exit.c
//  62      kill	            sys_kill	                kernel/signal.c
//  63      uname	            sys_newuname	            kernel/sys.c
//  64      semget	            sys_semget	                ipc/sem.c
//  65      semop	            sys_semop	                ipc/sem.c
//  66      semctl	            sys_semctl	                ipc/sem.c
//  67      shmdt	            sys_shmdt	                ipc/shm.c
//  68      msgget	            sys_msgget	                ipc/msg.c
//  69      msgsnd	            sys_msgsnd	                ipc/msg.c
//  70      msgrcv	            sys_msgrcv	                ipc/msg.c
//  71      msgctl	            sys_msgctl	                ipc/msg.c
//  72      fcntl	            sys_fcntl	                fs/fcntl.c
//  73      flock	            sys_flock	                fs/locks.c
//  74      fsync	            sys_fsync	                fs/sync.c
//  75      fdatasync	        sys_fdatasync	            fs/sync.c
//  76      truncate	        sys_truncate	            fs/open.c
//  77      ftruncate	        sys_ftruncate	            fs/open.c
//  78      getdents	        sys_getdents	            fs/readdir.c
//  79      getcwd	            sys_getcwd	                fs/dcache.c
//  80      chdir	            sys_chdir	                fs/open.c
//  81      fchdir	            sys_fchdir	                fs/open.c
//  82      rename	            sys_rename	                fs/namei.c
//  83      mkdir	            sys_mkdir	                fs/namei.c
//  84      rmdir	            sys_rmdir	                fs/namei.c
//  85      creat	            sys_creat	                fs/open.c
//  86      link	            sys_link	                fs/namei.c
//  87      unlink	            sys_unlink	                fs/namei.c
//  88      symlink	            sys_symlink	                fs/namei.c
//  89      readlink	        sys_readlink	            fs/stat.c
//  90      chmod	            sys_chmod	                fs/open.c
//  91      fchmod	            sys_fchmod	                fs/open.c
//  92      chown	            sys_chown	                fs/open.c
//  93      fchown	            sys_fchown	                fs/open.c
//  94      lchown	            sys_lchown	                fs/open.c
//  95      umask	            sys_umask	                kernel/sys.c
//  96      gettimeofday	    sys_gettimeofday            kernel/time.c
//  97      getrlimit           sys_getrlimit	            kernel/sys.c
//  98      getrusage	        sys_getrusage	            kernel/sys.c
//  99      sysinfo             sys_sysinfo	                kernel/sys.c
//  100	times	                sys_times	                kernel/sys.c
//  101	ptrace	                sys_ptrace	                kernel/ptrace.c
//  102	getuid	                sys_getuid	                kernel/sys.c
//  103	syslog	                sys_syslog	                kernel/printk/printk.c
//  104	getgid	                sys_getgid	                kernel/sys.c
//  105	setuid	                sys_setuid	                kernel/sys.c
//  106	setgid	                sys_setgid	                kernel/sys.c
//  107	geteuid	                sys_geteuid	                kernel/sys.c
//  108	getegid	                sys_getegid	                kernel/sys.c
//  109	setpgid	                sys_setpgid	                kernel/sys.c
//  110	getppid	                sys_getppid	                kernel/sys.c
//  111	getpgrp	                sys_getpgrp	                kernel/sys.c
//  112	setsid	                sys_setsid	                kernel/sys.c
//  113	setreuid	            sys_setreuid	            kernel/sys.c
//  114	setregid	            sys_setregid	            kernel/sys.c
//  115	getgroups	            sys_getgroups	            kernel/groups.c
//  116	setgroups	            sys_setgroups	            kernel/groups.c
//  117	setresuid	            sys_setresuid	            kernel/sys.c
//  118	getresuid	            sys_getresuid	            kernel/sys.c
//  119	setresgid	            sys_setresgid	            kernel/sys.c
//  120	getresgid	            sys_getresgid	            kernel/sys.c
//  121	getpgid	                sys_getpgid	                kernel/sys.c
//  122	setfsuid	            sys_setfsuid	            kernel/sys.c
//  123	setfsgid	            sys_setfsgid	            kernel/sys.c
//  124	getsid	                sys_getsid	                kernel/sys.c
//  125	capget	                sys_capget	                kernel/capability.c
//  126	capset	                sys_capset                  kernel/capability.c
//  127	rt_sigpending	        sys_rt_sigpending	        kernel/signal.c
//  128	rt_sigtimedwait	        sys_rt_sigtimedwait	        kernel/signal.c
//  129	rt_sigqueueinfo	        sys_rt_sigqueueinfo	        kernel/signal.c
//  130	rt_sigsuspend	        sys_rt_sigsuspend	        kernel/signal.c
//  131	sigaltstack	            sys_sigaltstack	            kernel/signal.c
//  132	utime	                sys_utime                   fs/utimes.c
//  133	mknod	                sys_mknod	                fs/namei.c
//  134	uselib                                              fs/exec.c
//  135	personality             sys_personality             kernel/exec_domain.c
//  136	ustat	                sys_ustat	                fs/statfs.c
//  137	statfs	                sys_statfs                  fs/statfs.c
//  138	fstatfs	                sys_fstatfs	                fs/statfs.c
//  139	sysfs	                sys_sysfs                   fs/filesystems.c
//  140	getpriority	            sys_getpriority	            kernel/sys.c
//  141	setpriority	            sys_setpriority             kernel/sys.c
//  142	sched_setparam	        sys_sched_setparam	        kernel/sched/core.c
//  143	sched_getparam	        sys_sched_getparam          kernel/sched/core.c
//  144	sched_setscheduler	    sys_sched_setscheduler	    kernel/sched/core.c
//  145	sched_getscheduler	    sys_sched_getscheduler      kernel/sched/core.c
//  146	sched_get_priority_max	sys_sched_get_priority_max	kernel/sched/core.c
//  147	sched_get_priority_min	sys_sched_get_priority_min	kernel/sched/core.c
//  148	sched_rr_get_interval	sys_sched_rr_get_interval	kernel/sched/core.c
//  149	mlock	                sys_mlock	                mm/mlock.c
//  150	munlock	                sys_munlock	                mm/mlock.c
//  151	mlockall	            sys_mlockall	            mm/mlock.c
//  152	munlockall	            sys_munlockall	            mm/mlock.c
//  153	vhangup	                sys_vhangup	                fs/open.c
//  154	modify_ldt	            sys_modify_ldt	            arch/x86/um/ldt.c
//  155	pivot_root	            sys_pivot_root              fs/namespace.c
//  156	_sysctl	                sys_sysctl	                kernel/sysctl_binary.c
//  157	prctl	                sys_prctl	                kernel/sys.c
//  158	arch_prctl	            sys_arch_prctl	            arch/x86/um/syscalls_64.c
//  159	adjtimex	            sys_adjtimex                kernel/time.c
//  160	setrlimit	            sys_setrlimit	            kernel/sys.c
//  161	chroot	                sys_chroot	                fs/open.c
//  162	sync	                sys_sync	                fs/sync.c
//  163	acct	                sys_acct	                kernel/acct.c
//  164	settimeofday	        sys_settimeofday            kernel/time.c
//  165	mount	                sys_mount	                fs/namespace.c
//  166	umount2	                sys_umount	                fs/namespace.c
//  167	swapon	                sys_swapon	                mm/swapfile.c
//  168	swapoff	                sys_swapoff	                mm/swapfile.c
//  169	reboot	                sys_reboot	                kernel/reboot.c
//  170	sethostname	            sys_sethostname	            kernel/sys.c
//  171	setdomainname           sys_setdomainname           kernel/sys.c
//  172	iopl                    stub_iopl	                arch/x86/kernel/ioport.c
//  173	ioperm	                sys_ioperm	                arch/x86/kernel/ioport.c
//  174	create_module           NOT IMPLEMENTED
//  175	init_module	            sys_init_module             kernel/module.c
//  176	delete_module           sys_delete_module	        kernel/module.c
//  177	get_kernel_syms         NOT IMPLEMENTED
//  178	query_module            NOT IMPLEMENTED
//  179	quotactl	            sys_quotactl                fs/quota/quota.c
//  180	nfsservctl              NOT IMPLEMENTED
//  181	getpmsg                 NOT IMPLEMENTED
//  182	putpmsg                 NOT IMPLEMENTED
//  183	afs_syscall             NOT IMPLEMENTED
//  184	tuxcall                 NOT IMPLEMENTED
//  185	security                NOT IMPLEMENTED
//  186	gettid	                sys_gettid	                kernel/sys.c
//  187	readahead	            sys_readahead	            mm/readahead.c
//  188	setxattr	            sys_setxattr	            fs/xattr.c
//  189	lsetxattr	            sys_lsetxattr	            fs/xattr.c
//  190	fsetxattr	            sys_fsetxattr	            fs/xattr.c
//  191	getxattr	            sys_getxattr	            fs/xattr.c
//  192	lgetxattr	            sys_lgetxattr	            fs/xattr.c
//  193	fgetxattr	            sys_fgetxattr	            fs/xattr.c
//  194	listxattr	            sys_listxattr	            fs/xattr.c
//  195	llistxattr	            sys_llistxattr	            fs/xattr.c
//  196	flistxattr	            sys_flistxattr	            fs/xattr.c
//  197	removexattr	            sys_removexattr	            fs/xattr.c
//  198	lremovexattr	        sys_lremovexattr	        fs/xattr.c
//  199	fremovexattr	        sys_fremovexattr            fs/xattr.c
//  200	tkill	                sys_tkill	                kernel/signal.c
//  201	time	                sys_time	                kernel/time.c
//  202	futex	                sys_futex	                kernel/futex.c
//  203	sched_setaffinity	    sys_sched_setaffinity	    kernel/sched/core.c
//  204	sched_getaffinity	    sys_sched_getaffinity       kernel/sched/core.c
//  205	set_thread_area                                     arch/x86/kernel/tls.c
//  206	io_setup	            sys_io_setup                fs/aio.c
//  207	io_destroy	            sys_io_destroy	            fs/aio.c
//  208	io_getevents	        sys_io_getevents            fs/aio.c
//  209	io_submit	            sys_io_submit               fs/aio.c
//  210	io_cancel	            sys_io_cancel               fs/aio.c
//  211	get_thread_area                                     arch/x86/kernel/tls.c
//  212	lookup_dcookie          sys_lookup_dcookie          fs/dcookies.c
//  213	epoll_create	        sys_epoll_create	        fs/eventpoll.c
//  214	epoll_ctl_old           NOT IMPLEMENTED
//  215	epoll_wait_old          NOT IMPLEMENTED
//  216	remap_file_pages	    sys_remap_file_pages        mm/fremap.c
//  217	getdents64	            sys_getdents64              fs/readdir.c
//  218	set_tid_address	        sys_set_tid_address	        kernel/fork.c
//  219	restart_syscall	        sys_restart_syscall         kernel/signal.c
//  220	semtimedop	            sys_semtimedop	            ipc/sem.c
//  221	fadvise64	            sys_fadvise64               mm/fadvise.c
//  222	timer_create	        sys_timer_create	        kernel/posix-timers.c
//  223	timer_settime	        sys_timer_settime	        kernel/posix-timers.c
//  224	timer_gettime	        sys_timer_gettime	        kernel/posix-timers.c
//  225	timer_getoverrun	    sys_timer_getoverrun	    kernel/posix-timers.c
//  226	timer_delete	        sys_timer_delete	        kernel/posix-timers.c
//  227	clock_settime	        sys_clock_settime	        kernel/posix-timers.c
//  228	clock_gettime	        sys_clock_gettime	        kernel/posix-timers.c
//  229	clock_getres	        sys_clock_getres	        kernel/posix-timers.c
//  230	clock_nanosleep	        sys_clock_nanosleep	        kernel/posix-timers.c
//  231	exit_group	            sys_exit_group	            kernel/exit.c
//  232	epoll_wait	            sys_epoll_wait	            fs/eventpoll.c
//  233	epoll_ctl	            sys_epoll_ctl	            fs/eventpoll.c
//  234	tgkill	                sys_tgkill	                kernel/signal.c
//  235	utimes	                sys_utimes	                fs/utimes.c
//  236	vserver                 NOT IMPLEMENTED
//  237	mbind	                sys_mbind	                mm/mempolicy.c
//  238	set_mempolicy	        sys_set_mempolicy	        mm/mempolicy.c
//  239	get_mempolicy	        sys_get_mempolicy           mm/mempolicy.c
//  240	mq_open	                sys_mq_open	                ipc/mqueue.c
//  241	mq_unlink	            sys_mq_unlink	            ipc/mqueue.c
//  242	mq_timedsend	        sys_mq_timedsend            ipc/mqueue.c
//  243	mq_timedreceive	        sys_mq_timedreceive         ipc/mqueue.c
//  244	mq_notify	            sys_mq_notify	            ipc/mqueue.c
//  245	mq_getsetattr	        sys_mq_getsetattr           ipc/mqueue.c
//  246	kexec_load	            sys_kexec_load              kernel/kexec.c
//  247	waitid	                sys_waitid	                kernel/exit.c
//  248	add_key	                sys_add_key                 security/keys/keyctl.c
//  249	request_key	            sys_request_key         	security/keys/keyctl.c
//  250	keyctl	                sys_keyctl                  security/keys/keyctl.c
//  251	ioprio_set	            sys_ioprio_set	            fs/ioprio.c
//  252	ioprio_get	            sys_ioprio_get	            fs/ioprio.c
//  253	inotify_init	        sys_inotify_init            fs/notify/inotify/inotify_user.c
//  254	inotify_add_watch	    sys_inotify_add_watch       fs/notify/inotify/inotify_user.c
//  255	inotify_rm_watch	    sys_inotify_rm_watch	    fs/notify/inotify/inotify_user.c
//  256	migrate_pages	        sys_migrate_pages	        mm/mempolicy.c
//  257	openat	                sys_openat	                fs/open.c
//  258	mkdirat	                sys_mkdirat	                fs/namei.c
//  259	mknodat	                sys_mknodat	                fs/namei.c
//  260	fchownat	            sys_fchownat	            fs/open.c
//  261	futimesat	            sys_futimesat	            fs/utimes.c
//  262	newfstatat	            sys_newfstatat	            fs/stat.c
//  263	unlinkat	            sys_unlinkat	            fs/namei.c
//  264	renameat	            sys_renameat	            fs/namei.c
//  265	linkat	                sys_linkat	                fs/namei.c
//  266	symlinkat	            sys_symlinkat	            fs/namei.c
//  267	readlinkat	            sys_readlinkat	            fs/stat.c
//  268	fchmodat	            sys_fchmodat	            fs/open.c
//  269	faccessat	            sys_faccessat	            fs/open.c
//  270	pselect6	            sys_pselect6	            fs/select.c
//  271	ppoll	                sys_ppoll	                fs/select.c
//  272	unshare	                sys_unshare	                kernel/fork.c
//  273	set_robust_list	        sys_set_robust_list         kernel/futex.c
//  274	get_robust_list	        sys_get_robust_list         kernel/futex.c
//  275	splice	                sys_splice	                fs/splice.c
//  276	tee                     sys_tee	                    fs/splice.c
//  277	sync_file_range	        sys_sync_file_range         fs/sync.c
//  278	vmsplice	            sys_vmsplice	            fs/splice.c
//  279	move_pages	            sys_move_pages	            mm/migrate.c
//  280	utimensat	            sys_utimensat	            fs/utimes.c
//  281	epoll_pwait	            sys_epoll_pwait	            fs/eventpoll.c
//  282	signalfd	            sys_signalfd	            fs/signalfd.c
//  283	timerfd_create	        sys_timerfd_create          fs/timerfd.c
//  284	eventfd	                sys_eventfd	                fs/eventfd.c
//  285	fallocate               sys_fallocate               fs/open.c
//  286	timerfd_settime	        sys_timerfd_settime	        fs/timerfd.c
//  287	timerfd_gettime	        sys_timerfd_gettime         fs/timerfd.c
//  288	accept4	                sys_accept4	                net/socket.c
//  289	signalfd4	            sys_signalfd4               fs/signalfd.c
//  290	eventfd2	            sys_eventfd2	            fs/eventfd.c
//  291	epoll_create1	        sys_epoll_create1           fs/eventpoll.c
//  292	dup3	                sys_dup3	                fs/file.c
//  293	pipe2	                sys_pipe2	                fs/pipe.c
//  294	inotify_init1           sys_inotify_init1           fs/notify/inotify/inotify_user.c
//  295	preadv	                sys_preadv	                fs/read_write.c
//  296	pwritev	                sys_pwritev	                fs/read_write.c
//  297	rt_tgsigqueueinfo	    sys_rt_tgsigqueueinfo       kernel/signal.c
//  298	perf_event_open         sys_perf_event_open	        kernel/events/core.c
//  299	recvmmsg	            sys_recvmmsg                net/socket.c
//  300	fanotify_init           sys_fanotify_init	        fs/notify/fanotify/fanotify_user.c
//  301	fanotify_mark	        sys_fanotify_mark           fs/notify/fanotify/fanotify_user.c
//  302	prlimit64               sys_prlimit64	            kernel/sys.c
//  303	name_to_handle_at	    sys_name_to_handle_at       fs/fhandle.c
//  304	open_by_handle_at       sys_open_by_handle_at       fs/fhandle.c
//  305	clock_adjtime	        sys_clock_adjtime	        kernel/posix-timers.c
//  306	syncfs	                sys_syncfs	                fs/sync.c
//  307	sendmmsg                sys_sendmmsg                net/socket.c
//  308	setns	                sys_setns	                kernel/nsproxy.c
//  309	getcpu	                sys_getcpu	                kernel/sys.c
//  310	process_vm_readv	    sys_process_vm_readv	    mm/process_vm_access.c
//  311	process_vm_writev	    sys_process_vm_writev       mm/process_vm_access.c
//  312	kcmp	                sys_kcmp	                kernel/kcmp.c
//  313	finit_module            sys_finit_module	        kernel/module.c
// Then use pages like:
//  http://linux.die.net/man/2/
//  http://repo-genesis3.cbi.utsa.edu/crossref/ns-sli/usr/include/bits/types.h.html
//  http://lxr.free-electrons.com/source/include/linux/types.h
//  http://man7.org/linux/man-pages/man2/stat.2.html
// to see implementation details of individual syscalls.

    ASSUMPTION(re.num_operands() == 0U);

    syscall::ret_type  ret_state;
    uint64_t const  rax = syscall::read_register<uint64_t>("rax",D,reg_fn(),ret_state);
    if (syscall::error_code(ret_state) == 0U)
        switch (rax)
        {
        case   1ULL: ret_state = syscall::recognise_syscall_1_write(reg_fn(),mem_fn(),D,re.num_bytes_of_instruction(),component()); break;
        case   5ULL: ret_state = syscall::recognise_syscall_5_fstat(reg_fn(),mem_fn(),D,re.num_bytes_of_instruction(),component()); break;
        case   9ULL: ret_state = syscall::recognise_syscall_9_mmap(reg_fn(),mem_fn(),D,re.num_bytes_of_instruction(),component()); break;
        case 231ULL: ret_state = syscall::recognise_syscall_231_exit_group(reg_fn(),mem_fn(),D,re.num_bytes_of_instruction(),component()); break;
        default:
            ret_state = syscall::make_failure();
            break;
        }

    if (syscall::succeeded(ret_state))
    {
        INVARIANT( syscall::error_code(ret_state) == 0U ||
                   syscall::error_code(ret_state) == 1U ||
                   syscall::error_code(ret_state) == 2U ||
                   syscall::error_code(ret_state) == 4U ||
                   syscall::error_code(ret_state) == 8U );

        if (syscall::error_code(ret_state) != 0U)
        {
            INVARIANT(syscall::error_code(ret_state) == 1U || syscall::address(ret_state) != 0ULL);
            INVARIANT(syscall::error_code(ret_state) == 1U || syscall::rights(ret_state) == 1U || syscall::rights(ret_state) == 2U);

            set_error_result(syscall::error_code(ret_state));
            set_error_address(syscall::address(ret_state));
            if (syscall::error_code(ret_state) != 1U)
                set_error_rights(syscall::rights(ret_state));
        }

        return true;
    }

    return false;
}


detail::recognition_data_ptr  recognise(descriptor::storage const&  description, uint64_t const  start_address, reg_fn_type const&  reg_fn, mem_fn_type const&  mem_fn)
{
    recognition_data_ptr const  data = std::make_shared<recognition_data>(start_address,reg_fn,mem_fn);
    data->recognise(description);
    return data;
}


}}}}
