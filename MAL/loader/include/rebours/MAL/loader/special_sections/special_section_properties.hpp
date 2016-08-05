#ifndef REBOURS_MAL_LOADER_SPECIAL_SECTIONS_SPECIAL_SECTION_PROPERTIES_HPP_INCLUDED
#   define REBOURS_MAL_LOADER_SPECIAL_SECTIONS_SPECIAL_SECTION_PROPERTIES_HPP_INCLUDED

#   include <rebours/MAL/loader/address.hpp>
#   include <string>
#   include <memory>

namespace loader {

/**
 * It is a base class for all special sections (see the header file Loader/special_sections.hpp).
 */
struct special_section_properties
{
    special_section_properties(address const  start_address,
                               address const  end_address,
                               std::string const&  description = "" //!< A textual description describing purpose
                                                                    //!< of a special section derived from this class.
                               );
    virtual ~special_section_properties() {}

    address  start_address() const noexcept { return m_start_address; }
    address  end_address() const noexcept { return m_end_address; }
    std::string const&  description() const noexcept { return m_description; }

private:
    address const  m_start_address;
    address const  m_end_address;
    std::string  m_description;
};

typedef std::shared_ptr<special_section_properties const>  special_section_properties_ptr;


}

#endif
