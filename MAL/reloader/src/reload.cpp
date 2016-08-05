#include <rebours/MAL/reloader/reload.hpp>
#include <rebours/MAL/reloader/assumptions.hpp>
#include <rebours/MAL/reloader/invariants.hpp>

namespace mal { namespace reloader {


descriptor::file_descriptor_ptr  reload(microcode::program const&  prologue,
                                        microcode::annotations const&  annotations,
                                        bool const  reload_also_file_info,
                                        std::string const&  error_message)
{
    ASSUMPTION(false); // not implemented yet!
}


}}
