#ifndef REBOURS_ANALYSIS_NATIVE_EXECUTION_EXECUTE_PROGRAM_HPP_INCLUDED
#   define REBOURS_ANALYSIS_NATIVE_EXECUTION_EXECUTE_PROGRAM_HPP_INCLUDED

#   include <rebours/analysis/native_execution/execution_properties.hpp>
#   include <rebours/analysis/native_execution/recovery_properties.hpp>
#   include <rebours/program/program.hpp>
#   include <rebours/program/assembly.hpp>
#   include <rebours/MAL/recogniser/recognise.hpp>
#   include <string>
#   include <vector>
#   include <map>
#   include <utility>
#   include <cstdint>

namespace analysis { namespace natexe {


/**
 * It performs a single native execution of 'prologue' program immediatelly followed by a native execution
 * of the recovered 'program'. The later execution start with the same execution states of all theards with
 * witch the former execution terminated. Also note that 'program' can be extended by newly recovered code
 * doring its native execution.
 */
std::string  perform_single_native_program_execution(
                recovery_properties&  rprops,   //!< Data about the whole program colleced during all preceeding native executions of the program.
                execution_properties&  eprops,  //!< Data related to the current execution of the program.
                microcode::program&  prologue,
                microcode::program&  program,   //!< A program to be recovered from a binary file.
                microcode::annotations&  annotations,
                mal::recogniser::recognise_callback_fn const&  recognise,  //!< A callback to MAL's recogniser providing disassembly of bytes to a Microcode program.
                /// Next follow parameters related to generation of log files from the analysis.
                std::string const&  logging_root_dir = "",
                        //!< A pathname of a directory under which all log files will be generated. If the directory
                        //!< does not exists, then it is created. If it exists and is not empty, then existing content
                        //!< will be overwritten. When the empty string is passed, then no log files will be generated.
                bool const  log_also_prologue_program = false,
                        //!< When true, then there will also be generated log files related to execution of the prolog program.
                        //!< Also note that log files are generated only if 'logging_root_dir' not empty.
                mal::recogniser::recognition_result_dump_fn const&  dump_recognition_results = [](mal::recogniser::recognition_result const& , std::string const&) -> bool { return false; },
                        //!< A callback to MAL-recogniser's function responsible for dumping info about unsiported instruction.
                        //!< This callback only improves logging output. It is not used in the analysis itself. So, you
                        //!< can pass a function doing nothing (no logging). Also, it is used only if 'logging_root_dir'
                        //!< not empty.
                std::map<std::pair<uint64_t,uint64_t>,std::string> const&  ranges_to_registers = {}
                        //!< A map from ranges in REG pool to names of CPU registers mapped into the ranges. This map is used
                        //!< only for user-friendly output into log files. The map is not used in the analysis itself. So, you
                        //!< can pass empty map without affecting results of the analysis. Also it is used only if 'logging_root_dir'
                        //!< not empty.
                );


/**
 * It performs a native execution of a complete program (i.e. there is no unexplored branch in it).
 * We use this function to execute a prologue program (which is always complete).
 */
std::string  execute_program(
                microcode::program&  P, //!< A complete program (without unexplored branches) to be executed.
                execution_properties&  eprops,  //!< Data related to the current execution of the program.
                recovery_properties&  rprops,   //!< Data about the whole program colleced during all preceeding native executions of the program.
                /// Next follow parameters related to generation of log files from the analysis.
                std::map<std::pair<uint64_t,uint64_t>,std::string> const&  ranges_to_registers = {}
                );

/**
 * It performs a native execution of a possibly incomplete program (i.e. there might be unexplored branches).
 * We use this function to execute a prologue program (which is always complete).
 */
std::string  execute_program(
                microcode::program&  P, //!< A program to be executed.
                microcode::annotations&  annotations, //!< New annotations may be added for a newly recovered code.
                execution_properties&  eprops,  //!< Data related to the current execution of the program.
                recovery_properties&  rprops,   //!< Data about the whole program colleced during all preceeding native executions of the program.
                mal::recogniser::recognise_callback_fn const&  recognise,  //!< A callback to MAL's recogniser providing disassembly of bytes to a Microcode program.
                /// Next follow parameters related to generation of log files from the analysis.
                mal::recogniser::recognition_result_dump_fn const&  dump_recognition_results,
                std::map<std::pair<uint64_t,uint64_t>,std::string> const&  ranges_to_registers = {}
                );


/**
 * It performs a single step in a native execution of a program. It means that for each thread scheduled for taking the step
 * effects of their current instructions (each thread has exactly one current instruction) are applied. Note that scheduled
 * threads are supposed to run simulaneously. We simulate it by running threads sequentially while bookkeeping memory accesses
 * and applying an inconsistency check afterwards.
 */
std::string  execution_step(
                microcode::program const&  P,   //!< A program the single execition step will be performed in.
                std::vector<thread>&  in_thds,  //!< Threads scheduled for performing the step for.
                execution_properties&  eprops,  //!< Data related to the current execution of the program.
                recovery_properties* const  rprops, //!< Data about the whole program colleced during all preceeding native executions of the program.
                std::vector<thread>&  out_thds, //!< Resulting threads after execution step is performed to the scheduled threads.
                /// Next follow parameters related to generation of log files from the analysis.
                std::map<std::pair<uint64_t,uint64_t>,std::string> const&  ranges_to_registers = {}
                );

/**
 * It applies an effect of the current instruction of a passed thread.
 */
std::string  execution_step(
                microcode::program const&  P,   //!< A program the single execition step will be performed in.
                thread&  thd,   //!< A thread whose current instruction will be executed.
                execution_properties&  eprops,  //!< Data related to the current execution of the program.
                recovery_properties* const  rprops, //!< Data about the whole program colleced during all preceeding native executions of the program.
                std::vector<thread>&  thds, //!< Resulting threads after execution step is performed to the passed thread 'thd'.
                mem_access_set&  w_d, mem_access_set&  r_d, mem_access_set&  w_c, mem_access_set&  r_c,
                    //!< The function updates these sets according to memory accesses performed during execution of the current instruction.
                    //!< This information later allows for a detection of illegal simultaneous memory accesses of scheduled threads.
                bool const  is_sequential,
                /// Next follow parameters related to generation of log files from the analysis.
                std::map<std::pair<uint64_t,uint64_t>,std::string> const&  ranges_to_registers = {});

/**
 * Given a set of threads it chooses its subsets. The chosen threads represent those scheduled for performing next execution step.
 */
void  choose_threads(std::vector<thread>&  T,   //!< All running threads. Some of them will be moved to 'W'.
                     std::vector<thread>&  W    //!< All those threads from 'T', which were chosen (scheduled) for performing the next execution step.
                     );

/**
 * This function accepts sets of memory accesses performed by all scheduled threads and checks for consistency of those accesses.
 * According to the result it returns an error message. The empty string means no error (the accesses were consistent).
 */
std::string  check_concurrent_accesses(mem_access_set const&  W_d, mem_access_set const&  R_d, mem_access_set const&  W_c, mem_access_set const&  R_c,
                                       mem_access_set const&  w_d, mem_access_set const&  r_d, mem_access_set const&  w_c, mem_access_set const&  r_c);


}}

#endif
