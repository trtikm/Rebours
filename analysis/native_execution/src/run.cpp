#include <rebours/analysis/native_execution/run.hpp>
#include <rebours/analysis/native_execution/execute_program.hpp>
#include <rebours/analysis/native_execution/exploration.hpp>
#include <rebours/analysis/native_execution/dump.hpp>
#include <rebours/utility/msgstream.hpp>
#include <rebours/utility/assumptions.hpp>
#include <rebours/utility/invariants.hpp>

namespace analysis { namespace natexe {


std::string  run(microcode::program&  prologue,
                 microcode::program&  program,
                 microcode::annotations&  annotations,
                 uint64_t const  heap_begin,
                 uint64_t const  heap_end,
                 uint64_t const  temporaries_begin,
                 std::map<uint64_t,uint64_t> const&  important_code,
                 std::map<uint64_t,uint64_t> const&  unexplored_exits,
                 std::vector<uint8_t> const&  default_stack_init_data,
                 mal::recogniser::recognise_callback_fn const&  recognise,
                 uint32_t const  timeout_in_seconds,
                 std::string const&  logging_root_dir,
                 bool const  log_also_prologue_program,
                 mal::recogniser::recognition_result_dump_fn const&  dump_recognition_results,
                 std::map<std::pair<uint64_t,uint64_t>,std::string> const&  ranges_to_registers
                 )
{
    ASSUMPTION(heap_begin <= heap_end);
    ASSUMPTION(temporaries_begin > 7ULL);
    ASSUMPTION(!default_stack_init_data.empty());
    ASSUMPTION(timeout_in_seconds > 0U);

    recovery_properties  rprops{heap_begin,heap_end,temporaries_begin,important_code,timeout_in_seconds * 1000ULL};
    {
        for (auto const&  exit_ip : unexplored_exits)
            rprops.add_unexplored_exits(exit_ip.second,{exit_ip.first});
    }

    execution_id  eid = 0ULL;

    execution_properties  eprops{eid,heap_begin,heap_end,temporaries_begin};
    {
        eprops.contents_of_streams().insert({{"#1",{}}});
        stream_open_info&  sinfo = eprops.stream_allocations()["#1"];
        sinfo.set_readable(true);
        sinfo.set_size(default_stack_init_data.size());
        stream_write(eprops.contents_of_streams().at("#1"),0ULL,default_stack_init_data.data(),default_stack_init_data.size());
    }

    std::string  error_message;

    while (true)
    {
        error_message = perform_single_native_program_execution(
                            rprops,eprops,
                            prologue,program,annotations,
                            recognise,
                            logging_root_dir,
                            log_also_prologue_program,
                            dump_recognition_results,
                            ranges_to_registers
                            );
        if (error_message == "Timeout!")
            break;

        eprops = execution_properties{++eid,heap_begin,heap_end,temporaries_begin};

        error_message = merge_recovered_traces(program,rprops);
        if (!error_message.empty())
            break;

        std::unordered_map<stream_id,std::vector<uint8_t> > input_streams;
        while (true)
        {
            node_id  next_exit = 0ULL;
            error_message = choose_next_unexplored_exit(program,rprops,next_exit);
            if (!error_message.empty())
                break;

            edge_id  next_goal = {0ULL,0ULL};
            error_message = find_next_goal_from_unexplored_exit(program,rprops,next_exit,next_goal);
            if (!error_message.empty())
                break;

            std::vector< std::pair<std::pair<execution_id,thread_id>,node_counter_type> >  traces;
            error_message = find_traces_related_to_next_goal(prologue,program,rprops,next_goal,traces);
            if (!error_message.empty())
                break;

            error_message = compute_input_for_reaching_next_goal(prologue,program,rprops,next_goal,traces,input_streams);
            if (!error_message.empty() || !input_streams.empty())
                break;

            close_unexplored_exit(program,rprops,next_exit);
        }
        if (!error_message.empty())
            break;

        error_message = setup_next_execution_properties(eprops,input_streams,default_stack_init_data);
        if (!error_message.empty())
            break;
    }

    dump_create_root_file(error_message);

    return error_message;
}


}}
