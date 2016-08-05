#ifndef REBOURS_MAL_PROLOGUE_BUILDER_HPP_INCLUDED
#   define REBOURS_MAL_PROLOGUE_BUILDER_HPP_INCLUDED

#   include <rebours/program/program.hpp>
#   include <rebours/program/assembly.hpp>
#   include <rebours/MAL/descriptor/storage.hpp>
#   include <tuple>
#   include <memory>

namespace mal { namespace prologue {


std::pair<std::unique_ptr<microcode::program>,std::unique_ptr<microcode::annotations> >  build(descriptor::storage const&  description);


}}

#endif
