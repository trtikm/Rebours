set(THIS_TARGET_NAME utility)


add_library(${THIS_TARGET_NAME}
    ./include/rebours/utility/config.hpp

    ./include/rebours/utility/fail_message.hpp
    ./src/fail_message.cpp

    ./include/rebours/utility/timeprof.hpp
    ./src/timeprof.cpp

    ./include/rebours/utility/timestamp.hpp
    ./src/timestamp.cpp

    ./include/rebours/utility/test.hpp
    ./src/test.cpp

    ./include/rebours/utility/assumptions.hpp
    ./include/rebours/utility/invariants.hpp

    ./include/rebours/utility/development.hpp

    ./include/rebours/utility/endian.hpp

    ./include/rebours/utility/msgstream.hpp

    ./include/rebours/utility/test.hpp

    ./include/rebours/utility/digraph.hpp

    ./include/rebours/utility/std_pair_hash.hpp

    ./include/rebours/utility/macro_to_string.hpp

    ./include/rebours/utility/file_utils.hpp
    ./src/file_utils.cpp
    ./include/rebours/utility/buffer_io.hpp
    ./src/buffer_io.cpp

    ./include/rebours/utility/to_string.hpp

    ./include/rebours/utility/large_types.hpp
    ./src/large_types.cpp

    ./include/rebours/utility/unique_handle.hpp
    ./src/unique_handle.cpp
    )


set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    RELWITHDEBINFO_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_RelWithDebInfo"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "lib")
