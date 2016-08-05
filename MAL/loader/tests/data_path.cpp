#include "./data_path.hpp"
#include <rebours/MAL/loader/file_utils.hpp>

#define xstr(s) str(s)
#define str(s) #s

std::string  data_path()
{
    static std::string const  data_path = normalise_path( absolute_path( xstr(TESTS_DATA_PATH) ) );
    return data_path;
}

std::string  benchmarks_path()
{
    return concatenate_file_paths(data_path(),"benchmarks/bin");
}
