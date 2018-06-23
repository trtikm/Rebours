set(THIS_TARGET_NAME bitvectors)


add_library(${THIS_TARGET_NAME}
    ./include/rebours/bitvectors/symbol.hpp
    ./src/symbol.cpp

    ./include/rebours/bitvectors/expression.hpp
    ./src/expression.cpp

    ./include/rebours/bitvectors/expression_algo.hpp
    ./src/expression_algo.cpp

    ./include/rebours/bitvectors/expression_io.hpp
    ./src/expression_save_smtlib2.cpp
    ./src/expression_load_smtlib2.cpp

    ./include/rebours/bitvectors/sat_checking.hpp
    ./src/sat_checking.cpp
    ./src/sat_engine_z3/sat_engine_z3.cpp
    ./src/sat_engine_boolector/sat_engine_boolector.cpp
    ./src/sat_engine_mathsat5/sat_engine_mathsat5.cpp

    ./include/rebours/bitvectors/detail/execute_shell_command_with_async_pipe.hpp
    ./src/detail/execute_shell_command_with_async_pipe.cpp
    ./include/rebours/bitvectors/detail/sat_checking_interruption_function.hpp
    ./src/detail/sat_checking_interruption_function.cpp
    )


set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    RELWITHDEBINFO_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_RelWithDebInfo"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "lib")
