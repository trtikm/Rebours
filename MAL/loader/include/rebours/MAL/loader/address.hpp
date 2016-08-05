#ifndef REBOURS_MAL_LOADER_ADDRESS_HPP_INCLUDED
#   define REBOURS_MAL_LOADER_ADDRESS_HPP_INCLUDED

#   include <cstdint>

namespace loader {


/**
 * We define an address into process's virtual address space as 64-bit unsigned integer.
 * This definition is NOT dependent on processor architectures of a loaded process. It means
 * that a loaded process can be for a 32-bit architecture (i.e. pointers inside the loaded
 * sections are 32-bit), but accesses to the loaded section from data stuctures of the Loader
 * are always 64-bit numbers. Also, pointers readed from sections are converted to 64-bit numbers.
 */
typedef uint64_t  address;


}

#endif
