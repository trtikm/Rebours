set(THIS_TARGET_NAME management_of_symbols)

add_executable(management_of_symbols
    main.cpp
    )

target_link_libraries(management_of_symbols
    bitvectors
    utility
    )

install(TARGETS management_of_symbols
    CONFIGURATIONS Debug
    DESTINATION "${CMAKE_SYSTEM_NAME}_Debug/test/${PROJECT_NAME}"
    )
install(TARGETS management_of_symbols
    CONFIGURATIONS Release
    DESTINATION "${CMAKE_SYSTEM_NAME}_Release/test/${PROJECT_NAME}"
    )
