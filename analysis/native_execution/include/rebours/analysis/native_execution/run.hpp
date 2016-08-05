#ifndef REBOURS_ANALYSIS_NATIVE_EXECUTION_RUN_HPP_INCLUDED
#   define REBOURS_ANALYSIS_NATIVE_EXECUTION_RUN_HPP_INCLUDED

#   include <rebours/program/program.hpp>
#   include <rebours/program/assembly.hpp>
#   include <rebours/MAL/recogniser/recognise.hpp>
#   include <vector>
#   include <string>
#   include <map>
#   include <utility>
#   include <limits>
#   include <cstdint>

namespace analysis { namespace natexe {


/**
 * This is the entry function to the whole analysis. It performs a series of native executions of the analysed
 * program until all code is recovered or a given timeout is exceeded. Each native execution is started on input
 * data computed from the information about program's behaviour collected during all preceeding executions.
 */
std::string  run(microcode::program&  prologue,
                 microcode::program&  program,  //!< A program to be recovered from a binary file.
                 microcode::annotations&  annotations,
                 uint64_t const  heap_begin,    //!< Start address of program's heap.
                 uint64_t const  heap_end,      //!< End address of program's heap.
                 uint64_t const  temporaries_begin,     //!< Address of the first temporary in the REG pool.
                 std::map<uint64_t, //!< Start address of a code section.
                          uint64_t  //!< End address of a code section.
                          > const&  important_code, //!< Ranges of code sections whose instructions we want to recover.
                 std::map<uint64_t, //!< Exit node of program's start component.
                          uint64_t  //!< Value of IP at the exit node.
                          > const&  unexplored_exits,  //!< Identification of exit nodes in the program where the recovery
                                                       //!< process of a new code will start from.
                 std::vector<uint8_t> const&  default_stack_init_data,
                 mal::recogniser::recognise_callback_fn const&  recognise, //!< A callback to MAL's recogniser providing disassembly of bytes to a Microcode program.
                 uint32_t const  timeout_in_seconds = std::numeric_limits<uint32_t>::max(),

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


}}

#endif
