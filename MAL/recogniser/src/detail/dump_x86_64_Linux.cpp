#include <rebours/MAL/recogniser/detail/recognise_x86_64_Linux.hpp>
#include <rebours/MAL/recogniser/assumptions.hpp>
#include <rebours/MAL/recogniser/invariants.hpp>
#include <rebours/MAL/recogniser/msgstream.hpp>
#include <rebours/MAL/recogniser/development.hpp>
#include <rebours/MAL/recogniser/file_utils.hpp>
#include <capstone/capstone.h>
#include <capstone/x86.h>
#include <fstream>
#include <iostream>
#include <iomanip>

namespace mal { namespace recogniser { namespace detail { namespace x86_64_Linux {


bool  recognition_data::dump(std::string const&  dump_file_pathname) const
{
    ASSUMPTION(!dump_file_pathname.empty());
    ASSUMPTION(!buffer().empty());

    fileutl::create_directory(fileutl::parse_path_in_pathname(dump_file_pathname));

    std::ofstream  ostr{dump_file_pathname,std::ofstream::binary};

    ostr << "<!DOCTYPE html>\n";
    ostr << "<html>\n";
    ostr << "<head>\n";
    ostr << "<style>\n";
    ostr << "table, th, td {\n";
    ostr << "    border: 1px solid black;\n";
    ostr << "    border-collapse: collapse;\n";
    ostr << "}\n";
    ostr << "th, td {\n";
    ostr << "    padding: 5px;\n";
    ostr << "}\n";
    ostr << "h1, h2, h3, h4, p, a, table, ul { font-family: \"Liberation serif\", serif; }\n";
    ostr << "p, a, table, ul { font-size: 12pt; }\n";
    ostr << "h4 { font-size: 12pt; }\n";
    ostr << "h3 { font-size: 14pt; }\n";
    ostr << "h2 { font-size: 18pt; }\n";
    ostr << "h1 { font-size: 24pt; }\n";
    ostr << "tt { font-family: \"Liberation Mono\", monospace; }\n";
    ostr << "tt { font-size: 10pt; }\n";
    ostr << "body {\n";
    ostr << "    background-color: white;\n";
    ostr << "    color: black;\n";
    ostr << "}\n";
    ostr << "</style>\n";
    ostr << "</head>\n";
    ostr << "<body>\n";

    csh handle;
    if (cs_open(CS_ARCH_X86, CS_MODE_64, &handle) != CS_ERR_OK)
    {
        ostr << "<p>"
                "ERROR: cannot open the capstone-next library ('cs_open' has FAILED)."
                "</p>\n</body>\n</html>\n";
        return ostr;
    }

    cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);

    cs_insn*  insn;
    size_t const  count = cs_disasm(handle,buffer().data(),buffer().size(),0ULL,1ULL,&insn);
    if (count != 1ULL)
    {
        ostr << "<p>"
                "ERROR: Cannot diassemly the instruction ('cs_disasm' has FAILED)."
                "</p>\n</body>\n</html>\n";
        return ostr;
    }

    cs_insn const* instr = &(insn[0ULL]);

    cs_detail *detail = instr->detail;
    if (detail == nullptr)
    {
        ostr << "<p>"
                "ERROR: Cannot access details of the disassembled instruction."
                "</p>\n</body>\n</html>\n";
        return ostr;
    }

    cs_x86 const* const  x86 = &(detail->x86);

