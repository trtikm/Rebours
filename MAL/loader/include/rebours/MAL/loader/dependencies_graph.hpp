#ifndef DEPENDENCIES_GRAPH_HPP_INCLUDED
#   define DEPENDENCIES_GRAPH_HPP_INCLUDED

#   include <string>
#   include <vector>
#   include <map>
#   include <memory>

namespace loader {


/**
 * Keys and values in this dictionary are full path-names of binary files.
 * For a given key there is stored a vector of values the key depends on
 * all of them. For example, if a binary file A depends on binary files B and C,
 * then the dictionary stores the vector [B,C] for the key A.
 *
 * In general, dependencies between binary files may by cyclic. So, the dictionary
 * can represent arbitrary directed graph. Nevertheless, dependencies are typically
 * acyclic in practice. The dictionary thus typically represents only a DAG.
 */
typedef std::map<std::string,std::vector<std::string> >  file_dependencies_map;


/**
 * It is basically a pair of directed graphs represented by the type 'file_dependencies_map'
 * defined above (please, read its description now). The first graph defines 'forward dependencies',
 * i.e. what binary file depend on (loads) what other binary files. The second graph defines 'backward dependencies',
 * i.e. what binary file is necessary for (is loaded by) what other binary files. In other words, both graphs differ
 * only in the orientation of their edges.
 *
 * Since the load always starts with a single binary file, we mark that binary file as the 'root file'
 * in this data structure.
 *
 * Note that all data are read-only.
 */
struct dependencies_graph
{
    /**
     * It takes a (shared) ownership of data passed as arguments, except the 'root_file'
     * which is coppied.
     */
    dependencies_graph(std::string const&  root_file,
                      std::shared_ptr<file_dependencies_map const> const  forward_dependencies,
                      std::shared_ptr<file_dependencies_map const> const  backward_dependencies);

    std::string const&  root_file() const { return m_root_file; }
    std::vector<std::string> const&  files_loaded_by(std::string const&  file) const;
    std::vector<std::string> const&  files_loading_file(std::string const&  file) const;

    std::shared_ptr<file_dependencies_map const>  forward_dependencies() const { return m_forward_dependencies; }
    std::shared_ptr<file_dependencies_map const>  backward_dependencies() const { return m_backward_dependencies; }

private:
    std::string  m_root_file;
    std::shared_ptr<file_dependencies_map const>  m_forward_dependencies;
    std::shared_ptr<file_dependencies_map const>  m_backward_dependencies;
};


typedef std::shared_ptr<dependencies_graph const>  dependencies_graph_ptr;


}


#endif
