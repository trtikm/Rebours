#include <rebours/analysis/native_execution/execute_instruction.hpp>
#include <rebours/program/assembly.hpp>
#include <rebours/analysis/native_execution/assumptions.hpp>
#include <rebours/analysis/native_execution/invariants.hpp>
#include <rebours/analysis/native_execution/development.hpp>
#include <rebours/analysis/native_execution/msgstream.hpp>
#include <type_traits>
#include <sstream>
#include <limits>

namespace analysis { namespace natexe { namespace {


void  in_reg(std::vector<io_relation_location>&  reads, address const  shift_from_begin, uint8_t const  n)
{
    for (uint8_t  i = 0U; i < n; ++i)
        reads.push_back({ true, shift_from_begin + i });
}

void  in_mem(std::vector<io_relation_location>&  reads, address const  shift_from_begin, uint8_t const  n)
{
    for (uint8_t  i = 0U; i < n; ++i)
        reads.push_back({ false, shift_from_begin + i });
}

void  in_stream(std::vector<io_relation_location>&  reads, stream_id const&  sid, address const  shift_from_begin, uint8_t const  n)
{
    for (uint8_t  i = 0U; i < n; ++i)
        reads.push_back({ sid, shift_from_begin + i });
}

template<typename  T>
void  in_reg(std::vector<io_relation_value>&  writes, T  value, address const  shift_from_begin, uint8_t const  n)
{
    static_assert(std::is_integral<T>::value,"We support only read of integral types.");
    ASSUMPTION(sizeof(T) >= n);
    byte*  ptr = reinterpret_cast<byte*>(&value);
    if (is_this_little_endian_machine())
        std::reverse(ptr,ptr + sizeof(value));
    ptr += (sizeof(T) - n);
    for (uint8_t  i = 0U; i < n; ++i)
        writes.push_back({ *(ptr + i), { true, shift_from_begin + i } });
}

template<typename  T>
void  in_mem(std::vector<io_relation_value>&  writes, T  value, address const  shift_from_begin, uint8_t const  n)
{
    static_assert(std::is_integral<T>::value,"We support only read of integral types.");
    ASSUMPTION(sizeof(T) >= n);
    byte*  ptr = reinterpret_cast<byte*>(&value);
    if (is_this_little_endian_machine())
        std::reverse(ptr,ptr + sizeof(value));
    ptr += (sizeof(T) - n);
    for (uint8_t  i = 0U; i < n; ++i)
        writes.push_back({ *(ptr + i), { false, shift_from_begin + i } });
}

template<typename  T>
void  in_stream(std::vector<io_relation_value>&  writes, T  value, stream_id const&  sid, address const  shift_from_begin, uint8_t const  n)
{
    static_assert(std::is_integral<T>::value,"We support only read of integral types.");
    ASSUMPTION(sizeof(T) >= n);
    byte*  ptr = reinterpret_cast<byte*>(&value);
    if (is_this_little_endian_machine())
        std::reverse(ptr,ptr + sizeof(value));
    ptr += (sizeof(T) - n);
    for (uint8_t  i = 0U; i < n; ++i)
        writes.push_back({ *(ptr + i), { sid, shift_from_begin + i } });
}

void  in_reg(std::vector<io_relation_value>&  writes, uint128_t  value, address const  shift_from_begin, uint8_t const  n)
{
    ASSUMPTION(value.size() >= n);
    byte*  ptr = value.data();
    if (is_this_little_endian_machine())
        std::reverse(ptr,ptr + sizeof(value));
    ptr += (value.size() - n);
    for (uint8_t  i = 0U; i < n; ++i)
        writes.push_back({ *(ptr + i), { true, shift_from_begin + i } });
}

void  in_mem(std::vector<io_relation_value>&  writes, uint128_t  value, address const  shift_from_begin, uint8_t const  n)
{
    ASSUMPTION(value.size() >= n);
    byte*  ptr = value.data();
    if (is_this_little_endian_machine())
        std::reverse(ptr,ptr + sizeof(value));
    ptr += (value.size() - n);
    for (uint8_t  i = 0U; i < n; ++i)
        writes.push_back({ *(ptr + i), { false, shift_from_begin + i } });
}

void  in_stream(std::vector<io_relation_value>&  writes, uint128_t  value, stream_id const&  sid, address const  shift_from_begin, uint8_t const  n)
{
    ASSUMPTION(value.size() >= n);
    byte*  ptr = value.data();
    if (is_this_little_endian_machine())
        std::reverse(ptr,ptr + sizeof(value));
    ptr += (value.size() - n);
    for (uint8_t  i = 0U; i < n; ++i)
        writes.push_back({ *(ptr + i), { sid, shift_from_begin + i } });
}


void  extend_1_to_1(io_relation&  ior,  std::vector<io_relation_location> const&  reads, std::vector<io_relation_value> const&  writes)
{
    ASSUMPTION(reads.size() == writes.size());
    for (index  i = 0ULL; i < reads.size(); ++i)
        ior.push_back({writes.at(i),{reads.at(i)}});
}

void  extend_1_to_1(io_relation&  ior,  std::vector<io_relation_location> const&  reads, std::vector<io_relation_value> const&  writes,
                    index const  start, index const  end)
{
    ASSUMPTION(reads.size() == writes.size() && start < end && end <= reads.size());
    for (index  i = start; i < end; ++i)
        ior.push_back({writes.at(i),{reads.at(i)}});
}

void  extend_1_to_2(io_relation&  ior,  std::vector<io_relation_location> const&  reads0, std::vector<io_relation_location> const&  reads1, std::vector<io_relation_value> const&  writes)
{
    ASSUMPTION(reads0.size() == writes.size() && reads1.size() == writes.size());
    for (index  i = 0ULL; i < reads0.size(); ++i)
        ior.push_back({writes.at(i),{reads0.at(i),reads1.at(i)}});
}

void  extend_1_to_n(io_relation&  ior,  std::vector<io_relation_location> const&  reads, std::vector<io_relation_value> const&  writes)
{
    for (io_relation_value const&  value : writes)
        ior.push_back({value,reads});
}

void  extend_1_to_n(io_relation&  ior,  std::vector<io_relation_location> const&  reads, std::vector<io_relation_value> const&  writes,
                    index const  start, index const  end)
{
    ASSUMPTION(start < end && end <= writes.size());
    for (index  i = start; i < end; ++i)
        ior.push_back({writes.at(i),reads});
}


}}}

