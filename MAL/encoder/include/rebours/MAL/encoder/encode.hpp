#ifndef REBOURS_MAL_ENCODER_ENCODE_HPP_INCLUDED
#   define REBOURS_MAL_ENCODER_ENCODE_HPP_INCLUDED

#   include <rebours/program/program.hpp>
#   include <rebours/program/assembly.hpp>
#   include <rebours/MAL/descriptor/storage.hpp>
#   include <iosfwd>

namespace mal { namespace encoder {


std::ostream&  encode(std::ostream&  output_stream,
                      descriptor::storage const&  description,
                      microcode::program const&  program,
                      microcode::annotations const* const  annotations
                      );


}}

#endif
