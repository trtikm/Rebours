#include <rebours/program/assembly.hpp>
#include <rebours/program/assumptions.hpp>
#include <rebours/program/invariants.hpp>
#include <rebours/program/development.hpp>
#include <iostream>
#include <algorithm>
#include <sstream>

namespace microcode { namespace detail {


using  node_id = program_component::node_id;
using  edge_id = program_component::edge_id;
using  graph_type = program_component::graph_type;


static std::string const  label_adjustment = "";
static std::string const  annotation_adjustment = "            ";
static std::string const  instruction_adjustment = "        ";
static uint64_t const  comment_column = 80ULL;

bool  has_annotations(annotations const* const  A, node_id const  n)
{
    return A != nullptr && A->count(n) != 0ULL;
}

void  save_annotations(std::ostream&  ostr, annotations const* const  A, node_id const  n)
{
    if (has_annotations(A,n))
        for (auto const&  kwd_value : A->at(n))
            if (kwd_value.first != "LABELNAME")
                ostr << annotation_adjustment << "#" << kwd_value.first << ": " << kwd_value.second << "\n";
}

void  save_raw_label(std::ostream&  ostr, node_id const  n, node_id const  program_entry, std::unordered_set<node_id> const&  exits, annotations const* const  A)
{
    ostr << "@";
    if (exits.count(n) != 0ULL)
        ostr << "EXIT";
    else if (n == program_entry)
        ostr << "ENTRY";
    else
    {
        std::string const* const  label_name = find(A,n,"LABELNAME");
        if (label_name != nullptr)
            ostr <<  *label_name;
        else
            ostr << n;
    }
}

void  save_label(std::ostream&  ostr, node_id const  n, node_id const  program_entry, annotations const* const  A,
                 bool const  print_node_edge_ids)
{
    std::string  value;
    {
        std::stringstream  sstr;
        sstr << label_adjustment;
        save_raw_label(sstr,n,program_entry,{},A);
        sstr << ":";
        value = sstr.str();
    }
    ostr << value;
    if (print_node_edge_ids)
    {
        ostr << std::string(comment_column > value.size() ? comment_column - value.size() : 1ULL,' ');
        ostr << "// node_id={" << n << "}";
    }
    ostr << "\n";
}

void  save_jump(std::ostream&  ostr, node_id const  n, node_id const  program_entry, std::unordered_set<node_id> const&  exits, annotations const* const  A,
                bool const  print_node_edge_ids)
{
    std::string  value;
    {
        std::stringstream  sstr;
        sstr << instruction_adjustment;
        save_raw_label(sstr,n,program_entry,exits,A);
        value = sstr.str();
    }
    ostr << value;
    if (print_node_edge_ids)
    {
        ostr << std::string(comment_column > value.size() ? comment_column - value.size() : 1ULL,' ');
        ostr << "// node_id={" << n << "}";
    }
    ostr << "\n";
}

void  save_instruction(std::ostream&  ostr, program_component const&  C, edge_id const  e, annotations const* const  A, node_id const  program_entry,
                       bool const  print_node_edge_ids)
{
    microcode::instruction const&  I = C.instruction(e);
    if (I.GIK() == GIK::GUARDS__REG_EQUAL_TO_ZERO || I.GIK() == GIK::GUARDS__REG_NOT_EQUAL_TO_ZERO)
    {
        ASSUMPTION(C.successors(e.first).size() == 2ULL);
        node_id const  other = C.successors(e.first).front() == e.second ? C.successors(e.first).back() : C.successors(e.first).front();
        std::string  value;
        {
            std::stringstream  sstr;
            sstr << instruction_adjustment;
            save_raw_label(sstr,e.second,program_entry,C.exits(),A);
            sstr << assembly_text(I," ");
            sstr << ' ';
            save_raw_label(sstr,other,program_entry,C.exits(),A);
            value = sstr.str();
        }
        ostr << value;
        if (print_node_edge_ids)
        {
            ostr << std::string(comment_column > value.size() ? comment_column - value.size() : 1ULL,' ');
            ostr << "// edge_id={" << e.first << "," << e.second << "},{" << e.first << "," << other << "}";
        }
        ostr << "\n";

        if (C.exits().count(other) != 0ULL)
            save_annotations(ostr,A,other);
    }
    else
    {
        std::string  value;
        {
            std::stringstream  sstr;
            sstr << assembly_text(C.instruction(e),instruction_adjustment,A);
            value = sstr.str();
        }
        ostr << value;
        if (print_node_edge_ids)
        {
            uint64_t const  last_line = value.find_last_of('\n');
            uint64_t const  size = value.size() - (last_line == std::string::npos ? 0ULL : last_line + 1ULL);
            ostr << std::string(comment_column > size ? comment_column - size : 1ULL,' ');
            ostr << "// edge_id={" << e.first << "," << e.second << "}";
        }
        ostr << "\n";
    }
}

void  save_component_as_assembly_text(std::ostream&  ostr, program_component const&  C, annotations const* const A, node_id const  program_entry,
                                      bool const  print_node_edge_ids)
{
    ostr << "// component: " << C.name() << "\n"
         << "// num nodes: " << C.nodes().size() << "\n"
         << "// num edges: " << C.edges().size() << "\n"
         ;

    if (C.edges().empty())
    {
        save_label(ostr,C.entry(),program_entry,A,print_node_edge_ids);
        save_jump(ostr,C.entry(),program_entry,C.exits(),A,print_node_edge_ids);
        return;
    }

    std::unordered_set<node_id,graph_type::node_id_hasher_type>  visited_nodes;
    std::unordered_set<edge_id,graph_type::edge_id_hasher_type>  visited_edges;
    std::vector<edge_id>  stack;
    for (node_id const succ : C.successors(C.entry()))
        stack.push_back({C.entry(),succ});

    while (!stack.empty())
    {
        edge_id const  e = stack.back();
        stack.pop_back();

        if (visited_edges.count(e) != 0ULL)
            continue;
        visited_edges.insert(e);
        if (C.exits().count(e.second) == 0ULL)
            for (node_id const succ : C.successors(e.second))
                stack.push_back({e.second,succ});

        if (visited_nodes.count(e.first) == 0ULL && (
                    e.first == C.entry() ||
                    C.predecessors(e.first).empty() ||
                    C.predecessors(e.first).size() > 1ULL  ||
                    C.successors(C.predecessors(e.first).front()).size() == 2ULL))
            save_label(ostr,e.first,program_entry,A,print_node_edge_ids);

        if (visited_nodes.count(e.first) == 0ULL)
            save_annotations(ostr,A,e.first);

        visited_nodes.insert(e.first);

        if (C.successors(e.first).back() == e.second)
        {
            save_instruction(ostr,C,e,A,program_entry,print_node_edge_ids);
            if (C.successors(e.second).empty())
                save_annotations(ostr,A,e.second);
            if (C.successors(e.first).size() < 2ULL && (
                        C.successors(e.second).empty() ||
                        visited_nodes.count(e.second) != 0ULL))
                save_jump(ostr,e.second,program_entry,C.exits(),A,print_node_edge_ids);
        }
    }

    INVARIANT(visited_edges.size() == C.edges().size());
}

bool  is_data_component(program_component const&  C)
{
    for (auto const&  id_instr : C.edges())
        if (id_instr.second.GIK() == GIK::DATATRANSFER__DEREF_ADDRESS_ASGN_DATA ||
            id_instr.second.GIK() == GIK::DATATRANSFER__DEREF_REG_ASGN_DATA)
            return true;
    return false;
}


}}

