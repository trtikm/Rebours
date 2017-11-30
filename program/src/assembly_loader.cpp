#include <rebours/program/assembly.hpp>
#include <rebours/utility/assumptions.hpp>
#include <rebours/utility/invariants.hpp>
#include <rebours/utility/development.hpp>
#include <iostream>

namespace microcode { namespace detail {



}}

namespace microcode {


std::pair<std::unique_ptr<program>,std::unique_ptr<annotations> >  create_program_from_assembly_text(std::istream&  assembly_text, std::string&  error_message)
{
    (void)assembly_text;
    (void)error_message;

    NOT_IMPLEMENTED_YET();
}


}