    ostr << "<h2>Disassembly info of hexadecimal bytes '";
    for (uint16_t  i = 0ULL; i < instr->size; ++i)
        ostr << std::hex << std::setw(2) << std::setfill('0') << (uint32_t)instr->bytes[i] << (i + 1ULL == instr->size ? "" : ",");
    ostr << "' aka '" << instr->mnemonic << " " << instr->op_str << "'</h2>\n";
    ostr << "<p>All numbers in all tables bellow are hexadecimal.\n</p>\n"
            "<table>\n"
            "  <caption>General properties of the instruction.</caption>\n"
            "  <tr>\n"
            "    <th>Property</th>\n"
            "    <th>Value</th>\n"
            "  </tr>\n"
            "  <tr>\n"
            "    <td>The processor architecture</td>\n"
            "    <td>X86_64</td>\n"
            "  </tr>\n"
            "  <tr>\n"
            "    <td>The number of bytes of the instruction.</td>\n"
            "    <td>" << instr->size << "</td>\n"
            "  </tr>\n"
            "  <tr>\n"
            "    <td>Opcode ID</td>\n"
            "    <td>" << std::hex << instr->id << "</td>\n"
            "  </tr>\n"
            "  <tr>\n"
            "    <td>Prefix</td>\n"
            "    <td>" << std::hex
                       << (uint32_t)x86->prefix[0] << " "
                       << (uint32_t)x86->prefix[1] << " "
                       << (uint32_t)x86->prefix[2] << " "
                       << (uint32_t)x86->prefix[3]
                       << "</td>\n"
            "  </tr>\n"
            "  <tr>\n"
            "    <td>Opcode</td>\n"
            "    <td>" << std::hex
                       << (uint32_t)x86->opcode[0] << " "
                       << (uint32_t)x86->opcode[1] << " "
                       << (uint32_t)x86->opcode[2] << " "
                       << (uint32_t)x86->opcode[3]
                       << "</td>\n"
            "  </tr>\n"
            "  <tr>\n"
            "    <td>The count of operands.</td>\n"
            "    <td>" << std::hex << (uint32_t)x86->op_count << "</td>\n"
            "  </tr>\n"
            "  <tr>\n"
            "    <td>Rex</td>\n"
            "    <td>" << std::hex << (uint32_t)x86->rex << "</td>\n"
            "  </tr>\n"
            "  <tr>\n"
            "    <td>Address size</td>\n"
            "    <td>" << std::hex << (uint32_t)x86->addr_size << "</td>\n"
            "  </tr>\n"
            "  <tr>\n"
            "    <td>Modrm</td>\n"
            "    <td>" << std::hex << (uint32_t)x86->modrm << "</td>\n"
            "  </tr>\n"
            "  <tr>\n"
            "    <td>Sib</td>\n"
            "    <td>" << std::hex << (uint32_t)x86->sib << "</td>\n"
            "  </tr>\n"
            "  <tr>\n"
            "    <td>Displacement</td>\n"
            "    <td>" << std::hex << x86->disp << "</td>\n"
            "  </tr>\n"
            "  <tr>\n"
            "    <td>Xop_cc</td>\n"
            "    <td>" << std::hex << x86->xop_cc << "</td>\n"
            "  </tr>\n"
            "  <tr>\n"
            "    <td>Sse_cc</td>\n"
            "    <td>" << std::hex << x86->sse_cc << "</td>\n"
            "  </tr>\n"
            "  <tr>\n"
            "    <td>Avx_cc</td>\n"
            "    <td>" << std::hex << x86->avx_cc << "</td>\n"
            "  </tr>\n"
            "  <tr>\n"
            "    <td>Avx_sae</td>\n"
            "    <td>" << std::hex << x86->avx_sae << "</td>\n"
            "  </tr>\n"
            "  <tr>\n"
            "    <td>Avx_rm</td>\n"
            "    <td>" << std::hex << x86->avx_rm << "</td>\n"
            "  </tr>\n"
            "  <tr>\n"
            "    <td>Eflags</td>\n"
            "    <td>" << std::hex << x86->eflags << "</td>\n"
            "  </tr>\n"
            "</table>\n"
            ;

    ostr << "<p></p>\n";

