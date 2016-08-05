#ifndef REBOURS_MAL_LOADER_INIT_FINI_HPP_INCLUDED
#   define REBOURS_MAL_LOADER_INIT_FINI_HPP_INCLUDED

#   include <rebours/MAL/loader/address.hpp>
#   include <vector>
#   include <unordered_map>
#   include <memory>

namespace loader {


/**
 * It is a dictionary where keys are path-names of loaded binary files and values are
 * vectors of entry points (start addresses) of special functions inside those binaries.
 * A special function is either initialisation or termination function of the loaded file.
 * One dictionary may store either initialisation or termination function. Mixing of
 * initialisation or termination functions in one dictionary is not allowed. Special
 * functions of a binary file must be called in the order as they appear the vector.
 * An order in which special functions of different loaded files are called is given
 * by a dependency graph (see file Loader/dependencies_graph.hpp).
 *
 * Note that the entry point to the root binary (i.e. the 'main' function) is also
 * considered as a special (initialisation) function of that binary file. It always
 * appears at the last position in the vector of that file.
 */
typedef  std::unordered_map< std::string,std::vector<address> >  special_functions_of_files;

typedef std::shared_ptr<special_functions_of_files const> init_functions_ptr;
typedef std::shared_ptr<special_functions_of_files const> fini_functions_ptr;


}

#endif