namespace analysis { namespace natexe {


std::string  execute_SETANDCOPY__REG_ASGN_NUMBER(uint8_t const  n, uint64_t const  a, uint64_t const  v, execution_context&  ctx)
{
    memory_write(ctx.reg(),a,n,v);

    std::vector<io_relation_value>  writes;
    in_reg(writes,v,a,n);

    extend_1_to_n(ctx.ior(),{},writes);

    return "";
}

std::string  execute_SETANDCOPY__REG_ASGN_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, execution_context&  ctx)
{
    uint64_t const  value = memory_read<uint64_t>(ctx.reg(),a1,n);
    memory_write(ctx.reg(),a0,n,value);

    std::vector<io_relation_location>  reads;
    in_reg(reads,a1,n);

    std::vector<io_relation_value>  writes;
    in_reg(writes,value,a0,n);

    extend_1_to_1(ctx.ior(),reads,writes);

    return "";
}


std::string  execute_INDIRECTCOPY__REG_ASGN_REG_REG(uint8_t const  n0, uint8_t const  n1, uint64_t const  a0, uint64_t const  a1, execution_context&  ctx)
{
    ASSUMPTION(a0 > 7ULL);
    address const  src_adr = memory_read<address>(ctx.reg(),a1,n1);
    if (src_adr > std::numeric_limits<address>::max() - n0)
        return msgstream() << "Attempt to read behing the end of REG pool.";
    uint64_t const  value = memory_read<uint64_t>(ctx.reg(),src_adr,n0);
    memory_write(ctx.reg(),a0,n0,value);

    std::vector<io_relation_value>  writes;
    in_reg(writes,value,a0,n0);

    std::vector<io_relation_location>  reads;
    in_reg(reads,a1,n1);
    extend_1_to_n(ctx.ior(),reads,writes);
    reads.clear();
    in_reg(reads,src_adr,n0);
    extend_1_to_1(ctx.ior(),reads,writes);


    return "";
}

std::string  execute_INDIRECTCOPY__REG_REG_ASGN_REG(uint8_t const  n0, uint8_t const  n1, uint64_t const  a0, uint64_t const  a1, execution_context&  ctx)
{
    address const  dst_adr = memory_read<address>(ctx.reg(),a0,n1);
    if (dst_adr > std::numeric_limits<address>::max() - n0)
        return msgstream() << "Attempt to write behing the end of REG pool.";
    uint64_t const  value = memory_read<uint64_t>(ctx.reg(),a1,n0);
    ASSUMPTION(dst_adr > 7ULL);
    memory_write(ctx.reg(),dst_adr,n0,value);

    std::vector<io_relation_value>  writes;
    in_reg(writes,value,dst_adr,n0);

    std::vector<io_relation_location>  reads;
    in_reg(reads,a0,n1);
    extend_1_to_n(ctx.ior(),reads,writes);
    reads.clear();
    in_reg(reads,a1,n0);
    extend_1_to_1(ctx.ior(),reads,writes);


    return "";
}


std::string  execute_DATATRANSFER__REG_ASGN_DEREF_INV_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, execution_context&  ctx)
{
    ASSUMPTION(a0 > 7ULL);
    address const  src_adr = memory_read<address>(ctx.reg(),a1,(uint8_t)8U);
    memory_block_info  info = compute_memory_block_info(ctx.mem_allocations(),src_adr,n);
    if (info.allocated != BOOL3::YES)
        return "Attempt to read from a not allocated memory.";
    if (info.readable != BOOL3::YES)
        return "Attempt to read from a not readable memory.";
    uint64_t const  value = memory_read<uint64_t>(ctx.mem(),src_adr,n,false);
    memory_write(ctx.reg(),a0,n,value);
    for (uint8_t i = 0ULL; i < n; ++i)
        ctx.r_d().insert(src_adr + i);

    std::vector<io_relation_value>  writes;
    in_reg(writes,value,a0,n);

    std::vector<io_relation_location>  reads;
    in_reg(reads,a1,8U);
    extend_1_to_n(ctx.ior(),reads,writes);
    reads.clear();
    in_mem(reads,src_adr,n);
    std::reverse(reads.begin(),reads.end());
    extend_1_to_1(ctx.ior(),reads,writes);

    return "";
}

std::string  execute_DATATRANSFER__REG_ASGN_DEREF_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, execution_context&  ctx)
{
    ASSUMPTION(a0 > 7ULL);
    address const  src_adr = memory_read<address>(ctx.reg(),a1,(uint8_t)8U);
    memory_block_info  info = compute_memory_block_info(ctx.mem_allocations(),src_adr,n);
    if (info.allocated != BOOL3::YES)
        return "Attempt to read from a not allocated memory.";
    if (info.readable != BOOL3::YES)
        return "Attempt to read from a not readable memory.";
    uint64_t const  value = memory_read<uint64_t>(ctx.mem(),src_adr,n,true);
    memory_write(ctx.reg(),a0,n,value);
    for (uint8_t i = 0ULL; i < n; ++i)
        ctx.r_d().insert(src_adr + i);

    std::vector<io_relation_value>  writes;
    in_reg(writes,value,a0,n);

    std::vector<io_relation_location>  reads;
    in_reg(reads,a1,8U);
    extend_1_to_n(ctx.ior(),reads,writes);
    reads.clear();
    in_mem(reads,src_adr,n);
    extend_1_to_1(ctx.ior(),reads,writes);

    return "";
}

std::string  execute_DATATRANSFER__DEREF_REG_ASGN_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, execution_context&  ctx)
{
    address const  dst_adr = memory_read<address>(ctx.reg(),a0,(uint8_t)8U);
    memory_block_info  info = compute_memory_block_info(ctx.mem_allocations(),dst_adr,n);
    if (info.allocated != BOOL3::YES)
        return "Attempt to write into a not allocated memory.";
    if (info.writable != BOOL3::YES)
        return "Attempt to write into a not writable memory.";
    uint64_t const  value = memory_read<uint64_t>(ctx.reg(),a1,n);
    memory_write(ctx.mem(),dst_adr,n,value);
    for (uint8_t i = 0ULL; i < n; ++i)
        ctx.w_d().insert(dst_adr + i);

    std::vector<io_relation_value>  writes;
    in_mem(writes,value,dst_adr,n);

    std::vector<io_relation_location>  reads;
    in_reg(reads,a0,8U);
    extend_1_to_n(ctx.ior(),reads,writes);
    reads.clear();
    in_reg(reads,a1,n);
    extend_1_to_1(ctx.ior(),reads,writes);

    return "";
}

std::string  execute_DATATRANSFER__DEREF_INV_REG_ASGN_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, execution_context&  ctx)
{
    address const  dst_adr = memory_read<address>(ctx.reg(),a0,(uint8_t)8U);
    memory_block_info  info = compute_memory_block_info(ctx.mem_allocations(),dst_adr,n);
    if (info.allocated != BOOL3::YES)
        return "Attempt to write into a not allocated memory.";
    if (info.writable != BOOL3::YES)
        return "Attempt to write into a not writable memory.";
    uint64_t const  value = memory_read<uint64_t>(ctx.reg(),a1,n);
    memory_write(ctx.mem(),dst_adr,n,value,false);
    for (uint8_t i = 0ULL; i < n; ++i)
        ctx.w_d().insert(dst_adr + i);

    std::vector<io_relation_value>  writes;
    in_mem(writes,value,dst_adr,n);

    std::vector<io_relation_location>  reads;
    in_reg(reads,a0,8U);
    extend_1_to_n(ctx.ior(),reads,writes);
    reads.clear();
    in_reg(reads,a1,n);
    std::reverse(reads.begin(),reads.end());
    extend_1_to_1(ctx.ior(),reads,writes);

    return "";
}

std::string  execute_DATATRANSFER__DEREF_ADDRESS_ASGN_DATA(uint64_t const  a, uint8_t const*  begin, uint64_t const  num_bytes, execution_context&  ctx)
{
    memory_block_info  info = compute_memory_block_info(ctx.mem_allocations(),a,num_bytes);
    if (info.allocated != BOOL3::YES)
        return "Attempt to write into a not allocated memory.";
    memory_write(ctx.mem(),a,begin,num_bytes);
    // We do not update 'ctx.w_d()', because this instruction assumes there is only one thread executed (i.e. it is a sequential execution).

    std::vector<io_relation_value>  writes;
    for (uint8_t const*  ptr = begin; ptr != begin + num_bytes; ++ptr)
        in_mem(writes,*ptr,a,1U);

    extend_1_to_n(ctx.ior(),{},writes);

    return "";
}

std::string  execute_DATATRANSFER__DEREF_REG_ASGN_NUMBER(uint8_t const  n, uint64_t const  a, uint64_t const  v, execution_context&  ctx)
{
    address const  dst_adr = memory_read<address>(ctx.reg(),a,(uint8_t)8U);
    memory_block_info  info = compute_memory_block_info(ctx.mem_allocations(),dst_adr,n);
    if (info.allocated != BOOL3::YES)
        return "Attempt to write into a not allocated memory.";
    if (info.writable != BOOL3::YES)
        return "Attempt to write into a not writable memory.";
    if (info.in_big_endian == BOOL3::YES_AND_NO)
        return "Cannot determine endiannes of bytes in the MEM pool.";
    memory_write(ctx.mem(),dst_adr,n,v);
    for (uint8_t i = 0ULL; i < n; ++i)
        ctx.w_d().insert(dst_adr + i);

    std::vector<io_relation_location>  reads;
    in_reg(reads,a,8U);

    std::vector<io_relation_value>  writes;
    in_mem(writes,v,dst_adr,n);

    extend_1_to_n(ctx.ior(),reads,writes);

    return "";
}