    if ((uint32_t)x86->op_count > 0U)
    {
        ostr << "<table>\n"
                "  <caption>Properties of individual operands. A cell is empty if it has no meaning for an operand.</caption>\n"
                "  <tr>\n"
                "    <th>#</th>\n"
                "    <th>Num bytes</th>\n"
                "    <th>Is input</th>\n"
                "    <th>Is output</th>\n"
                "    <th>Type</th>\n"
                "    <th>Name</th>\n"
                "    <th>Value</th>\n"
                "    <th>Segment REG</th>\n"
                "    <th>Base REG</th>\n"
                "    <th>Index REG</th>\n"
                "    <th>Scale IMM</th>\n"
                "    <th>Displacement IMM</th>\n"
                "  </tr>\n"
                ;

        for (uint8_t  i = 0U; i < x86->op_count; ++i)
        {
            cs_x86_op const* const  op = &(x86->operands[i]);

            ostr << "  <tr>\n"
                    "    <td>" << std::hex << (uint32_t)i << "</td>\n"
                    "    <td>" << std::hex << (uint32_t)op->size << "</td>\n";
                    ;

            switch (op->access)
            {
            case CS_AC_READ:
                ostr << "<td>true</td>\n"
                        "<td>false</td>\n"
                        ;
                break;
            case CS_AC_WRITE:
                ostr << "<td>false</td>\n"
                        "<td>true</td>\n"
                        ;
                break;
            case CS_AC_READ | CS_AC_WRITE:
                ostr << "<td>true</td>\n"
                        "<td>true</td>\n"
                        ;
                break;
            default:
                if (op->type == X86_OP_IMM)
                    ostr << "<td>true</td>\n"
                            "<td>false</td>\n"
                            ;
                else
                    ostr << "<td>false</td>\n"
                            "<td>false</td>\n"
                            ;
                break;
            }

            switch(op->type)
            {
            case X86_OP_REG:
                ostr << "<td>REG</td>\n"
                        "<td>"<< cs_reg_name(handle,op->reg) << "</td>\n"
                        "<td></td>\n"
                        "<td></td>\n"
                        "<td></td>\n"
                        "<td></td>\n"
                        "<td></td>\n"
                        "<td></td>\n"
                        ;
                break;
            case X86_OP_IMM:
                ostr << "<td>IMM</td>\n"
                        "<td></td>\n"
                        "<td>"<< std::hex << op->imm << "</td>\n"
                        "<td></td>\n"
                        "<td></td>\n"
                        "<td></td>\n"
                        "<td></td>\n"
                        "<td></td>\n"
                        ;
                break;
            case X86_OP_MEM:
                {
                    x86_op_mem const* const  mem = &(op->mem);
                    ostr << "<td>MEM</td>\n"
                            "<td></td>\n"
                            "<td></td>\n"
                            "<td>"<< (mem->segment == X86_REG_INVALID ? "" : cs_reg_name(handle,mem->segment)) << "</td>\n"
                            "<td>"<< (mem->base == X86_REG_INVALID ? "" : cs_reg_name(handle,mem->base)) << "</td>\n"
                            "<td>"<< (mem->index == X86_REG_INVALID ? "" : cs_reg_name(handle,mem->index)) << "</td>\n"
                            "<td>"<< std::hex << mem->scale << "</td>\n"
                            "<td>"<< std::hex << mem->disp << "</td>\n"
                            ;
                }
                break;
            case X86_OP_FP:
                ostr << "<td>FP</td>\n"
                        "<td></td>\n"
                        "<td>"<< op->fp << "</td>\n";
                break;
            default:
                ostr << "<td>INVALID</td>\n";
                break;
            }

            ostr << "  </tr>\n";
        }
        ostr << "</table>\n";
    }

    ostr << "<p></p>\n";

