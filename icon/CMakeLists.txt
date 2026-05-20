foreach(arg RANGE 0 ${CMAKE_ARGC})
    if("${CMAKE_ARGV${arg}}" STREQUAL "--run-sync-icons")
        file(COPY "$ENV{CMAKE_SOURCE_DIR}/icon" DESTINATION "$ENV{CMAKE_BINARY_DIR}")
        file(TOUCH "$ENV{CMAKE_BINARY_DIR}/icons.stamp")
        return() 
    endif()
endforeach()


set(ICON)
if (WIN32)
    set(ICON 
        "${CMAKE_CURRENT_SOURCE_DIR}/icon.rc"
    )
# else ()
endif()

set(ICON ${ICON} PARENT_SCOPE)

file(GLOB_RECURSE ALL_SAMPLES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*")

add_custom_command(
    OUTPUT "${CMAKE_BINARY_DIR}/icons.stamp"
    COMMAND ${CMAKE_COMMAND} -E env CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR} CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR}
            ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_LIST_FILE}" --run-sync-icons
    DEPENDS ${ALL_SAMPLES}
    COMMENT "Copying Icons"
)

add_custom_target(icons DEPENDS "${CMAKE_BINARY_DIR}/icons.stamp")
