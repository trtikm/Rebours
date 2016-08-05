#ifndef REBOURS_MAL_RECOGNISER_DETAIL_RECOGNISE_X86_64_LINUX_SYSCALL_SYSCALL_HPP_INCLUDED
#   define REBOURS_MAL_RECOGNISER_DETAIL_RECOGNISE_X86_64_LINUX_SYSCALL_SYSCALL_HPP_INCLUDED

#   include <rebours/MAL/recogniser/detail/recognise_x86_64_Linux_syscall_utils.hpp>
#   include <rebours/program/program.hpp>

namespace mal { namespace recogniser { namespace detail { namespace x86_64_Linux { namespace syscall {


ret_type  recognise_syscall_1_write(reg_fn_type const&  reg_fn, mem_fn_type const&  , descriptor::storage const&  D,
                                    uint8_t const  num_bytes_of_instruction, microcode::program_component&  C);
ret_type  recognise_syscall_5_fstat(reg_fn_type const&  reg_fn, mem_fn_type const&  , descriptor::storage const&  D,
                                    uint8_t const  num_bytes_of_instruction, microcode::program_component&  C);
ret_type  recognise_syscall_9_mmap(reg_fn_type const&  reg_fn, mem_fn_type const&  , descriptor::storage const&  D,
                                   uint8_t const  num_bytes_of_instruction, microcode::program_component&  C);
ret_type  recognise_syscall_231_exit_group(reg_fn_type const&  , mem_fn_type const&  , descriptor::storage const&  ,
                                           uint8_t const  , microcode::program_component&  C);


}}}}}

#endif