std::string  execute_DATATRANSFER__DEREF_INV_REG_ASGN_NUMBER(uint8_t const  n, uint64_t const  a, uint64_t const  v, execution_context&  ctx)
{
    address const  dst_adr = memory_read<address>(ctx.reg(),a,(uint8_t)8U);
    memory_block_info  info = compute_memory_block_info(ctx.mem_allocations(),dst_adr,n);
    if (info.allocated != BOOL3::YES)
        return "Attempt to write into a not allocated memory.";
    if (info.writable != BOOL3::YES)
        return "Attempt to write into a not writable memory.";
    if (info.in_big_endian == BOOL3::YES_AND_NO)
        return "Cannot determine endiannes of bytes in the MEM pool.";
    memory_write(ctx.mem(),dst_adr,n,v,false);
    for (uint8_t i = 0ULL; i < n; ++i)
        ctx.w_d().insert(dst_adr + i);

    std::vector<io_relation_location>  reads;
    in_reg(reads,a,8U);

    std::vector<io_relation_value>  writes;
    in_mem(writes,v,dst_adr,n);

    extend_1_to_n(ctx.ior(),reads,writes);

    return "";
}


std::string  execute_TYPECASTING__REG_ASGN_ZERO_EXTEND_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, execution_context&  ctx)
{
    ASSUMPTION(a0 > 7ULL);
    uint64_t const  value = memory_read<uint64_t>(ctx.reg(),a1,n);

    if (n == 8U)
    {
        uint128_t  v(value);
        memory_write(ctx.reg(),a0,v,true);
    }
    else
    {
        ASSUMPTION(n < 8ULL);
        memory_write(ctx.reg(),a0,(uint8_t)(2U*n),value);
    }

    std::vector<io_relation_location>  reads;
    in_reg(reads,a1,n);

    std::vector<io_relation_value>  writes;
    in_reg(writes,value,a0+n,n);

    extend_1_to_1(ctx.ior(),reads,writes);

    return "";
}

std::string  execute_TYPECASTING__REG_ASGN_SIGN_EXTEND_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, execution_context&  ctx)
{
    ASSUMPTION(a0 > 7ULL);
    uint64_t const  value = memory_read<uint64_t>(ctx.reg(),a1,n);

    std::vector<io_relation_location>  reads;
    in_reg(reads,a1,n);

    std::vector<io_relation_value>  writes;

    if (n == 8U)
    {
        uint128_t  v(value,(value & 0x8000000000000000) != 0ULL ? 0xffffffffffffffffULL : 0ULL);
        memory_write(ctx.reg(),a0,v,true);
        in_reg(writes,v,a0,sizeof(v));
    }
    else
    {
        uint64_t  result;
        switch (n)
        {
        case 1U: result = (int64_t)((int8_t)value); break;
        case 2U: result = (int64_t)((int16_t)value); break;
        case 4U: result = (int64_t)((int32_t)value); break;
        default: UNREACHABLE();
        }
        memory_write(ctx.reg(),a0,(uint8_t)(2U*n),result);
        in_reg(writes,result,a0,2U*n);
    }

    extend_1_to_n(ctx.ior(),reads,writes,0ULL,n);
    extend_1_to_1(ctx.ior(),reads,writes,n,2ULL*n);

    return "";
}


std::string  execute_MEMORYMANAGEMENT__REG_ASGN_MEM_STATIC(uint64_t const  a0, uint64_t const  a1, uint8_t const  v0, uint64_t const  v1, execution_context&  ctx)
{
    ASSUMPTION(a0 > 7ULL);
    memory_block_info  info = compute_memory_block_info(ctx.mem_allocations(),a1,v1);
    address const  output_value =  info.allocated == BOOL3::NO ? a1 : 0ULL;
    ctx.mem_allocations().insert({ a1,{
            v1,
            (v0 & 1U) != 0U,
            (v0 & 2U) != 0U,
            (v0 & 4U) != 0U,
            (v0 & 8U) != 0U,
            (v0 & 16U) != 0U,
            } });
    memory_write(ctx.reg(),a0,output_value);

    std::vector<io_relation_value>  writes;
    in_reg(writes,output_value,a0,1U);

    extend_1_to_n(ctx.ior(),{},writes);

    return "";
}

std::string  execute_MEMORYMANAGEMENT__REG_ASGN_MEM_ALLOC(uint64_t const  a0, uint64_t const  a1, uint64_t const  a2, uint64_t const  a3, execution_context&  ctx)
{
    ASSUMPTION(a0 > 7ULL);
    uint8_t const  rights = memory_read<uint8_t>(ctx.reg(),a1);
    uint64_t const  size = align_to_memory_page_size(memory_read<uint64_t>(ctx.reg(),a2));
    uint64_t const  hint_adr = align_to_memory_page_size(memory_read<uint64_t>(ctx.reg(),a3));

    std::vector<io_relation_location>  reads;
    in_reg(reads,a1,1U);
    in_reg(reads,a2,8U);
    in_reg(reads,a3,8U);

    std::vector<io_relation_value>  writes;

    if (hint_adr >= ctx.heap_begin())
    {
        memory_block_info  hint_info = compute_memory_block_info(ctx.mem_allocations(),hint_adr,size);
        if (hint_info.allocated == BOOL3::NO)
        {
            ctx.mem_allocations().insert({ hint_adr,{
                    size,
                    (rights & 1U) != 0U,
                    (rights & 2U) != 0U,
                    (rights & 4U) != 0U,
                    (rights & 8U) != 0U,
                    (rights & 16U) != 0U,
                    } });
            memory_write(ctx.reg(),a0,hint_adr);
            in_reg(writes,hint_adr,a0,8U);
            extend_1_to_n(ctx.ior(),reads,writes);
            return "";
        }
    }

    auto  it  = ctx.mem_allocations().lower_bound(ctx.heap_begin()),
          end = ctx.mem_allocations().lower_bound(ctx.heap_end());

    ASSUMPTION(it != ctx.mem_allocations().end());
    ASSUMPTION(end != ctx.mem_allocations().end());

    while (it != end)
    {
        uint64_t const  adr = align_to_memory_page_size(it->first + it->second.num_bytes());

        ++it;

        if (adr + size <= it->first)
        {
            ctx.mem_allocations().insert({ adr,{
                    size,
                    (rights & 1U) != 0U,
                    (rights & 2U) != 0U,
                    (rights & 4U) != 0U,
                    (rights & 8U) != 0U,
                    (rights & 16U) != 0U,
                    } });
            memory_write(ctx.reg(),a0,adr);
            in_reg(writes,adr,a0,8U);
            extend_1_to_n(ctx.ior(),reads,writes);
            return "";
        }
    }

    memory_write(ctx.reg(),a0,0ULL);
    in_reg(writes,0ULL,a0,8U);

    extend_1_to_n(ctx.ior(),reads,writes);

    return "";
}

std::string  execute_MEMORYMANAGEMENT__REG_ASGN_MEM_FREE(uint64_t const  a0, uint64_t const  a1, uint64_t const  a2, execution_context&  ctx)
{
    ASSUMPTION(a0 > 7ULL);
    uint64_t const  adr = memory_read<uint64_t>(ctx.reg(),a1);
    uint64_t const  size = align_to_memory_page_size(memory_read<uint64_t>(ctx.reg(),a2));

    std::vector<io_relation_location>  reads;
    in_reg(reads,a1,8ULL);

    std::vector<io_relation_value>  writes;

    auto const  it = ctx.mem_allocations().find(adr);
    if (it == ctx.mem_allocations().cend() || it->second.num_bytes() != size)
    {
        memory_write(ctx.reg(),a0,(byte)0U);
        in_reg(writes,0ULL,a0,1U);
    }
    else
    {
        memory_content::iterator  cit = ctx.mem().lower_bound(it->first);
        while (cit != ctx.mem().end() && cit->first + cit->second.size() <= adr + size)
            cit = ctx.mem().erase(cit);
        ctx.mem_allocations().erase(it);
        memory_write(ctx.reg(),a0,(byte)1U);
        in_reg(writes,1ULL,a0,1U);
    }

    extend_1_to_n(ctx.ior(),reads,writes);

    return "";
}

