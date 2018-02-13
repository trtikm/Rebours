#include <rebours/MAL/prologue/builder.hpp>
#include <rebours/utility/assumptions.hpp>
#include <rebours/utility/invariants.hpp>

namespace mal { namespace prologue { namespace detail {


namespace x86_64_Linux {
std::pair<std::unique_ptr<microcode::program>,std::unique_ptr<microcode::annotations> >  build(descriptor::storage const&  description);
}


}}}

namespace mal { namespace prologue {


std::pair<std::unique_ptr<microcode::program>,std::unique_ptr<microcode::annotations> >  build(descriptor::storage const&  description)
{
    if (description.processor() == descriptor::processor_architecture::X86_64 &&
        (description.system() == descriptor::operating_system::LINUX ||
         description.system() == descriptor::operating_system::UNIX))
        return detail::x86_64_Linux::build(description);

    UNREACHABLE();
}


}}
