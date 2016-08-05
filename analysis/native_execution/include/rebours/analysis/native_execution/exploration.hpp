#ifndef REBOURS_ANALYSIS_NATIVE_EXPLORATION_HPP_INCLUDED
#   define REBOURS_ANALYSIS_NATIVE_EXPLORATION_HPP_INCLUDED

#   include <rebours/analysis/native_execution/recovery_properties.hpp>
#   include <rebours/program/program.hpp>
#   include <string>
#   include <vector>
#   include <unordered_map>
#   include <utility>
#   include <cstdint>

namespace analysis { namespace natexe {


std::string  merge_recovered_traces(microcode::program&  program, recovery_properties&  rprops);

std::string  choose_next_unexplored_exit(microcode::program const&  program, recovery_properties const&  rprops, node_id&  exit);

std::string  find_next_goal_from_unexplored_exit(microcode::program const&  program, recovery_properties const&  rprops, node_id const  exit, edge_id&  next_goal);

std::string  find_traces_related_to_next_goal(microcode::program const&  prologue, microcode::program const&  program, recovery_properties const&  rprops,
                                              edge_id const  next_goal, std::vector< std::pair<std::pair<execution_id,thread_id>,node_counter_type> >&  traces);

std::string  compute_input_for_reaching_next_goal(microcode::program const&  prologue, microcode::program const&  program, recovery_properties const&  rprops,
                                                  edge_id const  next_goal, std::vector< std::pair<std::pair<execution_id,thread_id>,node_counter_type> > const&  traces,
                                                  std::unordered_map<stream_id,std::vector<uint8_t> > const&  input_streams);

void  close_unexplored_exit(microcode::program&  program, recovery_properties&  rprops, node_id const  exit);

std::string  setup_next_execution_properties(execution_properties&  eprops, std::unordered_map<stream_id,std::vector<uint8_t> > const&  input_streams,
                                             std::vector<uint8_t> const&  default_stack_init_data);


}}

#endif
