set(THIS_TARGET_NAME DOIFex)

add_executable(${THIS_TARGET_NAME}
    ${THIS_TARGET_NAME}.c
    )

set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_${CMAKE_BUILD_TYPE}"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "benchmarks/reachability")