std::string  execute_MEMORYMANAGEMENT__REG_ASGN_MEM_VALID_REG_NUMBER_NUMBER(uint64_t const  a0, uint64_t const  a1, uint8_t const  v0, uint64_t const  v1, execution_context&  ctx)
{
    ASSUMPTION(a0 > 7ULL);
    uint64_t const  adr = memory_read<uint64_t>(ctx.reg(),a1,(byte)8U);
    memory_block_info  info = compute_memory_block_info(ctx.mem_allocations(),adr,v1);
    byte const  result = ((v0 & 1U) != 0U && info.readable != BOOL3::YES)
                         || ((v0 & 2U) != 0U && info.writable != BOOL3::YES)
                         || ((v0 & 4U) != 0U && info.executable != BOOL3::YES)
                         || ((v0 & 8U) != 0U && info.in_big_endian != BOOL3::YES)
                         || ((v0 & 16U) != 0U && info.has_mutable_endian != BOOL3::YES)
                         ? 0U : 1U ;
    memory_write(ctx.reg(),a0,result);

    std::vector<io_relation_location>  reads;
    in_reg(reads,a1,8ULL);

    std::vector<io_relation_value>  writes;
    in_reg(writes,result,a0,1U);

    extend_1_to_n(ctx.ior(),reads,writes);

    return "";
}


std::string  execute_HAVOC__REG_ASGN_HAVOC(uint64_t const  v, uint64_t const  a, execution_context&  ctx)
{
    ASSUMPTION(a > 7ULL);
    memory_havoc(ctx.reg(),a,v);

    std::vector<io_relation_value>  writes;
    for (index  i = 0ULL; i < v; ++i)
        in_reg(writes,default_havoc_value(),a+i,1U);

    extend_1_to_n(ctx.ior(),{},writes);

    return "";
}

std::string  execute_HAVOC__REG_REG_ASGN_HAVOC(uint8_t const  n, uint64_t const  v, uint64_t const  a, execution_context&  ctx)
{
    address const  strat_adr = memory_read<address>(ctx.reg(),a,n);
    if (strat_adr > std::numeric_limits<address>::max() - v)
        return msgstream() << "Attempt to havoc-write behing the end of REG pool.";
    ASSUMPTION(strat_adr > 7ULL);
    memory_havoc(ctx.reg(),strat_adr,v);

    std::vector<io_relation_location>  reads;
    in_reg(reads,a,n);

    std::vector<io_relation_value>  writes;
    for (index  i = 0ULL; i < v; ++i)
        in_reg(writes,default_havoc_value(),strat_adr+i,1U);

    extend_1_to_n(ctx.ior(),reads,writes);

    return "";
}


std::string  execute_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  v, execution_context&  ctx)
{
    ASSUMPTION(n <= 8U);
    uint64_t const  u = memory_read<uint64_t>(ctx.reg(),a1,n);
    uint64_t const  w = u + v;
    memory_write(ctx.reg(),a0,n,w);

    std::vector<io_relation_location>  reads;
    in_reg(reads,a1,n);

    std::vector<io_relation_value>  writes;
    in_reg(writes,w,a0,n);

    extend_1_to_n(ctx.ior(),reads,writes);

    return "";
}

std::string  execute_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint128_t const  v, execution_context&  ctx)
{
    ASSUMPTION(a0 > 7ULL);
    ASSUMPTION(n == 16U);
    uint128_t const  u = memory_read<uint128_t>(ctx.reg(),a1,true);
    uint128_t const  w = u + v;
    memory_write(ctx.reg(),a0,w,true);

    std::vector<io_relation_location>  reads;
    in_reg(reads,a1,n);

    std::vector<io_relation_value>  writes;
    in_reg(writes,w,a0,n);

    extend_1_to_n(ctx.ior(),reads,writes);

    return "";
}

std::string  execute_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  a2, execution_context&  ctx)
{
    std::vector<io_relation_location>  reads;
    in_reg(reads,a1,n);
    in_reg(reads,a2,n);

    std::vector<io_relation_value>  writes;

    if (n == 16U)
    {
        ASSUMPTION(a0 > 7ULL);
        uint128_t const  u = memory_read<uint128_t>(ctx.reg(),a1,true);
        uint128_t const  v = memory_read<uint128_t>(ctx.reg(),a2,true);
        uint128_t const  w = u + v;
        memory_write(ctx.reg(),a0,w,true);
        in_reg(writes,w,a0,n);
    }
    else
    {
        uint64_t const  u = memory_read<uint64_t>(ctx.reg(),a1,n);
        uint64_t const  v = memory_read<uint64_t>(ctx.reg(),a2,n);
        uint64_t const  w = u + v;
        memory_write(ctx.reg(),a0,n,w);
        in_reg(writes,w,a0,n);
    }

    extend_1_to_n(ctx.ior(),reads,writes);

    return "";
}

std::string  execute_INTEGERARITHMETICS__REG_ASGN_REG_TIMES_NUMBER(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  v, execution_context&  ctx)
{
    ASSUMPTION(a0 > 7ULL);
    ASSUMPTION(n <= 8U);
    uint64_t const  u = memory_read<uint64_t>(ctx.reg(),a1,n);
    uint64_t const  w = u * v;
    memory_write(ctx.reg(),a0,n,w);

    std::vector<io_relation_location>  reads;
    in_reg(reads,a1,n);

    std::vector<io_relation_value>  writes;
    in_reg(writes,w,a0,n);

    extend_1_to_n(ctx.ior(),reads,writes);

    return "";
}

std::string  execute_INTEGERARITHMETICS__REG_ASGN_REG_TIMES_NUMBER(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint128_t const  v, execution_context&  ctx)
{
    ASSUMPTION(a0 > 7ULL);
    ASSUMPTION(n == 16U);
    uint128_t const  u = memory_read<uint128_t>(ctx.reg(),a1,true);
    uint128_t const  w = u * v;
    memory_write(ctx.reg(),a0,w,true);

    std::vector<io_relation_location>  reads;
    in_reg(reads,a1,n);

    std::vector<io_relation_value>  writes;
    in_reg(writes,w,a0,n);

    extend_1_to_n(ctx.ior(),reads,writes);

    return "";
}

std::string  execute_INTEGERARITHMETICS__REG_ASGN_REG_DIVIDE_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  a2, execution_context&  ctx)
{
    ASSUMPTION(a0 > 7ULL);

    std::vector<io_relation_location>  reads;
    in_reg(reads,a1,n);
    in_reg(reads,a2,n);

    std::vector<io_relation_value>  writes;

    if (n == 16U)
    {
        uint128_t const  u = memory_read<uint128_t>(ctx.reg(),a1,true);
        uint128_t const  v = memory_read<uint128_t>(ctx.reg(),a2,true);
        if (v == uint128_t(0ULL))
            return "Division by zero.";
        uint128_t const  w = u / v;
        memory_write(ctx.reg(),a0,w,true);
        in_reg(writes,w,a0,n);
    }
    else
    {
        uint64_t const  u = memory_read<uint64_t>(ctx.reg(),a1,n);
        uint64_t const  v = memory_read<uint64_t>(ctx.reg(),a2,n);
        if (v == 0ULL)
            return "Division by zero.";
        uint64_t const  w = u / v;
        memory_write(ctx.reg(),a0,n,w);
        in_reg(writes,w,a0,n);
    }

    extend_1_to_n(ctx.ior(),reads,writes);

    return "";
}

