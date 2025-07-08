cmake_minimum_required(VERSION 3.10)

project(Tester VERSION 1.0 LANGUAGES C)

add_subdirectory(dependencies)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(source)
add_subdirectory(icon)

add_executable(Tester
    ${SOURCE_FILES} 
    ${ICON}
    "${CMAKE_SOURCE_DIR}/source/main.c"
)

target_link_libraries(Tester ${LINK_LIBRARIES})
target_include_directories(Tester PRIVATE 
    ${INCLUDE_DIRECTORIES}
    ${SOURCE_DIRECTORIES}
)
set(COMPILE_OPTIONS
    -W
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wunused
    -Wno-logical-op-parentheses
    -Wno-parentheses
    -Werror
    -Wno-comment
    -pedantic
    -pedantic-errors
    -D_GLIBCXX_DEBUG
    -D_GLIBCXX_DEBUG_PEDANTIC
    -D_UCRT
)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(COMPILE_OPTIONS 
        ${COMPILE_OPTIONS}
        -O3
    )
    if (WIN32)
        target_link_options(Tester PRIVATE -mwindows)
    endif()
else()
    set(COMPILE_OPTIONS 
        ${COMPILE_OPTIONS}
        -g
    )
endif()

target_compile_options(Tester PUBLIC ${COMPILE_OPTIONS})

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/shaders)

add_custom_target(shaders DEPENDS
    ${CMAKE_SOURCE_DIR}/shaders
    ${CMAKE_SOURCE_DIR}/icon
    ${CMAKE_SOURCE_DIR}/samples
)

set(SHADER_SRC "${CMAKE_SOURCE_DIR}/shaders")
set(SHADER_DST "${CMAKE_BINARY_DIR}/shaders")

add_custom_command(TARGET shaders POST_BUILD
    COMMAND glslc ${SHADER_SRC}/shader.vert -o ${SHADER_DST}/vert.spv
    COMMAND glslc ${SHADER_SRC}/shader.frag -o ${SHADER_DST}/frag.spv
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/icon ${CMAKE_BINARY_DIR}/icon
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/samples ${CMAKE_BINARY_DIR}/samples
)

add_dependencies(Tester shaders)
