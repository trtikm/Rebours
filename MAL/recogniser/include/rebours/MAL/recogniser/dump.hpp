#ifndef MAL_RECOGNISER_DUMP_HPP_INCLUDED
#   define MAL_RECOGNISER_DUMP_HPP_INCLUDED

#   include <rebours/MAL/recogniser/recognise.hpp>
#   include <string>

namespace mal { namespace recogniser {


bool  dump_details_of_recognised_instruction(recognition_result const&  result, std::string const&  dump_file_pathname);


}}

#endif
