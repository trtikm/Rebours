#include <rebours/bitvectors/detail/file_utils.hpp>
#include <cstdio>
#   if defined(WIN32)
#       error "NOT IMPLEMENTED YET!"
#   elif defined(__linux__) || defined(__APPLE__)
#       include <glob.h>
#   else
#       error "Unsuported platform."
#   endif

namespace bv { namespace detail {


void  enumerate_files_by_pattern(const std::string&  pattern, std::vector<std::string>&  output_pathnames)
{
    ::glob_t glob_result;
    ::glob(pattern.c_str(),GLOB_TILDE,NULL,&glob_result);
    for(unsigned int i = 0U; i < glob_result.gl_pathc; ++i)
        output_pathnames.push_back(glob_result.gl_pathv[i]);
    globfree(&glob_result);
}

bool  delete_file(const std::string&  pathname)
{
    return std::remove(pathname.c_str()) != 0;
}


}}
