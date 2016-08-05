#ifndef TO_STRING_HPP_INCLUDED
#   define TO_STRING_HPP_INCLUDED

#   include <string>
#   include <sstream>


template<typename type>
std::string  to_string(type const  value, bool const  as_hexadecimal = true)
{
    std::ostringstream ostr;
    if (as_hexadecimal)
        ostr << std::hex << value;
    else
        ostr << value;
    return ostr.str();
}


#endif
