#include <rebours/MAL/recogniser/detail/recognition_data.hpp>
#include <rebours/MAL/recogniser/msgstream.hpp>
#include <rebours/MAL/recogniser/assumptions.hpp>
#include <rebours/MAL/recogniser/invariants.hpp>
#include <rebours/MAL/descriptor/storage.hpp>

namespace mal { namespace recogniser { namespace detail {


recognition_data::recognition_data(uint64_t const  start_address, reg_fn_type const&  reg_fn, mem_fn_type const&  mem_fn)
    : m_start_address(start_address)
    , m_reg_fn(reg_fn)
    , m_mem_fn(mem_fn)
    , m_program(microcode::create_initial_program("Rebours_recognition_result",
                                                  msgstream() << "Microcode representation of a native instruction stored at the address "
                                                              << start_address
                                                              << ".\nThis program was built by Rebours' MAL/recogniser."))
    , m_asm_text()
    , m_asm_bytes()
    , m_error_result(0U)
    , m_error_address(0ULL)
    , m_error_rights(0U)
    , m_buffer()
{}

void  recognition_data::set_error_result(uint8_t const  value) noexcept
{
    ASSUMPTION(value == 1U || value == 2U || value == 4U || value == 254U || value == 255U);
    m_error_result = value;
}

void  recognition_data::set_error_rights(uint8_t const  value) noexcept
{
    ASSUMPTION(value == 1U || value == 2U);
    m_error_rights = value;
}


}}}