namespace microcode {


std::ostream&  save_program_as_assembly_text(std::ostream&  output_stream, program const&  P, annotations const* const A,
                                             std::string const&  header_text, bool const  print_node_edge_ids)
{
    output_stream << "// program: " << P.name() << "\n"
                     "//\n"
                  << "// num comonents: " << P.num_components() << "\n"
                  << "// num nodes: " << [](program const&  P) -> uint64_t {
                                            uint64_t num = 0ULL;
                                            for (uint64_t  i = 0ULL; i < P.num_components(); ++i)
                                                num += P.component(i).nodes().size();
                                            return num;
                                         }(P) << "\n"
                  << "// num edges: " << [](program const&  P) -> uint64_t {
                                            uint64_t num = 0ULL;
                                            for (uint64_t  i = 0ULL; i < P.num_components(); ++i)
                                                num += P.component(i).edges().size();
                                            return num;
                                         }(P) << "\n"
                  << "\n\n";

    if (!header_text.empty())
    {
        std::stringstream  sstr;
        sstr << header_text;
        while (!sstr.eof())
        {
            std::string line;
            std::getline(sstr,line);
            output_stream << "// " << line << "\n";
        }
        output_stream << "\n";
    }

    std::vector<program_component const*>  skipped_components;
    for (uint64_t  i = 0ULL; i < P.num_components(); ++i)
    {
        program_component const&  C = P.component(i);
        if (detail::is_data_component(C))
        {
            detail::save_component_as_assembly_text(output_stream,C,A,P.start_component().entry(),print_node_edge_ids);
            output_stream << "\n\n\n";
        }
        else
            skipped_components.push_back(&C);
    }

    for (program_component const* const  pC : skipped_components)
    {
        detail::save_component_as_assembly_text(output_stream,*pC,A,P.start_component().entry(),print_node_edge_ids);
        output_stream << "\n\n\n";
    }

    return output_stream;
}


}