std::string  execute_INTEGERARITHMETICS__REG_ASGN_REG_MODULO_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  a2, execution_context&  ctx)
{
    ASSUMPTION(a0 > 7ULL);

    std::vector<io_relation_location>  reads;
    in_reg(reads,a1,n);
    in_reg(reads,a2,n);

    std::vector<io_relation_value>  writes;

    if (n == 16U)
    {
        uint128_t const  u = memory_read<uint128_t>(ctx.reg(),a1,true);
        uint128_t const  v = memory_read<uint128_t>(ctx.reg(),a2,true);
        if (v == uint128_t(0ULL))
            return "Division by zero (in modulo).";
        uint128_t const  w = u % v;
        memory_write(ctx.reg(),a0,w,true);
        in_reg(writes,w,a0,n);
    }
    else
    {
        uint64_t const  u = memory_read<uint64_t>(ctx.reg(),a1,n);
        uint64_t const  v = memory_read<uint64_t>(ctx.reg(),a2,n);
        if (v == 0ULL)
            return "Division by zero (in modulo).";
        uint64_t const  w = u % v;
        memory_write(ctx.reg(),a0,n,w);
        in_reg(writes,w,a0,n);
    }

    extend_1_to_n(ctx.ior(),reads,writes);

    return "";
}


std::string  execute_ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, execution_context&  ctx)
{
    ASSUMPTION(a0 > 7ULL);
    uint64_t const  u = memory_read<uint64_t>(ctx.reg(),a1,n);
    byte const  w = (u == 0ULL) ? 1U : 0U;
    memory_write(ctx.reg(),a0,w);

    std::vector<io_relation_location>  reads;
    in_reg(reads,a1,n);

    std::vector<io_relation_value>  writes;
    in_reg(writes,w,a0,1U);

    extend_1_to_n(ctx.ior(),reads,writes);

    return "";
}


std::string  execute_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  v, execution_context&  ctx)
{
    ASSUMPTION(a0 > 7ULL);
    uint64_t const  u = memory_read<uint64_t>(ctx.reg(),a1,n);
    uint64_t const  w = u & v;
    memory_write(ctx.reg(),a0,n,w);

    std::vector<io_relation_location>  reads;
    in_reg(reads,a1,n);

    std::vector<io_relation_value>  writes;
    in_reg(writes,w,a0,n);

    extend_1_to_1(ctx.ior(),reads,writes);

    return "";
}

std::string  execute_BITOPERATIONS__REG_ASGN_REG_AND_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  a2, execution_context&  ctx)
{
    ASSUMPTION(a0 > 7ULL);
    uint64_t const  u = memory_read<uint64_t>(ctx.reg(),a1,n);
    uint64_t const  v = memory_read<uint64_t>(ctx.reg(),a2,n);
    uint64_t const  w = u & v;
    memory_write(ctx.reg(),a0,n,w);

    std::vector<io_relation_location>  reads0;
    in_reg(reads0,a1,n);

    std::vector<io_relation_location>  reads1;
    in_reg(reads1,a2,n);

    std::vector<io_relation_value>  writes;
    in_reg(writes,w,a0,n);

    extend_1_to_2(ctx.ior(),reads0,reads1,writes);

    return "";
}

std::string  execute_BITOPERATIONS__REG_ASGN_REG_OR_NUMBER(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  v, execution_context&  ctx)
{
    ASSUMPTION(a0 > 7ULL);
    uint64_t const  u = memory_read<uint64_t>(ctx.reg(),a1,n);
    uint64_t const  w = u | v;
    memory_write(ctx.reg(),a0,n,w);

    std::vector<io_relation_location>  reads;
    in_reg(reads,a1,n);

    std::vector<io_relation_value>  writes;
    in_reg(writes,w,a0,n);

    extend_1_to_1(ctx.ior(),reads,writes);

    return "";
}

std::string  execute_BITOPERATIONS__REG_ASGN_REG_OR_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  a2, execution_context&  ctx)
{
    ASSUMPTION(a0 > 7ULL);
    uint64_t const  u = memory_read<uint64_t>(ctx.reg(),a1,n);
    uint64_t const  v = memory_read<uint64_t>(ctx.reg(),a2,n);
    uint64_t const  w = u | v;
    memory_write(ctx.reg(),a0,n,w);

    std::vector<io_relation_location>  reads0;
    in_reg(reads0,a1,n);

    std::vector<io_relation_location>  reads1;
    in_reg(reads1,a2,n);

    std::vector<io_relation_value>  writes;
    in_reg(writes,w,a0,n);

    extend_1_to_2(ctx.ior(),reads0,reads1,writes);

    return "";
}

std::string  execute_BITOPERATIONS__REG_ASGN_NOT_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, execution_context&  ctx)
{
    ASSUMPTION(a0 > 7ULL);
    uint64_t const  u = memory_read<uint64_t>(ctx.reg(),a1,n);
    uint64_t const  w = ~u;
    memory_write(ctx.reg(),a0,n,w);

    std::vector<io_relation_location>  reads;
    in_reg(reads,a1,n);

    std::vector<io_relation_value>  writes;
    in_reg(writes,w,a0,n);

    extend_1_to_1(ctx.ior(),reads,writes);

    return "";
}

std::string  execute_BITOPERATIONS__REG_ASGN_REG_XOR_NUMBER(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  v, execution_context&  ctx)
{
    ASSUMPTION(a0 > 7ULL);
    uint64_t const  u = memory_read<uint64_t>(ctx.reg(),a1,n);
    uint64_t const  w = u ^ v;
    memory_write(ctx.reg(),a0,n,w);

    std::vector<io_relation_location>  reads;
    in_reg(reads,a1,n);

    std::vector<io_relation_value>  writes;
    in_reg(writes,w,a0,n);

    extend_1_to_1(ctx.ior(),reads,writes);

    return "";
}

std::string  execute_BITOPERATIONS__REG_ASGN_REG_XOR_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  a2, execution_context&  ctx)
{
    ASSUMPTION(a0 > 7ULL);
    uint64_t const  u = memory_read<uint64_t>(ctx.reg(),a1,n);
    uint64_t const  v = memory_read<uint64_t>(ctx.reg(),a2,n);
    uint64_t const  w = u ^ v;
    memory_write(ctx.reg(),a0,n,w);

    std::vector<io_relation_location>  reads0;
    in_reg(reads0,a1,n);

    std::vector<io_relation_location>  reads1;
    in_reg(reads1,a2,n);

    std::vector<io_relation_value>  writes;
    in_reg(writes,w,a0,n);

    extend_1_to_2(ctx.ior(),reads0,reads1,writes);

    return "";
}

std::string  execute_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint8_t const  v, execution_context&  ctx)
{
    ASSUMPTION(a0 > 7ULL);
    uint64_t  u = memory_read<uint64_t>(ctx.reg(),a1,n);
    u = u >> v;
    memory_write(ctx.reg(),a0,n,u);

    std::vector<io_relation_location>  reads;
    in_reg(reads,a1,n);

    std::vector<io_relation_value>  writes;
    in_reg(writes,u,a0,n);

    extend_1_to_n(ctx.ior(),reads,writes);

    return "";
}

std::string  execute_BITOPERATIONS__REG_ASGN_REG_RSHIFT_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  a2, execution_context&  ctx)
{
    ASSUMPTION(a0 > 7ULL);
    uint64_t  u = memory_read<uint64_t>(ctx.reg(),a1,n);
    uint8_t  v = memory_read<uint8_t>(ctx.reg(),a2,(byte)1U);
    u = u >> v;
    memory_write(ctx.reg(),a0,n,u);

    std::vector<io_relation_location>  reads;
    in_reg(reads,a1,n);
    in_reg(reads,a2,1U);

    std::vector<io_relation_value>  writes;
    in_reg(writes,u,a0,n);

    extend_1_to_n(ctx.ior(),reads,writes);

    return "";
}

