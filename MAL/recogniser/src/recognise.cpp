#include <rebours/MAL/recogniser/recognise.hpp>
#include <rebours/utility/msgstream.hpp>
#include <rebours/utility/assumptions.hpp>
#include <rebours/utility/invariants.hpp>
#include <rebours/MAL/recogniser/detail/recognition_data.hpp>
#include <rebours/MAL/descriptor/storage.hpp>

namespace mal { namespace recogniser { namespace detail {


std::shared_ptr<detail::recognition_data const>  get_implementation_details(recognition_result const&  result) { return result.m_data; }


namespace x86_64_Linux {
recognition_data_ptr  recognise(descriptor::storage const&  description, uint64_t const  start_address, reg_fn_type const&  reg_fn, mem_fn_type const&  mem_fn);
}


}}}

namespace mal { namespace recogniser {


recognition_result::recognition_result(std::shared_ptr<detail::recognition_data> const  data)
    : m_data(data)
{}

std::shared_ptr<microcode::program>  recognition_result::program() const noexcept
{
    return result() != 0U || (m_data->program()->num_components() == 1ULL && m_data->program()->component(0ULL).edges().empty()) ?
                std::shared_ptr<microcode::program>() : m_data->program();
}

std::string const&  recognition_result::asm_text() const noexcept
{
    return m_data->asm_text();
}

std::string const&  recognition_result::asm_bytes() const noexcept
{
    return m_data->asm_bytes();
}

uint8_t  recognition_result::result() const noexcept
{
    return m_data->error_result();
}

uint64_t  recognition_result::address() const noexcept
{
    return m_data->error_address();
}

uint8_t  recognition_result::rights() const noexcept
{
    return m_data->error_rights();
}

std::vector<uint8_t> const&  recognition_result::buffer() const noexcept
{
    return m_data->buffer();
}


recognition_result  recognise(descriptor::storage const&  description,
                              uint64_t const  start_address,
                              reg_fn_type const&  reg_fn,
                              mem_fn_type const&  mem_fn)
{
    if (description.processor() == descriptor::processor_architecture::X86_64 &&
        (description.system() == descriptor::operating_system::LINUX ||
         description.system() == descriptor::operating_system::UNIX))
        return recognition_result( detail::x86_64_Linux::recognise(description,start_address,reg_fn,mem_fn) );

    UNREACHABLE();
}


}}
