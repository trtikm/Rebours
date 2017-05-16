#include <rebours/utility/buffer_io.hpp>
#include <rebours/utility/endian.hpp>
#include <rebours/utility/assumptions.hpp>
#include <rebours/utility/invariants.hpp>
#include <rebours/utility/config.hpp>
#include <algorithm>

namespace {


template<typename number_type>
number_type  buffer_to_number(uint8_t const* const  buffer_begin,
                              uint8_t const* const  buffer_end,
                              bool is_it_big_endian_buffer)
{
    ASSUMPTION(buffer_begin != nullptr);
    ASSUMPTION(buffer_end != nullptr);
    ASSUMPTION(buffer_end - buffer_begin > 0);
    ASSUMPTION((uint8_t) (buffer_end - buffer_begin) <= sizeof(number_type));

    uint8_t const  num_buffer_bytes = (uint8_t)(buffer_end - buffer_begin);

    INVARIANT(num_buffer_bytes > 0U);

    number_type  result{0};
    uint8_t* const  result_begin = reinterpret_cast<uint8_t*>(&result);
    uint8_t const  result_size = sizeof(number_type);

    INVARIANT(num_buffer_bytes <= result_size);

    uint8_t const result_shift =
            is_this_little_endian_machine() ? 0U : result_size - num_buffer_bytes;

    std::copy(buffer_begin, buffer_end, result_begin + result_shift);
    if (is_this_little_endian_machine() == is_it_big_endian_buffer)
        std::reverse(result_begin + result_shift, result_begin + result_shift + num_buffer_bytes);

    return result;
}


template<typename number_type>
void  number_to_buffer(number_type const  value, std::vector<uint8_t>&  buffer, bool const  use_big_endian_buffer)
{
    ASSUMPTION(buffer.empty());
    for (uint8_t const* ptr = (uint8_t const*)&value,
                * const end = ptr + sizeof(number_type);
            ptr < end;
            ++ptr)
        buffer.push_back(*ptr);
    if (is_this_little_endian_machine() == use_big_endian_buffer)
        std::reverse(buffer.begin(),buffer.end());
}

}

uint16_t  buffer_to_uint16_t(uint8_t const* const  buffer_begin,
                             uint8_t const* const  buffer_end,
                             bool is_it_big_endian_buffer)
{
    return buffer_to_number<uint16_t>(buffer_begin,buffer_end,is_it_big_endian_buffer);
}

int16_t  buffer_to_int16_t(uint8_t const* const  buffer_begin,
                           uint8_t const* const  buffer_end,
                           bool is_it_big_endian_buffer)
{
    return buffer_to_number<int16_t>(buffer_begin,buffer_end,is_it_big_endian_buffer);
}

uint32_t  buffer_to_uint32_t(uint8_t const* const  buffer_begin,
                             uint8_t const* const  buffer_end,
                             bool is_it_big_endian_buffer)
{
    return buffer_to_number<uint32_t>(buffer_begin,buffer_end,is_it_big_endian_buffer);
}


int32_t  buffer_to_int32_t(uint8_t const* const  buffer_begin,
                           uint8_t const* const  buffer_end,
                           bool is_it_big_endian_buffer)
{
    return buffer_to_number<int32_t>(buffer_begin,buffer_end,is_it_big_endian_buffer);
}


uint64_t  buffer_to_uint64_t(uint8_t const* const  buffer_begin,
                             uint8_t const* const  buffer_end,
                             bool is_it_big_endian_buffer)
{
    return buffer_to_number<uint64_t>(buffer_begin,buffer_end,is_it_big_endian_buffer);
}


int64_t  buffer_to_int64_t(uint8_t const* const  buffer_begin,
                           uint8_t const* const  buffer_end,
                           bool is_it_big_endian_buffer)
{
    return buffer_to_number<int64_t>(buffer_begin,buffer_end,is_it_big_endian_buffer);
}

void  address_to_buffer(uint64_t const  value, std::vector<uint8_t>&  buffer, bool const  use_big_endian_buffer)
{
    number_to_buffer<uint64_t>(value,buffer,use_big_endian_buffer);
}

void  uint16_t_to_buffer(uint16_t const  value, std::vector<uint8_t>&  buffer, bool const  use_big_endian_buffer)
{
    number_to_buffer<uint16_t>(value,buffer,use_big_endian_buffer);
}

void  uint32_t_to_buffer(uint32_t const  value, std::vector<uint8_t>&  buffer, bool const  use_big_endian_buffer)
{
    number_to_buffer<uint32_t>(value,buffer,use_big_endian_buffer);
}

void  uint64_t_to_buffer(uint64_t const  value, std::vector<uint8_t>&  buffer, bool const  use_big_endian_buffer)
{
    number_to_buffer<uint64_t>(value,buffer,use_big_endian_buffer);
}
