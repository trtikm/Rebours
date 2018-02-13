#include <rebours/analysis/native_execution/exploration.hpp>
#include <rebours/utility/msgstream.hpp>
#include <rebours/utility/assumptions.hpp>
#include <rebours/utility/invariants.hpp>
#include <rebours/utility/development.hpp>
#include <rebours/utility/msgstream.hpp>

namespace analysis { namespace natexe {


std::string  compute_input_for_reaching_next_goal(microcode::program const&  prologue, microcode::program const&  program, recovery_properties const&  rprops,
                                                  edge_id const  next_goal, std::vector< std::pair<std::pair<execution_id,thread_id>,node_counter_type> > const&  traces,
                                                  std::unordered_map<stream_id,std::vector<uint8_t> > const&  input_streams)
{
    (void)prologue;
    (void)program;
    (void)next_goal;
    std::vector< std::pair<std::pair<execution_id,thread_id>,node_counter_type> >  indirect_dependences;
    for (std::pair<std::pair<execution_id,thread_id>,node_counter_type> const&  eid_tid__cnt : traces)
    {
        execution_id const  eid = eid_tid__cnt.first.first;
        thread_id const  tid = eid_tid__cnt.first.second;
        node_counter_type const  cnt = eid_tid__cnt.second;

        std::unordered_set<input_impact_link> const&  links = rprops.input_impact_links().at(eid).at(tid).at(cnt);
        if (!links.empty())
        {
            // TODO: build path formula (a bitvector expression) and generate input using an SMT solver.

            return msgstream() << "ERROR in 'analysis::natexe::compute_input_for_reaching_next_goal()': building of path formula (for taking the unexplored branch {"
                               << next_goal.first << "," << next_goal.second << "}) is NOT IMPLEMENTED YET!";
        }
        else
        {
            return msgstream() << "ERROR in 'analysis::natexe::compute_input_for_reaching_next_goal()': cannot determine indirect input dependences (for taking the unexplored branch {"
                               << next_goal.first << "," << next_goal.second << "}), because computation of constant dependency links of program instructions is NOT IMPLEMENTED YET!";

            for (node_counter_type const  branch_cnt : rprops.branchings().at(eid).at(tid))
                // TODO: exclude branches taken before all constant dependences of the 'next_goal' edge.
            {
                if (branch_cnt >= cnt)
                    break;
                if (!rprops.input_impact_links().at(eid).at(tid).at(branch_cnt).empty())
                    indirect_dependences.push_back({{eid,tid},branch_cnt});
            }
        }
    }
    if (indirect_dependences.empty())
        return "";

    // TODO: guess new input based on input-output relations denoted by 'indirect_dependences' vector.

    return msgstream() << "ERROR in 'analysis::natexe::compute_input_for_reaching_next_goal()': guessing of new input (for taking the unexplored branch {"
                       << next_goal.first << "," << next_goal.second << "}) based on indirect input dependences is NOT IMPLEMENTED YET!";
}


}}
