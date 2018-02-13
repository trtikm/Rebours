#include <rebours/analysis/native_execution/execute_program.hpp>
#include <rebours/analysis/native_execution/execute_instruction.hpp>
#include <rebours/utility/assumptions.hpp>
#include <rebours/utility/invariants.hpp>
#include <rebours/utility/development.hpp>
#include <rebours/utility/msgstream.hpp>
#include <rebours/utility/file_utils.hpp>
#include <rebours/analysis/native_execution/dump.hpp>
#include <rebours/program/assembly.hpp>
#include <algorithm>
#include <functional>
#include <chrono>
#include <iomanip>

namespace analysis { namespace natexe { namespace detail {


int16_t nu_reg(memory_content const&  reg, uint64_t const  adr)
{
    byte  value;
    memory_read(reg,adr,&value,1ULL);
    return (uint16_t)value;
}

int16_t nu_mem(memory_allocations const&  allocations, memory_content const&  mem, uint64_t const  adr, uint8_t const  access_rights)
{
    ASSUMPTION(access_rights == 1U || access_rights == 2U);
    memory_block_info  info = compute_memory_block_info(allocations,adr,1ULL);
    INVARIANT(info.readable != BOOL3::YES_AND_NO && info.executable != BOOL3::YES_AND_NO);
    if ((access_rights == 1U && info.readable == BOOL3::NO) ||
        (access_rights == 2U && info.executable == BOOL3::NO) )
        return -2;
    byte  value;
    memory_read(mem,adr,&value,1ULL);
    return (uint16_t)value;
}

bool  was_execution_stopped(microcode::program const&  P, thread const& thd)
{
    if (thd.stack().empty())
        return false;
     node_id const  u = thd.stack().back();
     uint64_t const  comp_index = microcode::find_component(P,u);
     ASSUMPTION(comp_index < P.num_components());
     microcode::program_component const& C = P.component(comp_index);
     if (C.successors(u).size() != 1ULL)
        return false;
     node_id const  v = C.successors(u).back();
     return C.instruction({u,v}).GIK() == microcode::GIK::MISCELLANEOUS__STOP;
}


}}}

