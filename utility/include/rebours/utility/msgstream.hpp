#ifndef MSGSTREAM_HPP_INCLUDED
#   define MSGSTREAM_HPP_INCLUDED

#   include <memory>
#   include <sstream>


struct msgstream
{
    msgstream() : m_buffer(new std::ostringstream) {}
    template<typename T>
    msgstream operator<<(T const& value) const { *m_buffer << value; return *this;}
    std::ios_base::fmtflags  flags() const { return m_buffer->flags(); }
    operator std::string() const { return str(); }
    std::string str() const { return m_buffer->str(); }
    struct end {};
    std::string  operator<<(end const) const { return str();}
private:
    std::shared_ptr<std::ostringstream>  m_buffer;
};


#endif
