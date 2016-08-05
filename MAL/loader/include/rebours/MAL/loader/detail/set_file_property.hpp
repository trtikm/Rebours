#ifndef SET_FILE_PROPERTY_HPP_INCLUDED
#   define SET_FILE_PROPERTY_HPP_INCLUDED

#   include <rebours/MAL/loader/file_props.hpp>
#   include <string>

namespace loader { namespace detail {


inline void  set_file_property(file_property_map_ptr const  property_map,
                               std::string const&  property_name,
                               std::string const&  property_value)
{
    (*std::const_pointer_cast<file_property_map>(property_map))[property_name] = property_value;
}


}}

#endif
