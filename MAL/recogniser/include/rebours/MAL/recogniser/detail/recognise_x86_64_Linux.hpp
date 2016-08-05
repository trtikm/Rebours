#ifndef REBOURS_MAL_RECOGNISER_DETAIL_RECOGNISE_X86_64_LINUX_HPP_INCLUDED
#   define REBOURS_MAL_RECOGNISER_DETAIL_RECOGNISE_X86_64_LINUX_HPP_INCLUDED

#   include <rebours/MAL/recogniser/detail/recognition_data.hpp>
#   include <rebours/MAL/recogniser/detail/recognition_engine_x86_64_Linux.hpp>

namespace mal { namespace recogniser { namespace detail { namespace x86_64_Linux {


struct recognition_data : public detail::recognition_data
{
    recognition_data(uint64_t const  start_address, reg_fn_type const&  reg_fn, mem_fn_type const&  mem_fn);

    void  recognise(descriptor::storage const&  description);
    bool  dump(std::string const&  dump_file_pathname) const;

private:
    microcode::program_component&  component() noexcept { return program()->start_component(); }

    bool  recognise_NOP(recognition_engine const& re, descriptor::storage const&  D);

    bool  recognise_MOV(recognition_engine const& re, descriptor::storage const&  D);
    bool  recognise_MOVSXD(recognition_engine const& re, descriptor::storage const&  D);
    bool  recognise_MOVZX(recognition_engine const& re, descriptor::storage const&  D);
    bool  recognise_MOVDQU(recognition_engine const& re, descriptor::storage const&  D);
    bool  recognise_SETE(recognition_engine const& re, descriptor::storage const&  D);
    bool  recognise_SETNE(recognition_engine const& re, descriptor::storage const&  D);
    bool  recognise_PMOVMSKB(recognition_engine const& re, descriptor::storage const&  D);
    bool  recognise_CMOVE(recognition_engine const& re, descriptor::storage const&  D);
    bool  recognise_CMOVA(recognition_engine const& re, descriptor::storage const&  D);

    bool  recognise_LEA(recognition_engine const& re, descriptor::storage const&  D);

    bool  recognise_TEST(recognition_engine const& re, descriptor::storage const&  D);
    bool  recognise_PCMPEQB(recognition_engine const& re, descriptor::storage const&  D);
    bool  recognise_CMPXCHG(recognition_engine const& re, descriptor::storage const&  D);

    bool  recognise_INC(recognition_engine const& re, descriptor::storage const&  D);
    bool  recognise_DEC(recognition_engine const& re, descriptor::storage const&  D);
    bool  recognise_ADD(recognition_engine const& re, descriptor::storage const&  D);
    bool  recognise_SUB(recognition_engine const& re, descriptor::storage const&  D, bool const  compute_flags_only = false);
    bool  recognise_NEG(recognition_engine const& re, descriptor::storage const&  D);
    bool  recognise_DIV(recognition_engine const& re, descriptor::storage const&  D);

    bool  recognise_AND(recognition_engine const& re, descriptor::storage const&  D);
    bool  recognise_OR(recognition_engine const& re, descriptor::storage const&  D);
    bool  recognise_XOR(recognition_engine const& re, descriptor::storage const&  D);
    bool  recognise_PXOR(recognition_engine const& re, descriptor::storage const&  D);
    bool  recognise_SAR(recognition_engine const& re, descriptor::storage const&  D);
    bool  recognise_SHR(recognition_engine const& re, descriptor::storage const&  D);
    bool  recognise_SHL(recognition_engine const& re, descriptor::storage const&  D);
    bool  recognise_ROL(recognition_engine const& re, descriptor::storage const&  D);
    bool  recognise_BSF(recognition_engine const& re, descriptor::storage const&  D);

    bool  recognise_JMP(recognition_engine const& re, descriptor::storage const&  D);
    bool  recognise_JNE(recognition_engine const& re, descriptor::storage const&  D);
    bool  recognise_JE(recognition_engine const& re, descriptor::storage const&  D);
    bool  recognise_JS(recognition_engine const& re, descriptor::storage const&  D);
    bool  recognise_JNS(recognition_engine const& re, descriptor::storage const&  D);
    bool  recognise_JA(recognition_engine const& re, descriptor::storage const&  D);
    bool  recognise_JAE(recognition_engine const& re, descriptor::storage const&  D);
    bool  recognise_JG(recognition_engine const& re, descriptor::storage const&  D);
    bool  recognise_JLE(recognition_engine const& re, descriptor::storage const&  D);
    bool  recognise_JB(recognition_engine const& re, descriptor::storage const&  D);
    bool  recognise_JBE(recognition_engine const& re, descriptor::storage const&  D);

    bool  recognise_PUSH(recognition_engine const& re, descriptor::storage const&  D);
    bool  recognise_POP(recognition_engine const& re, descriptor::storage const&  D);

    bool  recognise_CALL(recognition_engine const& re, descriptor::storage const&  D);
    bool  recognise_RET(recognition_engine const& re, descriptor::storage const&  D);
    bool  recognise_LEAVE(recognition_engine const& re, descriptor::storage const&  D);

    bool  recognise_INT(recognition_engine const& re, descriptor::storage const&  D);
    bool  recognise_SYSCALL(recognition_engine const& re, descriptor::storage const&  D);
};


using  recognition_data_ptr = std::shared_ptr<recognition_data>;


}}}}

#endif
