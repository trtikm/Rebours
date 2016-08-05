project(natexe)
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


# Handling the dependence to 'analyses/native_execution' library.
if(NOT DEFINED NATIVE_EXECUTION_ROOT)
    set(NATIVE_EXECUTION_ROOT "${PROJECT_SOURCE_DIR}/../../analysis/native_execution" CACHE STRING "Specify an installation path to 'native_execution' library." FORCE)
endif()
set(NATIVE_EXECUTION_INCLUDE_DIR "${NATIVE_EXECUTION_ROOT}/include")
include_directories(${NATIVE_EXECUTION_INCLUDE_DIR})
if(NOT DEFINED REBOURS_GLOBAL_BUILD)
    add_subdirectory("${NATIVE_EXECUTION_ROOT}"  "analysis/native_execution")
endif()


# Handling the dependence to 'MAL/loader' library.
if(NOT DEFINED LOADER_ROOT)
    set(LOADER_ROOT "${PROJECT_SOURCE_DIR}/../../MAL/loader" CACHE STRING "Specify an installation path to 'MAL/loader' library." FORCE)
endif()
set(LOADER_INCLUDE_DIR "${LOADER_ROOT}/include")
include_directories(${LOADER_INCLUDE_DIR})
if(NOT DEFINED REBOURS_GLOBAL_BUILD)
    add_subdirectory("${LOADER_ROOT}"  "MAL/loader")
endif()


# Handling the dependence to 'MAL/reloader' library.
if(NOT DEFINED RELOADER_ROOT)
    set(RELOADER_ROOT "${PROJECT_SOURCE_DIR}/../../MAL/reloader" CACHE STRING "Specify an installation path to 'MAL/reloader' library." FORCE)
endif()
set(RELOADER_INCLUDE_DIR "${RELOADER_ROOT}/include")
include_directories(${RELOADER_INCLUDE_DIR})
if(NOT DEFINED REBOURS_GLOBAL_BUILD)
    add_subdirectory("${RELOADER_ROOT}"  "MAL/reloader")
endif()


# Handling the dependence to 'MAL/descriptor' library.
if(NOT DEFINED DESCRIPTOR_ROOT)
    set(DESCRIPTOR_ROOT "${PROJECT_SOURCE_DIR}/../../MAL/descriptor" CACHE STRING "Specify an installation path to 'MAL/descriptor' library." FORCE)
endif()
set(DESCRIPTOR_INCLUDE_DIR "${DESCRIPTOR_ROOT}/include")
include_directories(${DESCRIPTOR_INCLUDE_DIR})
if(NOT DEFINED REBOURS_GLOBAL_BUILD)
    add_subdirectory("${DESCRIPTOR_ROOT}"  "MAL/descriptor")
endif()


# Handling the dependence to 'MAL/program' library.
if(NOT DEFINED PROGRAM_ROOT)
    set(PROGRAM_ROOT "${PROJECT_SOURCE_DIR}/../../program" CACHE STRING "Specify an installation path to Microcode 'program' library." FORCE)
endif()
set(PROGRAM_INCLUDE_DIR "${PROGRAM_ROOT}/include")
include_directories(${PROGRAM_INCLUDE_DIR})
if(NOT DEFINED REBOURS_GLOBAL_BUILD)
    add_subdirectory("${PROGRAM_ROOT}"  "program")
endif()


# Handling the dependence to 'MAL/prologue' library.
if(NOT DEFINED PROLOGUE_ROOT)
    set(PROLOGUE_ROOT "${PROJECT_SOURCE_DIR}/../../MAL/prologue" CACHE STRING "Specify an installation path to 'MAL/prologue' library." FORCE)
endif()
set(PROLOGUE_INCLUDE_DIR "${PROLOGUE_ROOT}/include")
include_directories(${PROLOGUE_INCLUDE_DIR})
if(NOT DEFINED REBOURS_GLOBAL_BUILD)
    add_subdirectory("${PROLOGUE_ROOT}"  "MAL/proloque")
endif()


