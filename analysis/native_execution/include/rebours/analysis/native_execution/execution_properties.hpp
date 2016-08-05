#ifndef REBOURS_ANALYSIS_NATIVE_EXECUTION_EXECUTION_PROPERTIES_HPP_INCLUDED
#   define REBOURS_ANALYSIS_NATIVE_EXECUTION_EXECUTION_PROPERTIES_HPP_INCLUDED

#   include <rebours/analysis/native_execution/endian.hpp>
#   include <rebours/analysis/native_execution/invariants.hpp>
#   include <rebours/analysis/native_execution/assumptions.hpp>
#   include <rebours/analysis/native_execution/development.hpp>
#   include <rebours/analysis/native_execution/std_pair_hash.hpp>
#   include <rebours/program/program.hpp>
#   include <memory>
#   include <vector>
#   include <array>
#   include <string>
#   include <map>
#   include <set>
#   include <type_traits>
#   include <unordered_map>
#   include <algorithm>
#   include <cstdint>

/**
 * Here we define/declare types and functions related to a single native execution of a program.
 * Definitions/declarations of types and functions over many executions are provided in the header
 * file 'recovery_properties.hpp'. However, due to flow of data to 'recovery_properties' the
 * separation between both is not strict (e.g. some types defined here are intensively used in
 * 'recovery_properties.hpp').
 */

namespace analysis { namespace natexe {


////////////////////////////////////////////////////////////////////////////////////////////
///
/// GENERAL BASIC TYPES
///
////////////////////////////////////////////////////////////////////////////////////////////


using  execution_id = uint64_t; //!< Each native execution of a program is associated a unique 'execution id'.
using  thread_id = uint64_t;    //!< We assign an unique number to each created thread.

using  address = uint64_t;
using  size = uint64_t;
using  byte = uint8_t;
using  index = uint64_t;
using  node_counter_type = uint64_t; //!< During execution of a program a thread visits a sequence of program nodes.
                                     //!< A 'node counter' is an index into this sequence.

/**
 * It represents a reference (a link) into a table of chains of input dependances, see 'recovery_properties::input_impacts()'.
 */
using  input_impact_link = std::pair<node_counter_type, //!< Index into the sequence of nodes visited by a thread.
                                     index              //!< Index into a list of chains related to the position in the sequence of nodes.
                                     >;

using  node_id = microcode::program_component::node_id;
using  edge_id = microcode::program_component::edge_id;


/**
 * A representation of 3-valued truth states. The value 'YES_AND_NO' should be paraphrased as 'for some YES and also for some NO'.
 * For example, we may want to know whether all bytes in an address range [adr,adr+n) are all allocated. Then answer YES_AND_NO
 * from the check would mean that at least one of the bytes is allocated and also at least one byte of the bytes is not.
 */
enum struct BOOL3 : uint8_t
{
    NO          = 0U,
    YES         = 1U,
    YES_AND_NO  = 2U,
};

inline BOOL3  bool3(bool const  a) noexcept { return a ? BOOL3::YES : BOOL3::NO; }
inline BOOL3  weaken(BOOL3 const  a, BOOL3 const  b) noexcept { return a == b ? a : BOOL3::YES_AND_NO; }
inline BOOL3  weaken(BOOL3 const  a, bool const  b) noexcept { return weaken(a,bool3(b)); }
inline BOOL3  weaken(bool const  a, BOOL3 const  b) noexcept { return weaken(bool3(a),b); }
inline BOOL3  weaken(bool const  a, bool const  b) noexcept { return weaken(bool3(a),bool3(b)); }
inline BOOL3  weaken_YES(BOOL3 const  a) noexcept { return weaken(a,BOOL3::YES); }
inline BOOL3  weaken_NO(BOOL3 const  a) noexcept { return weaken(a,BOOL3::NO); }


////////////////////////////////////////////////////////////////////////////////////////////
///
/// MEMORY MANAGEMENT
///
////////////////////////////////////////////////////////////////////////////////////////////


/**
 * It captures properties of a single allocated memory block in the MEM pool.
 */
struct memory_allocation_info
{
    memory_allocation_info(
            size const  num_bytes,
            bool const  readable,
            bool const  writable,
            bool const  executable,
            bool const  is_in_big_endian,
            bool const  has_mutable_endian
            );
    size  num_bytes() const noexcept { return m_num_bytes; }
    bool  readable() const noexcept { return m_readable; }
    bool  writable() const noexcept { return m_writable; }
    bool  executable() const noexcept { return m_executable; }
    bool  is_in_big_endian() const noexcept { return m_is_in_big_endian; }
    bool  has_mutable_endian() const noexcept { return m_has_mutable_endian; }
private:
    size  m_num_bytes;
    bool  m_readable;
    bool  m_writable;
    bool  m_executable;
    bool  m_is_in_big_endian;
    bool  m_has_mutable_endian;
};
using  memory_allocations = std::map<address,                   //!< A start address of an allocated memory block.
                                     memory_allocation_info     //!< Properties of the allocated memory block.
                                     >;

/**
 * This is an abstraction of the struct 'memory_allocation_info' above for queries over any address ranges in the MEM pool.
 */
struct memory_block_info
{
    memory_block_info(
            BOOL3 const  a_allocated,
            BOOL3 const  a_readable,
            BOOL3 const  a_writable,
            BOOL3 const  a_executable,
            BOOL3 const  a_in_big_endian,
            BOOL3 const  a_has_mutable_endian
            )
        : allocated(a_allocated)
        , readable(a_readable)
        , writable(a_writable)
        , executable(a_executable)
        , in_big_endian(a_in_big_endian)
        , has_mutable_endian(a_has_mutable_endian)
    {}

