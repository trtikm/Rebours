#include <rebours/analysis/native_execution/recovery_properties.hpp>
#include <rebours/utility/invariants.hpp>
#include <rebours/utility/assumptions.hpp>
#include <rebours/utility/development.hpp>
#include <limits>
//#include <mutex>

namespace analysis { namespace natexe { namespace detail {



}}}

namespace analysis { namespace natexe {


switch_info::switch_info(node_id const  head_node)
    : m_head_node(head_node)
    , m_default_node(head_node)
    , m_cases()
    , m_havoc_instruction()
{}

void  switch_info::add_case(address const  adr, node_id const  case_node, node_id const  default_node)
{
    ASSUMPTION(cases().count(adr) == 0ULL);
    ASSUMPTION(case_node != head_node() && case_node != this->default_node() && case_node != default_node && default_node != head_node());

    m_cases.insert({adr,case_node});
    m_default_node = default_node;
}

void  switch_info::set_havoc_instruction(microcode::instruction const  I)
{
    ASSUMPTION(!havoc_instruction().operator bool());
    ASSUMPTION(I.operator bool() && I.GIK() == microcode::GIK::HAVOC__REG_ASGN_HAVOC);
    m_havoc_instruction = I;
}


bool  is_switch_instruction(microcode::instruction const&  I)
{
    return  I.operator bool()
            && ((I.GIK() == microcode::GIK::SETANDCOPY__REG_ASGN_REG
                    && I.argument<uint8_t>(0ULL) == 8ULL
                    && I.argument<uint64_t>(1ULL) == 0ULL)
                ||
                (I.GIK() == microcode::GIK::INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG
                    && I.argument<uint8_t>(0ULL) == 8ULL
                    && I.argument<uint64_t>(1ULL) == 0ULL
                    && I.argument<uint64_t>(2ULL) == 0ULL));
}


node_id  add_case_to_switch(microcode::program_component&  C, switch_info&  info, address const  case_IP_value, address const  tmp_begin, node_id&  erased)
{
    ASSUMPTION(info.cases().count(case_IP_value) == 0ULL);

    if (!info.havoc_instruction().operator bool())
    {
        ASSUMPTION(info.head_node() == info.default_node());
        ASSUMPTION(C.nodes().count(info.default_node()) != 0ULL);
        ASSUMPTION(C.predecessors(info.default_node()).size() == 1ULL);
        ASSUMPTION(is_switch_instruction(C.instruction({C.predecessors(info.default_node()).front(),info.default_node()})));

        uint64_t  num_bytes = 8ULL;
        if (!C.successors(info.default_node()).empty())
        {
            ASSUMPTION(C.successors(info.default_node()).size() == 1ULL);
            node_id const  succ_node = C.successors(info.default_node()).front();
            microcode::instruction const  succ_instr = C.instruction({info.default_node(),succ_node});
            ASSUMPTION(succ_instr.GIK() == microcode::GIK::HAVOC__REG_ASGN_HAVOC && C.successors(succ_node).empty());
            ASSUMPTION(succ_instr.argument<uint64_t>(1ULL) == tmp_begin);
            num_bytes = std::max(succ_instr.argument<uint64_t>(0ULL),(uint64_t)8UL);
            erased = succ_node;
            C.erase_nodes({succ_node});
        }
        info.set_havoc_instruction(
                    microcode::create_HAVOC__REG_ASGN_HAVOC(
                                        num_bytes,
                                        tmp_begin
                                        )
                    );
    }

    ASSUMPTION(C.successors(info.default_node()).empty());
    ASSUMPTION(info.havoc_instruction().operator bool() && info.havoc_instruction().GIK() == microcode::GIK::HAVOC__REG_ASGN_HAVOC);

    node_id  u =
    C.insert_sequence(info.default_node(),{
            microcode::create_BITOPERATIONS__REG_ASGN_REG_XOR_NUMBER(
                    8U,
                    tmp_begin,
                    0ULL,
                    case_IP_value
                    )
            });

    node_id  v;
    std::tie(u,v) =
            C.insert_branching(microcode::GIK::GUARDS__REG_EQUAL_TO_ZERO,8ULL,tmp_begin,u);

    u = C.insert_sequence(u,{info.havoc_instruction()});

    info.add_case(case_IP_value,u,v);

    return u;
}


unexplored_info::unexplored_info(
        node_id const  node,
        address const  IP
        )
    : m_node(node)
    , m_IP(IP)
{}


input_impact_value::input_impact_value(
        uint8_t const  value,
        bool const  is_in_reg_pool,
        address const  shift_from_begin,
        std::vector<input_impact_link> const&  links
        )
    : m_value(value)
    , m_is_in_reg_pool(is_in_reg_pool)
    , m_stream_id("")
    , m_shift(shift_from_begin)
    , m_links(links)
{}

input_impact_value::input_impact_value(
        uint8_t const  value,
        stream_id const&  sid,
        address const  shift_from_begin,
        std::vector<input_impact_link> const&  links
        )
    : m_value(value)
    , m_is_in_reg_pool(false)
    , m_stream_id(sid)
    , m_shift(shift_from_begin)
    , m_links(links)
{
    ASSUMPTION(m_stream_id.size() > 1ULL);
    ASSUMPTION(m_stream_id.front() == '#');
}


recovery_properties::recovery_properties(
        uint64_t const  heap_begin,
        uint64_t const  heap_end,
        uint64_t const  temporaries_begin,
        std::map<uint64_t,uint64_t> const&  important_code,
        uint64_t const  timeout_in_milliseconds
        )
    : m_heap_begin(heap_begin)
    , m_heap_end(heap_end)
    , m_temporaries_begin(temporaries_begin)
    , m_important_code()
    , m_timeout_in_milliseconds(timeout_in_milliseconds)
    , m_start_time(std::chrono::system_clock::now())
    , m_switches()
    , m_unexplored()
    , m_node_histories()
    , m_threads_of_executions()
    , m_interleaving_of_threads()
    , m_begins_of_concurrent_groups()
    , m_branchings()
    , m_visided_branchings()
    , m_input_impacts()
    , m_input_impact_links()
{
    ASSUMPTION(m_heap_begin <= m_heap_end);
    ASSUMPTION(m_temporaries_begin > 7ULL);
    ASSUMPTION(!important_code.empty());
    for (auto const b_e : important_code)
        add_important_code_range(b_e.first,b_e.second);
}

void  recovery_properties::add_important_code_range(uint64_t const  begin, uint64_t const end)
{
    ASSUMPTION(begin <= end);
    m_important_code.insert({end,begin});
}

switch_info const&  recovery_properties::get_switch(node_id const  head_node) const
{
    ASSUMPTION(has_switch(head_node));
    return m_switches.at(head_node);
}

switch_info&  recovery_properties::get_switch(node_id const  head_node)
{
    ASSUMPTION(has_switch(head_node));
    return m_switches.at(head_node);
}

std::unordered_map<node_id,switch_info>::iterator  recovery_properties::add_switch(node_id const  head_node)
{
    ASSUMPTION(!has_switch(head_node));
    auto const  it_state = m_switches.insert({head_node,switch_info(head_node)});
    INVARIANT(it_state.second == true);
    return it_state.first;
}

void  recovery_properties::update_unexplored(node_id const  just_visited/*, std::vector<node_id> const&  successors_of_just_visited*/)
{
    m_unexplored.erase(just_visited);
}

void  recovery_properties::add_unexplored_exits(address const  IP, std::unordered_set<node_id> const&  exits)
{
    for (node_id const  u : exits)
    {
        ASSUMPTION(m_unexplored.count(u) == 0ULL);
        m_unexplored.insert({u,{u,IP}});
    }
}


node_counter_type  recovery_properties::node_couter(execution_id const  eid, thread_id const  tid)
{
    ASSUMPTION(!m_node_histories.empty());
    ASSUMPTION(m_node_histories.size() > eid);
    ASSUMPTION(m_node_histories.at(eid).count(tid) != 0ULL);
    return m_node_histories.at(eid).at(tid).size() - 1ULL;
}

void  recovery_properties::insert_node_to_history(execution_id const  eid, thread_id const  tid, node_id const  node)
{
    ASSUMPTION(m_node_histories.size() >= eid);
    if (m_node_histories.size() <= eid)
        m_node_histories.resize(eid + 1ULL);
    m_node_histories[eid][tid].push_back(node);
}


void  recovery_properties::on_new_thread(execution_id const  eid, thread_id const  tid)
{
    ASSUMPTION(m_threads_of_executions.size() >= eid);
    if (m_threads_of_executions.size() <= eid)
        m_threads_of_executions.resize(eid + 1ULL);
    ASSUMPTION(m_threads_of_executions.at(eid).empty() || m_threads_of_executions.at(eid).back() < tid);
    m_threads_of_executions[eid].push_back(tid);
}

std::vector<thread_id> const&  recovery_properties::threads_of_execution(execution_id const  eid) const
{
    ASSUMPTION(m_threads_of_executions.size() > eid);
    return m_threads_of_executions.at(eid);
}

bool  recovery_properties::execution_has_thread(execution_id const  eid, thread_id const  tid) const
{
    ASSUMPTION(m_threads_of_executions.size() > eid);
    return std::binary_search(m_threads_of_executions.at(eid).cbegin(),m_threads_of_executions.at(eid).cend(),tid);
}


void  recovery_properties::on_thread_step(execution_id const  eid, thread_id const  tid)
{
    ASSUMPTION(m_interleaving_of_threads.size() >= eid);
    if (m_interleaving_of_threads.size() <= eid)
        m_interleaving_of_threads.resize(eid + 1ULL);
    m_interleaving_of_threads[eid].push_back(tid);
}

std::vector<thread_id> const&  recovery_properties::interleaving_of_threads(execution_id const  eid) const
{
    ASSUMPTION(m_interleaving_of_threads.size() > eid);
    return m_interleaving_of_threads.at(eid);
}


void  recovery_properties::on_concurrent_group_begin(execution_id const  eid)
{
    ASSUMPTION(m_begins_of_concurrent_groups.size() >= eid);
    if (m_begins_of_concurrent_groups.size() <= eid)
        m_begins_of_concurrent_groups.resize(eid + 1ULL);
    node_counter_type const  counter = m_interleaving_of_threads.size() > eid ? m_interleaving_of_threads.at(eid).size() : 0ULL;
    ASSUMPTION(m_begins_of_concurrent_groups.at(eid).empty() || m_begins_of_concurrent_groups.at(eid).back() < counter);
    m_begins_of_concurrent_groups[eid].push_back(counter);
}

std::vector<node_counter_type> const&  recovery_properties::begins_of_concurrent_groups(execution_id const  eid) const
{
    ASSUMPTION(m_begins_of_concurrent_groups.size() > eid);
    return m_begins_of_concurrent_groups.at(eid);
}

bool  recovery_properties::is_begin_of_concurrent_group(execution_id const  eid, node_counter_type const  counter) const
{
    ASSUMPTION(m_begins_of_concurrent_groups.size() > eid);
    return std::binary_search(m_begins_of_concurrent_groups.at(eid).cbegin(),m_begins_of_concurrent_groups.at(eid).cend(),counter);
}


void  recovery_properties::insert_branching(execution_id const  eid, thread_id const  tid, node_counter_type const  counter)
{
    ASSUMPTION(m_branchings.size() >= eid);
    ASSUMPTION(counter <= node_couter(eid,tid));
    if (m_branchings.size() <= eid)
        m_branchings.resize(eid + 1ULL);
    m_branchings[eid][tid].push_back(counter);
}

void  recovery_properties::on_branching_visited(edge_id const  eid)
{
    m_visided_branchings.insert(eid);
}


input_impact_link  recovery_properties::add_input_impact(input_impact_value const&  value, execution_id const  eid, thread_id const  tid, node_counter_type const  counter)
{
    if (eid >= m_input_impacts.size())
        m_input_impacts.resize(eid + 1ULL);
    std::vector<std::vector<input_impact_value> >&  values = m_input_impacts[eid][tid];
    if (counter >= values.size())
        values.resize(counter + 1ULL);
    values.at(counter).push_back(value);
    ASSUMPTION(
            [](std::vector<std::vector<input_impact_value> > const&  values, std::vector<input_impact_link> const&  links) -> bool {
                for (input_impact_link const& link : links)
                    if (link.first >= values.size() || link.second >= values.at(link.first).size())
                        return false;
                return true;
            }(values,value.links())
            );
    return { counter, values.at(counter).size() - 1ULL };
}

input_impact_value const*  recovery_properties::find_input_impact_value(execution_id const  eid, thread_id const  tid, input_impact_link const&  link) const
{
    if (eid < m_input_impacts.size())
    {
        auto const  it = m_input_impacts.at(eid).find(tid);
        if (it != m_input_impacts.at(eid).cend() && link.first < it->second.size() && link.second < it->second.at(link.first).size())
            return &it->second.at(link.first).at(link.second);
    }
    return nullptr;
}

void  recovery_properties::add_input_impact_links(execution_id const  eid, thread_id const  tid, std::vector<input_impact_link> const&  links, node_counter_type const  counter)
{
    if (eid >= m_input_impact_links.size())
        m_input_impact_links.resize(eid + 1ULL);
    std::vector<std::unordered_set<input_impact_link> >&  values = m_input_impact_links[eid][tid];
    if (counter >= values.size())
        values.resize(counter + 1ULL);
    for (input_impact_link const&  lnk : links)
        values.at(counter).insert(lnk);
}


bool is_inside_important_code(address const  adr, important_code_ranges const&  important_code)
{
    auto const  it = important_code.lower_bound(adr);
    return (it == important_code.cend() || it->first == adr) ? false : adr >= it->second;
}


}}
