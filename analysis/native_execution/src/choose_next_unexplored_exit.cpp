#include <rebours/analysis/native_execution/exploration.hpp>
#include <rebours/analysis/native_execution/msgstream.hpp>
#include <rebours/analysis/native_execution/assumptions.hpp>
#include <rebours/analysis/native_execution/invariants.hpp>
#include <rebours/analysis/native_execution/development.hpp>

namespace analysis { namespace natexe {


std::string  choose_next_unexplored_exit(microcode::program const&  program, recovery_properties const&  rprops, node_id&  exit)
{
    (void)program;
    for (auto const& u_i : rprops.unexplored())
        if (is_inside_important_code(u_i.second.IP(),rprops.important_code()))
        {
            exit = u_i.first;
            return "";
        }
    for (auto const& u_i : rprops.unexplored())
        if (!is_inside_important_code(u_i.second.IP(),rprops.important_code()))
        {
            exit = u_i.first;
            return "";
        }

    return "Done!";
}


}}