    if (x86->eflags != 0ULL)
    {
        ostr << "<table>\n"
                "  <caption>Actions performed on flags.</caption>\n"
                "  <tr>\n"
                "    <th>Flag name</th>\n"
                "    <th>Action name</th>\n"
                "  </tr>\n"
                ;

        for (uint8_t i = 0U; i < 46U; ++i)
            if ((x86->eflags & (1ULL << i)) != 0ULL)
            {
                ostr << "  <tr>";
                switch (1ULL << i)
                {
                case X86_EFLAGS_MODIFY_AF:
                    ostr << "    <td>af</td>\n"
                            "    <td>MODIFY</td>\n"
                            ;
                    break;
                case X86_EFLAGS_MODIFY_CF:
                    ostr << "    <td>cf</td>\n"
                            "    <td>MODIFY</td>\n"
                            ;
                    break;
                case X86_EFLAGS_MODIFY_SF:
                    ostr << "    <td>sf</td>\n"
                            "    <td>MODIFY</td>\n"
                            ;
                    break;
                case X86_EFLAGS_MODIFY_ZF:
                    ostr << "    <td>zf</td>\n"
                            "    <td>MODIFY</td>\n"
                            ;
                    break;
                case X86_EFLAGS_MODIFY_PF:
                    ostr << "    <td>pf</td>\n"
                            "    <td>MODIFY</td>\n"
                            ;
                    break;
                case X86_EFLAGS_MODIFY_OF:
                    ostr << "    <td>of</td>\n"
                            "    <td>MODIFY</td>\n"
                            ;
                    break;
                case X86_EFLAGS_MODIFY_TF:
                    ostr << "    <td>tf</td>\n"
                            "    <td>MODIFY</td>\n"
                            ;
                    break;
                case X86_EFLAGS_MODIFY_IF:
                    ostr << "    <td>if</td>\n"
                            "    <td>MODIFY</td>\n"
                            ;
                    break;
                case X86_EFLAGS_MODIFY_DF:
                    ostr << "    <td>df</td>\n"
                            "    <td>MODIFY</td>\n"
                            ;
                    break;
                case X86_EFLAGS_MODIFY_NT:
                    ostr << "    <td>nt</td>\n"
                            "    <td>MODIFY</td>\n"
                            ;
                    break;
                case X86_EFLAGS_MODIFY_RF:
                    ostr << "    <td>rf</td>\n"
                            "    <td>MODIFY</td>\n"
                            ;
                    break;
                case X86_EFLAGS_PRIOR_OF:
                    ostr << "    <td>of</td>\n"
                            "    <td>PRIOR</td>\n"
                            ;
                    break;
                case X86_EFLAGS_PRIOR_SF:
                    ostr << "    <td>sf</td>\n"
                            "    <td>PRIOR</td>\n"
                            ;
                    break;
                case X86_EFLAGS_PRIOR_ZF:
                    ostr << "    <td>zf</td>\n"
                            "    <td>PRIOR</td>\n"
                            ;
                    break;
                case X86_EFLAGS_PRIOR_AF:
                    ostr << "    <td>af</td>\n"
                            "    <td>PRIOR</td>\n"
                            ;
                    break;
                case X86_EFLAGS_PRIOR_PF:
                    ostr << "    <td>pf</td>\n"
                            "    <td>PRIOR</td>\n"
                            ;
                    break;
                case X86_EFLAGS_PRIOR_CF:
                    ostr << "    <td>cf</td>\n"
                            "    <td>PRIOR</td>\n"
                            ;
                    break;
                case X86_EFLAGS_PRIOR_TF:
                    ostr << "    <td>tf</td>\n"
                            "    <td>PRIOR</td>\n"
                            ;
                    break;
                case X86_EFLAGS_PRIOR_IF:
                    ostr << "    <td>if</td>\n"
                            "    <td>PRIOR</td>\n"
                            ;
                    ostr << "";
                    break;
                case X86_EFLAGS_PRIOR_DF:
                    ostr << "    <td>df</td>\n"
                            "    <td>PRIOR</td>\n"
                            ;
                    break;
                case X86_EFLAGS_PRIOR_NT:
                    ostr << "    <td>nt</td>\n"
                            "    <td>PRIOR</td>\n"
                            ;
                    break;
                case X86_EFLAGS_RESET_OF:
                    ostr << "    <td>of</td>\n"
                            "    <td>RESET</td>\n"
                            ;
                    break;
                case X86_EFLAGS_RESET_CF:
                    ostr << "    <td>cf</td>\n"
                            "    <td>RESET</td>\n"
                            ;
                    break;
                case X86_EFLAGS_RESET_DF:
                    ostr << "    <td>df</td>\n"
                            "    <td>RESET</td>\n"
                            ;
                    break;
                case X86_EFLAGS_RESET_IF:
                    ostr << "    <td>if</td>\n"
                            "    <td>RESET</td>\n"
                            ;
                    break;
                case X86_EFLAGS_RESET_SF:
                    ostr << "    <td>sf</td>\n"
                            "    <td>RESET</td>\n"
                            ;
                    break;
                case X86_EFLAGS_RESET_AF:
                    ostr << "    <td>af</td>\n"
                            "    <td>RESET</td>\n"
                            ;
                    break;
                case X86_EFLAGS_RESET_TF:
                    ostr << "    <td>tf</td>\n"
                            "    <td>RESET</td>\n"
                            ;
                    break;
                case X86_EFLAGS_RESET_NT:
                    ostr << "    <td>nt</td>\n"
                            "    <td>RESET</td>\n"
                            ;
                    break;
                case X86_EFLAGS_RESET_PF:
                    ostr << "    <td>pf</td>\n"
                            "    <td>RESET</td>\n"
                            ;
                    break;
                case X86_EFLAGS_SET_CF:
                    ostr << "    <td>cf</td>\n"
                            "    <td>SET</td>\n"
                            ;
                    break;
                case X86_EFLAGS_SET_DF:
                    ostr << "    <td>df</td>\n"
                            "    <td>SET</td>\n"
                            ;
                    break;
                case X86_EFLAGS_SET_IF:
                    ostr << "    <td>if</td>\n"
                            "    <td>SET</td>\n"
                            ;
                    break;
                case X86_EFLAGS_TEST_OF:
                    ostr << "    <td>of</td>\n"
                            "    <td>TEST</td>\n"
                            ;
                    break;
                case X86_EFLAGS_TEST_SF:
                    ostr << "    <td>sf</td>\n"
                            "    <td>TEST</td>\n"
                            ;
                    break;
                case X86_EFLAGS_TEST_ZF:
                    ostr << "    <td>zf</td>\n"
                            "    <td>TEST</td>\n"
                            ;
                    break;
                case X86_EFLAGS_TEST_PF:
                    ostr << "    <td>pf</td>\n"
                            "    <td>TEST</td>\n"
                            ;
                    break;
                case X86_EFLAGS_TEST_CF:
                    ostr << "    <td>cf</td>\n"
                            "    <td>TEST</td>\n"
                            ;
                    break;
                case X86_EFLAGS_TEST_NT:
                    ostr << "    <td>nt</td>\n"
                            "    <td>TEST</td>\n"
                            ;
                    break;
                case X86_EFLAGS_TEST_DF:
                    ostr << "    <td>df</td>\n"
                            "    <td>TEST</td>\n"
                            ;
                    break;
                case X86_EFLAGS_UNDEFINED_OF:
                    ostr << "    <td>of</td>\n"
                            "    <td>UNDEFINED</td>\n"
                            ;
                    break;
                case X86_EFLAGS_UNDEFINED_SF:
                    ostr << "    <td>sf</td>\n"
                            "    <td>UNDEFINED</td>\n"
                            ;
                    break;
                case X86_EFLAGS_UNDEFINED_ZF:
                    ostr << "    <td>zf</td>\n"
                            "    <td>UNDEFINED</td>\n"
                            ;
                    break;
                case X86_EFLAGS_UNDEFINED_PF:
                    ostr << "    <td>pf</td>\n"
                            "    <td>UNDEFINED</td>\n"
                            ;
                    break;
                case X86_EFLAGS_UNDEFINED_AF:
                    ostr << "    <td>af</td>\n"
                            "    <td>UNDEFINED</td>\n"
                            ;
                    break;
                case X86_EFLAGS_UNDEFINED_CF:
                    ostr << "    <td>cf</td>\n"
                            "    <td>UNDEFINED</td>\n"
                            ;
                    break;
                default:
                    UNREACHABLE();
                }
                ostr << "</tr>";
            }
        ostr << "</table>";
    }

    cs_free(insn, count);
    cs_close(&handle);

    ostr << "</body>\n";
    ostr << "</html>\n";

    return true;
}


}}}}
