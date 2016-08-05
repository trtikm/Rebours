#ifndef REBOURS_MAL_LOADER_SPECIAL_SECTION_EXCEPTIONS_HPP_INCLUDED
#   define REBOURS_MAL_LOADER_SPECIAL_SECTION_EXCEPTIONS_HPP_INCLUDED

#   include <rebours/MAL/loader/special_sections/special_section_properties.hpp>
#   include <vector>
#   include <tuple>

namespace loader { namespace special_section {


struct exceptions : public special_section_properties
{
    exceptions(address const  start_address, address const  end_address,
               std::vector< std::tuple<address,address,address> > const&  records);

    uint64_t  num_records() const { return m_records.size(); }

    address  start_address(uint64_t const  index) const { return std::get<0>(m_records.at(index)); }
    address  end_address(uint64_t const  index) const { return std::get<1>(m_records.at(index)); }
    address  unwind_info_address(uint64_t const  index) const { return std::get<2>(m_records.at(index)); }

private:
    std::vector< std::tuple<address,address,address> >  m_records;
};

typedef std::shared_ptr<exceptions const>  exceptions_ptr;

}}

#endif
