#ifndef REBOURS_PROGRAM_MICROCODE_PROGRAM_HPP_INCLUDED
#   define REBOURS_PROGRAM_MICROCODE_PROGRAM_HPP_INCLUDED

#   include <rebours/utility/digraph.hpp>
#   include <rebours/program/instruction.hpp>
#   include <memory>
#   include <vector>
#   include <cstdint>

namespace microcode {


struct program_component
{
    using  graph_type = digraph<void,microcode::instruction>;
    using  node_id = program_component::graph_type::node_id;
    using  edge_id = program_component::graph_type::edge_id;
    using  edge_id_hasher_type = program_component::graph_type::edge_id_hasher_type;

    program_component(std::string const&  program_name = "", std::string const&  component_name = "");

    node_id  entry() const noexcept { return m_entry; }
    std::unordered_set<node_id> const&  exits() const noexcept { return m_exits; }

    graph_type::nodes_container_type const&  nodes() const noexcept { return graph().nodes(); }
    graph_type::edges_container_type const&  edges() const noexcept { return graph().edges(); }

    std::vector<node_id> const&  successors(node_id const  n) const { return graph().successors(n); }
    std::vector<node_id> const&  predecessors(node_id const  n) const { return graph().predecessors(n); }

    microcode::instruction const&  instruction(edge_id const  e) const { return graph().data(e); }

    void  mark_entry(node_id const  node);

    void  insert_nodes(std::vector<node_id> const&  nodes);
    void  erase_nodes(std::vector<node_id> const&  nodes);

    void  insert_edges(std::vector< std::pair<edge_id,microcode::instruction> > const&  edges);
    void  erase_edges(std::vector<edge_id> const&  edges);

    node_id  insert_sequence(node_id  src_node,  std::vector<microcode::instruction> const&  instructions, node_id const  tgt_node = 0ULL, bool const  ignore_empty_instructions = true);
    std::pair<node_id,node_id>  insert_branching(GIK const  gik, uint8_t const  n, uint64_t const  a, node_id  src_node, std::pair<node_id,node_id> const&  tgt_nodes = {0ULL,0ULL});

    void  append_by_edge(program_component const&  other, node_id const  exit, microcode::instruction const I = microcode::create_MISCELLANEOUS__NOP());
    void  append_by_merging_exit_and_entry(program_component const&  other, node_id const  exit);

    std::string const&  name() const noexcept { return m_name; }
    std::string&  name() noexcept { return m_name; }

private:
    graph_type const&  graph() const noexcept { return m_graph; }
    graph_type&  graph() noexcept { return m_graph; }

    graph_type  m_graph;
    node_id  m_entry;
    std::unordered_set<node_id>  m_exits;
    std::string  m_name;
};


using  program_component_ptr = std::shared_ptr<program_component>;
using  program_component_const_ptr = std::shared_ptr<program_component const>;


struct program
{
    program(std::string const&  program_name = "", std::string const&  start_component_name = "MAIN");
    program(std::vector<program_component_ptr> const&  components, std::string const&  program_name = "");

    void  push_back(program_component_ptr const  C);

    uint64_t  num_components() const { return m_components.size(); }
    program_component const&  component(uint64_t const  index) const;
    program_component&  component(uint64_t const  index);

    program_component_const_ptr  share_component(uint64_t const  index) const;
    program_component_ptr  share_component(uint64_t const  index);

    program_component const&  start_component() const { return *m_components.front(); }
    program_component&  start_component() { return *m_components.front(); }

    std::string const&  name() const noexcept { return m_name; }
    std::string&  name() noexcept { return m_name; }

private:
    std::vector<program_component_ptr>  m_components;
    std::string  m_name;
};


program_component::node_id  generate_next_fresh_node_id();


std::unique_ptr<microcode::program>  create_initial_program(std::string const&  program_name = "", std::string const&  start_component_name = "MAIN");

uint64_t  find_component_with_entry_node(program const&  P, program_component::node_id const  entry);
uint64_t  find_component(program const&  P, program_component::node_id const  node_id);


}

#endif