std::string  execute_BITOPERATIONS__REG_ASGN_REG_LSHIFT_NUMBER(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint8_t const  v, execution_context&  ctx)
{
    ASSUMPTION(a0 > 7ULL);
    uint64_t  u = memory_read<uint64_t>(ctx.reg(),a1,n);
    u = u << v;
    memory_write(ctx.reg(),a0,n,u);

    std::vector<io_relation_location>  reads;
    in_reg(reads,a1,n);

    std::vector<io_relation_value>  writes;
    in_reg(writes,u,a0,n);

    extend_1_to_n(ctx.ior(),reads,writes);

    return "";
}

std::string  execute_BITOPERATIONS__REG_ASGN_REG_LSHIFT_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, uint64_t const  a2, execution_context&  ctx)
{
    ASSUMPTION(a0 > 7ULL);
    uint64_t  u = memory_read<uint64_t>(ctx.reg(),a1,n);
    uint8_t  v = memory_read<uint8_t>(ctx.reg(),a2,(byte)1U);
    u = u << v;
    memory_write(ctx.reg(),a0,n,u);

    std::vector<io_relation_location>  reads;
    in_reg(reads,a1,n);
    in_reg(reads,a2,1U);

    std::vector<io_relation_value>  writes;
    in_reg(writes,u,a0,n);

    extend_1_to_n(ctx.ior(),reads,writes);

    return "";
}


std::string  execute_INPUTOUTPUT__REG_ASGN_STREAM_OPEN_NUMBER(uint64_t const  a, uint8_t const  v0, uint64_t const  v1, execution_context&  ctx)
{
    ASSUMPTION(a > 7ULL);
    std::string  stream_id;
    {
        std::stringstream  sstr;
        sstr << '#' << v1;
        stream_id = sstr.str();
    }
    stream_open_info&  info = ctx.stream_allocations()[stream_id];
    if (info.is_open())
        return msgstream() << "The stream '" << stream_id << "' is already open.";
    info.set_is_open(true);
    info.set_readable(v0 == 1U ? true : false);
    info.set_writable(v0 == 2U ? true : false);
    info.set_seekable(false);
    info.set_cursor(0ULL);
    memory_write(ctx.reg(),a,v1);

    std::vector<io_relation_value>  writes;
    in_reg(writes,v1,a,8U);

    extend_1_to_n(ctx.ior(),{},writes);

    return "";
}

std::string  execute_INPUTOUTPUT__REG_ASGN_STREAM_READ_NUMBER(uint64_t const  a, uint64_t const  v, execution_context&  ctx)
{
    ASSUMPTION(a > 7ULL);
    std::string  stream_id;
    {
        std::stringstream  sstr;
        sstr << '#' << v;
        stream_id = sstr.str();
    }
    auto const  it = ctx.stream_allocations().find(stream_id);
    if (it == ctx.stream_allocations().end() || !it->second.is_open())
        return msgstream() << "Cannot read a byte from the stream '" << stream_id << "'. The stream either "
                              "does not exist or is not open.";
    if (!it->second.readable())
        return msgstream() << "Cannot read a byte from the stream '" << stream_id << "'. The stream is not "
                              "readable.";
    if (it->second.cursor() >= it->second.size())
        return msgstream() << "Cannot read a byte from the stream '" << stream_id << "'. The cursor of the "
                              "stream has invalid value.";
    INVARIANT(ctx.contents_of_streams().count(stream_id) != 0ULL);

    std::vector<io_relation_location>  reads;
    in_stream(reads,stream_id,it->second.cursor(),1U);

    byte  value;
    stream_read(ctx.contents_of_streams().at(stream_id),it->second.cursor(),&value,1ULL);
    it->second.set_cursor(it->second.cursor() + 1ULL);
    INVARIANT(it->second.cursor() <= it->second.size());
    memory_write(ctx.reg(),a,value);

    std::vector<io_relation_value>  writes;
    in_reg(writes,value,a,1U);

    extend_1_to_1(ctx.ior(),reads,writes);

    return "";
}

std::string  execute_INPUTOUTPUT__REG_ASGN_STREAM_WRITE_NUMBER_REG(uint64_t const  a0, uint64_t const  v, uint64_t const  a1, execution_context&  ctx)
{
    ASSUMPTION(a0 > 7ULL);
    uint8_t const  value_to_write = memory_read<uint8_t>(ctx.reg(),a1);

    std::string  stream_id;
    {
        std::stringstream  sstr;
        sstr << '#' << v;
        stream_id = sstr.str();
    }
    auto const  it = ctx.stream_allocations().find(stream_id);
    if (it == ctx.stream_allocations().end() || !it->second.is_open())
        return msgstream() << "Cannot write a byte to the stream '" << stream_id << "'. The stream either "
                              "does not exist or is not open.";
    if (!it->second.writable())
        return msgstream() << "Cannot write a byte to the stream '" << stream_id << "'. The stream is not "
                              "writable.";

    std::vector<io_relation_location>  reads;
    in_reg(reads,a1,1U);

    std::vector<io_relation_value>  writes;
    in_stream(writes,value_to_write,stream_id,it->second.cursor(),1U);
    in_reg(writes,1U,a0,1U);
    extend_1_to_n(ctx.ior(),reads,writes);

    stream_write<uint8_t>(ctx.contents_of_streams()[stream_id],it->second.cursor(),value_to_write);
    if (it->second.cursor() == it->second.size())
        it->second.set_size(it->second.size() + 1ULL);
    it->second.set_cursor(it->second.cursor() + 1ULL);
    INVARIANT(it->second.cursor() <= it->second.size());
    memory_write<uint8_t>(ctx.reg(),a0,(byte)1);

    return "";
}

std::string  execute_INPUTOUTPUT__REG_ASGN_STREAM_WRITE_REG_REG(uint64_t const  a0, uint64_t const  a1, uint64_t const  a2, execution_context&  ctx)
{
    ASSUMPTION(a0 > 7ULL);
    uint64_t const  stream_number = memory_read<uint64_t>(ctx.reg(),a1);
    uint8_t const  value_to_write = memory_read<uint8_t>(ctx.reg(),a2);

    std::string  stream_id;
    {
        std::stringstream  sstr;
        sstr << '#' << stream_number;
        stream_id = sstr.str();
    }
    auto const  it = ctx.stream_allocations().find(stream_id);
    if (it == ctx.stream_allocations().end() || !it->second.is_open())
        return msgstream() << "Cannot write a byte to the stream '" << stream_id << "'. The stream either "
                              "does not exist or is not open.";
    if (!it->second.writable())
        return msgstream() << "Cannot write a byte to the stream '" << stream_id << "'. The stream is not "
                              "writable.";

    std::vector<io_relation_location>  reads;
    in_reg(reads,a1,8U);
    in_reg(reads,a2,1U);

    std::vector<io_relation_value>  writes;
    in_stream(writes,value_to_write,stream_id,it->second.cursor(),1U);
    in_reg(writes,1U,a0,1U);
    extend_1_to_n(ctx.ior(),reads,writes);

    stream_write<uint8_t>(ctx.contents_of_streams()[stream_id],it->second.cursor(),value_to_write);
    if (it->second.cursor() == it->second.size())
        it->second.set_size(it->second.size() + 1ULL);
    it->second.set_cursor(it->second.cursor() + 1ULL);
    INVARIANT(it->second.cursor() <= it->second.size());
    memory_write<uint8_t>(ctx.reg(),a0,(byte)1);

    return "";
}


