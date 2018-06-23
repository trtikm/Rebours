set(THIS_TARGET_NAME management_of_expressions)
set(TESTED_MODULE_NAME bitvectors)

add_executable(${THIS_TARGET_NAME}
    main.cpp
    )

target_link_libraries(${THIS_TARGET_NAME}
    bitvectors
    utility
    )

set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    RELWITHDEBINFO_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_RelWithDebInfo"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "test/${TESTED_MODULE_NAME}")