    explicit memory_block_info(memory_allocation_info const&  info)
        : memory_block_info(
              BOOL3::YES,
              bool3(info.readable()),
              bool3(info.writable()),
              bool3(info.executable()),
              bool3(info.is_in_big_endian()),
              bool3(info.has_mutable_endian())
              )
    {}

    BOOL3  allocated;
    BOOL3  readable;
    BOOL3  writable;
    BOOL3  executable;
    BOOL3  in_big_endian;
    BOOL3  has_mutable_endian;
};

/**
 * It computes an abstaction 'memory_block_info' for a query over address ranges in the MEM pool.
 */
memory_block_info  compute_memory_block_info(memory_allocations const&  alloc, address  begin, size const  num_bytes);


size constexpr  memory_page_size = 0x1000ULL;   //!< ~ 4kB
                                                //!< TODO: This value should be passed into the analysis instead of to be fixed constant!

uint64_t  align_to_memory_page_size(uint64_t const  value);


struct memory_page
{
    using data_type = std::array<byte,memory_page_size>;

    memory_page();

    natexe::size  size() const { return m_data.size(); }
    byte  at(natexe::size const  i) const { return m_data.at(i); }
    byte&  at(natexe::size const  i) { return m_data.at(i); }

    byte const*  data() const { return m_data.data(); }
    byte*  data() { return m_data.data(); }

private:
    std::array<byte,memory_page_size>  m_data;
};

using  memory_content = std::map<address,           //!< Start address.
                                 memory_page        //!< Values of bytes from the start address.
                                 >;


inline byte  default_havoc_value() { return 0xdcU; } //!< TODO: Instead of using this fixed value we might consider to do bookkeeping
                                                     //!< of havocked addresses and check for read accesses to them.

void  memory_read(memory_content const&  content, address const  begin, byte* const  data_begin, size const  num_bytes);
void  memory_write(memory_content&  content, address const  begin, byte const* const  data_begin, size const  num_bytes);
void  memory_havoc(memory_content&  content, address const  begin, size const  num_bytes, byte const  value = default_havoc_value());
inline void  memory_write(memory_content&  content, address const  begin, byte* const  data_begin, size const  num_bytes) { memory_write(content,begin,(byte const*)data_begin,num_bytes); }


template<typename T>
T  memory_read(memory_content const&  content, address const  begin, byte const  num_bytes, bool const  are_data_in_big_endian = true)
{
    static_assert(std::is_integral<T>::value,"We support only read of integral types.");
    ASSUMPTION(num_bytes <= sizeof(T));
    T  value = 0;
    byte*  b = reinterpret_cast<byte*>(&value);
    memory_read(content,begin,b + (are_data_in_big_endian ? sizeof(T) - num_bytes : 0ULL),num_bytes);
    if (are_data_in_big_endian == is_this_little_endian_machine())
        std::reverse(b,b+sizeof(T));
    return *reinterpret_cast<T*>(b);
}

template<typename T>
T  memory_read(memory_content const&  content, address const  begin, bool const  are_data_in_big_endian = true,
               typename std::conditional<sizeof(T) <= sizeof(uint64_t),T*,void>::type = nullptr)
{
    static_assert(std::is_integral<T>::value,"We support only read of integral types.");
    std::array<byte,sizeof(T)>  data;
    memory_read(content,begin,data.data(),sizeof(T));
    if (are_data_in_big_endian == is_this_little_endian_machine())
        std::reverse(data.begin(),data.end());
    return *reinterpret_cast<T*>(data.data());
}

template<typename T>
T  memory_read(memory_content const&  content, address const  begin, bool const  are_data_in_big_endian = true,
               typename std::conditional<sizeof(T) <= sizeof(uint64_t),void,T*>::type = nullptr)
{
    static_assert(std::is_same<T,uint128_t>::value || std::is_same<T,float80_t>::value,"We support only read of int128_t and float80_t types.");
    T  result;
    byte*  b = result.data();
    memory_read(content,begin,b,result.size());
    if (are_data_in_big_endian == is_this_little_endian_machine())
        std::reverse(b,b + result.size());
    return result;
}


template<typename T>
void  memory_write(memory_content&  content, address const  begin, T  value, bool const  write_data_in_big_endian = true)
{
    static_assert(std::is_integral<T>::value,"We support only conversion to integral types.");
    byte*  b = reinterpret_cast<byte*>(&value);
    if (write_data_in_big_endian == is_this_little_endian_machine())
        std::reverse(b,b + sizeof(value));
    memory_write(content,begin,(byte const*)b,sizeof(value));
}

void  memory_write(memory_content&  content, address const  begin, uint128_t  value, bool const  write_data_in_big_endian = true);

template<typename T>
void  memory_write(memory_content&  content, address const  begin, uint8_t const  num_bytes, T  value, bool const  write_data_in_big_endian = true)
{
    static_assert(std::is_integral<T>::value,"We support only conversion to integral types.");
    ASSUMPTION(num_bytes <= sizeof(T));
    byte*  b = reinterpret_cast<byte*>(&value);
    if (write_data_in_big_endian == is_this_little_endian_machine())
        std::reverse(b,b + sizeof(value));
    memory_write(content,begin,(byte const*)b + (write_data_in_big_endian ? (size)(sizeof(T) - num_bytes) : 0ULL),num_bytes);
}


////////////////////////////////////////////////////////////////////////////////////////////
///
/// STREAM MANAGEMENT
///
////////////////////////////////////////////////////////////////////////////////////////////


/**
 * It hold complete information about a stream except its content (i.e. bytes in the stream).
 */
struct stream_open_info
{
    stream_open_info();

