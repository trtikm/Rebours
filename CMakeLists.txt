cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(rebours)

set(REBOURS_GLOBAL_BUILD "yes")
set(BUILD_TESTS "yes" CACHE STRING "Build tests (yes/no)?" FORCE)
set(TESTS_DATA_PATH "${CMAKE_SOURCE_DIR}/data" CACHE STRING "${CMAKE_SOURCE_DIR}/data" FORCE)

add_subdirectory(bitvectors)
add_subdirectory(program)
add_subdirectory(MAL/descriptor)
add_subdirectory(MAL/loader)
add_subdirectory(MAL/reloader)
add_subdirectory(MAL/prologue)
add_subdirectory(MAL/recogniser)
add_subdirectory(MAL/encoder)
add_subdirectory(analysis/native_execution)
add_subdirectory(tool/ldexe)
add_subdirectory(tool/natexe)

set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/bin" CACHE STRING "${PROJECT_SOURCE_DIR}/bin" FORCE)
