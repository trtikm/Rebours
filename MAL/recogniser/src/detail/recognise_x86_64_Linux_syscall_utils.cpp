#include <rebours/MAL/recogniser/detail/recognise_x86_64_Linux_syscall_utils.hpp>
#include <rebours/MAL/recogniser/detail/register_info.hpp>
#include <rebours/MAL/recogniser/assumptions.hpp>
#include <rebours/MAL/recogniser/invariants.hpp>

namespace mal { namespace recogniser { namespace detail { namespace x86_64_Linux { namespace syscall {


ret_type  make_succes()
{
    return ret_type{0U,0ULL,0U,true};
}

ret_type  make_failure()
{
    return ret_type{0U,0ULL,0U,false};
}

ret_type  make_reg_error(uint64_t const  address)
{
    ASSUMPTION(address != 0ULL);
    return ret_type{1U,address,0U,true};
}

ret_type  make_mem_error(int16_t const  fn_retval, uint64_t const  address, uint8_t const  rights)
{
    ASSUMPTION(fn_retval == -1 || fn_retval == -2 || fn_retval == -3);
    ASSUMPTION(rights == 1U || rights == 2U);
    return ret_type{1U << (-fn_retval),address,rights,true};
}


ret_type  read_register(std::string const&  reg_name, descriptor::storage const&  D, reg_fn_type const&  reg_fn, std::vector<uint8_t>&  buffer)
{
    register_info const  reg_info = get_register_info(reg_name,D);
    ASSUMPTION(reg_info.first != 0ULL);
    for (uint64_t  adr = reg_info.first;  adr != reg_info.first + reg_info.second; ++adr)
    {
        int16_t const  result = reg_fn(adr);
        ASSUMPTION(result < 256 && result > -2);
        if (result < 0)
            return make_reg_error(adr);
        buffer.push_back((uint8_t)result);
    }
    return make_succes();
}


}}}}}
