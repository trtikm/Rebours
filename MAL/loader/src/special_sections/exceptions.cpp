#include <rebours/MAL/loader/special_sections/exceptions.hpp>

namespace loader { namespace special_section {


exceptions::exceptions(address const  start_address, address const  end_address,
                       std::vector< std::tuple<address,address,address> > const&  records)
    : special_section_properties(start_address,end_address,"An address-sorted table of exception handlers.")
    , m_records(records)
{}


}}
