set(THIS_TARGET_NAME descriptor)


add_library(${THIS_TARGET_NAME}
    ./include/rebours/MAL/descriptor/storage.hpp
    ./src/storage.cpp
    ./src/x86_64_Linux/build_register_maps.cpp
    ./src/x86_64_Linux/build_stack_sections.cpp
    ./src/x86_64_Linux/build_stack_init_data.cpp
    ./src/x86_64_Linux/build_tls_template.cpp

    ./include/rebours/MAL/descriptor/dump.hpp
    ./src/dump.cpp
    )


set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    RELWITHDEBINFO_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_RelWithDebInfo"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "lib/MAL")
