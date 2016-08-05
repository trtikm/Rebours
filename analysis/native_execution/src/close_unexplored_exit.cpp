#include <rebours/analysis/native_execution/exploration.hpp>
#include <rebours/analysis/native_execution/msgstream.hpp>
#include <rebours/analysis/native_execution/assumptions.hpp>
#include <rebours/analysis/native_execution/invariants.hpp>
#include <rebours/analysis/native_execution/development.hpp>

namespace analysis { namespace natexe {


void  close_unexplored_exit(microcode::program&  program, recovery_properties&  rprops, node_id const  exit)
{
    uint64_t const  component_index = microcode::find_component(program,exit);
    INVARIANT(component_index < program.num_components());
    microcode::program_component&  C = program.component(component_index);

    C.insert_sequence(exit,{microcode::create_MISCELLANEOUS__STOP()});
    rprops.update_unexplored(exit);
}


}}
