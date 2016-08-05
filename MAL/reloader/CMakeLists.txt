project(reloader)
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


# Handling the dependence to 'loader' library.
if(NOT DEFINED LOADER_ROOT)
    set(LOADER_ROOT "${PROJECT_SOURCE_DIR}/../loader" CACHE STRING "Install path to the 'MAL/loader' library." FORCE)
endif()
set(LOADER_INCLUDE_DIR "${LOADER_ROOT}/include")
include_directories(${LOADER_INCLUDE_DIR})


# Handling the dependence to 'descriptor' library.
if(NOT DEFINED DESCRIPTOR_ROOT)
    set(DESCRIPTOR_ROOT "${PROJECT_SOURCE_DIR}/../descriptor" CACHE STRING "Install path to the 'MAL/descriptor' library." FORCE)
endif()
set(DESCRIPTOR_INCLUDE_DIR "${DESCRIPTOR_ROOT}/include")
include_directories(${DESCRIPTOR_INCLUDE_DIR})


# Handling the dependence to 'program' library.
if(NOT DEFINED PROGRAM_ROOT)
    set(PROGRAM_ROOT "${PROJECT_SOURCE_DIR}/../../program" CACHE STRING "Install path to the Microcode 'program' library." FORCE)
endif()
set(PROGRAM_INCLUDE_DIR "${PROGRAM_ROOT}/include")
include_directories(${PROGRAM_INCLUDE_DIR})


find_package(Threads)


add_library(reloader
    ./include/rebours/MAL/reloader/assumptions.hpp
    ./include/rebours/MAL/reloader/invariants.hpp
    ./include/rebours/MAL/reloader/endian.hpp
    ./include/rebours/MAL/reloader/msgstream.hpp
    ./include/rebours/MAL/reloader/test.hpp
    ./include/rebours/MAL/reloader/development.hpp

    ./include/rebours/MAL/reloader/reload.hpp
    ./src/reload.cpp
    )


if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/bin" CACHE STRING "Install path of the 'MAL/reloader' library." FORCE)
    set(CMAKE_INSTALL_RPATH "./")
endif()

install(TARGETS reloader
    CONFIGURATIONS Debug
    DESTINATION "${CMAKE_SYSTEM_NAME}_Debug/lib/MAL"
    )
install(TARGETS reloader
    CONFIGURATIONS Release
    DESTINATION "${CMAKE_SYSTEM_NAME}_Release/lib/MAL"
    )


# Tests building
if(DEFINED BUILD_TESTS)
    set(RELOADER_BUILD_TESTS ${BUILD_TESTS})
elseif(NOT DEFINED RELOADER_BUILD_TESTS)
    set(RELOADER_BUILD_TESTS "no" CACHE STRING "Build also tests? (yes/no)" FORCE)
endif()
message("Build also tests: " ${RELOADER_BUILD_TESTS})
string( TOLOWER "${RELOADER_BUILD_TESTS}" RELOADER_TEMPORARY_VARIABLE)
if(RELOADER_TEMPORARY_VARIABLE STREQUAL "yes")
#    add_subdirectory(./tests/test01)
#        message("-- test01")
endif()

message("*** PROJECT_END: ${PROJECT_NAME} ***")
