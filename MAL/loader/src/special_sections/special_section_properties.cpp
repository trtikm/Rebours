#include <rebours/MAL/loader/special_sections/special_section_properties.hpp>

namespace loader {


special_section_properties::special_section_properties(
        address const  start_address,
        address const  end_address,
        std::string const&  description)
    : m_start_address(start_address)
    , m_end_address(end_address)
    , m_description(description)
{}


}
