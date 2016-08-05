#ifndef REBOURS_MAL_LOADER_WARNINGS_HPP_INCLUDED
#   define REBOURS_MAL_LOADER_WARNINGS_HPP_INCLUDED

#   include <map>
#   include <vector>
#   include <string>
#   include <memory>

namespace loader {


/**
 * This dictionary holds for each loaded binary a list of all detected warnings
 * describing inconsistences in a structure or content of the loaded binary.
 */
typedef std::map<std::string,               //!< Binary file path-name.
                std::vector<std::string>    //!< Warnings occured during the load of the binary.
                >
        warnings;

typedef std::shared_ptr<warnings const>  warnings_ptr;


}

#endif