    stream_open_info(
            bool const  is_open,
            bool const  readable,
            bool const  writable,
            bool const  seekable,
            natexe::size const  size,
            index const  cursor
            );

    bool  is_open() const noexcept { return m_is_open; }
    bool  readable() const noexcept { return m_readable; }
    bool  writable() const noexcept { return m_writable; }
    bool  seekable() const noexcept { return m_seekable; }
    natexe::size  size() const noexcept { return m_size; }
    index  cursor() const noexcept { return m_cursor; }

    void  set_is_open(bool const  state) { m_is_open = state; }
    void  set_readable(bool const  state) { m_readable = state; }
    void  set_writable(bool const  state) { m_writable = state; }
    void  set_seekable(bool const  state) { m_seekable = state; }
    void  set_size(natexe::size const  new_size) { m_size = new_size; }
    void  set_cursor(index const  new_cursor) { m_cursor = new_cursor; }

private:
    bool  m_is_open;
    bool  m_readable;
    bool  m_writable;
    bool  m_seekable;
    natexe::size  m_size;
    index  m_cursor;
};

using  stream_id = std::string; //!< It is a unique identifier of a stream. We use the string type, because
                                //!< the most common stream types are files (identified by pathname). We represent
                                //!< standard streams by their numbers prefixed by the symbol '#', e.g. #1 is the std::in.

using  stream_allocations = std::map<stream_id,         //!< A unique identifier of the stream. For standard streams #1, #2, #3, and #4,
                                                        //!< the corresponding strings are "#1", "#2", "#3", and "#4" respectively. For all
                                                        //!< other streams it can be any other string, typically path-names on the disc.
                                     stream_open_info   //!< Open properties of the stream.
                                     >;


/**
 * Internally we represent contents of streams in terms of memory pages (just like memory pages in operational memory).
 * However, insted of 'addressing' bytes in a stream we use the term 'indexing'.
 */


size constexpr  stream_page_size = 0x1000ULL;  //!< ~ 4kB


struct stream_page
{
    using data_type = std::array<byte,memory_page_size>;

