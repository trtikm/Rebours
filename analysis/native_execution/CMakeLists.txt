project(native_execution)
message("*** PROJECT_BEGIN: ${PROJECT_NAME} ***")

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

if(NOT CMAKE_BUILD_TYPE)
    message("Build type not set => setting 'Release' as default.")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Release" FORCE)
endif()

# Select build type (Release, Debug, ReleaseWithDebugInfo)
message("Build type = " ${CMAKE_BUILD_TYPE})
if(CMAKE_BUILD_TYPE MATCHES "Debug")
    add_definitions(-DDEBUG)
elseif(CMAKE_BUILD_TYPE MATCHES "Release")
    add_definitions(-DRELEASE)
elseif(CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
    add_definitions(-DRELEASE)
else()
    message(FATAL_ERROR "Unknown build type. Use either Debug, Release, or RelWithDebInfo. E.g.: -DCMAKE_BUILD_TYPE=Release")
endif()

# OS specific settings
if (APPLE)
    set(CMAKE_MACOSX_RPATH ON)
    message("Operating System = Darwin")
elseif(UNIX)
    message("Operating System = Unix")
elseif(WIN32)
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
    message("Operating System = MS-Windows")
else()
    message(FATAL_ERROR "Unsupported Operating System ! Aborting...
(supported OS: MacOS X, Linux, *BSD, MS-Windows)")
endif()

# Compiler specific settings
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_COMPILER_IS_GNUCXX OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))
    add_definitions("-std=c++11 -Wall -Wextra -pthread")
    if ((CMAKE_BUILD_TYPE STREQUAL Debug) OR
        (CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo))
        add_definitions("-ggdb3")
    endif()
    message("Compiler = ${CMAKE_CXX_COMPILER}")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    #add_definitions("/Wall")
    message("Compiler = MSVC")
else()
    message(FATAL_ERROR "Unknown (unsupported) compiler detected. Supported is only GCC (on Linux) and MSVC (on Windows).")
endif()


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/src")

include_directories(
    "${PROJECT_SOURCE_DIR}/include"
    )


# Handling the dependence to 'MAL/program' library.
if(NOT DEFINED PROGRAM_ROOT)
    set(PROGRAM_ROOT "${PROJECT_SOURCE_DIR}/../../program" CACHE STRING "Install path to the Microcode 'program' library." FORCE)
endif()
set(PROGRAM_INCLUDE_DIR "${PROGRAM_ROOT}/include")
include_directories(${PROGRAM_INCLUDE_DIR})
link_directories(
    "${PROGRAM_ROOT}/bin/${CMAKE_SYSTEM_NAME}_${CMAKE_BUILD_TYPE}/lib"
    )


# Handling the dependence to 'MAL/recogniser' library.
if(NOT DEFINED RECOGNISER_ROOT)
    set(RECOGNISER_ROOT "${PROJECT_SOURCE_DIR}/../../MAL/recogniser" CACHE STRING "Specify an installation path to 'MAL/recogniser' library." FORCE)
endif()
set(RECOGNISER_INCLUDE_DIR "${RECOGNISER_ROOT}/include")
include_directories(${RECOGNISER_INCLUDE_DIR})
link_directories(
    "${RECOGNISER_ROOT}/bin/${CMAKE_SYSTEM_NAME}_${CMAKE_BUILD_TYPE}/lib"
    )


# Handling the dependence to 'bitvectors' library.
if(NOT DEFINED BITVECTORS_ROOT)
    set(BITVECTORS_ROOT "${PROJECT_SOURCE_DIR}/../../bitvectors" CACHE STRING "Specify an installation path to 'bitvectors' library of Rebours." FORCE)
endif()
set(BITVECTORS_INCLUDE_DIR "${BITVECTORS_ROOT}/include")
include_directories(${BITVECTORS_INCLUDE_DIR})
#if(NOT DEFINED REBOURS_GLOBAL_BUILD)
#    add_subdirectory("${BITVECTORS_ROOT}"  "bitvectors")
#endif()
link_directories(
    "${BITVECTORS_ROOT}/bin/${CMAKE_SYSTEM_NAME}_${CMAKE_BUILD_TYPE}/lib"
    )


set(REBOURS_UTILITY_ROOT "${PROJECT_SOURCE_DIR}/../../utility" CACHE STRING "Specify path to the 'utility' library of Rebours." FORCE)
include_directories(
    "${REBOURS_UTILITY_ROOT}/include"
    )
link_directories(
    "${REBOURS_UTILITY_ROOT}/bin/${CMAKE_SYSTEM_NAME}_${CMAKE_BUILD_TYPE}/lib"
    )

find_package(Threads)


add_library(native_execution
    ./include/rebours/analysis/native_execution/dump.hpp
    ./src/natexe_dump.cpp

    ./include/rebours/analysis/native_execution/execution_properties.hpp
    ./src/execution_properties.cpp
    ./include/rebours/analysis/native_execution/execute_program.hpp
    ./src/execute_program.cpp
    ./include/rebours/analysis/native_execution/execute_instruction.hpp
    ./src/execute_instruction.cpp

    ./include/rebours/analysis/native_execution/recovery_properties.hpp
    ./src/recovery_properties.cpp

    ./include/rebours/analysis/native_execution/branching_condition.hpp
    ./src/branching_condition.cpp

    ./include/rebours/analysis/native_execution/exploration.hpp
    ./src/merge_recovered_traces.cpp
    ./src/choose_next_unexplored_exit.cpp
    ./src/find_next_goal_from_unexplored_exit.cpp
    ./src/find_traces_related_to_next_goal.cpp
    ./src/compute_input_for_reaching_next_goal.cpp
    ./src/close_unexplored_exit.cpp
    ./src/setup_next_execution_properties.cpp

    ./include/rebours/analysis/native_execution/run.hpp
    ./src/run.cpp
    )


if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/bin" CACHE STRING "Install path of the 'native execution' analysis library." FORCE)
    set(CMAKE_INSTALL_RPATH "./")
endif()

install(TARGETS native_execution
    CONFIGURATIONS Debug
    DESTINATION "${CMAKE_SYSTEM_NAME}_Debug/lib/analysis"
    )
install(TARGETS native_execution
    CONFIGURATIONS Release
    DESTINATION "${CMAKE_SYSTEM_NAME}_Release/lib/analysis"
    )


# Tests building
if(DEFINED BUILD_TESTS)
    set(NATIVE_EXECUTION_BUILD_TESTS ${BUILD_TESTS})
elseif(NOT DEFINED NATIVE_EXECUTION_BUILD_TESTS)
    set(NATIVE_EXECUTION_BUILD_TESTS "no" CACHE STRING "Build also tests? (yes/no)" FORCE)
endif()
message("Build also tests: " ${NATIVE_EXECUTION_BUILD_TESTS})
string( TOLOWER "${NATIVE_EXECUTION_BUILD_TESTS}" NATIVE_EXECUTION_TEMPORARY_VARIABLE)
if(NATIVE_EXECUTION_TEMPORARY_VARIABLE STREQUAL "yes")
#    add_subdirectory(./tests/test01)
#        message("-- test01")
endif()

message("*** PROJECT_END: ${PROJECT_NAME} ***")
