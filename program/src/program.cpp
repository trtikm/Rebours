#include <rebours/program/program.hpp>
#include <rebours/program/assumptions.hpp>
#include <rebours/program/invariants.hpp>
#include <rebours/program/development.hpp>
#include <rebours/program/msgstream.hpp>
#include <limits>
#include <algorithm>
#include <mutex>

namespace microcode { namespace detail {


static std::mutex  s_mutex_for_id_generation;


}}

namespace microcode {


program_component::program_component(std::string const&  program_name, std::string const&  component_name)
    : m_graph()
    , m_entry{generate_next_fresh_node_id()}
    , m_exits({m_entry})
    , m_name(program_name.empty() && component_name.empty() ? "" : (msgstream() << program_name << "::" << component_name << msgstream::end()))
{
    graph().insert_nodes({entry()});
}


void  program_component::mark_entry(node_id const  node)
{
    ASSUMPTION(nodes().count(node) == 1ULL);
    m_entry = node;
}

void  program_component::insert_nodes(std::vector<node_id> const&  nodes)
{
    graph().insert_nodes(nodes);
    for (node_id const  n :  nodes)
        m_exits.insert(n);
}

void  program_component::erase_nodes(std::vector<node_id> const&  nodes)
{
    for (node_id const  n :  nodes)
    {
        ASSUMPTION(n != entry());
        m_exits.erase(n);

        std::vector<node_id> const  preds = predecessors(n);

        graph().erase_nodes({n});

        for (node_id const  m : preds)
            if (successors(m).empty())
                m_exits.insert(m);
    }
}

void  program_component::insert_edges(std::vector< std::pair<edge_id,microcode::instruction> > const&  edges)
{
    graph().insert_edges(edges);
    for (auto const  id_insr : edges)
        m_exits.erase(id_insr.first.first);
}

void  program_component::erase_edges(std::vector<edge_id> const&  edges)
{
    graph().erase_edges(edges);
    for (auto const  id_insr : edges)
        if (successors(id_insr.first).empty())
            m_exits.insert(id_insr.first);
}

//program_component::node_id  program_component::insert_sequence(node_id  node,  std::vector<microcode::instruction> const&  instructions)
//{
//    ASSUMPTION(!instructions.empty());
//    ASSUMPTION(nodes().count(node) == 1ULL);
//    for (auto I : instructions)
//    {
//        node_id const  v = generate_next_fresh_node_id();
//        insert_nodes({v});
//        insert_edges({ {{node,v},I} });
//        node = v;
//    }
//    return node;
//}

program_component::node_id  program_component::insert_sequence(node_id  src_node,  std::vector<microcode::instruction> const&  instructions, node_id const  tgt_node,
                                                               bool const  ignore_empty_instructions)
{
    ASSUMPTION(!instructions.empty());
    ASSUMPTION(nodes().count(src_node) == 1ULL);
    ASSUMPTION(tgt_node == 0ULL || nodes().count(tgt_node) == 1ULL);
    ASSUMPTION(successors(src_node).empty());
    for (uint64_t i = 0ULL; i < instructions.size(); ++i)
    {
        if (!instructions.at(i))
        {
            ASSUMPTION(ignore_empty_instructions);
            continue;
        }

        ASSUMPTION(instructions.at(i).GIK() != GIK::GUARDS__REG_EQUAL_TO_ZERO && instructions.at(i).GIK() != GIK::GUARDS__REG_NOT_EQUAL_TO_ZERO);

        node_id  v;
        {
            if (i + 1ULL == instructions.size() && tgt_node != 0ULL)
                v = tgt_node;
            else
            {
                v = generate_next_fresh_node_id();
                insert_nodes({v});
            }
        }
        insert_edges({ {{src_node,v},instructions.at(i)} });
        src_node = v;
    }
    return src_node;
}

std::pair<program_component::node_id,program_component::node_id>
program_component::insert_branching(GIK const  gik, uint8_t const  n, uint64_t const  a, node_id  src_node, std::pair<node_id,node_id> const&  tgt_nodes)
{
    ASSUMPTION(gik == GIK::GUARDS__REG_EQUAL_TO_ZERO || gik == GIK::GUARDS__REG_NOT_EQUAL_TO_ZERO);
    ASSUMPTION(nodes().count(src_node) == 1ULL);
    ASSUMPTION(tgt_nodes.first == 0ULL || nodes().count(tgt_nodes.first) == 1ULL);
    ASSUMPTION(tgt_nodes.second == 0ULL || nodes().count(tgt_nodes.second) == 1ULL);
    ASSUMPTION(successors(src_node).empty());
    microcode::instruction  I0,I1;
    switch (gik)
    {
    case GIK::GUARDS__REG_EQUAL_TO_ZERO:
        I0 = create_GUARDS__REG_EQUAL_TO_ZERO(n,a);
        I1 = create_GUARDS__REG_NOT_EQUAL_TO_ZERO(n,a);
        break;
    case GIK::GUARDS__REG_NOT_EQUAL_TO_ZERO:
        I0 = create_GUARDS__REG_NOT_EQUAL_TO_ZERO(n,a);
        I1 = create_GUARDS__REG_EQUAL_TO_ZERO(n,a);
        break;
    default:
        UNREACHABLE();
    }

    node_id  v0;
    {
        if (tgt_nodes.first != 0ULL)
            v0 = tgt_nodes.first;
        else
        {
            v0 = generate_next_fresh_node_id();
            insert_nodes({v0});
        }
    }

    node_id  v1;
    {
        if (tgt_nodes.second != 0ULL)
            v1 = tgt_nodes.second;
        else
        {
            v1 = generate_next_fresh_node_id();
            insert_nodes({v1});
        }
    }

    insert_edges({ {{src_node,v1},I1},{{src_node,v0},I0} });

    return {v0,v1};
}

void  program_component::append_by_edge(program_component const&  other, node_id const  exit, microcode::instruction const I)
{
    ASSUMPTION(exits().count(exit) != 0ULL);
    graph().insert_nodes(other.nodes().cbegin(),other.nodes().cend());
    graph().insert_edges(other.edges().cbegin(),other.edges().cend());
    m_exits.insert(other.exits().cbegin(),other.exits().cend());
    insert_edges({ {{exit,other.entry()},I} });
}

void  program_component::append_by_merging_exit_and_entry(program_component const&  other, node_id const  exit)
{
    append_by_edge(other,exit);
    erase_nodes({other.entry()});
    for (node_id const  n : other.successors(other.entry()))
        insert_edges({ {{exit,n},other.instruction({other.entry(),n})} });
    for (node_id const  n : other.predecessors(other.entry()))
        insert_edges({ {{n,exit},other.instruction({n,other.entry()})} });
}

program_component::node_id  generate_next_fresh_node_id()
{
    std::lock_guard<std::mutex> lock(detail::s_mutex_for_id_generation);
    {
        static program_component::node_id  id = 0ULL;
        ASSUMPTION(id != std::numeric_limits<program_component::node_id>::max());
        return ++id;
    }
}


program::program(std::string const&  program_name, std::string const&  start_component_name)
    : m_components{std::make_shared<program_component>(program_name,start_component_name)}
    , m_name(program_name)
{}

program::program(std::vector<program_component_ptr> const&  components, std::string const&  program_name)
    : m_components{components}
    , m_name(program_name)
{
    ASSUMPTION(!m_components.empty());
}

void  program::push_back(program_component_ptr const  C)
{
    m_components.push_back(C);
}

program_component const&  program::component(uint64_t const  index) const
{
    ASSUMPTION(index < m_components.size());
    return *m_components.at(index);
}

program_component&  program::component(uint64_t const  index)
{
    ASSUMPTION(index < m_components.size());
    return *m_components.at(index);
}

program_component_const_ptr  program::share_component(uint64_t const  index) const
{
    ASSUMPTION(index < m_components.size());
    return m_components.at(index);
}

program_component_ptr  program::share_component(uint64_t const  index)
{
    ASSUMPTION(index < m_components.size());
    return m_components.at(index);
}


std::unique_ptr<microcode::program>  create_initial_program(std::string const&  program_name, std::string const&  start_component_name)
{
    return std::unique_ptr<microcode::program>( new program(program_name,start_component_name) );
}


uint64_t  find_component_with_entry_node(program const&  P, program_component::node_id const  entry)
{
    for (uint64_t i = 0ULL; i < P.num_components(); ++i)
        if (P.component(i).entry() == entry)
            return i;
    return P.num_components();
}

uint64_t  find_component(program const&  P, program_component::node_id const  node_id)
{
    if (P.num_components() == 1ULL)
    {
        ASSUMPTION(P.start_component().nodes().count(node_id) != 0ULL);
        return 0ULL;
    }
    for (uint64_t i = 0ULL; i < P.num_components(); ++i)
        if (P.component(i).nodes().count(node_id) != 0ULL)
            return i;
    return P.num_components();
}


}
