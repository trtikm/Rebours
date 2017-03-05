#ifndef IOBUFFER_HPP
#   define IOBUFFER_HPP

#   include <cstdint>
#   include <vector>

/**
 * These utility functions allow for conversion of raw sequence of bytes in the memory into a numeric constant.
 * Each function converts the memory into a differnt type of number.
 */

void  address_to_buffer(uint64_t const  value, std::vector<uint8_t>&  buffer, bool const  use_big_endian_buffer);
void  uint16_t_to_buffer(uint16_t const  value, std::vector<uint8_t>&  buffer, bool const  use_big_endian_buffer);
void  uint32_t_to_buffer(uint32_t const  value, std::vector<uint8_t>&  buffer, bool const  use_big_endian_buffer);
void  uint64_t_to_buffer(uint64_t const  value, std::vector<uint8_t>&  buffer, bool const  use_big_endian_buffer);


uint16_t  buffer_to_uint16_t(uint8_t const* const  buffer_begin, uint8_t const* const  buffer_end, bool is_it_big_endian_buffer);
int16_t  buffer_to_int16_t(uint8_t const* const  buffer_begin, uint8_t const* const  buffer_end, bool is_it_big_endian_buffer);
uint32_t  buffer_to_uint32_t(uint8_t const* const  buffer_begin, uint8_t const* const  buffer_end, bool is_it_big_endian_buffer);
int32_t  buffer_to_int32_t(uint8_t const* const  buffer_begin, uint8_t const* const  buffer_end, bool is_it_big_endian_buffer);
uint64_t  buffer_to_uint64_t(uint8_t const* const  buffer_begin, uint8_t const* const  buffer_end, bool is_it_big_endian_buffer);
int64_t  buffer_to_int64_t(uint8_t const* const  buffer_begin, uint8_t const* const  buffer_end, bool is_it_big_endian_buffer);


inline uint16_t  buffer_to_uint16_t(char const* const  buffer_begin, char const* const  buffer_end, bool is_it_big_endian_buffer)
{ return buffer_to_uint16_t((uint8_t const*)buffer_begin, (uint8_t const*)buffer_end, is_it_big_endian_buffer); }

inline int16_t  buffer_to_int16_t(char const* const  buffer_begin, char const* const  buffer_end, bool is_it_big_endian_buffer)
{ return buffer_to_int16_t((uint8_t const*)buffer_begin, (uint8_t const*)buffer_end, is_it_big_endian_buffer); }

inline uint32_t  buffer_to_uint32_t(char const* const  buffer_begin, char const* const  buffer_end, bool is_it_big_endian_buffer)
{ return buffer_to_uint32_t((uint8_t const*)buffer_begin, (uint8_t const*)buffer_end, is_it_big_endian_buffer); }

inline int32_t  buffer_to_int32_t(char const* const  buffer_begin, char const* const  buffer_end, bool is_it_big_endian_buffer)
{ return buffer_to_int32_t((uint8_t const*)buffer_begin, (uint8_t const*)buffer_end, is_it_big_endian_buffer); }

inline uint64_t  buffer_to_uint64_t(char const* const  buffer_begin, char const* const  buffer_end, bool is_it_big_endian_buffer)
{ return buffer_to_uint64_t((uint8_t const*)buffer_begin, (uint8_t const*)buffer_end, is_it_big_endian_buffer); }

inline int64_t  buffer_to_int64_t(char const* const  buffer_begin, char const* const  buffer_end, bool is_it_big_endian_buffer)
{ return buffer_to_int64_t((uint8_t const*)buffer_begin, (uint8_t const*)buffer_end, is_it_big_endian_buffer); }

#endif
