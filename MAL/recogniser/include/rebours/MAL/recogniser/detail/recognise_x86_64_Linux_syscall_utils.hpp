#ifndef REBOURS_MAL_RECOGNISER_DETAIL_RECOGNISE_X86_64_LINUX_SYSCALL_UTILS_HPP_INCLUDED
#   define REBOURS_MAL_RECOGNISER_DETAIL_RECOGNISE_X86_64_LINUX_SYSCALL_UTILS_HPP_INCLUDED

#   include <rebours/MAL/recogniser/recognise.hpp>
#   include <rebours/MAL/descriptor/storage.hpp>
#   include <string>
#   include <vector>
#   include <tuple>
#   include <cstdint>

namespace mal { namespace recogniser { namespace detail { namespace x86_64_Linux { namespace syscall {


using  ret_type = std::tuple<uint8_t,   //!< Error code from {0,1,2,4,8}; 0 indicates no error. 1 indicates an error from REG fn, and 2,4,8 indicate an error from MEM fn
                             uint64_t,  //!< Address passed to a failed REG/MEM fn; Set this to 0 if no error occurred.
                             uint8_t,   //!< Access rights passed to a failed REG/MEM fn; Set this to 0 if no error occurred.
                             bool       //!< It is success/fail, i.e. true/false, state of syscall's proceessing.
                             >;

ret_type  make_succes();
ret_type  make_failure();
ret_type  make_reg_error(uint64_t const  address);
ret_type  make_mem_error(int16_t const  fn_retval, uint64_t const  address, uint8_t const  rights);

inline bool      succeeded(ret_type const&  rt) { return std::get<3>(rt); }
inline bool      failed(ret_type const&  rt) { return !succeeded(rt); }
inline uint8_t   error_code(ret_type const&  rt) { return std::get<0>(rt); }
inline uint64_t  address(ret_type const&  rt) { return std::get<1>(rt); }
inline uint8_t   rights(ret_type const&  rt) { return std::get<2>(rt); }


template<typename T>
T  num(uint8_t const*  data, uint8_t const  size, bool const  are_data_in_big_endian = true)
{
    static_assert(std::is_integral<T>::value,"We support only conversion to integral types.");
    ASSUMPTION(size <= sizeof(T));
    T  value = 0;
    uint8_t*  b = reinterpret_cast<uint8_t*>(&value);
    std::copy(data,data+size,b + (are_data_in_big_endian ? sizeof(T) - size : 0ULL));
    if (are_data_in_big_endian == is_this_little_endian_machine())
        std::reverse(b,b+sizeof(T));
    return *reinterpret_cast<T*>(b);
}

template<typename T>
T  num(std::vector<uint8_t> const&  data, bool const  are_data_in_big_endian = true)
{
    return num<T>(data.data(),(uint8_t)data.size(),are_data_in_big_endian);
}

ret_type  read_register(std::string const&  reg_name, descriptor::storage const&  D, reg_fn_type const&  reg_fn, std::vector<uint8_t>&  buffer);

template<typename T>
T  read_register(std::string const&  reg_name, descriptor::storage const&  D, reg_fn_type const&  reg_fn, ret_type&  ret_state, bool const  are_data_in_big_endian = true)
{
    std::vector<uint8_t>  bytes;
    ret_state = read_register(reg_name,D,reg_fn,bytes);
    INVARIANT(succeeded(ret_state));
    return error_code(ret_state) == 0U ? num<T>(bytes,are_data_in_big_endian) : 0;
}


}}}}}

#endif
