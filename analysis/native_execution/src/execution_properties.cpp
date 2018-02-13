#include <rebours/analysis/native_execution/execution_properties.hpp>
#include <rebours/utility/invariants.hpp>
#include <rebours/utility/assumptions.hpp>
#include <rebours/utility/development.hpp>
#include <limits>
//#include <mutex>

namespace analysis { namespace natexe { namespace detail {

//static std::mutex  thread_id_counter_mutex;
static thread_id  thread_id_counter = 0ULL;


}}}

namespace analysis { namespace natexe {


thread_id  generate_fresh_thread_id()
{
    //std::lock_guard<std::mutex> const  lock(detail::thread_id_counter_mutex);
    INVARIANT(detail::thread_id_counter != std::numeric_limits<thread_id>::max());
    return ++detail::thread_id_counter;
}


memory_allocation_info::memory_allocation_info(
        size const  num_bytes,
        bool const  readable,
        bool const  writable,
        bool const  executable,
        bool const  is_in_big_endian,
        bool const  has_mutable_endian
        )
    : m_num_bytes(num_bytes)
    , m_readable(readable)
    , m_writable(writable)
    , m_executable(executable)
    , m_is_in_big_endian(is_in_big_endian)
    , m_has_mutable_endian(has_mutable_endian)
{}


memory_block_info  compute_memory_block_info(memory_allocations const&  alloc, address  begin, size const  num_bytes)
{
    ASSUMPTION(begin <= std::numeric_limits<address>::max() - num_bytes);
    ASSUMPTION(num_bytes > 0ULL);

//    memory_allocations::const_iterator  block_begin = alloc.lower_bound(begin);
//    if (block_begin == alloc.cend() || block_begin->first > begin)
//    {
//        block_begin = block_begin == alloc.cbegin() ? block_begin : std::prev(block_begin,1);
//        if (block_begin == alloc.cend() || block_begin->first > begin || block_begin->first + block_begin->second.num_bytes() <= begin)
//            return { BOOL3::NO, BOOL3::NO, BOOL3::NO, BOOL3::NO, BOOL3::YES_AND_NO, BOOL3::YES_AND_NO };
//    }
    memory_allocations::const_iterator  block_begin;
    {
        block_begin = alloc.lower_bound(begin);
        memory_allocations::const_iterator const  prev_block_begin = block_begin == alloc.cbegin() ? alloc.cend() : std::prev(block_begin,1);
        if (prev_block_begin != alloc.cend() && prev_block_begin->first + prev_block_begin->second.num_bytes() > begin)
            block_begin =  prev_block_begin;
    }
    if (block_begin == alloc.cend() || block_begin->first >= begin + num_bytes)
        return { BOOL3::NO, BOOL3::NO, BOOL3::NO, BOOL3::NO, BOOL3::YES_AND_NO, BOOL3::YES_AND_NO };
    address const end = begin + num_bytes;
    memory_allocations::const_iterator const  block_end = alloc.lower_bound(end);
    INVARIANT(block_begin != block_end);
    memory_block_info  info{block_begin->second};
    for ( ; block_begin != block_end && begin < end; ++block_begin)
    {
        if (block_begin->first > begin)
            return { BOOL3::YES_AND_NO,
                     weaken_YES(info.readable),
                     weaken_YES(info.writable),
                     weaken_YES(info.executable),
                     BOOL3::YES_AND_NO,
                     BOOL3::YES_AND_NO };

        info.readable = weaken(info.readable,block_begin->second.readable());
        info.writable = weaken(info.writable,block_begin->second.writable());
        info.executable = weaken(info.executable,block_begin->second.executable());
        info.in_big_endian = weaken(info.in_big_endian,block_begin->second.is_in_big_endian());
        info.has_mutable_endian = weaken(info.has_mutable_endian,block_begin->second.has_mutable_endian());

        INVARIANT(block_begin->first + block_begin->second.num_bytes() > begin);
        size const  block_rest = block_begin->first + block_begin->second.num_bytes() - begin;
        begin += block_rest;
    }

    return info;
}


uint64_t  align_to_memory_page_size(uint64_t const  value)
{
    return value + (value % memory_page_size == 0ULL ? 0ULL : memory_page_size - (value % memory_page_size));
}



memory_page::memory_page()
    : m_data()
{
    std::fill(data(),data()+size(),0xcdU);
}


void  memory_read(memory_content const&  content, address const  begin, byte* const  data_begin, size const  num_bytes)
{
    index  num_output_bytes = 0ULL;
    address  page_begin = begin & ~(memory_page_size - 1ULL);
    index  page_offset = begin - page_begin;

    while (num_output_bytes < num_bytes)
    {
        size const  num_bytes_to_copy = std::min(memory_page_size,num_bytes - num_output_bytes);

        memory_page&  page = const_cast<memory_content&>(content)[page_begin];  //!< The const_cast is used to automatically create
                                                                                //!< a page with uninitialised data, if it is not in
                                                                                //!< the map yet.

        INVARIANT(page_offset + num_bytes_to_copy <= memory_page_size);
        std::copy(page.data() + page_offset,
                  page.data() + page_offset + num_bytes_to_copy,
                  data_begin + num_output_bytes);

        page_begin += memory_page_size;
        page_offset = 0ULL;
        num_output_bytes += num_bytes_to_copy;
    }

    INVARIANT(num_output_bytes == num_bytes);
}

void  memory_write(memory_content&  content, address const  begin, byte const* const  data_begin, size const  num_bytes)
{
    index  num_output_bytes = 0ULL;
    address  page_begin = begin & ~(memory_page_size - 1ULL);
    index  page_offset = begin - page_begin;

    while (num_output_bytes < num_bytes)
    {
        size const  num_bytes_to_copy = std::min(memory_page_size - page_offset, num_bytes - num_output_bytes);

        memory_page&  page = content[page_begin];

        INVARIANT(num_bytes_to_copy > 0ULL);
        INVARIANT(page_offset + num_bytes_to_copy <= memory_page_size);
        std::copy(data_begin + num_output_bytes,
                  data_begin + num_output_bytes + num_bytes_to_copy,
                  page.data() + page_offset
                  );

        page_begin += memory_page_size;
        page_offset = 0ULL;
        num_output_bytes += num_bytes_to_copy;
    }

    INVARIANT(num_output_bytes == num_bytes);
}


void  memory_write(memory_content&  content, address const  begin, uint128_t  value, bool const  write_data_in_big_endian)
{
    byte*  b = value.data();
    if (write_data_in_big_endian == is_this_little_endian_machine())
        std::reverse(b,b + value.size());
    memory_write(content,begin,(byte const*)b,value.size());
}

void  memory_havoc(memory_content&  content, address const  begin, size const  num_bytes, byte const  value)
{
    index  num_output_bytes = 0ULL;
    address  page_begin = begin & ~(memory_page_size - 1ULL);
    index  page_offset = begin - page_begin;

    while (num_output_bytes < num_bytes)
    {
        size const  num_bytes_to_copy = std::min(memory_page_size - page_offset,num_bytes - num_output_bytes);

        memory_page&  page = content[page_begin];

        INVARIANT(num_bytes_to_copy > 0ULL);
        INVARIANT(page_offset + num_bytes_to_copy <= memory_page_size);
        std::fill(page.data() + page_offset,
                  page.data() + page_offset + num_bytes_to_copy,
                  value
                  );

        page_begin += memory_page_size;
        page_offset = 0ULL;
        num_output_bytes += num_bytes_to_copy;
    }

    INVARIANT(num_output_bytes == num_bytes);
}

thread::thread(node_id const  program_entry,
               std::shared_ptr<memory_content> const  reg,
               thread_id const  id)
    : m_id(id)
    , m_stack{program_entry}
    , m_reg(reg.operator bool() ? reg : std::make_shared<memory_content>())
{
    ASSUMPTION(m_reg.operator bool());
}

thread::thread(call_stack const&  stack, memory_content const&  reg, thread_id const  id)
    : m_id(id)
    , m_stack(stack)
    , m_reg(std::make_shared<memory_content>(reg))
{}

void thread::swap(thread&  other)
{
    std::swap(m_id,other.m_id);
    std::swap(m_stack,other.m_stack);
    std::swap(m_reg,other.m_reg);
}


stream_open_info::stream_open_info()
    : m_is_open(false)
    , m_readable(false)
    , m_writable(false)
    , m_seekable(false)
    , m_size(0ULL)
    , m_cursor(0ULL)
{}

stream_open_info::stream_open_info(
        bool const  is_open,
        bool const  readable,
        bool const  writable,
        bool const  seekable,
        natexe::size const  size,
        index const  cursor
        )
    : m_is_open(is_open)
    , m_readable(readable)
    , m_writable(writable)
    , m_seekable(seekable)
    , m_size(size)
    , m_cursor(cursor)
{}


stream_page::stream_page()
    : m_data()
{
    std::fill(data(),data()+size(),0xcdU);
}


void  stream_read(stream_content const&  content, index const  begin, byte* const  data_begin, size const  num_bytes)
{
    index  num_output_bytes = 0ULL;
    index  page_begin = begin & ~(stream_page_size - 1ULL);
    index  page_offset = begin - page_begin;

    while (num_output_bytes < num_bytes)
    {
        size const  num_bytes_to_copy = std::min(stream_page_size - page_offset,num_bytes - num_output_bytes);

        stream_page&  page = const_cast<stream_content&>(content)[page_begin];  //!< The const_cast is used to automatically create
                                                                                //!< a page with uninitialised data, if it is not in
                                                                                //!< the map yet.

        INVARIANT(num_bytes_to_copy > 0ULL);
        INVARIANT(page_offset + num_bytes_to_copy <= stream_page_size);
        std::copy(page.data() + page_offset,
                  page.data() + page_offset + num_bytes_to_copy,
                  data_begin + num_output_bytes);

        page_begin += stream_page_size;
        page_offset = 0ULL;
        num_output_bytes += num_bytes_to_copy;
    }

    INVARIANT(num_output_bytes == num_bytes);
}

void  stream_write(stream_content&  content, index const  begin, byte const* const  data_begin, size const  num_bytes)
{
    index  num_output_bytes = 0ULL;
    index  page_begin = begin & ~(stream_page_size - 1ULL);
    index  page_offset = begin - page_begin;

    while (num_output_bytes < num_bytes)
    {
        size const  num_bytes_to_copy = std::min(stream_page_size - page_offset,num_bytes - num_output_bytes);

        stream_page&  page = content[page_begin];

        INVARIANT(num_bytes_to_copy > 0ULL);
        INVARIANT(page_offset + num_bytes_to_copy <= stream_page_size);
        std::copy(data_begin + num_output_bytes,
                  data_begin + num_output_bytes + num_bytes_to_copy,
                  page.data() + page_offset
                  );

        page_begin += stream_page_size;
        page_offset = 0ULL;
        num_output_bytes += num_bytes_to_copy;
    }

    INVARIANT(num_output_bytes == num_bytes);
}


void  input_frontier_of_thread::on_reg_impact(address const  adr, input_impact_link const&  link)
{
    m_reg_impacts[adr] = link;
}

void  input_frontier_of_thread::on_mem_impact(address const  adr, input_impact_link const&  link)
{
    m_mem_impacts[adr] = link;
}

void  input_frontier_of_thread::on_stream_impact(stream_id const&  sid, address const  shift, input_impact_link const&  link)
{
    m_stream_impacts[{sid,shift}] = link;
}

input_impact_link const* input_frontier_of_thread::find_in_reg(address const  adr) const
{
    auto const  it = m_reg_impacts.find(adr);
    return it == m_reg_impacts.cend() ? nullptr : &it->second;
}

input_impact_link const* input_frontier_of_thread::find_in_mem(address const  adr) const
{
    auto const  it = m_mem_impacts.find(adr);
    return it == m_mem_impacts.cend() ? nullptr : &it->second;
}

input_impact_link const* input_frontier_of_thread::find_in_stream(stream_id const&  sid, address const  adr) const
{
    auto const  it = m_stream_impacts.find({sid,adr});
    return it == m_stream_impacts.cend() ? nullptr : &it->second;
}


execution_properties::execution_properties(
        execution_id const  eid,
        uint64_t const  heap_begin,
        uint64_t const  heap_end,
        uint64_t const  temporaries_begin
        )
    : m_execution_id(eid)
    , m_mem_allocations()
    , m_mem_content()
    , m_interrupts()
    , m_stream_allocations()
    , m_contents_of_streams()
    , m_final_regs()
    , m_heap_begin(heap_begin)
    , m_heap_end(heap_end)
    , m_temporaries_begin(temporaries_begin)
    , m_input_frontier_of_threads()
{}

void  execution_properties::add_final_reg(thread_id const  id, std::shared_ptr<memory_content> const  reg)
{
    ASSUMPTION(
        [](std::vector< std::pair<thread_id,std::shared_ptr<memory_content> > > const&  final_regs, thread_id const  id) {
            for (auto const&  id_reg : final_regs)
                if (id_reg.first == id)
                    return false;
            return true;
        }(m_final_regs,id)
        );
    m_final_regs.push_back({id,reg});
}


io_relation_location::io_relation_location(
        bool const  is_in_reg_pool,
        address const  shift_from_begin
        )
    : m_is_in_reg_pool(is_in_reg_pool)
    , m_stream_id("")
    , m_shift(shift_from_begin)
{}

io_relation_location::io_relation_location(
        stream_id const&  sid,
        address const  shift_from_begin
        )
    : m_is_in_reg_pool(false)
    , m_stream_id(sid)
    , m_shift(shift_from_begin)
{
    ASSUMPTION(m_stream_id.size() > 1ULL);
    ASSUMPTION(m_stream_id.front() == '#');
}


execution_context::execution_context(memory_content&  reg, execution_properties&  eprops,
                                     mem_access_set&  w_d, mem_access_set&  r_d, mem_access_set&  w_c, mem_access_set&  r_c)
    : m_reg(&reg)
    , m_eprops(&eprops)
    , m_w_d(&w_d)
    , m_r_d(&r_d)
    , m_w_c(&w_c)
    , m_r_c(&r_c)
    , m_io_relation()
{}


}}
