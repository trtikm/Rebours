set(THIS_TARGET_NAME program)


add_library(${THIS_TARGET_NAME}
    ./include/rebours/program/program.hpp
    ./src/program.cpp

    ./include/rebours/program/instruction.hpp
    ./src/instruction.cpp

    ./include/rebours/program/assembly.hpp
    ./src/assembly.cpp
    ./src/assembly_instruction.cpp
    ./src/assembly_loader.cpp
    ./src/assembly_writer.cpp
    )


set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    RELWITHDEBINFO_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_RelWithDebInfo"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "lib")
