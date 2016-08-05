#include <rebours/MAL/recogniser/detail/register_info.hpp>
#include <rebours/MAL/recogniser/msgstream.hpp>
#include <rebours/MAL/recogniser/assumptions.hpp>
#include <rebours/MAL/recogniser/invariants.hpp>
#include <rebours/MAL/recogniser/development.hpp>
#include <rebours/MAL/descriptor/storage.hpp>
#include <string>

namespace mal { namespace recogniser { namespace detail {


register_info  get_register_info(std::string const&  reg_name, mal::descriptor::storage const&  D)
{
    ASSUMPTION(D.registers_to_ranges().count(reg_name) != 0ULL);
    mal::descriptor::address_range const  rng = D.registers_to_ranges().at(reg_name);
    ASSUMPTION(rng.second > rng.first && rng.second - rng.first < 256);
    return {rng.first,rng.second - rng.first};
}


}}}
