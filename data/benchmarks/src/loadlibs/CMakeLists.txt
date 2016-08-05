project(loadlibs)

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
    add_definitions("-std=c++11 -Wall -Wextra")
    if ((CMAKE_BUILD_TYPE STREQUAL Debug) OR
        (CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo))
        add_definitions("-ggdb3")
    endif()
    if(NOT DEFINED LOADLIBS_USE_32BIT_BUILD)
        set(LOADLIBS_USE_32BIT_BUILD "No" CACHE STRING "Build 32 bit application? (Yes/No)" FORCE)
    endif()
    message("Build 32bit application: " ${LOADLIBS_USE_32BIT_BUILD})
    string( TOLOWER "${LOADLIBS_USE_32BIT_BUILD}" LOADER_TEMPORARY_VARIABLE)
    if(LOADER_TEMPORARY_VARIABLE STREQUAL "yes")
        set(CMAKE_CXX_FLAGS -m32)
    endif()
    message("Compiler = ${CMAKE_CXX_COMPILER}")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    #add_definitions("/Wall")
    message("Compiler = MSVC")
else()
    message(FATAL_ERROR "Unknown (unsupported) compiler detected. Supported is only GCC (on Linux) and MSVC (on Windows).")
endif()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/bin" CACHE STRING "Install path" FORCE)
endif()
set(CMAKE_INSTALL_RPATH "./")
message("Sources directory = ${PROJECT_SOURCE_DIR}/src")
message("Install directory = ${CMAKE_INSTALL_PREFIX}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Add project specific code
add_subdirectory(./src)

message("Generating build files ...")