    stream_page();

    natexe::size  size() const { return m_data.size(); }
    byte  at(natexe::size const  i) const { return m_data.at(i); }
    byte&  at(natexe::size const  i) { return m_data.at(i); }

    byte const*  data() const { return m_data.data(); }
    byte*  data() { return m_data.data(); }

private:
    std::array<byte,stream_page_size>  m_data;
};
//using  stream_page = std::array<byte,stream_page_size>;
using  stream_content = std::map<index,             //!< Start index of a stream-page in the stream.
                                 stream_page        //!< Values of bytes of the stream-page from the start index.
                                 >;
using  contents_of_streams = std::unordered_map<stream_id,stream_content>;

void  stream_read(stream_content const&  content, index const  begin, byte* const  data_begin, size const  num_bytes);
void  stream_write(stream_content&  content, index const  begin, byte const* const  data_begin, size const  num_bytes);

template<typename T>
T  stream_read(stream_content const&  content, index const  begin, bool const  are_data_in_big_endian = true)
{
    static_assert(std::is_integral<T>::value,"We support only conversion to integral types.");
    std::array<byte,sizeof(T)>  data;
    stream_read(content,begin,data.data(),sizeof(T));
    if (are_data_in_big_endian == is_this_little_endian_machine())
        std::reverse(data.begin(),data.end());
    return *reinterpret_cast<T*>(data.data());
}

template<typename T>
void  stream_write(stream_content&  content, index const  begin, T  value, bool const  write_data_in_big_endian = true)
{
    static_assert(std::is_integral<T>::value,"We support only conversion to integral types.");
    byte*  b = reinterpret_cast<byte*>(&value);
    if (write_data_in_big_endian == is_this_little_endian_machine())
        std::reverse(b,b + sizeof(value));
    stream_write(content,begin,(byte const*)b,sizeof(value));
}



////////////////////////////////////////////////////////////////////////////////////////////
///
/// INPUT DEPENDENCES FRONTIER
///
////////////////////////////////////////////////////////////////////////////////////////////



struct input_frontier_of_thread
{
    input_frontier_of_thread() : m_reg_impacts(), m_mem_impacts(), m_stream_impacts() {}

    std::unordered_map<address,input_impact_link> const&  reg_impacts() const noexcept { return m_reg_impacts; }
    std::unordered_map<address,input_impact_link> const&  mem_impacts() const noexcept { return m_mem_impacts; }
    std::unordered_map<std::pair<stream_id,address>,input_impact_link> const&  stream_impacts() const noexcept { return m_stream_impacts; }

    void  on_reg_impact(address const  adr, input_impact_link const&  link);
    void  on_mem_impact(address const  adr, input_impact_link const&  link);
    void  on_stream_impact(stream_id const&  sid, address const  shift, input_impact_link const&  link);

    void  delete_reg_impact(address const  adr) { m_reg_impacts.erase(adr); }
    void  delete_mem_impact(address const  adr) { m_mem_impacts.erase(adr); }
    void  delete_stream_impact(stream_id const&  sid, address const  shift) { m_stream_impacts.erase({sid,shift}); }

