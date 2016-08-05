# Here follow all libraries of loadlibs
include_directories(
    "${PROJECT_SOURCE_DIR}/src"
    "${PROJECT_SOURCE_DIR}/src/static_lib_one/include"
    "${PROJECT_SOURCE_DIR}/src/dynamic_lib_one/include"
    "${PROJECT_SOURCE_DIR}/src/dynamic_lib_two/include"
    "${PROJECT_SOURCE_DIR}/src/dynamic_lib_three/include"
    "${PROJECT_SOURCE_DIR}/src/dynamic_lib_four/include"
    "${PROJECT_SOURCE_DIR}/src/dynamic_lib_five/include"
    "${PROJECT_SOURCE_DIR}/src/dynamic_lib_six/include"
    "${PROJECT_SOURCE_DIR}/src/dynamic_lib_seven/include"
    )
message("Including the following loadlibs modules to the build:")

add_subdirectory(./loadlibs)
    message("-- loadlibs")

add_subdirectory(./static_lib_one)
    message("-- static_lib_one")

add_subdirectory(./dynamic_lib_one)
    message("-- dynamic_lib_one")
add_subdirectory(./dynamic_lib_two)
    message("-- dynamic_lib_two")
add_subdirectory(./dynamic_lib_three)
    message("-- dynamic_lib_three")
add_subdirectory(./dynamic_lib_four)
    message("-- dynamic_lib_four")
add_subdirectory(./dynamic_lib_five)
    message("-- dynamic_lib_five")
add_subdirectory(./dynamic_lib_six)
    message("-- dynamic_lib_six")
add_subdirectory(./dynamic_lib_seven)
    message("-- dynamic_lib_seven")
