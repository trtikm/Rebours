#include <rebours/MAL/recogniser/dump.hpp>
#include <rebours/MAL/recogniser/detail/recognition_data.hpp>

namespace mal { namespace recogniser {


bool  dump_details_of_recognised_instruction(recognition_result const&  result, std::string const&  dump_file_pathname)
{
    return (result.program().operator bool() || result.result() == 255U) ? detail::get_implementation_details(result)->dump(dump_file_pathname) : false;
}


}}



//namespace decoder { namespace {




//}}


//namespace decoder {


//std::ostream&  dump_disassembly_info(std::ostream&  ostr, uint8_t const*  begin, uint8_t const*  end, microcode::cpu_props_ptr const  cpu_props, uint8_t const*  processor_mode_begin)
//{
//    ostr << "<!DOCTYPE html>\n"
//            "<html>\n"
//            "<head>\n"
//            "<style>\n"
//            "table, th, td {\n"
//            "    border: 1px solid black\n"
//            "    border-collapse: collapse\n"
//            "}\n"
//            "th, td {\n"
//            "    padding: 5px\n"
//            "}\n"
//            "h1, h2, h3, h4, p, a, table, ul { font-family: \"Liberation serif\", serif }\n"
//            "p, a, table, ul { font-size: 12pt }\n"
//            "h4 { font-size: 12pt }\n"
//            "h3 { font-size: 14pt }\n"
//            "h2 { font-size: 18pt }\n"
//            "h1 { font-size: 24pt }\n"
//            "tt { font-family: \"Liberation Mono\", monospace }\n"
//            "tt { font-size: 10pt }\n"
//            "body {\n"
//            "    background-color: white\n"
//            "    color: black\n"
//            "}\n"
//            "</style>\n"
//            "</head>\n"
//            "<body>\n"
//            ;

//    csh handle;
//    if (cpu_props->platform()->architecture_name() == loader::architecture_names::X86_64())
//    {
//        if (cs_open(CS_ARCH_X86, CS_MODE_64, &handle) != CS_ERR_OK)
//        {
//            ostr << "<p>"
//                    "ERROR: cannot open the capstone-next library ('cs_open' has FAILED)."
//                    "</p>\n</body>\n</html>\n";
//            return ostr;
//        }
//    }
//    else
//    {
//        ostr << "<p>"
//                "ERROR: Unsupported processor architecture."
//                "</p>\n</body>\n</html>\n";
//        return ostr;
//    }

//    cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);

//    cs_insn*  insn;
//    size_t const  count = cs_disasm(handle,begin,end-begin,0ULL,1ULL,&insn);
//    if (count != 1ULL)
//    {
//        ostr << "<p>"
//                "ERROR: Cannot diassemly the instruction ('cs_disasm' has FAILED)."
//                "</p>\n</body>\n</html>\n";
//        return ostr;
//    }

//    cs_insn const* instr = &(insn[0ULL]);

//    cs_detail *detail = instr->detail;
//    if (detail == nullptr)
//    {
//        ostr << "<p>"
//                "ERROR: Cannot access details of the disassembled instruction."
//                "</p>\n</body>\n</html>\n";
//        return ostr;
//    }

//    cs_x86 const* const  x86 = &(detail->x86);

//    ostr << "<h2>Disassembly info</h2>\n"
//            "<p>\nThe table bellow containts general properties of the disassembled instruction.\nAll numbers are hexadecimal.\n</p>\n"
//            "<table align=\"center\">\n"
//            "  <tr>\n"
//            "    <th align=\"right\">Property</th>\n"
//            "    <th align=\"left\">Description</th>\n"
//            "  </tr>\n"
//            "  <tr>\n"
//            "    <td align=\"right\">" << loader::architecture_names::X86_64() << "</td>\n"
//            "    <td>The processor architecture considered during the disasembly.</td>\n"
//            "  </tr>\n"
//            "  <tr>\n"
//            "    <td align=\"right\">"
//            ;

//    for (uint64_t  i = 0ULL, n = cpu_props->processor_mode_end() - cpu_props->processor_mode_begin(); i < n; ++i)
//        ostr << std::hex << (uint32_t)*(processor_mode_begin + i) << (i + 1ULL == n ? "" : " ");