    input_impact_link const* find_in_reg(address const  adr) const;
    input_impact_link const* find_in_mem(address const  adr) const;
    input_impact_link const* find_in_stream(stream_id const&  sid, address const  adr) const;

private:
    std::unordered_map<address,input_impact_link>  m_reg_impacts;
    std::unordered_map<address,input_impact_link>  m_mem_impacts;
    std::unordered_map<std::pair<stream_id,address>,input_impact_link>  m_stream_impacts;
};

struct io_relation_location
{
    io_relation_location(
            bool const  is_in_reg_pool,
            address const  shift_from_begin
            );
    io_relation_location(
            stream_id const&  sid,
            address const  shift_from_begin
            );
    bool  is_in_reg_pool() const noexcept { return !is_in_stream() && m_is_in_reg_pool; }
    bool  is_in_mem_pool() const noexcept { return !is_in_stream() && !m_is_in_reg_pool; }
    bool  is_in_stream() const noexcept { return !m_stream_id.empty(); }
    stream_id const&  stream() const noexcept { return m_stream_id; }
    address  shift_from_begin() const noexcept { return m_shift; }
private:
    bool  m_is_in_reg_pool;
    stream_id  m_stream_id;
    address  m_shift;
};

struct io_relation_value
{
    io_relation_value(uint8_t const  value, io_relation_location const&  location) : m_value(value), m_location(location) {}
    uint8_t  value() const noexcept { return m_value; }
    io_relation_location const&  location() const noexcept { return m_location; }
private:
    uint8_t  m_value;
    io_relation_location  m_location;
};

using  io_relation = std::vector<std::pair<io_relation_value, std::vector<io_relation_location> > >;



////////////////////////////////////////////////////////////////////////////////////////////
///
/// EXECUTION PROPERTIES
///
////////////////////////////////////////////////////////////////////////////////////////////



// NOTE: We currently avoid use of the interrupt vector table by checking assumptions of executed
//       instructions and by taking fixed actions (like termination of a thread/program) when
//       they are not satisfied. So far everything went fine, but if this solution is not sufficint
//       we will have to start to use this vector. Therfore, we at least introduce it.
using  interrupts_vector_table = std::unordered_map<index, //!< Index (number) of the interruption
                                                    address //!< Address into the MEM (or possibly SYS) pool where is the handling routine.
                                                    >;

/**
 * It is a collection of all data required during the whole process of a single native execution of a prolog and a recovered program.
 * Its instance is passed along all functions assisting in the execution. It also includes support for tracking data, which should
 * go into the global storage (over all performed execution) represented by 'recovery_properties' structure, see file 'recovery_properties.hpp'.
 */
struct execution_properties
{
    execution_properties(
            execution_id const  eid,
            uint64_t const  heap_begin,
            uint64_t const  heap_end,
            uint64_t const  temporaries_begin
            );

    execution_id  get_execution_id() const noexcept { return m_execution_id; }

    memory_allocations const&  mem_allocations() const noexcept { return m_mem_allocations; }
    memory_content const&  mem_content() const noexcept { return m_mem_content; }
    interrupts_vector_table const&  interrupts() const noexcept { return m_interrupts; }
    natexe::stream_allocations const&  stream_allocations() const noexcept { return m_stream_allocations; }
    natexe::contents_of_streams const&  contents_of_streams() const noexcept { return m_contents_of_streams; }

    memory_allocations&  mem_allocations() { return m_mem_allocations; }
    memory_content&  mem_content() { return m_mem_content; }
    interrupts_vector_table&  interrupts() { return m_interrupts; } //!< NOTE: not used so far.
    natexe::stream_allocations&  stream_allocations() { return m_stream_allocations; }
    natexe::contents_of_streams&  contents_of_streams() { return m_contents_of_streams; }

    std::vector< std::pair<thread_id,std::shared_ptr<memory_content> > > const&  final_regs() const noexcept { return m_final_regs; }
    void  add_final_reg(thread_id const  id, std::shared_ptr<memory_content> const  reg);
    void  clear_final_regs() { m_final_regs.clear(); }

    uint64_t  heap_begin() const noexcept { return m_heap_begin; }
    uint64_t  heap_end() const noexcept { return m_heap_end; }
    uint64_t temporaries_begin() const noexcept { return m_temporaries_begin; }

