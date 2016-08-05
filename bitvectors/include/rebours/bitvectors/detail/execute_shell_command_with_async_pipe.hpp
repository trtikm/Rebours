#ifndef REBOURS_BITVECTORS_DETAIL_EXECUTE_SHELL_COMMAND_WITH_ASYNC_PIPE_HPP_INCLUDED
#   define REBOURS_BITVECTORS_DETAIL_EXECUTE_SHELL_COMMAND_WITH_ASYNC_PIPE_HPP_INCLUDED

#   include <functional>
#   include <string>
#   include <sstream>

namespace bv { namespace detail {


bool  execute_shell_command_with_async_pipe(std::string const& shell_command, std::function<bool()> const&  interrupt,
                                            std::stringstream&  ostr);


}}

#endif
