#ifndef REBOURS_MAL_RECOGNISER_DETAIL_REGISTER_INFO_HPP_INCLUDED
#   define REBOURS_MAL_RECOGNISER_DETAIL_REGISTER_INFO_HPP_INCLUDED

#   include <string>
#   include <utility>
#   include <cstdint>

namespace mal { namespace descriptor {
struct storage;
}}

namespace mal { namespace recogniser { namespace detail {


using  register_info = std::pair<uint64_t,  //!< Start address in the REG pool
                                 uint8_t    //!< Number of bytes of the register
                                 >;

register_info  get_register_info(std::string const&  reg_name, mal::descriptor::storage const&  D);


}}}

#endif
