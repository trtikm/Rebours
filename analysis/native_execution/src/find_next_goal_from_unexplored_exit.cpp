#include <rebours/analysis/native_execution/exploration.hpp>
#include <rebours/analysis/native_execution/msgstream.hpp>
#include <rebours/analysis/native_execution/assumptions.hpp>
#include <rebours/analysis/native_execution/invariants.hpp>
#include <rebours/analysis/native_execution/development.hpp>
#include <deque>

namespace analysis { namespace natexe {


std::string  find_next_goal_from_unexplored_exit(microcode::program const&  program, recovery_properties const&  rprops, node_id const  exit, edge_id&  next_goal)
{
    uint64_t const  component_index = microcode::find_component(program,exit);
    INVARIANT(component_index < program.num_components());
    microcode::program_component const&  C = program.component(component_index);

    std::deque<node_id>  queue{exit};
    std::unordered_set<node_id,microcode::program_component::graph_type::node_id_hasher_type>  visited;
    do
    {
        node_id const  u = queue.front();
        queue.pop_front();

        if (visited.count(u) != 0ULL)
            continue;
        visited.insert(u);

        std::vector<node_id> const&  succ = C.successors(u);
        if (succ.size() == 2ULL)
        {
            bool const  uf = rprops.visited_branchings().count({u,succ.front()});
            bool const  ub = rprops.visited_branchings().count({u,succ.back()});
            if (uf && !ub)
            {
                next_goal = {u,succ.back()};
                return "";
            }
            if (!uf && ub)
            {
                next_goal = {u,succ.front()};
                return "";
            }
        }

        for (node_id const v : C.predecessors(u))
            queue.push_back(v);
    }
    while (!queue.empty());

    return msgstream() << "Cannot find goal edge from the unexplored exit node " << exit << ".";
}


}}
