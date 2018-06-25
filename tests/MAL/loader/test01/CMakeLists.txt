set(THIS_TARGET_NAME test01)
set(TESTED_MODULE_NAME loader)


add_executable(${THIS_TARGET_NAME}
    main.cpp

    ../data_path.hpp
    ../data_path.cpp
    )


target_link_libraries(${THIS_TARGET_NAME}
    loader
    utility
    )

set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    RELWITHDEBINFO_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_RelWithDebInfo"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "bin/tests/${TESTED_MODULE_NAME}")
