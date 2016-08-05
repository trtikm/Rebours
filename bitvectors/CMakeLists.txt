project(bitvectors)
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

find_package(Threads)

# Add Z3 SMT solver
if(NOT DEFINED Z3_ROOT)
    set(Z3_ROOT "" CACHE STRING "Specify Z3's install path" FORCE)
elseif(NOT (Z3_ROOT MATCHES ""))
    set(Z3_INCLUDE_DIRS ${Z3_ROOT}/include)
    if(EXISTS "${Z3_INCLUDE_DIRS}" AND IS_DIRECTORY "${Z3_INCLUDE_DIRS}")
        include_directories(${Z3_INCLUDE_DIRS})
        message("Z3 include directory: " ${Z3_INCLUDE_DIRS})
    else()
        message(WARNING "WARNING: Installation directory of Z3 SMT is not correct => We won't use that solver!")
        message("Z3_ROOT = " ${Z3_ROOT})
        message("Z3_INCLUDE_DIRS = " ${Z3_INCLUDE_DIRS})
        message("Specify a path to the Z3 SMT solver using -DZ3_ROOT=<path-to-z3>.")
    endif()
    set(Z3_LIB_DIR ${Z3_ROOT}/lib)
    if(EXISTS "${Z3_LIB_DIR}" AND IS_DIRECTORY "${Z3_LIB_DIR}")
        link_directories(${Z3_LIB_DIR})
        message("Z3 library directory: " ${Z3_LIB_DIR})
        if (APPLE)
            set(Z3_LIBRARIES_TO_LINK_WITH "libz3.so")
        elseif(UNIX)
            set(Z3_LIBRARIES_TO_LINK_WITH "libz3.so")
        elseif(WIN32)
            set(Z3_LIBRARIES_TO_LINK_WITH "z3.dll")
        endif()
    else()
        set(Z3_LIBRARIES_TO_LINK_WITH "")
        message(WARNING "WARNING: Installation directory of Z3 SMT is not correct => We won't use that solver!")
        message("Z3_ROOT = " ${Z3_ROOT})
        message("Z3_LIB_DIR = " ${Z3_LIB_DIR})
        message("Specify a path to the Z3 SMT solver using -DZ3_ROOT=<path-to-z3>.")
    endif()
endif()
add_definitions("-DZ3_ROOT=${Z3_ROOT}")

if(NOT DEFINED BOOLECTOR_ROOT)
    set(BOOLECTOR_ROOT "" CACHE STRING "Specify Boolector's install path" FORCE)
    message(WARNING "WARNING: Installation directory of Boolector SMT solver was NOT specified!")
elseif(NOT (BOOLECTOR_ROOT MATCHES ""))
    message("BOOLECTOR_ROOT = " ${BOOLECTOR_ROOT})
endif()
add_definitions("-DBOOLECTOR_ROOT=${BOOLECTOR_ROOT}")

if(NOT DEFINED MATHSAT5_ROOT)
    set(MATHSAT5_ROOT "" CACHE STRING "Specify MathSAT5's install path" FORCE)
    message(WARNING "WARNING: Installation directory of MATHSAT5 SMT solver was NOT specified!")
elseif(NOT (MATHSAT5_ROOT MATCHES ""))
    message("MATHSAT5_ROOT = " ${MATHSAT5_ROOT})
endif()
add_definitions("-DMATHSAT5_ROOT=${MATHSAT5_ROOT}")


add_library(bitvectors
    ./include/rebours/bitvectors/assumptions.hpp
    ./include/rebours/bitvectors/invariants.hpp
    ./include/rebours/bitvectors/endian.hpp
    ./include/rebours/bitvectors/msgstream.hpp
    ./include/rebours/bitvectors/test.hpp

    ./include/rebours/bitvectors/large_types.hpp
    ./src/large_types.cpp

    ./include/rebours/bitvectors/symbol.hpp
    ./src/symbol.cpp

    ./include/rebours/bitvectors/expression.hpp
    ./src/expression.cpp

    ./include/rebours/bitvectors/expression_algo.hpp
    ./src/expression_algo.cpp

    ./include/rebours/bitvectors/expression_io.hpp
    ./src/expression_save_smtlib2.cpp
    ./src/expression_load_smtlib2.cpp

    ./include/rebours/bitvectors/sat_checking.hpp
    ./src/sat_checking.cpp
    ./src/sat_engine_z3/sat_engine_z3.cpp
    ./src/sat_engine_boolector/sat_engine_boolector.cpp
    ./src/sat_engine_mathsat5/sat_engine_mathsat5.cpp

    ./include/rebours/bitvectors/detail/unique_handles.hpp
    ./src/detail/unique_handles.cpp
    ./include/rebours/bitvectors/detail/execute_shell_command_with_async_pipe.hpp
    ./src/detail/execute_shell_command_with_async_pipe.cpp
    ./include/rebours/bitvectors/detail/sat_checking_interruption_function.hpp
    ./src/detail/sat_checking_interruption_function.cpp
    ./include/rebours/bitvectors/detail/file_utils.hpp
    ./src/detail/file_utils.cpp
    )


if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/bin" CACHE STRING "Install path of the 'bitvectors' library." FORCE)
    set(CMAKE_INSTALL_RPATH "./")
endif()

install(TARGETS bitvectors
    CONFIGURATIONS Debug
    DESTINATION "${CMAKE_SYSTEM_NAME}_Debug/lib"
    )
install(TARGETS bitvectors
    CONFIGURATIONS Release
    DESTINATION "${CMAKE_SYSTEM_NAME}_Release/lib"
    )


# Tests building
if(DEFINED BUILD_TESTS)
    set(BITVECTORS_BUILD_TESTS ${BUILD_TESTS})
elseif(NOT DEFINED BITVECTORS_BUILD_TESTS)
    set(BITVECTORS_BUILD_TESTS "no" CACHE STRING "Build also tests? (yes/no)" FORCE)
endif()
message("Build also tests: " ${BITVECTORS_BUILD_TESTS})
string( TOLOWER "${BITVECTORS_BUILD_TESTS}" BITVECTORS_TEMPORARY_VARIABLE)
if(BITVECTORS_TEMPORARY_VARIABLE STREQUAL "yes")
    message("Inserting tests:")
    add_subdirectory(./tests/management_of_symbols)
        message("-- management_of_symbols")
    add_subdirectory(./tests/management_of_expressions)
        message("-- management_of_expressions")
    add_subdirectory(./tests/expressions_io)
        message("-- expressions_io")
    add_subdirectory(./tests/communication_with_solver)
        message("-- communication_with_solver")
endif()

message("*** PROJECT_END: ${PROJECT_NAME} ***")
