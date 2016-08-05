#ifndef REBOURS_PROGRAM_MICROCODE_ASSEMBLY_HPP_INCLUDED
#   define REBOURS_PROGRAM_MICROCODE_ASSEMBLY_HPP_INCLUDED

#   include <rebours/program/program.hpp>
#   include <tuple>
#   include <unordered_map>
#   include <vector>
#   include <tuple>
#   include <string>
#   include <memory>
#   include <iosfwd>

namespace microcode {


using  annotation = std::pair<std::string,std::string>;
using  node_annotations = std::vector<annotation>;
using  annotations = std::unordered_map<program_component::node_id,node_annotations>;


//void  insert(annotations const&  src, annotations&  dst, bool const  overwrite_old = false);
void  append(annotations const&  src, annotations&  dst);

node_annotations::const_iterator  find(node_annotations const&  A, std::string const&  keyword);
node_annotations::iterator  find(node_annotations&  A, std::string const&  keyword);

std::string const*  find(annotations const&  A, program_component::node_id const  node, std::string const&  keyword);
std::string*  find(annotations&  A, program_component::node_id const  node, std::string const&  keyword);

inline std::string const*  find(annotations const* const  A, program_component::node_id const  node, std::string const&  keyword)
{
    return A == nullptr ? nullptr : find(*A,node,keyword);
}

inline std::string*  find(annotations* const  A, program_component::node_id const  node, std::string const&  keyword)
{
    return A == nullptr ? nullptr : find(*A,node,keyword);
}

std::unique_ptr<annotations>  create_initial_annotations();


std::pair<std::unique_ptr<program>,std::unique_ptr<annotations> >  create_program_from_assembly_text(std::istream&  assembly_text, std::string&  error_message);

std::ostream&  save_program_as_assembly_text(std::ostream&  output_stream, program const&  P, annotations const* const A,
                                             std::string const&  header_text = "", bool const  print_node_edge_ids = true);


std::string  assembly_text(instruction const&  I, std::string const&  line_adjustment = "", annotations const* const  A = nullptr);

}

#endif