namespace analysis { namespace natexe {


std::string  check_concurrent_accesses(mem_access_set const&  W_d, mem_access_set const&  R_d, mem_access_set const&  W_c, mem_access_set const&  R_c,
                                       mem_access_set const&  w_d, mem_access_set const&  r_d, mem_access_set const&  w_c, mem_access_set const&  r_c)
{
    mem_access_set  test;
    mem_access_set  sum;

    std::set_intersection(R_c.cbegin(), R_c.cend(), w_d.cbegin(), w_d.cend(), std::inserter(test,test.begin()));
    if (!test.empty())
        return "Undefined behaviour: Illegall concurrent accesses - intersection(R_c,W'_d) is not empty.";

    std::set_union(w_d.cbegin(), w_d.cend(), r_d.cbegin(), r_d.cend(), std::inserter(sum,sum.begin()));
    std::set_intersection(W_c.cbegin(), W_c.cend(), sum.cbegin(), sum.cend(), std::inserter(test,test.begin()));
    if (!test.empty())
        return "Undefined behaviour: Illegall concurrent accesses - intersection(W_c,union(W'_d,R'_d)) is not empty.";

    sum.clear();
    std::set_union(w_d.cbegin(), w_d.cend(), w_c.cbegin(), w_c.cend(), std::inserter(sum,sum.begin()));
    std::set_intersection(R_d.cbegin(), R_d.cend(), sum.cbegin(), sum.cend(), std::inserter(test,test.begin()));
    if (!test.empty())
        return "Undefined behaviour: Illegall concurrent accesses - intersection(R_d,union(W'_d,W'_c)) is not empty.";

    sum.clear();
    std::set_union(w_d.cbegin(), w_d.cend(), r_d.cbegin(), r_d.cend(), std::inserter(sum,sum.begin()));
    sum.insert(w_c.cbegin(),w_c.cend());
    sum.insert(r_c.cbegin(),r_c.cend());
    std::set_intersection(W_d.cbegin(), W_d.cend(), sum.cbegin(), sum.cend(), std::inserter(test,test.begin()));
    if (!test.empty())
        return "Undefined behaviour: Illegall concurrent accesses - intersection(W_d,union(W'_d,R'_d,W'_c,R'_c)) is not empty.";

    return "";
}


std::string  execution_step(microcode::program&  P, thread&  thd, execution_properties&  eprops, recovery_properties&  rprops,
                            std::vector<thread>&  thds, mem_access_set&  w_d, mem_access_set&  r_d, mem_access_set&  w_c, mem_access_set&  r_c,
                            bool const  is_sequential, std::map<std::pair<uint64_t,uint64_t>,std::string> const&  ranges_to_registers)
{
    ASSUMPTION(!thd.stack().empty());
    node_id const  u = thd.stack().back();
    microcode::program_component&  C = P.component(microcode::find_component(P,u));
    std::vector<node_id> const&  successors = C.successors(u);

    rprops.update_unexplored(u/*,successors*/);
    rprops.on_thread_step(eprops.get_execution_id(),thd.id());

//if (std::unordered_set<node_id>({194ULL,198ULL,201ULL,205ULL,231ULL}).count(u) != 0ULL)
//if (memory_read<byte>(thd.reg(),0x3fULL) == 0xff)
//if (u == 32ULL)
//{
//    static uint64_t  counter = 0ULL;
//    ++counter;
//    dump_add_file(
//            msgstream() << "Breakpoint at node " << u << ", hit number " << counter << ".",
//            dump_at_breakpoint(P,eprops,rprops/*,pprops*/,thd,u,counter,msgstream() << "execution_00000/internal_node_" << u << "_hit_" << counter,ranges_to_registers)
//            );
//}

    std::string  error_message;

    if (successors.size() == 0ULL)
    {
        INVARIANT(C.exits().count(u) != 0ULL);
        if (thd.stack().size() == 1ULL)
        {
            dump_add_file(
                    msgstream() << "Correct termination of a thread #" << thd.id() << " at the exit node " << u << ".",
                    dump_at_breakpoint(P,eprops,rprops/*,pprops*/,thd,u,0ULL,msgstream() << "execution_00000/internal_node_" << u << "_thread_" << thd.id(),ranges_to_registers)
                    );

            if (P.start_component().exits().count(u) != 0ULL)
                eprops.add_final_reg(thd.id(),thd.reg_ptr());

            rprops.insert_node_to_history(eprops.get_execution_id(),thd.id(),thd.stack().front());

            return "";
        }
        thd.stack().pop_back();

        ASSUMPTION(!thd.stack().empty());
        rprops.insert_node_to_history(eprops.get_execution_id(),thd.id(),thd.stack().back());
    }
    else if (successors.size() == 1ULL)
    {
        node_id const  v = successors.front();
        microcode::instruction const&  I = C.instruction({u,v});

        if (I.GIK() == microcode::GIK::MODULARITY__CALL)
        {
            node_id const  entry = I.argument<node_id>(0ULL);
            INVARIANT(microcode::find_component_with_entry_node(P,entry) != P.num_components());

            thd.stack().pop_back();
            thd.stack().push_back(v);
            thd.stack().push_back(entry);

            rprops.insert_node_to_history(eprops.get_execution_id(),thd.id(),entry);
        }
        else if (I.GIK() == microcode::GIK::CONCURRENCY__REG_ASGN_THREAD)
        {
            thd.stack().pop_back();
            thd.stack().push_back(v);

            thds.push_back(thread(thd.stack(),thd.reg()));
            memory_write<byte>(thds.back().reg(),I.argument<address>(0ULL),1U);

            memory_write<byte>(thd.reg(),I.argument<address>(0ULL),0U);

            rprops.on_new_thread(eprops.get_execution_id(),thds.back().id());
            rprops.insert_node_to_history(eprops.get_execution_id(),thd.id(),v);
            rprops.insert_node_to_history(eprops.get_execution_id(),thds.back().id(),v);

            input_frontier_of_thread const&  frontier = eprops.input_frontier(thd.id());
            for (auto const&  adr_link : frontier.reg_impacts())
            {
                input_impact_value const*  pvalue = rprops.find_input_impact_value(eprops.get_execution_id(),thd.id(),adr_link.second);
                INVARIANT(pvalue != nullptr);
                INVARIANT(pvalue->is_in_reg_pool() && pvalue->shift_from_begin() == adr_link.first);
                input_impact_value  value_copy = *pvalue;
                value_copy.clear_links();
                input_impact_link const  link = rprops.add_input_impact(value_copy,eprops.get_execution_id(),thds.back().id());
                eprops.input_frontier(thds.back().id()).on_reg_impact(value_copy.shift_from_begin(),link);
            }
            for (auto const&  adr_link : frontier.mem_impacts())
            {
                input_impact_value const*  pvalue = rprops.find_input_impact_value(eprops.get_execution_id(),thd.id(),adr_link.second);
                INVARIANT(pvalue != nullptr);
                INVARIANT(pvalue->is_in_mem_pool() && pvalue->shift_from_begin() == adr_link.first);
                input_impact_value  value_copy = *pvalue;
                value_copy.clear_links();
                input_impact_link const  link = rprops.add_input_impact(value_copy,eprops.get_execution_id(),thds.back().id());
                eprops.input_frontier(thds.back().id()).on_mem_impact(value_copy.shift_from_begin(),link);
            }
            for (auto const&  sid_adr__link : frontier.stream_impacts())
            {
                input_impact_value const*  pvalue = rprops.find_input_impact_value(eprops.get_execution_id(),thd.id(),sid_adr__link.second);
                INVARIANT(pvalue != nullptr);
                INVARIANT(pvalue->is_in_stream() && pvalue->stream() == sid_adr__link.first.first && pvalue->shift_from_begin() == sid_adr__link.first.second);
                input_impact_value  value_copy = *pvalue;
                value_copy.clear_links();
                input_impact_link const  link = rprops.add_input_impact(value_copy,eprops.get_execution_id(),thds.back().id());
                eprops.input_frontier(thds.back().id()).on_stream_impact(value_copy.stream(),value_copy.shift_from_begin(),link);
            }
        }
        else
        {
#define  USE_MEM_BRK()  0
#if USE_MEM_BRK() != 0
uint64_t const __brk_adr = 0x7f40003bdd90ULL;
uint8_t const __brk_size = 0x8U;
memory_content const&  __brk_mem =
        //thd.reg();
        eprops.mem_content();
uint64_t const __brk_orig_value = memory_read<uint64_t>(__brk_mem,__brk_adr,__brk_size);
#endif

#define  USE_IP_UPDATE_CHECK()  0
#if USE_IP_UPDATE_CHECK() != 0
uint64_t const __old_IP_value = memory_read<uint64_t>(thd.reg(),0ULL,(uint8_t)8U);
#endif

            execution_context  ctx(thd.reg(),eprops,w_d,r_d,w_c,r_c);
            error_message = execution_instruction(I,ctx,is_sequential);


#if USE_IP_UPDATE_CHECK() != 0
uint64_t const __new_IP_value = memory_read<uint64_t>(thd.reg(),0ULL,(uint8_t)8U);
INVARIANT(__old_IP_value == __new_IP_value
                || I.GIK() == microcode::GIK::SETANDCOPY__REG_ASGN_NUMBER
                || I.GIK() == microcode::GIK::SETANDCOPY__REG_ASGN_REG
                || I.GIK() == microcode::GIK::INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER
                || I.GIK() == microcode::GIK::INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG
                );
#endif

#if USE_MEM_BRK() != 0
uint64_t const __brk_new_value = memory_read<uint64_t>(__brk_mem,__brk_adr,__brk_size);
if (__brk_new_value != __brk_orig_value)
{
    static uint64_t  counter = 0ULL;
    ++counter;
    dump_add_file(
            msgstream() << "Memory breakpoint at " << std::hex << __brk_adr << "h at node " << std::dec << u << ", hit number " << counter << ".",
            dump_at_breakpoint(P,eprops,rprops,thd,u,counter,msgstream() << "execution_00000/mem_" << std::hex << __brk_adr
                                                                         << "_node_" << std::dec << u << "_hit_" << counter,ranges_to_registers)
            );
}
#endif

            if (error_message.empty())
            {
                if (is_switch_instruction(I))
                {
                    if (!rprops.has_switch(v))
                        rprops.add_switch(v);
                    switch_info&  info = rprops.get_switch(v);
                    address const  case_IP_value = memory_read<uint64_t>(thd.reg(),0ULL,(uint8_t)8U);
                    if (info.cases().count(case_IP_value) == 0ULL)
                    {
                        node_id  erased = 0ULL;
                        node_id const  case_begin = add_case_to_switch(C,info,case_IP_value,eprops.temporaries_begin(),erased);
                        if (erased != 0ULL)
                            rprops.update_unexplored(erased);
                        rprops.add_unexplored_exits(case_IP_value,{case_begin,info.default_node()});
                    }
                }

                thd.stack().pop_back();
                thd.stack().push_back(v);

                node_counter_type const  old_counter = rprops.node_couter(eprops.get_execution_id(),thd.id());
                rprops.insert_node_to_history(eprops.get_execution_id(),thd.id(),v);

                for (auto const&  value_locations : ctx.ior())
                {
                    std::vector<input_impact_link>  links;
                    for (io_relation_location const&  loc : value_locations.second)
                    {
                        input_impact_link const* const  plink =
                                loc.is_in_reg_pool() ? eprops.input_frontier(thd.id()).find_in_reg(loc.shift_from_begin()) :
                                loc.is_in_mem_pool() ? eprops.input_frontier(thd.id()).find_in_mem(loc.shift_from_begin()) :
                                                       eprops.input_frontier(thd.id()).find_in_stream(loc.stream(),loc.shift_from_begin()) ;
                        if (plink != nullptr)
                            links.push_back(*plink);
                    }
                    io_relation_location const&  loc = value_locations.first.location();
                    if (links.empty())
                    {
                        if (loc.is_in_reg_pool())
                            eprops.input_frontier(thd.id()).delete_reg_impact(loc.shift_from_begin());
                        else if (loc.is_in_mem_pool())
                            eprops.input_frontier(thd.id()).delete_mem_impact(loc.shift_from_begin());
                        else
                            eprops.input_frontier(thd.id()).delete_stream_impact(loc.stream(),loc.shift_from_begin());
                    }
                    else
                    {
                        if (loc.is_in_reg_pool())
                            eprops.input_frontier(thd.id()).on_reg_impact(
                                    loc.shift_from_begin(),
                                    rprops.add_input_impact({value_locations.first.value(),true,loc.shift_from_begin(),links},eprops.get_execution_id(),thd.id())
                                    );
                        else if (loc.is_in_mem_pool())
                            eprops.input_frontier(thd.id()).on_mem_impact(
                                    loc.shift_from_begin(),
                                    rprops.add_input_impact({value_locations.first.value(),false,loc.shift_from_begin(),links},eprops.get_execution_id(),thd.id())
                                    );
                        else
                            eprops.input_frontier(thd.id()).on_stream_impact(
                                    loc.stream(),
                                    loc.shift_from_begin(),
                                    rprops.add_input_impact({value_locations.first.value(),loc.stream(),loc.shift_from_begin(),links},eprops.get_execution_id(),thd.id())
                                    );
                        rprops.add_input_impact_links(eprops.get_execution_id(),thd.id(),links,old_counter);
                    }
                }
            }
            else
                error_message = msgstream() << "On edge {" << u << "," << v << " ; " << microcode::assembly_text(I) << "} : " << error_message;
        }

//        if (pprops != nullptr)
//            pprops->instructions_to_edges()[I].insert({u,v});
    }
    else  if (successors.size() == 2ULL)
    {
        node_id  v = successors.front();
        microcode::instruction const&  I = C.instruction({u,v});
        address const  adr = I.argument<address>(1ULL);
        byte const  num_bytes = I.argument<uint8_t>(0ULL);

        INVARIANT((I.GIK() == microcode::GIK::GUARDS__REG_EQUAL_TO_ZERO
                   && C.instruction({u,successors.back()}).GIK() == microcode::GIK::GUARDS__REG_NOT_EQUAL_TO_ZERO)
                  ||
                  (I.GIK() == microcode::GIK::GUARDS__REG_NOT_EQUAL_TO_ZERO
                   && C.instruction({u,successors.back()}).GIK() == microcode::GIK::GUARDS__REG_EQUAL_TO_ZERO));

        uint64_t const  value = memory_read<uint64_t>(thd.reg(),adr,num_bytes);

        if ( (I.GIK() == microcode::GIK::GUARDS__REG_EQUAL_TO_ZERO && value != 0ULL)       ||
             (I.GIK() == microcode::GIK::GUARDS__REG_NOT_EQUAL_TO_ZERO && value == 0ULL)   )
        {
            v = successors.back();

            INVARIANT( adr == C.instruction({u,v}).argument<address>(1ULL) && num_bytes == C.instruction({u,v}).argument<byte>(0ULL));
            INVARIANT( (C.instruction({u,v}).GIK() == microcode::GIK::GUARDS__REG_EQUAL_TO_ZERO && value == 0ULL) ||
                       (C.instruction({u,v}).GIK() == microcode::GIK::GUARDS__REG_NOT_EQUAL_TO_ZERO && value != 0ULL) );
        }

        thd.stack().pop_back();
        thd.stack().push_back(v);

        for (byte  i = 0U; i < num_bytes; ++i)
            if (input_impact_link const*  plink = eprops.input_frontier(thd.id()).find_in_reg(adr + i))
                rprops.add_input_impact_links(eprops.get_execution_id(),thd.id(),{*plink});

        rprops.insert_branching(eprops.get_execution_id(),thd.id());
        rprops.on_branching_visited({u,v});
        rprops.insert_node_to_history(eprops.get_execution_id(),thd.id(),v);

//        if (pprops != nullptr)
//            pprops->instructions_to_edges()[C.instruction({u,v})].insert({u,v});
    }
    else
        UNREACHABLE();

    thds.push_back(thread());
    thd.swap(thds.back());

    return error_message;
}


std::string  execution_step(microcode::program&  P, std::vector<thread>&  in_thds, execution_properties&  eprops, recovery_properties&  rprops,
                            std::vector<thread>&  out_thds, std::map<std::pair<uint64_t,uint64_t>,std::string> const&  ranges_to_registers)
{
    rprops.on_concurrent_group_begin(eprops.get_execution_id());

    bool const  is_sequential = in_thds.size() + out_thds.size() < 2ULL;

    mem_access_set  W_d;
    mem_access_set  R_d;
    mem_access_set  W_c;
    mem_access_set  R_c;
    while (!in_thds.empty())
    {
        mem_access_set  w_d;
        mem_access_set  r_d;
        mem_access_set  w_c;
        mem_access_set  r_c;

        std::string  error_message = execution_step(P,in_thds.back(),eprops,rprops,out_thds,w_d,r_d,w_c,r_c,is_sequential,ranges_to_registers);
        in_thds.pop_back();
        if (!error_message.empty())
            return error_message;

        error_message = check_concurrent_accesses(W_d,R_d,W_c,R_c,w_d,r_d,w_c,r_c);
        if (!error_message.empty())
            return error_message;

        W_d.insert(w_d.cbegin(),w_d.cend());
        R_d.insert(r_d.cbegin(),r_d.cend());
        W_c.insert(w_c.cbegin(),w_c.cend());
        R_c.insert(r_c.cbegin(),r_c.cend());
    }
    return "";
}


void  choose_threads(std::vector<thread>&  T, std::vector<thread>&  W)
{
    ASSUMPTION(!T.empty());
    ASSUMPTION(W.empty());
    W.resize(1ULL,thread());
    T.back().swap(W.back());
    T.pop_back();
}


std::string  execute_program(microcode::program&  P, execution_properties&  eprops, recovery_properties&  rprops,
                             std::map<std::pair<uint64_t,uint64_t>,std::string> const&  ranges_to_registers)
{
    std::vector<thread>  T;
    if (eprops.final_regs().empty())
        T.push_back(thread(P.start_component().entry()));
    else
    {
        for (auto const&  id_reg : eprops.final_regs())
            T.push_back(thread(P.start_component().entry(),id_reg.second,id_reg.first));
        eprops.clear_final_regs();
    }

    for (thread const&  thd : T)
    {
        rprops.on_new_thread(eprops.get_execution_id(),thd.id());
        rprops.insert_node_to_history(eprops.get_execution_id(),thd.id(),thd.stack().back());

        for (uint64_t  i = 0ULL, n = eprops.stream_allocations().at("#1").size(); i < n; ++i)
        {
            input_impact_link const  link =
                    rprops.add_input_impact(
                            { stream_read<uint8_t>(eprops.contents_of_streams().at("#1"),i), "#1", i, {} },
                            eprops.get_execution_id(),
                            thd.id()
                            );
            eprops.input_frontier(thd.id()).on_stream_impact("#1",i,link);
        }

        rprops.add_input_impact_links(eprops.get_execution_id(),thd.id(),{}); // Just in initiate the dictionary for the thread.
    }

    dump_push_guard const  dump_root(fileutl::concatenate_file_paths(dump_root_directory(),P.name()));

    dump_add_file(
            msgstream() << "Initial state of execution #" << std::setw(6) << std::setfill('0') << std::dec << eprops.get_execution_id() << " of program '" << P.name() << "'.",
            dump_execution_state(P,eprops,rprops/*,nullptr*/,T,T.size(),"",
                                 msgstream() << "execution_" << std::setw(6) << std::setfill('0') << std::dec << eprops.get_execution_id() << "/initial",
                                 ranges_to_registers)
            );

    do
    {
        std::vector<thread>  W;
        choose_threads(T,W);
        INVARIANT(!W.empty());
        std::string  error_message = execution_step(P,W,eprops,rprops,T,ranges_to_registers);
        if (!error_message.empty())
        {
            INVARIANT(!T.empty());
            index const  current = T.size() - 1ULL;
            bool const  was_stopped = detail::was_execution_stopped(P,T.at(current));
            for (thread& thd : W)
            {
                T.push_back(thread());
                T.back().swap(thd);
            }
            W.clear();
            dump_add_file(
                    msgstream() << "Final " << (was_stopped ? "stopped " : "errorneous ") << "state of execution #"
                                << std::setw(6) << std::setfill('0') << std::dec << eprops.get_execution_id() << " of program '" << P.name() << "'.",
                    dump_execution_state(P,eprops,rprops/*,nullptr*/,T,current,error_message,
                                         msgstream() << "execution_" << std::setw(6) << std::setfill('0') << std::dec << eprops.get_execution_id() << "/final_error",
                                         ranges_to_registers)
                    );
            return error_message;
        }
        INVARIANT(W.empty());
        if (rprops.passed_milliseconds() >= rprops.timeout_in_milliseconds())
        {
            error_message = "Timeout!";
            dump_add_file(
                    msgstream() << "Final timeouted state of execution #" << std::setw(6) << std::setfill('0') << std::dec << eprops.get_execution_id()
                                << " of program '" << P.name() << "'.",
                    dump_execution_state(P,eprops,rprops/*,nullptr*/,T,T.size(),error_message,
                                         msgstream() << "execution_" << std::setw(6) << std::setfill('0') << std::dec << eprops.get_execution_id() << "/final_timeout",
                                         ranges_to_registers)
                    );
            return error_message;
        }
    }
    while (!T.empty());

    dump_add_file(
            msgstream() << "Final state of execution #" << std::setw(6) << std::setfill('0') << std::dec << eprops.get_execution_id() << " of program '" << P.name() << "'.",
            dump_execution_state(P,eprops,rprops/*,nullptr*/,T,T.size(),"",
                                 msgstream() << "execution_" << std::setw(6) << std::setfill('0') << std::dec << eprops.get_execution_id() << "/final_done",
                                 ranges_to_registers)
            );

    return "";
}


std::string  execute_program(microcode::program&  P, microcode::annotations&  annotations,
                             execution_properties&  eprops, recovery_properties&  rprops,
                             mal::recogniser::recognise_callback_fn const&  recognise,
                             mal::recogniser::recognition_result_dump_fn const&  dump_recognition_results,
                             std::map<std::pair<uint64_t,uint64_t>,std::string> const&  ranges_to_registers
                             )
{
    std::vector<thread>  T;
    if (eprops.final_regs().empty())
    {
        T.push_back(thread(P.start_component().entry()));
        rprops.on_new_thread(eprops.get_execution_id(),T.back().id());
    }
    else
    {
        for (auto const&  id_reg : eprops.final_regs())
        {
            ASSUMPTION(rprops.execution_has_thread(eprops.get_execution_id(),id_reg.first));
            T.push_back(thread(P.start_component().entry(),id_reg.second,id_reg.first));
        }
        eprops.clear_final_regs();
    }

    for (thread const&  thd : T)
        rprops.insert_node_to_history(eprops.get_execution_id(),thd.id(),thd.stack().back());

    dump_push_guard const  dump_root(fileutl::concatenate_file_paths(dump_root_directory(),P.name()));

    dump_add_file(
            msgstream() << "Initial state of execution #" << std::setw(6) << std::setfill('0') << std::dec << eprops.get_execution_id() << " of program '" << P.name() << "'.",
            dump_execution_state(P,eprops,rprops/*,&pprops*/,T,T.size(),"",
                                 msgstream() << "execution_" << std::setw(6) << std::setfill('0') << std::dec << eprops.get_execution_id() << "/initial",
                                 ranges_to_registers)
            );

    bool  error_occured = false;

    do
    {
        std::vector<thread>  W;
        choose_threads(T,W);
        INVARIANT(!W.empty());
        for (uint64_t  i = 0ULL; i < W.size(); ++i)
        {
            thread&  thd = W.at(i);
            ASSUMPTION(!thd.stack().empty());
            node_id const  n = thd.stack().back();
            microcode::program_component&  C = P.component(microcode::find_component(P,n));
            if (C.successors(n).empty())
            {
//{
//    static uint64_t  counter = 0ULL;
//    node_id const  u = n;
//    ++counter;
//    dump_add_file(
//            msgstream() << "Breakpoint at node " << u << ", hit number " << counter << ", of program '" << P.name() << "'.",
//            dump_at_breakpoint(0ULL,P,eprops,rprops,thd,u,counter,msgstream() << "execution_" << std::setw(6) << std::setfill('0') << std::dec << eprops.get_execution_id()
//                                                                              << "/internal_node_" << u << "_hit_" << counter)
//            );
//    return "";
//}

                address const  start_address = memory_read<address>(thd.reg(),0ULL);

                mal::recogniser::recognition_result const  recog_result =
                        recognise(
                            start_address,
                            std::bind(&detail::nu_reg,std::cref(thd.reg()),std::placeholders::_1),
                            std::bind(&detail::nu_mem,std::cref(eprops.mem_allocations()),std::cref(eprops.mem_content()),
                                      std::placeholders::_1,std::placeholders::_2)
                            );
                if (recog_result.program().operator bool())
                {
                    C.append_by_merging_exit_and_entry(recog_result.program()->start_component(),n);
                    rprops.add_unexplored_exits(start_address,recog_result.program()->start_component().exits());
                    rprops.update_unexplored(recog_result.program()->start_component().entry());
                    for (node_id const v : recog_result.program()->start_component().exits())
                    {
                        bool  is_after_stop = true;
                        for (node_id const u : recog_result.program()->start_component().predecessors(v))
                            if (recog_result.program()->start_component().instruction({u,v}).GIK() != microcode::GIK::MISCELLANEOUS__STOP)
                                is_after_stop = false;
                        if (is_after_stop)
                            rprops.update_unexplored(v);
                    }

                    for (uint64_t i = 1ULL; i < recog_result.program()->num_components(); ++i)
                        P.push_back(recog_result.program()->share_component(i));
                    if (n == P.start_component().entry() && microcode::find(&annotations,n,"COMPONENT.NAME") == nullptr)
                        microcode::append({ {n, { {"COMPONENT.NAME",C.name()} } } }, annotations);
                    microcode::append({ {n, { {"CPU.IP",msgstream() << std::hex << start_address << "h" << msgstream::end()},
                                              {"ASM.TEXT",recog_result.asm_text()},
                                              {"ASM.BYTES",recog_result.asm_bytes()} } } },
                                      annotations);
                }
                else
                    switch (recog_result.result())
                    {
                    case 1U:    //!< There was accessed a byte in REG at address 'recog_result.address()' of unknown or ambiguous value.
                        dump_add_file(
                                msgstream() << "Final errorneous state of execution #" << std::setw(6) << std::setfill('0') << std::dec << eprops.get_execution_id()
                                            << " of program '" << P.name() << "'.",
                                dump_execution_state(P,eprops,rprops/*,&pprops*/,W,i,
                                                     msgstream() << "Thread #" << std::dec << thd.id() << " : Access of REG valuation function to address '"
                                                                 << std::hex << recog_result.address() << "h' failed due to unknown or ambiguous value at that address"
                                                                    " => killing the thread.",
                                                     msgstream() << "execution_" << std::setw(6) << std::setfill('0') << std::dec << eprops.get_execution_id() << "/final",
                                                     ranges_to_registers)
                                );
                        thd.swap(W.back());
                        W.pop_back();
                        error_occured = true;
                        break;
                    case 2U:    //!< There was accessed a byte in MEM at address 'recog_result.address()' of unknown or ambiguous value.
                        dump_add_file(
                                msgstream() << "Final errorneous state of execution #" << std::setw(6) << std::setfill('0') << std::dec << eprops.get_execution_id()
                                            << " of program '" << P.name() << "'.",
                                dump_execution_state(P,eprops,rprops/*,&pprops*/,W,i,
                                                     msgstream() << "Thread #" << std::dec << thd.id() << " : Access of MEM valuation function to address '"
                                                                 << std::hex << recog_result.address() << "h' failed due to unknown or ambiguous value at that address"
                                                                    " => killing the thread.",
                                                     msgstream() << "execution_" << std::setw(6) << std::setfill('0') << std::dec << eprops.get_execution_id() << "/final",
                                                     ranges_to_registers)
                                );
                        thd.swap(W.back());
                        W.pop_back();
                        error_occured = true;
                        break;
                    case 4U:    //!< There was accessed a byte in MEM at address 'recog_result.address()' of unknown or ambiguous access rights.
                        dump_add_file(
                                msgstream() << "Final errorneous state of execution #" << std::setw(6) << std::setfill('0') << std::dec << eprops.get_execution_id()
                                            << " of program '" << P.name() << "'.",
                                dump_execution_state(P,eprops,rprops/*,&pprops*/,W,i,
                                                     msgstream() << "Thread #" << std::dec << thd.id() << " : Access of MEM valuation function to address '"
                                                                 << std::hex << recog_result.address() << "h' failed due to unknown or ambiguous access rights '"
                                                                 << std::hex << (uint32_t)recog_result.rights() << "' => killing the thread.",
                                                     msgstream() << "execution_" << std::setw(6) << std::setfill('0') << std::dec << eprops.get_execution_id() << "/final",
                                                     ranges_to_registers)
                                );
                        thd.swap(W.back());
                        W.pop_back();
                        error_occured = true;
                        break;
                    case 254U:
                        dump_add_file(
                                msgstream() << "Final errorneous state of execution #" << std::setw(6) << std::setfill('0') << std::dec << eprops.get_execution_id()
                                            << " of program '" << P.name() << "'.",
                                dump_execution_state(P,eprops,rprops/*,&pprops*/,W,i,
                                                     msgstream() << "Thread #" << std::dec << thd.id() << " : MAL/recogniser cannot recognise an instruction at addresses ["
                                                                 << std::hex << start_address << "h," << start_address + recog_result.buffer().size()
                                                                << "h) in MEM pool => killing the thread.",
                                                     msgstream() << "execution_" << std::setw(6) << std::setfill('0') << std::dec << eprops.get_execution_id() << "/final",
                                                     ranges_to_registers)
                                );
                        thd.swap(W.back());
                        W.pop_back();
                        error_occured = true;
                        break;
                    case 255U:
                        if (dump_enabled())
                        {
                            std::string const  dump_dir =
                                    fileutl::concatenate_file_paths(dump_directory(),msgstream() << "execution_" << std::setw(6) << std::setfill('0') << std::dec
                                                                                                 << eprops.get_execution_id() << "/final");
                            std::string const  recog_details_file =
                                    fileutl::concatenate_file_paths(dump_dir,msgstream() << "recog_details_thread_" << thd.id() << ".html");
                            std::string  recog_details = "";
                            if (dump_recognition_results(recog_result,recog_details_file))
                                recog_details = msgstream() << " Deails are <a href=\"./" << fileutl::get_relative_path(recog_details_file,dump_dir) << "\">here</a>\n";

                            dump_add_file(
                                    msgstream() << "Final errorneous state of thread #" << std::dec << thd.id() << " of execution #" << std::setw(6) << std::setfill('0')
                                                << std::dec << eprops.get_execution_id() << " of program '" << P.name() << "'.",
                                    dump_execution_state(P,eprops,rprops/*,&pprops*/,W,i,
                                                         msgstream() << "Thread #" << std::dec << thd.id() << " : MAL/recogniser cannot express instruction at addresses ["
                                                                     << std::hex << start_address << "h," << start_address + recog_result.buffer().size()
                                                                     << "h) in MEM pool => killing the thread."
                                                                     << recog_details,
                                                         msgstream() << "execution_" << std::setw(6) << std::setfill('0') << std::dec << eprops.get_execution_id() << "/final",
                                                         ranges_to_registers)
                                    );
                        }
                        thd.swap(W.back());
                        W.pop_back();
                        error_occured = true;
                        break;
                    default:
                        UNREACHABLE();
                    }
            }
        }
        if (W.empty())
            continue;

        std::string  error_message = execution_step(P,W,eprops,rprops,T,ranges_to_registers);
        if (!error_message.empty())
        {
            INVARIANT(!T.empty());
            index const  current = T.size() - 1ULL;
            bool const  was_stopped = detail::was_execution_stopped(P,T.at(current));
            for (thread& thd : W)
            {
                T.push_back(thread());
                T.back().swap(thd);
            }
            W.clear();
            dump_add_file(
                    msgstream() << "Final " << (was_stopped ? "stopped " : "errorneous ") << "state of execution #" << std::setw(6) << std::setfill('0') << std::dec
                                << eprops.get_execution_id() << " of program '" << P.name() << "'.",
                    dump_execution_state(P,eprops,rprops/*,&pprops*/,T,current,error_message,
                                         msgstream() << "execution_" << std::setw(6) << std::setfill('0') << std::dec << eprops.get_execution_id() << "/final",
                                         ranges_to_registers)
                    );
            return error_message;
        }
        if (rprops.passed_milliseconds() >= rprops.timeout_in_milliseconds())
        {
            error_message = "Timeout!";
            dump_add_file(
                    msgstream() << "Final timeouted state of execution #" << std::setw(6) << std::setfill('0') << std::dec << eprops.get_execution_id()
                                << " of program '" << P.name() << "'.",
                    dump_execution_state(P,eprops,rprops/*,&pprops*/,T,T.size(),error_message,
                                         msgstream() << "execution_" << std::setw(6) << std::setfill('0') << std::dec << eprops.get_execution_id() << "/final",
                                         ranges_to_registers)
                    );
            return error_message;
        }
    }
    while (!T.empty());

    if (!error_occured)
        dump_add_file(
                msgstream() << "Final state of execution #" << std::setw(6) << std::setfill('0') << std::dec << eprops.get_execution_id() << " of program '" << P.name() << "'.",
                dump_execution_state(P,eprops,rprops/*,&pprops*/,T,T.size(),"",
                                     msgstream() << "execution_" << std::setw(6) << std::setfill('0') << std::dec << eprops.get_execution_id() << "/final_done",
                                     ranges_to_registers)
                );

    return "";
}

std::string  perform_single_native_program_execution(
        recovery_properties&  rprops,
        execution_properties&  eprops,
        microcode::program&  prologue,
        microcode::program&  program,
        microcode::annotations&  annotations,
        mal::recogniser::recognise_callback_fn const&  recognise,
        std::string const&  logging_root_dir,
        bool const  log_also_prologue_program,
        mal::recogniser::recognition_result_dump_fn const&  dump_recognition_results,
        std::map<std::pair<uint64_t,uint64_t>,std::string> const&  ranges_to_registers
        )
{
    std::string  error_message;

    if (rprops.passed_milliseconds() < rprops.timeout_in_milliseconds())
    {
        dump_push_guard const  log_pusher(logging_root_dir,log_also_prologue_program);
        error_message = execute_program(prologue,eprops,rprops,ranges_to_registers);
        if (!error_message.empty())
        {
            error_message = msgstream() << prologue.name() << " : " << error_message;
            dump_create_root_file(error_message);
            return error_message;
        }
        if (eprops.final_regs().size() != 1ULL)
        {
            error_message = msgstream() << prologue.name() << " : None or ambiguous final content of REG pool.";
            dump_create_root_file(error_message);
            return error_message;
        }
    }
    else
    {
        error_message = "Timeout!";
        dump_add_file(
                msgstream() << "Final timeouted state of execution #" << eprops.get_execution_id() << " of program '" << prologue.name() << "'.",
                dump_execution_state(prologue,eprops,rprops,{},0ULL,error_message,
                                     msgstream() << "execution_" << eprops.get_execution_id() << "/final_timeout",
                                     ranges_to_registers)
                );
        return error_message;
    }

    INVARIANT(error_message.empty());

    if (rprops.passed_milliseconds() < rprops.timeout_in_milliseconds())
    {
        dump_push_guard const  log_pusher(logging_root_dir,true);
        error_message = execute_program(program,annotations,eprops,rprops,recognise,dump_recognition_results,ranges_to_registers);
        {
            dump_push_guard const  guard(fileutl::concatenate_file_paths(dump_directory(),"recovery_properties"));
            dump_add_file("Recovery properties",dump_recovery_properties(rprops,program));
        }
        if (!error_message.empty())
        {
            error_message = msgstream() << program.name() << " : " << error_message;
            dump_create_root_file(error_message);
            return error_message;
        }
    }
    else
    {
        error_message = "Timeout!";
        dump_add_file(
                msgstream() << "Final timeouted state of execution #" << eprops.get_execution_id() << " of program '" << program.name() << "'.",
                dump_execution_state(program,eprops,rprops,{},0ULL,error_message,
                                     msgstream() << "execution_" << eprops.get_execution_id() << "/final_timeout",
                                     ranges_to_registers)
                );
        return error_message;
    }

    return error_message;
}


}}
