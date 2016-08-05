#ifndef REBOURS_BITVECTORS_DETAIL_FILE_UTILS_HPP_INCLUDED
#   define REBOURS_BITVECTORS_DETAIL_FILE_UTILS_HPP_INCLUDED

#   include <vector>
#   include <string>

namespace bv { namespace detail {


void  enumerate_files_by_pattern(const std::string&  pattern, std::vector<std::string>&  output_pathnames);

bool  delete_file(const std::string&  pathname);

}}

#endif
