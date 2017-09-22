set(THIS_TARGET_NAME communication_with_solver)

add_executable(communication_with_solver
    main.cpp
    )

target_link_libraries(communication_with_solver
    bitvectors
    utility
    ${Z3_LIBRARIES_TO_LINK_WITH}
    ${CMAKE_THREAD_LIBS_INIT}
    )

install(TARGETS communication_with_solver
    CONFIGURATIONS Debug
    DESTINATION "${CMAKE_SYSTEM_NAME}_Debug/test/${PROJECT_NAME}"
    )
install(TARGETS communication_with_solver
    CONFIGURATIONS Release
    DESTINATION "${CMAKE_SYSTEM_NAME}_Release/test/${PROJECT_NAME}"
    )
