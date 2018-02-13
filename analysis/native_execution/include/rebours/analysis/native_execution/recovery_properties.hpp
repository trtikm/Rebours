#ifndef REBOURS_ANALYSIS_NATIVE_EXECUTION_RECOVERY_PROPERTIES_HPP_INCLUDED
#   define REBOURS_ANALYSIS_NATIVE_EXECUTION_RECOVERY_PROPERTIES_HPP_INCLUDED

#   include <rebours/analysis/native_execution/execution_properties.hpp>
#   include <rebours/utility/std_pair_hash.hpp>
#   include <rebours/program/program.hpp>
#   include <unordered_map>
#   include <unordered_set>
#   include <map>
#   include <vector>
#   include <utility>
#   include <memory>
#   include <chrono>
#   include <string>
#   include <cstdint>

namespace analysis { namespace natexe {


struct  switch_info
{
    explicit switch_info(node_id const  head_node);

    node_id  head_node() const noexcept { return m_head_node; }
    node_id  default_node() const noexcept { return m_default_node; }
    std::unordered_map<address,node_id> const&  cases() const noexcept { return m_cases; }

    void  add_case(address const  adr, node_id const  case_node, node_id const  default_node);

    microcode::instruction  havoc_instruction() const noexcept { return m_havoc_instruction; }
    void  set_havoc_instruction(microcode::instruction const  I);

private:
    node_id  m_head_node;
    node_id  m_default_node;
    std::unordered_map<address,node_id>  m_cases;
    microcode::instruction  m_havoc_instruction;
};


bool  is_switch_instruction(microcode::instruction const&  I);
microcode::instruction  compute_switch_havoc_instruction(microcode::program_component const&  C, node_id const  head_node);
node_id  add_case_to_switch(microcode::program_component&  C, switch_info&  info, address const  case_IP_value, address const  tmp_begin, node_id&  erased);


struct  unexplored_info
{
    unexplored_info(
            node_id const  node,
            address const  IP
            );

    node_id  node() const noexcept { return m_node; }
    address  IP() const noexcept { return m_IP; }

private:
    node_id  m_node;
    address  m_IP;
};


using  important_code_ranges = std::map<uint64_t,   //!< End address.
                                        uint64_t    //!< Begin address.
                                        >;

using  branchings_of_threads = std::unordered_map<thread_id,std::vector<node_counter_type> >;
using  nodes_history_of_threads = std::unordered_map<thread_id,std::vector<node_id> >;

struct  input_impact_value
{
    input_impact_value(
            uint8_t const  value,
            bool const  is_in_reg_pool,
            address const  shift_from_begin,
            std::vector<input_impact_link> const&  links
            );
    input_impact_value(
            uint8_t const  value,
            stream_id const&  sid,
            address const  shift_from_begin,
            std::vector<input_impact_link> const&  links
            );
    input_impact_value(
            uint8_t const  value,
            stream_id::value_type const* const  sid,
            address const  shift_from_begin,
            std::vector<input_impact_link> const&  links
            ) : input_impact_value(value,stream_id(sid),shift_from_begin,links) {}

    uint8_t  value() const noexcept { return m_value; }
    bool  is_in_reg_pool() const noexcept { return !is_in_stream() && m_is_in_reg_pool; }
    bool  is_in_mem_pool() const noexcept { return !is_in_stream() && !m_is_in_reg_pool; }
    bool  is_in_stream() const noexcept { return !m_stream_id.empty(); }
    stream_id const&  stream() const noexcept { return m_stream_id; }
    address  shift_from_begin() const noexcept { return m_shift; }
    std::vector<input_impact_link> const&  links() const noexcept { return m_links; }
    void  clear_links() { m_links.clear(); }
private:
    uint8_t  m_value;
    bool  m_is_in_reg_pool;
    stream_id  m_stream_id;
    address  m_shift;
    std::vector<input_impact_link>  m_links;
};

using  input_impacts_of_threads = std::unordered_map<thread_id,std::vector<std::vector<input_impact_value> > >;
using  input_impact_links_of_threads = std::unordered_map<thread_id,std::vector<std::unordered_set<input_impact_link> > >;

struct  recovery_properties
{
    recovery_properties(
            uint64_t const  heap_begin,
            uint64_t const  heap_end,
            uint64_t const  temporaries_begin,
            important_code_ranges const&  important_code,
            uint64_t const  timeout_in_milliseconds
            );

    uint64_t  heap_begin() const noexcept { return m_heap_begin; }
    uint64_t  heap_end() const noexcept { return m_heap_end; }
    uint64_t temporaries_begin() const noexcept { return m_temporaries_begin; }

    important_code_ranges const&  important_code() const noexcept { return m_important_code; }
    void  add_important_code_range(uint64_t const  begin, uint64_t const end);