    std::unordered_map<thread_id,input_frontier_of_thread> const&  input_frontier_of_threads() const  noexcept { return m_input_frontier_of_threads; }
    input_frontier_of_thread&  input_frontier(thread_id const  tid) { return m_input_frontier_of_threads[tid]; }
    input_frontier_of_thread const&  input_frontier(thread_id const  tid) const { return m_input_frontier_of_threads.at(tid); }

private:
    execution_id  m_execution_id;
    memory_allocations  m_mem_allocations;
    memory_content  m_mem_content;
    interrupts_vector_table  m_interrupts; //!< NOTE: not used so far.
    natexe::stream_allocations  m_stream_allocations;
    natexe::contents_of_streams  m_contents_of_streams;
    std::vector< std::pair<thread_id,std::shared_ptr<memory_content> > >  m_final_regs;
    uint64_t  m_heap_begin;
    uint64_t  m_heap_end;
    uint64_t  m_temporaries_begin;
    std::unordered_map<thread_id,input_frontier_of_thread>  m_input_frontier_of_threads;
};


////////////////////////////////////////////////////////////////////////////////////////////
///
/// THREADS
///
////////////////////////////////////////////////////////////////////////////////////////////


/**
 * A call stack is a sequence of nodes [e0,e1,...,eN,u] where N>=0, all e0,...,eN are
 * exit nodes of program components, and u is any program node. The nodes e0,...,eN
 * represent targets of jumps when returning from called component and the node u
 * represents the current node (i.e. instructions of its out-edges will be executed
 * next).
 */
using  call_stack = std::vector<node_id>;


thread_id  generate_fresh_thread_id(); //!< This is a global generator of unique thread ids.


/**
 * It represent a single thread of execution.
 */
struct thread
{
    thread() : m_id(0ULL), m_stack(), m_reg() {}
    explicit thread(node_id const  program_entry,
                    std::shared_ptr<memory_content> const  reg = std::shared_ptr<memory_content>(),
                    thread_id const  id = generate_fresh_thread_id());

    thread(call_stack const&  stack, memory_content const&  reg,
           thread_id const  id = generate_fresh_thread_id());

    thread_id  id() const noexcept { return m_id; }
    call_stack const&  stack() const noexcept { return m_stack; }
    memory_content const&  reg() const noexcept { return *m_reg; }

    call_stack&  stack() { return m_stack; }
    memory_content&  reg() { return *m_reg; }

    void swap(thread&  other);

    std::shared_ptr<memory_content>  reg_ptr() const noexcept { return m_reg; }

private:
    thread_id  m_id;
    call_stack  m_stack;
    std::shared_ptr<memory_content>  m_reg;
};


using  mem_access_set = std::set<address>;


/**
 * Threads share MEM pool and streams. This data structure holds all this common data
 * necessary for performing execution steps of threads. This also include data for
 * checking illegal simultaneous memory accesses of concurrently executed threads.
 */
struct execution_context
{
    execution_context(memory_content&  reg, execution_properties&  eprops,
                      mem_access_set&  w_d, mem_access_set&  r_d, mem_access_set&  w_c, mem_access_set&  r_c);

    memory_content&  reg() noexcept { return *m_reg; }
    memory_content&  mem() noexcept { return m_eprops->mem_content(); }
    memory_allocations&  mem_allocations() noexcept { return m_eprops->mem_allocations(); }
    natexe::stream_allocations&  stream_allocations() { return m_eprops->stream_allocations(); }
    natexe::contents_of_streams&  contents_of_streams() { return m_eprops->contents_of_streams(); }
    interrupts_vector_table&  interrupts() { return m_eprops->interrupts(); }
    uint64_t  heap_begin() const noexcept { return m_eprops->heap_begin(); }
    uint64_t  heap_end() const noexcept { return m_eprops->heap_end(); }
    mem_access_set&  w_d() noexcept { return *m_w_d; }
    mem_access_set&  r_d() noexcept { return *m_r_d; }
    mem_access_set&  w_c() noexcept { return *m_w_c; }
    mem_access_set&  r_c() noexcept { return *m_r_c; }
    io_relation const&  ior() const noexcept { return m_io_relation; }
    io_relation&  ior() { return m_io_relation; }

private:
    memory_content*  m_reg;
    execution_properties*  m_eprops;
    mem_access_set*  m_w_d;
    mem_access_set*  m_r_d;
    mem_access_set*  m_w_c;
    mem_access_set*  m_r_c;
    io_relation  m_io_relation;
};


}}


#endif
