foreach(arg RANGE 0 ${CMAKE_ARGC})
    if("${CMAKE_ARGV${arg}}" STREQUAL "--run-sync-samples")
        file(COPY "$ENV{CMAKE_SOURCE_DIR}/samples" DESTINATION "$ENV{CMAKE_BINARY_DIR}")
        file(TOUCH "$ENV{CMAKE_BINARY_DIR}/samples.stamp")
        return() 
    endif()
endforeach()

file(GLOB_RECURSE ALL_SAMPLES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*")
list(FILTER ALL_SAMPLES EXCLUDE REGEX "Unicode")

add_custom_command(
    OUTPUT "${CMAKE_BINARY_DIR}/samples.stamp"
    COMMAND ${CMAKE_COMMAND} -E env CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR} CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR}
            ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_LIST_FILE}" --run-sync-samples
    DEPENDS ${ALL_SAMPLES}
    COMMENT "Copying Samples"
)

add_custom_target(samples DEPENDS "${CMAKE_BINARY_DIR}/samples.stamp")
