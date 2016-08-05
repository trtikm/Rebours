set(THIS_TARGET_NAME static_lib_one)

add_library(${THIS_TARGET_NAME}
    ./include/static_lib_one/functions.hpp
    ./src/functions.cpp
    )

set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_${CMAKE_BUILD_TYPE}"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "./")

