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


# Handling the dependence to 'analyses/native_execution' library.
if(NOT DEFINED NATIVE_EXECUTION_ROOT)
    set(NATIVE_EXECUTION_ROOT "${PROJECT_SOURCE_DIR}/../../analysis/native_execution" CACHE STRING "Specify an installation path to 'native_execution' library." FORCE)
endif()
set(NATIVE_EXECUTION_INCLUDE_DIR "${NATIVE_EXECUTION_ROOT}/include")
include_directories(${NATIVE_EXECUTION_INCLUDE_DIR})
link_directories(
    "${NATIVE_EXECUTION_ROOT}/bin/${CMAKE_SYSTEM_NAME}_${CMAKE_BUILD_TYPE}/lib/analysis"
    )


# Handling the dependence to 'bitvectors' library.
if(NOT DEFINED BITVECTORS_ROOT)
    set(BITVECTORS_ROOT "${PROJECT_SOURCE_DIR}/../../bitvectors" CACHE STRING "Specify an installation path to 'bitvectors' library of Rebours." FORCE)
endif()
set(BITVECTORS_INCLUDE_DIR "${BITVECTORS_ROOT}/include")
include_directories(${BITVECTORS_INCLUDE_DIR})
link_directories(
    "${BITVECTORS_ROOT}/bin/${CMAKE_SYSTEM_NAME}_${CMAKE_BUILD_TYPE}/lib"
    )


# Handling the dependence to 'MAL/loader' library.
if(NOT DEFINED LOADER_ROOT)
    set(LOADER_ROOT "${PROJECT_SOURCE_DIR}/../../MAL/loader" CACHE STRING "Specify an installation path to 'MAL/loader' library." FORCE)
endif()
set(LOADER_INCLUDE_DIR "${LOADER_ROOT}/include")
include_directories(${LOADER_INCLUDE_DIR})
link_directories(
    "${LOADER_ROOT}/bin/${CMAKE_SYSTEM_NAME}_${CMAKE_BUILD_TYPE}/lib/MAL"
    )


# Handling the dependence to 'MAL/reloader' library.
if(NOT DEFINED RELOADER_ROOT)
    set(RELOADER_ROOT "${PROJECT_SOURCE_DIR}/../../MAL/reloader" CACHE STRING "Specify an installation path to 'MAL/reloader' library." FORCE)
endif()
set(RELOADER_INCLUDE_DIR "${RELOADER_ROOT}/include")
include_directories(${RELOADER_INCLUDE_DIR})
link_directories(
    "${RELOADER_ROOT}/bin/${CMAKE_SYSTEM_NAME}_${CMAKE_BUILD_TYPE}/lib/MAL"
    )


# Handling the dependence to 'MAL/descriptor' library.
if(NOT DEFINED DESCRIPTOR_ROOT)
    set(DESCRIPTOR_ROOT "${PROJECT_SOURCE_DIR}/../../MAL/descriptor" CACHE STRING "Specify an installation path to 'MAL/descriptor' library." FORCE)
endif()
set(DESCRIPTOR_INCLUDE_DIR "${DESCRIPTOR_ROOT}/include")
include_directories(${DESCRIPTOR_INCLUDE_DIR})
link_directories(
    "${DESCRIPTOR_ROOT}/bin/${CMAKE_SYSTEM_NAME}_${CMAKE_BUILD_TYPE}/lib/MAL"
    )


# Handling the dependence to 'MAL/program' library.
if(NOT DEFINED PROGRAM_ROOT)
    set(PROGRAM_ROOT "${PROJECT_SOURCE_DIR}/../../program" CACHE STRING "Specify an installation path to Microcode 'program' library." FORCE)
endif()
set(PROGRAM_INCLUDE_DIR "${PROGRAM_ROOT}/include")
include_directories(${PROGRAM_INCLUDE_DIR})
link_directories(
    "${PROGRAM_ROOT}/bin/${CMAKE_SYSTEM_NAME}_${CMAKE_BUILD_TYPE}/lib"
    )


