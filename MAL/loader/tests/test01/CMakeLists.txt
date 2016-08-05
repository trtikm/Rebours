set(THIS_TARGET_NAME test01)

add_executable(test01
    main.cpp

    ../data_path.hpp
    ../data_path.cpp
    )

target_compile_definitions(test01 PRIVATE TESTS_DATA_PATH=${TESTS_DATA_PATH})

target_link_libraries(test01
    loader
    )

install(TARGETS test01
    CONFIGURATIONS Debug
    DESTINATION "${CMAKE_SYSTEM_NAME}_Debug/test/MAL/${PROJECT_NAME}"
    )
install(TARGETS test01
    CONFIGURATIONS Release
    DESTINATION "${CMAKE_SYSTEM_NAME}_Release/test/MAL/${PROJECT_NAME}"
    )