//    ostr << "</td>\n"
//            "    <td>The processor mode considered during the disasembly.</td>\n"
//            "  </tr>\n"
//            "  <tr>\n"
//            "    <td align=\"right\">" << instr->mnemonic << " " << instr->op_str << "</td>\n"
//            "    <td>Instruction disassembly.</td>\n"
//            "  </tr>\n"
//            "  <tr>\n"
//            "    <td align=\"right\">" << instr->size << "</td>\n"
//            "    <td>The number of bytes of the instruction.</td>\n"
//            "  </tr>\n"
//            "  <tr>\n"
//            "    <td align=\"right\">"
//            ;

//    for (uint16_t  i = 0ULL; i < instr->size; ++i)
//        ostr << std::hex << (uint32_t)instr->bytes[i] << (i + 1ULL == instr->size ? "" : " ");

//    ostr << "</td>\n"
//            "    <td>Bytes forming the instruction.</td>\n"
//            "  </tr>\n"
//            "  <tr>\n"
//            "    <td align=\"right\">" << std::hex << instr->id << "</td>\n"
//            "    <td>Opcode ID</td>\n"
//            "  </tr>\n"
//            "  <tr>\n"
//            "    <td align=\"right\">" << std::hex
//                       << (uint32_t)x86->prefix[0] << " "
//                       << (uint32_t)x86->prefix[1] << " "
//                       << (uint32_t)x86->prefix[2] << " "
//                       << (uint32_t)x86->prefix[3]
//                       << "</td>\n"
//            "    <td>Prefix</td>\n"
//            "  </tr>\n"
//            "  <tr>\n"
//            "    <td align=\"right\">" << std::hex
//                       << (uint32_t)x86->opcode[0] << " "
//                       << (uint32_t)x86->opcode[1] << " "
//                       << (uint32_t)x86->opcode[2] << " "
//                       << (uint32_t)x86->opcode[3]
//                       << "</td>\n"
//            "    <td>Opcode</td>\n"
//            "  </tr>\n"
//            "  <tr>\n"
//            "    <td align=\"right\">" << std::hex << (uint32_t)x86->rex << "</td>\n"
//            "    <td>Rex</td>\n"
//            "  </tr>\n"
//            "  <tr>\n"
//            "    <td align=\"right\">" << std::hex << (uint32_t)x86->addr_size << "</td>\n"
//            "    <td>Address size</td>\n"
//            "  </tr>\n"
//            "  <tr>\n"
//            "    <td align=\"right\">" << std::hex << (uint32_t)x86->modrm << "</td>\n"
//            "    <td>Modrm</td>\n"
//            "  </tr>\n"
//            "  <tr>\n"
//            "    <td align=\"right\">" << std::hex << (uint32_t)x86->sib << "</td>\n"
//            "    <td>Sib</td>\n"
//            "  </tr>\n"
//            "  <tr>\n"
//            "    <td align=\"right\">" << std::hex << x86->disp << "</td>\n"
//            "    <td>Displacement</td>\n"
//            "  </tr>\n"
//            "  <tr>\n"
//            "    <td align=\"right\">" << std::hex << x86->xop_cc << "</td>\n"
//            "    <td>Xop_cc</td>\n"
//            "  </tr>\n"
//            "  <tr>\n"
//            "    <td align=\"right\">" << std::hex << x86->sse_cc << "</td>\n"
//            "    <td>Sse_cc</td>\n"
//            "  </tr>\n"
//            "  <tr>\n"
//            "    <td align=\"right\">" << std::hex << x86->avx_cc << "</td>\n"
//            "    <td>Avx_cc</td>\n"
//            "  </tr>\n"
//            "  <tr>\n"
//            "    <td align=\"right\">" << std::hex << x86->avx_sae << "</td>\n"
//            "    <td>Avx_sae</td>\n"
//            "  </tr>\n"
//            "  <tr>\n"
//            "    <td align=\"right\">" << std::hex << x86->avx_rm << "</td>\n"
//            "    <td>Avx_rm</td>\n"
//            "  </tr>\n"
//            "  <tr>\n"
//            "    <td align=\"right\">" << std::hex << x86->eflags << "</td>\n"
//            "    <td>Eflags</td>\n"
//            "  </tr>\n"
//            "  <tr>\n"
//            "    <td align=\"right\">" << std::hex << (uint32_t)x86->op_count << "</td>\n"
//            "    <td>The count of operands.</td>\n"
//            "  </tr>\n"
//            "</table>\n"
//            ;

