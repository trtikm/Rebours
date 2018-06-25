# Include also build of benchmarks?
if(NOT DEFINED INCLUDE_BENCHMARKS)
    set(INCLUDE_BENCHMARKS "yes" CACHE STRING "Include also benchmarks? (yes/no)" FORCE)
endif()
if((INCLUDE_BENCHMARKS STREQUAL "yes") OR (INCLUDE_BENCHMARKS STREQUAL "YES") OR (INCLUDE_BENCHMARKS STREQUAL "Yes"))
    add_subdirectory(./benchmarks)
endif()