std::string  execute_MISCELLANEOUS__REG_ASGN_PARITY_REG(uint8_t const  n, uint64_t const  a0, uint64_t const  a1, execution_context&  ctx)
{
    ASSUMPTION(a0 > 7ULL);
    uint64_t  u = memory_read<uint64_t>(ctx.reg(),a1,n);
    uint8_t  count = 0U;
    while (u != 0ULL)
    {
        if ((u & 1ULL) != 0)
            ++count;
        u = u >> 1U;
    }
    byte const  result = (count % 2U) == 0U ? 1U : 0U;
    memory_write(ctx.reg(),a0,result);

    std::vector<io_relation_location>  reads;
    in_reg(reads,a1,n);

    std::vector<io_relation_value>  writes;
    in_reg(writes,result,a0,1U);

    extend_1_to_n(ctx.ior(),reads,writes);

    return "";
}


std::string  execution_instruction(microcode::instruction const& I, execution_context&  ctx, bool const  is_sequential)
{
    switch (I.GIK())
    {
    case microcode::GIK::SETANDCOPY__REG_ASGN_NUMBER:
        return execute_SETANDCOPY__REG_ASGN_NUMBER(I.argument<uint8_t>(0ULL),I.argument<uint64_t>(1ULL),I.argument<uint64_t>(2ULL),ctx);
    case microcode::GIK::SETANDCOPY__REG_ASGN_REG:
        return execute_SETANDCOPY__REG_ASGN_REG(I.argument<uint8_t>(0ULL),I.argument<uint64_t>(1ULL),I.argument<uint64_t>(2ULL),ctx);

    case microcode::GIK::INDIRECTCOPY__REG_ASGN_REG_REG:
        return execute_INDIRECTCOPY__REG_ASGN_REG_REG(I.argument<uint8_t>(0ULL),I.argument<uint8_t>(1ULL),I.argument<uint64_t>(2ULL),I.argument<uint64_t>(3ULL),ctx);
    case microcode::GIK::INDIRECTCOPY__REG_REG_ASGN_REG:
        return execute_INDIRECTCOPY__REG_REG_ASGN_REG(I.argument<uint8_t>(0ULL),I.argument<uint8_t>(1ULL),I.argument<uint64_t>(2ULL),I.argument<uint64_t>(3ULL),ctx);

    case microcode::GIK::DATATRANSFER__REG_ASGN_DEREF_INV_REG:
        return execute_DATATRANSFER__REG_ASGN_DEREF_INV_REG(I.argument<uint8_t>(0ULL),I.argument<uint64_t>(1ULL),I.argument<uint64_t>(2ULL),ctx);
    case microcode::GIK::DATATRANSFER__REG_ASGN_DEREF_REG:
        return execute_DATATRANSFER__REG_ASGN_DEREF_REG(I.argument<uint8_t>(0ULL),I.argument<uint64_t>(1ULL),I.argument<uint64_t>(2ULL),ctx);
    case microcode::GIK::DATATRANSFER__DEREF_REG_ASGN_REG:
        return execute_DATATRANSFER__DEREF_REG_ASGN_REG(I.argument<uint8_t>(0ULL),I.argument<uint64_t>(1ULL),I.argument<uint64_t>(2ULL),ctx);
    case microcode::GIK::DATATRANSFER__DEREF_INV_REG_ASGN_REG:
        return execute_DATATRANSFER__DEREF_INV_REG_ASGN_REG(I.argument<uint8_t>(0ULL),I.argument<uint64_t>(1ULL),I.argument<uint64_t>(2ULL),ctx);
    case microcode::GIK::DATATRANSFER__DEREF_ADDRESS_ASGN_DATA:
        if (!is_sequential)
            return "Execution of 'DATATRANSFER__DEREF_ADDRESS_ASGN_DATA' has failed, because there are running more than one thread.";
        return execute_DATATRANSFER__DEREF_ADDRESS_ASGN_DATA(I.argument<uint64_t>(0ULL),I.argument_begin(1ULL),I.num_arguments() - 1ULL,ctx);
    case microcode::GIK::DATATRANSFER__DEREF_REG_ASGN_NUMBER:
        return execute_DATATRANSFER__DEREF_REG_ASGN_NUMBER(I.argument<uint8_t>(0ULL),I.argument<uint64_t>(1ULL),I.argument<uint64_t>(2ULL),ctx);
    case microcode::GIK::DATATRANSFER__DEREF_INV_REG_ASGN_NUMBER:
        return execute_DATATRANSFER__DEREF_INV_REG_ASGN_NUMBER(I.argument<uint8_t>(0ULL),I.argument<uint64_t>(1ULL),I.argument<uint64_t>(2ULL),ctx);

    case microcode::GIK::TYPECASTING__REG_ASGN_ZERO_EXTEND_REG:
        return execute_TYPECASTING__REG_ASGN_ZERO_EXTEND_REG(I.argument<uint8_t>(0ULL),I.argument<uint64_t>(1ULL),I.argument<uint64_t>(2ULL),ctx);
    case microcode::GIK::TYPECASTING__REG_ASGN_SIGN_EXTEND_REG:
        return execute_TYPECASTING__REG_ASGN_SIGN_EXTEND_REG(I.argument<uint8_t>(0ULL),I.argument<uint64_t>(1ULL),I.argument<uint64_t>(2ULL),ctx);

    case microcode::GIK::MEMORYMANAGEMENT__REG_ASGN_MEM_STATIC:
        return execute_MEMORYMANAGEMENT__REG_ASGN_MEM_STATIC(I.argument<uint64_t>(0ULL),I.argument<uint64_t>(1ULL),I.argument<uint8_t>(2ULL),I.argument<uint64_t>(3ULL),ctx);
    case microcode::GIK::MEMORYMANAGEMENT__REG_ASGN_MEM_ALLOC:
        return execute_MEMORYMANAGEMENT__REG_ASGN_MEM_ALLOC(I.argument<uint64_t>(0ULL),I.argument<uint64_t>(1ULL),I.argument<uint64_t>(2ULL),I.argument<uint64_t>(3ULL),ctx);
    case microcode::GIK::MEMORYMANAGEMENT__REG_ASGN_MEM_FREE:
        return execute_MEMORYMANAGEMENT__REG_ASGN_MEM_FREE(I.argument<uint64_t>(0ULL),I.argument<uint64_t>(1ULL),I.argument<uint64_t>(2ULL),ctx);
    case microcode::GIK::MEMORYMANAGEMENT__REG_ASGN_MEM_VALID_REG_NUMBER_NUMBER:
        return execute_MEMORYMANAGEMENT__REG_ASGN_MEM_VALID_REG_NUMBER_NUMBER(I.argument<uint64_t>(0ULL),I.argument<uint64_t>(1ULL),I.argument<uint8_t>(2ULL),I.argument<uint64_t>(3ULL),ctx);

    case microcode::GIK::HAVOC__REG_ASGN_HAVOC:
        return execute_HAVOC__REG_ASGN_HAVOC(I.argument<uint64_t>(0ULL),I.argument<uint64_t>(1ULL),ctx);
    case microcode::GIK::HAVOC__REG_REG_ASGN_HAVOC:
        return execute_HAVOC__REG_REG_ASGN_HAVOC(I.argument<uint8_t>(0ULL),I.argument<uint64_t>(1ULL),I.argument<uint64_t>(2ULL),ctx);

    case microcode::GIK::INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER:
        if (I.argument<uint8_t>(0ULL) != 16U)
            return execute_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(I.argument<uint8_t>(0ULL),I.argument<uint64_t>(1ULL),I.argument<uint64_t>(2ULL),I.argument<uint64_t>(3ULL),ctx);
        else
            return execute_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(I.argument<uint8_t>(0ULL),I.argument<uint64_t>(1ULL),I.argument<uint64_t>(2ULL),I.argument<uint128_t>(3ULL),ctx);
    case microcode::GIK::INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG:
        return execute_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(I.argument<uint8_t>(0ULL),I.argument<uint64_t>(1ULL),I.argument<uint64_t>(2ULL),I.argument<uint64_t>(3ULL),ctx);
    case microcode::GIK::INTEGERARITHMETICS__REG_ASGN_REG_TIMES_NUMBER:
        if (I.argument<uint8_t>(0ULL) != 16U)
            return execute_INTEGERARITHMETICS__REG_ASGN_REG_TIMES_NUMBER(I.argument<uint8_t>(0ULL),I.argument<uint64_t>(1ULL),I.argument<uint64_t>(2ULL),I.argument<uint64_t>(3ULL),ctx);
        else
            return execute_INTEGERARITHMETICS__REG_ASGN_REG_TIMES_NUMBER(I.argument<uint8_t>(0ULL),I.argument<uint64_t>(1ULL),I.argument<uint64_t>(2ULL),I.argument<uint128_t>(3ULL),ctx);
    case microcode::GIK::INTEGERARITHMETICS__REG_ASGN_REG_DIVIDE_REG:
        return execute_INTEGERARITHMETICS__REG_ASGN_REG_DIVIDE_REG(I.argument<uint8_t>(0ULL),I.argument<uint64_t>(1ULL),I.argument<uint64_t>(2ULL),I.argument<uint64_t>(3ULL),ctx);
    case microcode::GIK::INTEGERARITHMETICS__REG_ASGN_REG_MODULO_REG:
        return execute_INTEGERARITHMETICS__REG_ASGN_REG_MODULO_REG(I.argument<uint8_t>(0ULL),I.argument<uint64_t>(1ULL),I.argument<uint64_t>(2ULL),I.argument<uint64_t>(3ULL),ctx);

    case microcode::GIK::ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO:
        return execute_ZEROTEST__REG_ASGN_REG_EQUAL_TO_ZERO(I.argument<uint8_t>(0ULL),I.argument<uint64_t>(1ULL),I.argument<uint64_t>(2ULL),ctx);

    case microcode::GIK::BITOPERATIONS__REG_ASGN_REG_AND_NUMBER:
        return execute_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(I.argument<uint8_t>(0ULL),I.argument<uint64_t>(1ULL),I.argument<uint64_t>(2ULL),I.argument<uint64_t>(3ULL),ctx);
    case microcode::GIK::BITOPERATIONS__REG_ASGN_REG_AND_REG:
        return execute_BITOPERATIONS__REG_ASGN_REG_AND_REG(I.argument<uint8_t>(0ULL),I.argument<uint64_t>(1ULL),I.argument<uint64_t>(2ULL),I.argument<uint64_t>(3ULL),ctx);
    case microcode::GIK::BITOPERATIONS__REG_ASGN_REG_OR_NUMBER:
        return execute_BITOPERATIONS__REG_ASGN_REG_OR_NUMBER(I.argument<uint8_t>(0ULL),I.argument<uint64_t>(1ULL),I.argument<uint64_t>(2ULL),I.argument<uint64_t>(3ULL),ctx);
    case microcode::GIK::BITOPERATIONS__REG_ASGN_REG_OR_REG:
        return execute_BITOPERATIONS__REG_ASGN_REG_OR_REG(I.argument<uint8_t>(0ULL),I.argument<uint64_t>(1ULL),I.argument<uint64_t>(2ULL),I.argument<uint64_t>(3ULL),ctx);
    case microcode::GIK::BITOPERATIONS__REG_ASGN_NOT_REG:
        return execute_BITOPERATIONS__REG_ASGN_NOT_REG(I.argument<uint8_t>(0ULL),I.argument<uint64_t>(1ULL),I.argument<uint64_t>(2ULL),ctx);
    case microcode::GIK::BITOPERATIONS__REG_ASGN_REG_XOR_NUMBER:
        return execute_BITOPERATIONS__REG_ASGN_REG_XOR_NUMBER(I.argument<uint8_t>(0ULL),I.argument<uint64_t>(1ULL),I.argument<uint64_t>(2ULL),I.argument<uint64_t>(3ULL),ctx);
    case microcode::GIK::BITOPERATIONS__REG_ASGN_REG_XOR_REG:
        return execute_BITOPERATIONS__REG_ASGN_REG_XOR_REG(I.argument<uint8_t>(0ULL),I.argument<uint64_t>(1ULL),I.argument<uint64_t>(2ULL),I.argument<uint64_t>(3ULL),ctx);
    case microcode::GIK::BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER:
        return execute_BITOPERATIONS__REG_ASGN_REG_RSHIFT_NUMBER(I.argument<uint8_t>(0ULL),I.argument<uint64_t>(1ULL),I.argument<uint64_t>(2ULL),I.argument<uint8_t>(3ULL),ctx);
    case microcode::GIK::BITOPERATIONS__REG_ASGN_REG_RSHIFT_REG:
        return execute_BITOPERATIONS__REG_ASGN_REG_RSHIFT_REG(I.argument<uint8_t>(0ULL),I.argument<uint64_t>(1ULL),I.argument<uint64_t>(2ULL),I.argument<uint64_t>(3ULL),ctx);
    case microcode::GIK::BITOPERATIONS__REG_ASGN_REG_LSHIFT_NUMBER:
        return execute_BITOPERATIONS__REG_ASGN_REG_LSHIFT_NUMBER(I.argument<uint8_t>(0ULL),I.argument<uint64_t>(1ULL),I.argument<uint64_t>(2ULL),I.argument<uint8_t>(3ULL),ctx);
    case microcode::GIK::BITOPERATIONS__REG_ASGN_REG_LSHIFT_REG:
        return execute_BITOPERATIONS__REG_ASGN_REG_LSHIFT_REG(I.argument<uint8_t>(0ULL),I.argument<uint64_t>(1ULL),I.argument<uint64_t>(2ULL),I.argument<uint64_t>(3ULL),ctx);

    case microcode::GIK::INPUTOUTPUT__REG_ASGN_STREAM_OPEN_NUMBER:
        return execute_INPUTOUTPUT__REG_ASGN_STREAM_OPEN_NUMBER(I.argument<uint64_t>(0ULL),I.argument<uint8_t>(1ULL),I.argument<uint64_t>(2ULL),ctx);
    case microcode::GIK::INPUTOUTPUT__REG_ASGN_STREAM_READ_NUMBER:
        return execute_INPUTOUTPUT__REG_ASGN_STREAM_READ_NUMBER(I.argument<uint64_t>(0ULL),I.argument<uint64_t>(1ULL),ctx);
    case microcode::GIK::INPUTOUTPUT__REG_ASGN_STREAM_WRITE_NUMBER_REG:
        return execute_INPUTOUTPUT__REG_ASGN_STREAM_WRITE_NUMBER_REG(I.argument<uint64_t>(0ULL),I.argument<uint64_t>(1ULL),I.argument<uint64_t>(2ULL),ctx);
    case microcode::GIK::INPUTOUTPUT__REG_ASGN_STREAM_WRITE_REG_REG:
        return execute_INPUTOUTPUT__REG_ASGN_STREAM_WRITE_REG_REG(I.argument<uint64_t>(0ULL),I.argument<uint64_t>(1ULL),I.argument<uint64_t>(2ULL),ctx);

    case microcode::GIK::MISCELLANEOUS__REG_ASGN_PARITY_REG:
        return execute_MISCELLANEOUS__REG_ASGN_PARITY_REG(I.argument<uint8_t>(0ULL),I.argument<uint64_t>(1ULL),I.argument<uint64_t>(2ULL),ctx);
    case microcode::GIK::MISCELLANEOUS__NOP:
        return "";
    case microcode::GIK::MISCELLANEOUS__STOP:
        return "The execution was terminated by the 'STOP' instruction.";

    default:
        return msgstream() << "Attempt to execute not implemented instruction "
                           << "GIK=" << (uint64_t)microcode::num(I.GIK())
                           << ", GID=" << (uint64_t)microcode::decompose(I.GIK()).first
                           << ", SID=" << (uint64_t)microcode::decompose(I.GIK()).second
                           << ".";
    }
}


}}
