set(THIS_TARGET_NAME loadlibs_dynamic_lib_five)

add_library(${THIS_TARGET_NAME} SHARED
    ./include/dynamic_lib_five/functions.hpp
    ./src/functions.cpp
    )

set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_${CMAKE_BUILD_TYPE}"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "benchmarks/loading")