////    cs_regs regs_read;
////    cs_regs  regs_write;
////    uint8_t regs_read_count, regs_write_count;
////    if (cs_regs_access(handle,instr,regs_read,&regs_read_count,regs_write,&regs_write_count) == 0)
////    {
////        for (uint8_t i = 0U; i < regs_read_count; ++i)
////            std::cout << "in reg #" << (uint32_t)i << " = " << cs_reg_name(handle, regs_read[i]) << "\n";
////        for (uint8_t i = 0U; i < regs_write_count; ++i)
////            std::cout << "out reg #" << (uint32_t)i << " = " << cs_reg_name(handle, regs_write[i]) << "\n";
////    }

////    for (uint8_t n = 0U; n < detail->regs_read_count; ++n)
////        std::cout << "in reg " << n << " = " << cs_reg_name(handle, detail->regs_read[n]) << "\n";
////    for (uint8_t n = 0U; n < detail->regs_write_count; ++n)
////        std::cout << "out reg " << n << " = " << cs_reg_name(handle, detail->regs_write[n]) << "\n";
////    for (uint8_t n = 0; n < detail->groups_count; ++n)
////        std::cout << "out reg " << n << " = " << detail->groups[n] << "\n";

//    ostr << "<p>\nThe table bellow shows individual arguments of the disassembled instruction.\nAll numbers are hexadecimal.\n</p>\n"
//            "<table align=\"center\">\n"
//            "  <tr align=\"right\">\n"
//            "    <th>#</th>\n"
//            "    <th>Num bytes</th>\n"
//            "    <th>Is input</th>\n"
//            "    <th>Is output</th>\n"
//            "    <th>Type</th>\n"
//            "    <th>Name</th>\n"
//            "    <th>Value</th>\n"
//            "    <th>Segment REG</th>\n"
//            "    <th>Base REG</th>\n"
//            "    <th>Index REG</th>\n"
//            "    <th>Scale IMM</th>\n"
//            "    <th>Displacement IMM</th>\n"
//            "  </tr>\n"
//            ;

//    for (uint8_t  i = 0U; i < x86->op_count; ++i)
//    {
//        cs_x86_op const* const  op = &(x86->operands[i]);

//        ostr << "  <tr align=\"right\">\n"
//                "    <td>" << std::hex << (uint32_t)i << "</td>\n"
//                "    <td>" << std::hex << (uint32_t)op->size << "</td>\n";
//                ;

//        switch (op->access)
//        {
//        case CS_AC_READ:
//            ostr << "<td>true</td>\n"
//                    "<td>false</td>\n"
//                    ;
//            break;
//        case CS_AC_WRITE:
//            ostr << "<td>false</td>\n"
//                    "<td>true</td>\n"
//                    ;
//            break;
//        case CS_AC_READ | CS_AC_WRITE:
//            ostr << "<td>true</td>\n"
//                    "<td>true</td>\n"
//                    ;
//            break;
//        default:
//            if (op->type == X86_OP_IMM)
//                ostr << "<td>true</td>\n"
//                        "<td>false</td>\n"
//                        ;
//            else
//                ostr << "<td>false</td>\n"
//                        "<td>false</td>\n"
//                        ;
//            break;
//        }

