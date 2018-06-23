set(THIS_TARGET_NAME native_execution)


add_library(${THIS_TARGET_NAME}
    ./include/rebours/analysis/native_execution/dump.hpp
    ./src/natexe_dump.cpp

    ./include/rebours/analysis/native_execution/execution_properties.hpp
    ./src/execution_properties.cpp
    ./include/rebours/analysis/native_execution/execute_program.hpp
    ./src/execute_program.cpp
    ./include/rebours/analysis/native_execution/execute_instruction.hpp
    ./src/execute_instruction.cpp

    ./include/rebours/analysis/native_execution/recovery_properties.hpp
    ./src/recovery_properties.cpp

    ./include/rebours/analysis/native_execution/branching_condition.hpp
    ./src/branching_condition.cpp

    ./include/rebours/analysis/native_execution/exploration.hpp
    ./src/merge_recovered_traces.cpp
    ./src/choose_next_unexplored_exit.cpp
    ./src/find_next_goal_from_unexplored_exit.cpp
    ./src/find_traces_related_to_next_goal.cpp
    ./src/compute_input_for_reaching_next_goal.cpp
    ./src/close_unexplored_exit.cpp
    ./src/setup_next_execution_properties.cpp

    ./include/rebours/analysis/native_execution/run.hpp
    ./src/run.cpp
    )


set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    RELWITHDEBINFO_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_RelWithDebInfo"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "lib/analysis")
