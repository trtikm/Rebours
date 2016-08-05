#ifndef REBOURS_MAL_RELOADER_RELOAD_HPP_INCLUDED
#   define REBOURS_MAL_RELOADER_RELOAD_HPP_INCLUDED

#   include <rebours/program/program.hpp>
#   include <rebours/program/assembly.hpp>
#   include <rebours/MAL/descriptor/storage.hpp>
#   include <string>

namespace mal { namespace reloader {


descriptor::file_descriptor_ptr  reload(microcode::program const&  prologue,
                                        microcode::annotations const&  annotations,
                                        bool const  reload_also_file_info,
                                        std::string const&  error_message);


}}

#endif
