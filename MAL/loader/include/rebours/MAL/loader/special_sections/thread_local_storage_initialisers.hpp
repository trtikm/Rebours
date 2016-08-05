#ifndef REBOURS_MAL_LOADER_SPECIAL_SECTION_THREAD_LOCAL_STORAGE_INITIALISERS_HPP_INCLUDED
#   define REBOURS_MAL_LOADER_SPECIAL_SECTION_THREAD_LOCAL_STORAGE_INITIALISERS_HPP_INCLUDED

#   include <rebours/MAL/loader/special_sections/special_section_properties.hpp>
#   include <ctime>
#   include <vector>

namespace loader { namespace special_section {


struct thread_local_storage_initialisers : public special_section_properties
{
    thread_local_storage_initialisers(
            address const  start_address,
            address const  end_address,
            std::vector<address> const&  init_functions
            );

    std::vector<address> const&  init_functions() const { return m_init_functions; }

private:
    std::vector<address>  m_init_functions;
};

typedef std::shared_ptr<thread_local_storage_initialisers const>  thread_local_storage_initialisers_ptr;

}}

#endif
