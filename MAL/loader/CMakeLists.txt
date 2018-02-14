project(loader)
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


set(REBOURS_UTILITY_ROOT "${PROJECT_SOURCE_DIR}/../../utility" CACHE STRING "Specify path to the 'utility' library of Rebours." FORCE)
include_directories(
    "${REBOURS_UTILITY_ROOT}/include"
    )
link_directories(
    "${REBOURS_UTILITY_ROOT}/bin/${CMAKE_SYSTEM_NAME}_${CMAKE_BUILD_TYPE}/lib"
    )


add_library(loader
    ./include/rebours/MAL/loader/address.hpp

    ./include/rebours/MAL/loader/platform.hpp
    ./src/platform.cpp

    ./include/rebours/MAL/loader/sections_table.hpp
    ./src/sections_table.cpp

    ./include/rebours/MAL/loader/special_sections.hpp
    ./src/special_sections.cpp
    ./include/rebours/MAL/loader/special_sections/special_section_properties.hpp
    ./src/special_sections/special_section_properties.cpp
    ./include/rebours/MAL/loader/special_sections/exceptions.hpp
    ./src/special_sections/exceptions.cpp
    ./include/rebours/MAL/loader/special_sections/load_configuration_structure.hpp
    ./src/special_sections/load_configuration_structure.cpp
    ./include/rebours/MAL/loader/special_sections/thread_local_storage_initialisers.hpp
    ./src/special_sections/thread_local_storage_initialisers.cpp
    ./include/rebours/MAL/loader/special_sections/elf_tls.hpp
    ./src/special_sections/elf_tls.cpp

    ./include/rebours/MAL/loader/file_props.hpp
    ./src/file_props.cpp

    ./include/rebours/MAL/loader/descriptor.hpp
    ./src/descriptor.cpp

    ./include/rebours/MAL/loader/dependencies_graph.hpp
    ./src/dependencies_graph.cpp

    ./include/rebours/MAL/loader/init_fini.hpp

    ./include/rebours/MAL/loader/relocations.hpp
    ./src/relocations.cpp

    ./include/rebours/MAL/loader/warnings.hpp

    ./include/rebours/MAL/loader/dump.hpp
    ./src/dump.cpp

    ./include/rebours/MAL/loader/load.hpp
    ./src/load.cpp

    ./include/rebours/MAL/loader/detail/set_file_property.hpp
    ./include/rebours/MAL/loader/detail/std_pair_hash.hpp

    ./include/rebours/MAL/loader/detail/address_fixing.hpp
    ./src/detail/address_fixing.cpp

    ./include/rebours/MAL/loader/detail/mutable_sections_table.hpp
    ./src/detail/mutable_sections_table.cpp

    ./include/rebours/MAL/loader/detail/abi_loaders.hpp
    ./src/detail/abi_loaders/ld_linux_x86_64_so_2.cpp
    ./src/detail/abi_loaders/ld_linux_x86_32_so_2.cpp

    ./include/rebours/MAL/loader/detail/load_elf.hpp
    ./src/detail/load_elf.cpp

    ./include/rebours/MAL/loader/detail/load_props_elf.hpp
    ./src/detail/load_props_elf.cpp

    ./src/detail/load_elf_64bit.cpp
    ./src/detail/load_elf_32bit.cpp

    ./include/rebours/MAL/loader/detail/load_pe.hpp
    ./src/detail/load_pe.cpp

    ./include/rebours/MAL/loader/detail/load_props_pe.hpp
    ./src/detail/load_props_pe.cpp

    ./src/detail/load_pe_64bit.cpp
    ./src/detail/load_pe_32bit.cpp

    ./include/rebours/MAL/loader/detail/load_mach.hpp
    ./src/detail/load_mach.cpp

    ./include/rebours/MAL/loader/detail/load_props_mach.hpp
    ./src/detail/load_props_mach.cpp

    ./src/detail/load_mach_64bit.cpp
    ./src/detail/load_mach_32bit.cpp
    )


if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/bin" CACHE STRING "Install path of the 'MAL/loader' library." FORCE)
    set(CMAKE_INSTALL_RPATH "./")
endif()

install(TARGETS loader
    CONFIGURATIONS Debug
    DESTINATION "${CMAKE_SYSTEM_NAME}_Debug/lib/MAL"
    )
install(TARGETS loader
    CONFIGURATIONS Release
    DESTINATION "${CMAKE_SYSTEM_NAME}_Release/lib/MAL"
    )

# Tests building
if(DEFINED BUILD_TESTS)
    set(TESTS ${BUILD_TESTS})
elseif(NOT DEFINED TESTS)
    set(TESTS "no" CACHE STRING "Make tests? (yes/no)" FORCE)
endif()
message("Build also tests: " ${TESTS})
string(TOLOWER "${TESTS}" TEMPORARY_VARIABLE)
if(TESTS STREQUAL "yes")
  enable_testing()
endif()
if(TEMPORARY_VARIABLE STREQUAL "yes")
    if(NOT DEFINED TESTS_DATA_PATH)
        set(TESTS_DATA_PATH "${PROJECT_SOURCE_DIR}/../../data" CACHE STRING "${PROJECT_SOURCE_DIR}/../../data" FORCE)
    endif()
    message("data path = ${TESTS_DATA_PATH}")
    message("Inserting tests:")
    add_subdirectory(./tests/test01)
        message("-- test01")
endif()

message("*** PROJECT_END: ${PROJECT_NAME} ***")
