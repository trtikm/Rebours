#include <rebours/analysis/native_execution/exploration.hpp>
#include <rebours/analysis/native_execution/msgstream.hpp>
#include <rebours/analysis/native_execution/assumptions.hpp>
#include <rebours/analysis/native_execution/invariants.hpp>
#include <rebours/analysis/native_execution/development.hpp>

namespace analysis { namespace natexe {


std::string  find_traces_related_to_next_goal(microcode::program const&  prologue, microcode::program const&  program, recovery_properties const&  rprops,
                                              edge_id const  next_goal, std::vector< std::pair<std::pair<execution_id,thread_id>,node_counter_type> >&  traces)
{
    (void)prologue;
    (void)program;
    for (index  eid = 0ULL; eid < rprops.num_executions_performed(); ++eid)
        for (thread_id const  tid : rprops.threads_of_execution(eid))
            for (node_counter_type const  cnt : rprops.branchings().at(eid).at(tid))
                if (rprops.node_histories().at(eid).at(tid).at(cnt) == next_goal.first)
                    traces.push_back({{eid,tid},cnt});
    INVARIANT(!traces.empty());
    return "";
}


}}
