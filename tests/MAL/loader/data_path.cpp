#include "./data_path.hpp"
#include <rebours/utility/file_utils.hpp>

#define xstr(s) str(s)
#define str(s) #s

std::string  data_path()
{
    //static std::string const  data_path = fileutl::normalise_path( fileutl::absolute_path( std::string(xstr(TESTS_DATA_PATH)) ) );
    //return data_path;
    return "./";
}

std::string  benchmarks_path()
{
    return fileutl::concatenate_file_paths(data_path(),"benchmarks/bin");
}