# Handling the dependence to 'MAL/prologue' library.
if(NOT DEFINED PROLOGUE_ROOT)
    set(PROLOGUE_ROOT "${PROJECT_SOURCE_DIR}/../../MAL/prologue" CACHE STRING "Specify an installation path to 'MAL/prologue' library." FORCE)
endif()
set(PROLOGUE_INCLUDE_DIR "${PROLOGUE_ROOT}/include")
include_directories(${PROLOGUE_INCLUDE_DIR})
link_directories(
    "${PROLOGUE_ROOT}/bin/${CMAKE_SYSTEM_NAME}_${CMAKE_BUILD_TYPE}/lib/MAL"
    )


# Handling the dependence to 'MAL/recogniser' library.
if(NOT DEFINED RECOGNISER_ROOT)
    set(RECOGNISER_ROOT "${PROJECT_SOURCE_DIR}/../../MAL/recogniser" CACHE STRING "Specify an installation path to 'MAL/recogniser' library." FORCE)
endif()
set(RECOGNISER_INCLUDE_DIR "${RECOGNISER_ROOT}/include")
include_directories(${RECOGNISER_INCLUDE_DIR})
link_directories(
    "${RECOGNISER_ROOT}/bin/${CMAKE_SYSTEM_NAME}_${CMAKE_BUILD_TYPE}/lib/MAL"
    )


# Handling the dependence to 'MAL/encoder' library.
if(NOT DEFINED ENCODER_ROOT)
    set(ENCODER_ROOT "${PROJECT_SOURCE_DIR}/../../MAL/encoder" CACHE STRING "Specify an installation path to 'MAL/encoder' library." FORCE)
endif()
set(ENCODER_INCLUDE_DIR "${ENCODER_ROOT}/include")
include_directories(${ENCODER_INCLUDE_DIR})
link_directories(
    "${ENCODER_ROOT}/bin/${CMAKE_SYSTEM_NAME}_${CMAKE_BUILD_TYPE}/lib/MAL"
    )


# Handling the dependence to 'MAL/loader' library.
if(NOT DEFINED LOADER_ROOT)
    set(LOADER_ROOT "${PROJECT_SOURCE_DIR}/../../MAL/loader" CACHE STRING "${PROJECT_SOURCE_DIR}/../../MAL/loader" FORCE)
endif()
set(LOADER_INCLUDE_DIR "${LOADER_ROOT}/include")
include_directories(${LOADER_INCLUDE_DIR})
link_directories(
    "${LOADER_ROOT}/bin/${CMAKE_SYSTEM_NAME}_${CMAKE_BUILD_TYPE}/lib/MAL"
    )


# Handling the dependence to 'Utility' library.
set(REBOURS_UTILITY_ROOT "${PROJECT_SOURCE_DIR}/../../utility" CACHE STRING "Specify path to the 'utility' library of Rebours." FORCE)
include_directories(
    "${REBOURS_UTILITY_ROOT}/include"
    )
link_directories(
    "${REBOURS_UTILITY_ROOT}/bin/${CMAKE_SYSTEM_NAME}_${CMAKE_BUILD_TYPE}/lib"
    )


# Handling the dependence to 'Capstone-next' 3rd library.
if(NOT DEFINED CAPSTONE_NEXT_ROOT)
    set(CAPSTONE_NEXT_ROOT "Specify Capstone-next's installation directory." CACHE STRING "Specify Capstone-next's installation directory." FORCE)
    message(SEND_ERROR "The installation path to the library 'Capstone-next' 3rd library was not specified.")
endif()
set(CAPSTONE_NEXT_INCLUDE_DIR "${CAPSTONE_NEXT_ROOT}/dist/include")
set(CAPSTONE_NEXT_LIB_DIR "${CAPSTONE_NEXT_ROOT}/dist/lib")
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
                thread
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


find_package(Threads)


add_executable(natexe
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
    utility
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
