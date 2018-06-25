project(rebours)

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)


# Select build type (Release, Debug, ReleaseWithDebugInfo)
if(NOT CMAKE_BUILD_TYPE)
    message("Build type not set => setting 'Release' as default.")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Release" FORCE)
endif()
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
    message("Operating System = MS Windows")
else()
    message(FATAL_ERROR "Unsupported Operating System ! Aborting... (supported OS: MS-Windows, Linux, MacOS X)")
endif()


# Compiler specific settings
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_COMPILER_IS_GNUCXX OR (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang"))
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


# Handling the dependence to 'Capstone-next' 3rd library.
if(NOT DEFINED CAPSTONE_NEXT_ROOT)
    set(CAPSTONE_NEXT_ROOT "Specify Capstone-next's installation directory." CACHE STRING "Specify Capstone-next's installation directory." FORCE)
    message(SEND_ERROR "The installation path to the library 'Capstone-next' 3rd library was not specified.")
endif()
set(CAPSTONE_NEXT_INCLUDE_DIR "${CAPSTONE_NEXT_ROOT}/include")
set(CAPSTONE_NEXT_LIB_DIR "${CAPSTONE_NEXT_ROOT}/lib")
message("Capstone includes dir = ${CAPSTONE_NEXT_INCLUDE_DIR}")
message("Capstone libs dir = ${CAPSTONE_NEXT_LIB_DIR}")
if (APPLE)
    set(CAPSTONE_NEXT_LIBRARIES_TO_LINK_WITH "libcapstone.a")
elseif(UNIX)
    set(CAPSTONE_NEXT_LIBRARIES_TO_LINK_WITH "libcapstone.a")
elseif(WIN32)
    set(CAPSTONE_NEXT_LIBRARIES_TO_LINK_WITH "capstone.lib")
endif()
include_directories(${CAPSTONE_NEXT_INCLUDE_DIR})
link_directories(${CAPSTONE_NEXT_LIB_DIR})


# Add Z3 SMT solver
if(NOT DEFINED Z3_ROOT)
    set(Z3_ROOT "Z3-NOTFOUND" CACHE STRING "Specify Z3's install path" FORCE)
    message(SEND_ERROR "The installation path to the 'Z3' SMT solver was not specified.")
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


# Add Boolector SMT solver
if(NOT DEFINED BOOLECTOR_ROOT)
    set(BOOLECTOR_ROOT "BOOLECTOR-NOTFOUND" CACHE STRING "Specify Boolector's install path" FORCE)
    message(WARNING "WARNING: Installation directory of Boolector SMT solver was NOT specified!")
elseif(NOT (BOOLECTOR_ROOT MATCHES ""))
    message("BOOLECTOR_ROOT = " ${BOOLECTOR_ROOT})
endif()
add_definitions("-DBOOLECTOR_ROOT=${BOOLECTOR_ROOT}")


# Add MATHSAT5 SMT solver
if(NOT DEFINED MATHSAT5_ROOT)
    set(MATHSAT5_ROOT "MATHSAT5-NOTFOUND" CACHE STRING "Specify MathSAT5's install path" FORCE)
    message(WARNING "WARNING: Installation directory of MATHSAT5 SMT solver was NOT specified!")
elseif(NOT (MATHSAT5_ROOT MATCHES ""))
    message("MATHSAT5_ROOT = " ${MATHSAT5_ROOT})
endif()
add_definitions("-DMATHSAT5_ROOT=${MATHSAT5_ROOT}")


# find and add Boost
message("Searching for Boost library ...")
if(NOT DEFINED BOOST_INSTALL_DIR)
    set(BOOST_INSTALL_DIR "Specify boost's installation directory." CACHE STRING "Specify boost's installation directory." FORCE)
endif()
if(EXISTS "${BOOST_INSTALL_DIR}" AND IS_DIRECTORY "${BOOST_INSTALL_DIR}")
    set(BOOST_ROOT ${BOOST_INSTALL_DIR})
endif()
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED)
find_package(Boost COMPONENTS
                system
                chrono
                log_setup
                log
                filesystem
                program_options
             REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})
add_definitions("-DBOOST_LOG_SETUP")
set(BOOST_LIST_OF_LIBRARIES_TO_LINK_WITH
        ${Boost_LIBRARIES}
    )
if(DEFINED BOOST_ROOT)
    message("Boost was found in directory: " ${BOOST_ROOT})
    message("Boost includes were found in directory: " ${Boost_INCLUDE_DIRS})
    message("Boost libraries were found in directory: " ${Boost_LIBRARY_DIRS})
else()
    message(SEND_ERROR "The installation directory of Boost was not FOUND!")
endif()


# find and add threads library
# find_package(Threads)   # TODO: check whether we really need Threads library!


# Set install root directory for all modules.
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/dist" CACHE STRING "Install path for the 'natexe' tool." FORCE)
endif()
set(CMAKE_INSTALL_RPATH "./")
message("Install directory = ${CMAKE_INSTALL_PREFIX}")


# Include core modules to the project
include_directories(
    "${PROJECT_SOURCE_DIR}/utility/include"
    "${PROJECT_SOURCE_DIR}/bitvectors/include"
    "${PROJECT_SOURCE_DIR}/program/include"
    "${PROJECT_SOURCE_DIR}/MAL/descriptor/include"
    "${PROJECT_SOURCE_DIR}/MAL/loader/include"
    "${PROJECT_SOURCE_DIR}/MAL/reloader/include"
    "${PROJECT_SOURCE_DIR}/MAL/prologue/include"
    "${PROJECT_SOURCE_DIR}/MAL/recogniser/include"
    "${PROJECT_SOURCE_DIR}/MAL/encoder/include"
    )
add_subdirectory(utility)
add_subdirectory(bitvectors)
add_subdirectory(program)
add_subdirectory(MAL/descriptor)
add_subdirectory(MAL/loader)
add_subdirectory(MAL/reloader)
add_subdirectory(MAL/prologue)
add_subdirectory(MAL/recogniser)
add_subdirectory(MAL/encoder)


# Include program analyses to the project
include_directories(
    "${PROJECT_SOURCE_DIR}/analysis/native_execution/include"
    )
add_subdirectory(analysis/native_execution)


# Include tools to the project.
add_subdirectory(tool/ldexe)
add_subdirectory(tool/natexe)


# Include build and install of data (benchmarks, etc.)
add_subdirectory(./data)


# Include also tests to the build?
if(NOT DEFINED INCLUDE_TESTS)
    set(INCLUDE_TESTS "yes" CACHE STRING "Include also tests? (yes/no)" FORCE)
endif()
if((INCLUDE_TESTS STREQUAL "yes") OR (INCLUDE_TESTS STREQUAL "YES") OR (INCLUDE_TESTS STREQUAL "Yes"))
    add_subdirectory(./tests)
endif()