    uint64_t  timeout_in_milliseconds() const noexcept { return m_timeout_in_milliseconds; }
    std::chrono::time_point<std::chrono::system_clock> const&  start_time() const noexcept { return m_start_time; }
    uint64_t  passed_milliseconds() const { return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start_time()).count(); }

    std::unordered_map<node_id,switch_info> const&  switches() const noexcept { return m_switches; }
    bool  has_switch(node_id const  head_node) const { return m_switches.count(head_node) != 0ULL; }
    switch_info const&  get_switch(node_id const  head_node) const;
    switch_info&  get_switch(node_id const  head_node);
    std::unordered_map<node_id,switch_info>::iterator  add_switch(node_id const  head_node);

    std::unordered_map<node_id,unexplored_info> const&  unexplored() const noexcept { return m_unexplored; }
    void  update_unexplored(node_id const  just_visited/*, std::vector<node_id> const&  successors_of_just_visited*/);
    void  add_unexplored_exits(address const  IP, std::unordered_set<node_id> const&  exits);

    std::vector<nodes_history_of_threads> const&  node_histories() const noexcept { return m_node_histories; }
    node_counter_type  node_couter(execution_id const  eid, thread_id const  tid);
    void  insert_node_to_history(execution_id const  eid, thread_id const  tid, node_id const  node);

    uint64_t  num_executions_performed() const { return m_threads_of_executions.size(); }

    void  on_new_thread(execution_id const  eid, thread_id const  tid);
    std::vector<thread_id> const&  threads_of_execution(execution_id const  eid) const;
    bool  execution_has_thread(execution_id const  eid, thread_id const  tid) const;

    void  on_thread_step(execution_id const  eid, thread_id const  tid);
    std::vector<thread_id> const&  interleaving_of_threads(execution_id const  eid) const;

    void  on_concurrent_group_begin(execution_id const  eid);
    std::vector<node_counter_type> const&  begins_of_concurrent_groups(execution_id const  eid) const;
    bool  is_begin_of_concurrent_group(execution_id const  eid, node_counter_type const  counter) const;

    std::vector<branchings_of_threads> const&  branchings() const noexcept { return m_branchings; }
    void  insert_branching(execution_id const  eid, thread_id const  tid, node_counter_type const  counter);
    void  insert_branching(execution_id const  eid, thread_id const  tid) { insert_branching(eid,tid,node_couter(eid,tid)); }

    std::unordered_set<edge_id> const&  visited_branchings() const noexcept { return m_visided_branchings; }
    void  on_branching_visited(edge_id const  eid);

    std::vector<input_impacts_of_threads> const&  input_impacts() const noexcept { return m_input_impacts; }
    input_impact_link  add_input_impact(input_impact_value const&  value, execution_id const  eid, thread_id const  tid, node_counter_type const  counter);
    input_impact_link  add_input_impact(input_impact_value const&  value, execution_id const  eid, thread_id const  tid)
    { return add_input_impact(value,eid,tid,node_couter(eid,tid)); }
    input_impact_value const*  find_input_impact_value(execution_id const  eid, thread_id const  tid, input_impact_link const&  link) const;

    std::vector<input_impact_links_of_threads> const&  input_impact_links() const noexcept { return m_input_impact_links; }
    void  add_input_impact_links(execution_id const  eid, thread_id const  tid, std::vector<input_impact_link> const&  links, node_counter_type const  counter);
    void  add_input_impact_links(execution_id const  eid, thread_id const  tid, std::vector<input_impact_link> const&  links)
    { return add_input_impact_links(eid,tid,links,node_couter(eid,tid)); }

private:
    uint64_t  m_heap_begin;
    uint64_t  m_heap_end;
    uint64_t  m_temporaries_begin;
    important_code_ranges  m_important_code;
    uint64_t  m_timeout_in_milliseconds;
    std::chrono::time_point<std::chrono::system_clock>  m_start_time;
    std::unordered_map<node_id,switch_info>  m_switches;
    std::unordered_map<node_id,unexplored_info>  m_unexplored;
    std::vector<nodes_history_of_threads>  m_node_histories;
    std::vector< std::vector<thread_id> >  m_threads_of_executions;
    std::vector< std::vector<thread_id> >  m_interleaving_of_threads;
    std::vector< std::vector<node_counter_type> >  m_begins_of_concurrent_groups;
    std::vector<branchings_of_threads>  m_branchings;
    std::unordered_set<edge_id>  m_visided_branchings;
    std::vector<input_impacts_of_threads>  m_input_impacts;
    std::vector<input_impact_links_of_threads>  m_input_impact_links;
};


using  recovery_properties_ptr = std::shared_ptr<recovery_properties>;


bool is_inside_important_code(address const  adr, important_code_ranges const&  important_code);


}}


#endif
