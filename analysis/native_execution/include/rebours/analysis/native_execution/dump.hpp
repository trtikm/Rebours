#ifndef REBOURS_ANALYSIS_NATIVE_EXECUTION_DUMP_HPP_INCLUDED
#   define REBOURS_ANALYSIS_NATIVE_EXECUTION_DUMP_HPP_INCLUDED

#   include <rebours/analysis/native_execution/execution_properties.hpp>
#   include <rebours/analysis/native_execution/recovery_properties.hpp>
#   include <rebours/program/program.hpp>
#   include <vector>
#   include <string>
#   include <map>
#   include <utility>
#   include <cstdint>

namespace analysis { namespace natexe {


bool  dump_enabled();
std::string  dump_root_directory();
std::string  dump_directory();
void  dump_push_directory(std::string const&  new_dir);
void  dump_pop_directory();

struct dump_push_guard
{
    dump_push_guard(std::string const&  new_dir, bool const  allow_to_enable_dumping = false);
    ~dump_push_guard();
    std::string const&  dir() const noexcept { return m_dir; }
private:
    std::string  m_dir;
};

void  dump_add_file(std::string const&  description, std::string const&  pathname);
void  dump_create_root_file(std::string const&  error_message, std::string const&  dump_subdir = "");


}}

namespace analysis { namespace natexe {


std::string  dump_memory_allocations(memory_allocations const&  alloc, std::string const&  filename);
std::string  dump_memory_content(memory_content const&  content, std::string const&  filename, memory_allocations const* const  alloc = nullptr);

std::string  dump_content_of_registers(memory_content const&  content,
                                       std::map<std::pair<uint64_t,uint64_t>,std::string> const&  ranges_to_registers,
                                       std::string const&  filename);

std::string  dump_stream_content(stream_content const&  content, std::string const&  filename, stream_allocations const* const  alloc = nullptr);
std::string  dump_streams(stream_allocations const&  alloc, contents_of_streams const&  contents, std::string const&  filename);

std::string  dump_interrupts(interrupts_vector_table const&  ivt, std::string const&  filename);

//std::string  dump_probe_properties(
//        probe_properties const&  pprops,
//        microcode::program const&  P
//        );

std::string  dump_control_switches(
        std::unordered_map<node_id,switch_info> const&  switches,
        important_code_ranges const&  important_code,
        std::string const&  filename
        );

std::string  dump_unexplored_exits(
        std::unordered_map<node_id,unexplored_info> const&  unexplored,
        important_code_ranges const&  important_code,
        std::string const&  filename);

std::string  dump_branchings_of_executed_threads(
        std::vector<branchings_of_threads> const&  branchings,
        std::vector<nodes_history_of_threads> const&  node_histories,
        std::string const&  filename);

std::string  dump_visited_branchings(
        std::unordered_set<edge_id> const&  branchings,
        std::string const&  filename);

std::string  dump_node_histories_of_executed_threads(
        std::vector<nodes_history_of_threads> const&  node_histories,
        std::string const&  filename);

std::string  dump_nodes_history_of_one_executed_thread(
        std::vector<node_id> const&  nodes,
        std::string const&  filename);

std::string  dump_threads_of_performed_executions(
        recovery_properties const&  rprops,
        std::string const&  filename);

std::string  dump_interleaving_of_threads(
        recovery_properties const&  rprops,
        std::string const&  filename);

std::string  dump_input_impacts_of_one_executed_thread(
        std::vector<std::vector<input_impact_value> > const&  impacts,
        std::vector<node_id> const&  nodes_history,
        std::string const&  filename);

std::string  dump_input_impacts_of_executed_threads(
        std::vector<input_impacts_of_threads> const&  impacts,
        std::vector<nodes_history_of_threads> const&  node_histories,
        std::string const&  filename);

std::string  dump_input_impact_links_of_one_executed_thread(
        std::vector<std::unordered_set<input_impact_link> > const&  links,
        std::vector<node_id> const&  nodes_history,
        std::string const&  filename);

std::string  dump_input_impact_links_of_executed_threads(
        std::vector<input_impact_links_of_threads> const&  links,
        std::vector<nodes_history_of_threads> const&  node_histories,
        std::string const&  filename);

std::string  dump_recovery_properties(
        recovery_properties const&  rprops,
        microcode::program const&  P
        );

std::string  dump_input_frontier_of_thread(
        input_frontier_of_thread const&  frontier,
        std::vector<node_id> const* const  nodes_history,
        std::string const&  filename);

std::string  dump_thread_properties(
        microcode::program const&  P,
        thread const&  thd,
        execution_properties const&  eprops,
        recovery_properties const&  rprops,
        std::map<std::pair<uint64_t,uint64_t>,std::string> const&  ranges_to_registers,
        std::string const&  filename);

std::string  dump_execution_state(
        microcode::program const&  P,
        execution_properties const&  eprops,
        recovery_properties const&  rprops,
        //probe_properties const* const  pprops,
        std::vector<thread> const&  thds,
        index const  fail_thread_index,
        std::string const&  error_message,
        std::string const&  dump_subdir = "",
        std::map<std::pair<uint64_t,uint64_t>,std::string> const&  ranges_to_registers = {}
        );

std::string  dump_at_breakpoint(
        microcode::program const&  P,
        execution_properties const&  eprops,
        recovery_properties const&  rprops,
        //probe_properties const* const  pprops,
        thread const&  thd,
        node_id const  node,
        uint64_t const  hit_number,
        std::string const&  dump_subdir,
        std::map<std::pair<uint64_t,uint64_t>,std::string> const&  ranges_to_registers = {}
        );


}}

#endif