//        switch(op->type)
//        {
//        case X86_OP_REG:
//            ostr << "<td>REG</td>\n"
//                    "<td>"<< cs_reg_name(handle,op->reg) << "</td>\n";
//            break;
//        case X86_OP_IMM:
//            ostr << "<td>IMM</td>\n"
//                    "<td></td>\n"
//                    "<td>"<< std::hex << op->imm << "</td>\n";
//            break;
//        case X86_OP_MEM:
//            {
//                x86_op_mem const* const  mem = &(op->mem);
//                ostr << "<td>MEM</td>\n"
//                        "<td></td>\n"
//                        "<td></td>\n"
//                        "<td>"<< (mem->segment == X86_REG_INVALID ? "" : cs_reg_name(handle,mem->segment)) << "</td>\n"
//                        "<td>"<< (mem->base == X86_REG_INVALID ? "" : cs_reg_name(handle,mem->base)) << "</td>\n"
//                        "<td>"<< (mem->index == X86_REG_INVALID ? "" : cs_reg_name(handle,mem->index)) << "</td>\n"
//                        "<td>"<< std::hex << mem->scale << "</td>\n"
//                        "<td>"<< std::hex << mem->disp << "</td>\n"
//                        ;
//            }
//            break;
//        case X86_OP_FP:
//            ostr << "<td>FP</td>\n"
//                    "<td></td>\n"
//                    "<td>"<< op->fp << "</td>\n";
//            break;
//        default:
//            ostr << "<td>INVALID</td>\n";
//            break;
//        }

//        ostr << "  </tr>\n";
//    }

//    ostr << "</table>\n"
//            "<p>\nAnd the table bellow shows usage of processor flags in the disassembled instruction.\n</p>\n"
//            "<table align=\"center\">\n"
//            "  <tr>\n"
//            "    <th>Usage of flags</th>\n"
//            "  </tr>\n"
//            ;

