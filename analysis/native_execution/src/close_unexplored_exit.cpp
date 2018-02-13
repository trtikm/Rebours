#include <rebours/analysis/native_execution/exploration.hpp>
#include <rebours/utility/msgstream.hpp>
#include <rebours/utility/assumptions.hpp>
#include <rebours/utility/invariants.hpp>
#include <rebours/utility/development.hpp>

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
