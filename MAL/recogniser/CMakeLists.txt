set(THIS_TARGET_NAME recogniser)


add_library(${THIS_TARGET_NAME}
    ./include/rebours/MAL/recogniser/detail/recognition_data.hpp
    ./src/detail/recognition_data.cpp
    ./include/rebours/MAL/recogniser/detail/register_info.hpp
    ./src/detail/register_info.cpp
    ./include/rebours/MAL/recogniser/detail/recognition_engine_x86_64_Linux.hpp
    ./src/detail/recognition_engine_x86_64_Linux.cpp
    ./include/rebours/MAL/recogniser/detail/recognise_x86_64_Linux.hpp
    ./src/detail/recognise_x86_64_Linux.cpp
    ./src/detail/dump_x86_64_Linux.cpp
    ./include/rebours/MAL/recogniser/detail/recognise_x86_64_Linux_syscall_utils.hpp
    ./src/detail/recognise_x86_64_Linux_syscall_utils.cpp
    ./include/rebours/MAL/recogniser/detail/recognise_x86_64_Linux_syscall_syscall.hpp
    ./src/detail/recognise_x86_64_Linux_syscall_syscall.cpp
    ./include/rebours/MAL/recogniser/recognise.hpp
    ./src/recognise.cpp

    ./include/rebours/MAL/recogniser/dump.hpp
    ./src/dump.cpp
    )


set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    RELWITHDEBINFO_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_RelWithDebInfo"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "lib/MAL")
