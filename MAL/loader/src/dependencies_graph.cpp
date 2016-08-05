#include <rebours/MAL/loader/dependencies_graph.hpp>
#include <rebours/MAL/loader/assumptions.hpp>

namespace loader {


dependencies_graph::dependencies_graph(std::string const&  root_file,
                                     std::shared_ptr<file_dependencies_map const> const  forward_dependencies,
                                     std::shared_ptr<file_dependencies_map const> const  backward_dependencies)
    : m_root_file(root_file)
    , m_forward_dependencies(forward_dependencies)
    , m_backward_dependencies(backward_dependencies)
{
    ASSUMPTION(m_forward_dependencies.operator bool());
    ASSUMPTION(m_forward_dependencies->find(m_root_file) != m_forward_dependencies->cend());

    ASSUMPTION(m_backward_dependencies.operator bool());
    ASSUMPTION(m_backward_dependencies->find(m_root_file) != m_backward_dependencies->cend());
    ASSUMPTION(m_backward_dependencies->find(m_root_file)->second.empty());
}


std::vector<std::string> const&  dependencies_graph::files_loaded_by(std::string const&  file) const
{
    auto const  it = m_forward_dependencies->find(file);
    ASSUMPTION(it != m_forward_dependencies->cend());
    return it->second;
}

std::vector<std::string> const&  dependencies_graph::files_loading_file(std::string const&  file) const
{
    auto const  it = m_backward_dependencies->find(file);
    ASSUMPTION(it != m_backward_dependencies->cend());
    return it->second;
}



}
