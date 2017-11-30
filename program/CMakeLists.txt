project(program)
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


find_package(Threads)


set(REBOURS_UTILITY_ROOT "${PROJECT_SOURCE_DIR}/../utility" CACHE STRING "Specify path to the 'utility' library of Rebours." FORCE)
include_directories(
    "${REBOURS_UTILITY_ROOT}/include"
    )
link_directories(
    "${REBOURS_UTILITY_ROOT}/bin/${CMAKE_SYSTEM_NAME}_${CMAKE_BUILD_TYPE}/lib"
    )


add_library(program
    ./include/rebours/program/program.hpp
    ./src/program.cpp

    ./include/rebours/program/instruction.hpp
    ./src/instruction.cpp

    ./include/rebours/program/assembly.hpp
    ./src/assembly.cpp
    ./src/assembly_instruction.cpp
    ./src/assembly_loader.cpp
    ./src/assembly_writer.cpp
    )


if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/bin" CACHE STRING "Install path of the Microcode 'program' library." FORCE)
    set(CMAKE_INSTALL_RPATH "./")
endif()

install(TARGETS program
    CONFIGURATIONS Debug
    DESTINATION "${CMAKE_SYSTEM_NAME}_Debug/lib"
    )
install(TARGETS program
    CONFIGURATIONS Release
    DESTINATION "${CMAKE_SYSTEM_NAME}_Release/lib"
    )


# Tests building
if(DEFINED BUILD_TESTS)
    set(PROGRAM_BUILD_TESTS ${BUILD_TESTS})
elseif(NOT DEFINED PROGRAM_BUILD_TESTS)
    set(PROGRAM_BUILD_TESTS "no" CACHE STRING "Build also tests? (yes/no)" FORCE)
endif()
message("Build also tests: " ${PROGRAM_BUILD_TESTS})
string( TOLOWER "${PROGRAM_BUILD_TESTS}" PROGRAM_TEMPORARY_VARIABLE)
if(PROGRAM_TEMPORARY_VARIABLE STREQUAL "yes")
#    add_subdirectory(./tests/test01)
#        message("-- test01")
endif()

message("*** PROJECT_END: ${PROJECT_NAME} ***")