# Handling the dependence to 'MAL/recogniser' library.
if(NOT DEFINED RECOGNISER_ROOT)
    set(RECOGNISER_ROOT "${PROJECT_SOURCE_DIR}/../../MAL/recogniser" CACHE STRING "Specify an installation path to 'MAL/recogniser' library." FORCE)
endif()
set(RECOGNISER_INCLUDE_DIR "${RECOGNISER_ROOT}/include")
include_directories(${RECOGNISER_INCLUDE_DIR})
if(NOT DEFINED REBOURS_GLOBAL_BUILD)
    add_subdirectory("${RECOGNISER_ROOT}"  "MAL/recogniser")
endif()


# Handling the dependence to 'MAL/encoder' library.
if(NOT DEFINED ENCODER_ROOT)
    set(ENCODER_ROOT "${PROJECT_SOURCE_DIR}/../../MAL/encoder" CACHE STRING "Specify an installation path to 'MAL/encoder' library." FORCE)
endif()
set(ENCODER_INCLUDE_DIR "${ENCODER_ROOT}/include")
include_directories(${ENCODER_INCLUDE_DIR})
if(NOT DEFINED REBOURS_GLOBAL_BUILD)
    add_subdirectory("${ENCODER_ROOT}"  "MAL/encoder")
endif()


# Handling the dependence to 'Capstone-next' 3rd library.
if(NOT DEFINED CAPSTONE_NEXT_ROOT)
    set(CAPSTONE_NEXT_ROOT "Specify Capstone-next's installation directory." CACHE STRING "Specify Capstone-next's installation directory." FORCE)
    message(SEND_ERROR "The installation path to the library 'Capstone-next' 3rd library was not specified.")
endif()
set(CAPSTONE_NEXT_INCLUDE_DIR "${CAPSTONE_NEXT_ROOT}/include")
set(CAPSTONE_NEXT_LIB_DIR "${CAPSTONE_NEXT_ROOT}/lib")
set(CAPSTONE_NEXT_LIBRARIES_TO_LINK_WITH "libcapstone.a")
include_directories(${CAPSTONE_NEXT_INCLUDE_DIR})
link_directories(${CAPSTONE_NEXT_LIB_DIR})


find_package(Threads)


add_executable(natexe
    ./include/natexe/assumptions.hpp
    ./include/natexe/invariants.hpp
    ./include/natexe/endian.hpp
    ./include/natexe/msgstream.hpp
    ./include/natexe/test.hpp
    ./include/natexe/development.hpp

    ./include/natexe/file_utils.hpp
    ./src/file_utils.cpp
    ./include/natexe/argparser.hpp
    ./src/argparser.cpp

    ./src/main.cpp
    )

target_link_libraries(natexe
    native_execution
    bitvectors
    descriptor
    loader
    reloader
    prologue
    recogniser
    encoder
    program
    ${CAPSTONE_NEXT_LIBRARIES_TO_LINK_WITH}
    )


if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/bin" CACHE STRING "Install path for the 'natexe' tool." FORCE)
    set(CMAKE_INSTALL_RPATH "./")
endif()

install(TARGETS natexe
    CONFIGURATIONS Debug
    DESTINATION "${CMAKE_SYSTEM_NAME}_Debug/tool"
    )
install(TARGETS natexe
    CONFIGURATIONS Release
    DESTINATION "${CMAKE_SYSTEM_NAME}_Release/tool"
    )


# Tests building
if(DEFINED BUILD_TESTS)
    set(NATEXE_BUILD_TESTS ${BUILD_TESTS})
elseif(NOT DEFINED NATEXE_BUILD_TESTS)
    set(NATEXE_BUILD_TESTS "no" CACHE STRING "Build also tests? (yes/no)" FORCE)
endif()
message("Build also tests: " ${NATEXE_BUILD_TESTS})
string( TOLOWER "${NATEXE_BUILD_TESTS}" NATEXE_TEMPORARY_VARIABLE)
if(NATEXE_TEMPORARY_VARIABLE STREQUAL "yes")
#    add_subdirectory(./tests/test01)
#        message("-- test01")
endif()

message("*** PROJECT_END: ${PROJECT_NAME} ***")