//    for (uint8_t i = 0U; i < 46U; ++i)
//        if ((x86->eflags & (1ULL << i)) != 0ULL)
//        {
//            ostr << "<tr><td>";
//            switch (1ULL << i)
//            {
//            case X86_EFLAGS_MODIFY_AF:
//                std::cout << "EFLAGS_MODIFY_AF\n";
//                break;
//            case X86_EFLAGS_MODIFY_CF:
//                std::cout << "EFLAGS_MODIFY_CF\n";
//                break;
//            case X86_EFLAGS_MODIFY_SF:
//                std::cout << "EFLAGS_MODIFY_SF\n";
//                break;
//            case X86_EFLAGS_MODIFY_ZF:
//                std::cout << "EFLAGS_MODIFY_ZF\n";
//                break;
//            case X86_EFLAGS_MODIFY_PF:
//                std::cout << "EFLAGS_MODIFY_PF\n";
//                break;
//            case X86_EFLAGS_MODIFY_OF:
//                std::cout << "EFLAGS_MODIFY_OF\n";
//                break;
//            case X86_EFLAGS_MODIFY_TF:
//                std::cout << "EFLAGS_MODIFY_TF\n";
//                break;
//            case X86_EFLAGS_MODIFY_IF:
//                std::cout << "EFLAGS_MODIFY_IF\n";
//                break;
//            case X86_EFLAGS_MODIFY_DF:
//                std::cout << "EFLAGS_MODIFY_DF\n";
//                break;
//            case X86_EFLAGS_MODIFY_NT:
//                std::cout << "EFLAGS_MODIFY_NT\n";
//                break;
//            case X86_EFLAGS_MODIFY_RF:
//                std::cout << "EFLAGS_MODIFY_RF\n";
//                break;
//            case X86_EFLAGS_PRIOR_OF:
//                std::cout << "EFLAGS_PRIOR_OF\n";
//                break;
//            case X86_EFLAGS_PRIOR_SF:
//                std::cout << "EFLAGS_PRIOR_SF\n";
//                break;
//            case X86_EFLAGS_PRIOR_ZF:
//                std::cout << "EFLAGS_PRIOR_ZF\n";
//                break;
//            case X86_EFLAGS_PRIOR_AF:
//                std::cout << "EFLAGS_PRIOR_AF\n";
//                break;
//            case X86_EFLAGS_PRIOR_PF:
//                std::cout << "EFLAGS_PRIOR_PF\n";
//                break;
//            case X86_EFLAGS_PRIOR_CF:
//                std::cout << "EFLAGS_PRIOR_CF\n";
//                break;
//            case X86_EFLAGS_PRIOR_TF:
//                std::cout << "EFLAGS_PRIOR_TF\n";
//                break;
//            case X86_EFLAGS_PRIOR_IF:
//                std::cout << "EFLAGS_PRIOR_IF\n";
//                break;
//            case X86_EFLAGS_PRIOR_DF:
//                std::cout << "EFLAGS_PRIOR_DF\n";
//                break;
//            case X86_EFLAGS_PRIOR_NT:
//                std::cout << "EFLAGS_PRIOR_NT\n";
//                break;
//            case X86_EFLAGS_RESET_OF:
//                std::cout << "EFLAGS_RESET_OF\n";
//                break;
//            case X86_EFLAGS_RESET_CF:
//                std::cout << "EFLAGS_RESET_CF\n";
//                break;
//            case X86_EFLAGS_RESET_DF:
//                std::cout << "EFLAGS_RESET_DF\n";
//                break;
//            case X86_EFLAGS_RESET_IF:
//                std::cout << "EFLAGS_RESET_IF\n";
//                break;
//            case X86_EFLAGS_RESET_SF:
//                std::cout << "EFLAGS_RESET_SF\n";
//                break;
//            case X86_EFLAGS_RESET_AF:
//                std::cout << "EFLAGS_RESET_AF\n";
//                break;
//            case X86_EFLAGS_RESET_TF:
//                std::cout << "EFLAGS_RESET_TF\n";
//                break;
//            case X86_EFLAGS_RESET_NT:
//                std::cout << "EFLAGS_RESET_NT\n";
//                break;
//            case X86_EFLAGS_RESET_PF:
//                std::cout << "EFLAGS_RESET_PF\n";
//                break;
//            case X86_EFLAGS_SET_CF:
//                std::cout << "EFLAGS_SET_CF\n";
//                break;
//            case X86_EFLAGS_SET_DF:
//                std::cout << "EFLAGS_SET_DF\n";
//                break;
//            case X86_EFLAGS_SET_IF:
//                std::cout << "EFLAGS_SET_IF\n";
//                break;
//            case X86_EFLAGS_TEST_OF:
//                std::cout << "EFLAGS_TEST_OF\n";
//                break;
//            case X86_EFLAGS_TEST_SF:
//                std::cout << "EFLAGS_TEST_SF\n";
//                break;
//            case X86_EFLAGS_TEST_ZF:
//                std::cout << "EFLAGS_TEST_ZF\n";
//                break;
//            case X86_EFLAGS_TEST_PF:
//                std::cout << "EFLAGS_TEST_PF\n";
//                break;
//            case X86_EFLAGS_TEST_CF:
//                std::cout << "EFLAGS_TEST_CF\n";
//                break;
//            case X86_EFLAGS_TEST_NT:
//                std::cout << "EFLAGS_TEST_NT\n";
//                break;
//            case X86_EFLAGS_TEST_DF:
//                std::cout << "EFLAGS_TEST_DF\n";
//                break;
//            case X86_EFLAGS_UNDEFINED_OF:
//                std::cout << "EFLAGS_UNDEFINED_OF\n";
//                break;
//            case X86_EFLAGS_UNDEFINED_SF:
//                std::cout << "EFLAGS_UNDEFINED_SF\n";
//                break;
//            case X86_EFLAGS_UNDEFINED_ZF:
//                std::cout << "EFLAGS_UNDEFINED_ZF\n";
//                break;
//            case X86_EFLAGS_UNDEFINED_PF:
//                std::cout << "EFLAGS_UNDEFINED_PF\n";
//                break;
//            case X86_EFLAGS_UNDEFINED_AF:
//                std::cout << "EFLAGS_UNDEFINED_AF\n";
//                break;
//            case X86_EFLAGS_UNDEFINED_CF:
//                std::cout << "EFLAGS_UNDEFINED_CF\n";
//                break;
//            default:
//                UNREACHABLE();
//            }
//            ostr << "</td></tr>";
//        }
//    ostr << "</table>";

//    cs_free(insn, count);
//    cs_close(&handle);

//    ostr << "</body>\n";
//    ostr << "</html>\n";

//    return ostr;
//}


//}
