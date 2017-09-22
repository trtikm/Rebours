set(THIS_TARGET_NAME expressions_io)

add_executable(expressions_io
    main.cpp
    )

target_link_libraries(expressions_io
    bitvectors
    utility
    )

install(TARGETS expressions_io
    CONFIGURATIONS Debug
    DESTINATION "${CMAKE_SYSTEM_NAME}_Debug/test/${PROJECT_NAME}"
    )
install(TARGETS expressions_io
    CONFIGURATIONS Release
    DESTINATION "${CMAKE_SYSTEM_NAME}_Release/test/${PROJECT_NAME}"
    )
