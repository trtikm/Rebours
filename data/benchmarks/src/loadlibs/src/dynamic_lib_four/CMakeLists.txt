set(THIS_TARGET_NAME loadlibs_dynamic_lib_four)

add_library(${THIS_TARGET_NAME} SHARED
    ./include/dynamic_lib_four/functions.hpp
    ./src/functions.cpp
    )

target_link_libraries(${THIS_TARGET_NAME}
    loadlibs_dynamic_lib_five
    loadlibs_dynamic_lib_six
    )

set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_${CMAKE_BUILD_TYPE}"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "./")

