#include <rebours/MAL/loader/special_sections/thread_local_storage_initialisers.hpp>

namespace loader { namespace special_section {

thread_local_storage_initialisers::thread_local_storage_initialisers(
        address const  start_address,
        address const  end_address,
        std::vector<address> const&  init_functions
        )
    : special_section_properties(start_address,end_address,
                                 "Section contains pointers to init functions of the thread local data.")
    , m_init_functions(init_functions)
{}


}}
