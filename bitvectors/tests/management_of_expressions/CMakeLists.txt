set(THIS_TARGET_NAME management_of_expressions)

add_executable(management_of_expressions
    main.cpp
    )

target_link_libraries(management_of_expressions
    bitvectors
    utility
    )

install(TARGETS management_of_expressions
    CONFIGURATIONS Debug
    DESTINATION "${CMAKE_SYSTEM_NAME}_Debug/test/${PROJECT_NAME}"
    )
install(TARGETS management_of_expressions
    CONFIGURATIONS Release
    DESTINATION "${CMAKE_SYSTEM_NAME}_Release/test/${PROJECT_NAME}"
    )
