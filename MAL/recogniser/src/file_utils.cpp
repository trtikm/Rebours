
#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <sstream>
#include <vector>

#if defined(WIN32)
#   include <windows.h>
#elif defined(__linux__) || defined(__APPLE__)
#   include <sys/stat.h>
#endif

#   ifndef DISABLE_ASSUMPTION_CHECKING
#       include <cassert>
#       define ASSUMPTION(C) assert(C)
#   else
#       define ASSUMPTION(C) {}
#   endif

#   ifndef DISABLE_INVARIANT_CHECKING
#       include <cassert>
#       define INVARIANT(C) assert(C)
#       define UNREACHABLE() do { assert(false); throw 0; } while (false)
#   else
#       define INVARIANT(C) {}
#       define UNREACHABLE() throw 0
#   endif

namespace fileutl { namespace {


std::string::size_type  parse_last_dir_pos(std::string const&  file_pathname)
{
    std::string::size_type const  last_slash_pos = file_pathname.find_last_of('/');
    std::string::size_type const  last_backslash_pos = file_pathname.find_last_of('\\');
    std::string::size_type const  last_dir_pos =
            last_slash_pos == std::string::npos ?
                   (last_backslash_pos == std::string::npos ? 0ULL :
                                                              last_backslash_pos + 1ULL) :
                   (last_backslash_pos == std::string::npos ? last_slash_pos + 1ULL :
                                                              (last_slash_pos < last_backslash_pos ? last_backslash_pos :
                                                                                                     last_slash_pos))
                   ;
    INVARIANT(last_dir_pos <= file_pathname.size());
    return last_dir_pos;
}


}}

namespace fileutl {


bool  file_exists(std::string const&  pathname)
{
    std::ifstream  f{pathname,std::ios::binary};
    return f.good();
}

bool  is_directory(std::string const&  pathname)
{
    if (!file_exists(pathname))
        return false;
#   if defined(WIN32)
    return PathIsDirectory(pathname.c_str());
#   elif defined(__linux__) || defined(__APPLE__)
    struct stat buf;
    stat(pathname.c_str(), &buf);
    return S_ISDIR(buf.st_mode);
    //return S_ISREG(buf.st_mode); // is file ?
#   else
#       error "Unsuported platform."
#   endif
}

uint64_t  file_size(std::string const&  file_pathname)
{
    std::ifstream  f{file_pathname,std::ios::binary};
    std::streampos const  begin  = f.tellg();
    f.seekg(0ULL,std::ios::end);
    std::streampos const  end = f.tellg();
    return end - begin;
}

std::string  parse_name_in_pathname(std::string const&  file_pathname)
{
    return file_pathname.substr(parse_last_dir_pos(file_pathname));
}

std::string  parse_path_in_pathname(std::string const&  file_pathname)
{
    return file_pathname.substr(0U,parse_last_dir_pos(file_pathname));
}

void  create_directory(std::string const&  pathname)
{
#   if defined(WIN32)
        std::system((std::string("mkdir \"") + pathname + "\"").c_str());
#   elif defined(__linux__) || defined(__APPLE__)
    auto ignore = std::system((std::string("mkdir -p \"") + pathname + "\"").c_str());
    (void)ignore;
#   else
#       error "Unsuported platform."
#   endif
}

std::string  concatenate_file_paths(std::string const&  left_path, std::string const&  right_path)
{
    if (left_path.empty())
        return right_path;
    if (right_path.empty())
        return left_path;
    return left_path + "/" + right_path;
}

std::string  absolute_path(std::string const&  path)
{
    // TODO: portability - this implementation won't probably work on Windows...
    ASSUMPTION(file_exists(path));
    std::vector<char> buffer(10000,0);
    return realpath(path.c_str(),&buffer.at(0));
}

std::string  normalise_path(std::string const&  path)
{
    std::string  result = path;
    std::replace(result.begin(),result.end(),'\\','/');
    std::string::size_type  pos = 0;
    while((pos = result.find("/./",0)) != std::string::npos)
        result.replace(pos,3,"/");
    // TODO: more fixes should be applied (e.g. /../, //, etc.)
    return result;
}

void  split_pathname(std::string const&  pathname, std::vector<std::string>& output)
{
    std::istringstream  istr(normalise_path(pathname));
    std::string  token;
    while (std::getline(istr,token,'/'))
        output.push_back(token);
}

std::string  join_path_parts(std::vector<std::string> const&  parts)
{
    if (parts.empty())
        return "";
    std::string  result = parts.at(0);
    for (uint64_t  i = 1ULL; i < parts.size(); ++i)
    {
        result.push_back('/');
        result.append(parts.at(i));
    }
    return result;
}

std::string  get_common_preffix(std::string const&  pathname1, std::string const&  pathname2)
{
    std::vector<std::string>  split1;
    split_pathname(pathname1,split1);

    std::vector<std::string>  split2;
    split_pathname(pathname2,split2);

    std::vector<std::string>  common_split;
    for (uint64_t  i = 0ULL, size = std::min(split1.size(),split2.size());
         i < size && split1.at(i) == split2.at(i);
         ++i)
        common_split.push_back(split1.at(i));

    std::string const  result = join_path_parts(common_split);
    return result;
}

std::string  get_relative_path(std::string const&  pathname, std::string const&  directory)
{
    std::vector<std::string>  split1;
    split_pathname(pathname,split1);
    std::reverse(split1.begin(),split1.end());

    std::vector<std::string>  split2;
    split_pathname(directory,split2);
    std::reverse(split2.begin(),split2.end());

    while (!split1.empty() && !split2.empty() && split1.back() == split2.back())
    {
        split1.pop_back();
        split2.pop_back();
    }

    std::reverse(split1.begin(),split1.end());
    std::string const  result = join_path_parts(split1);
    return result;
}


}
