# Conformance test suite

set(CONFORMANCE_SOURCES
    src/tests_main.c
    src/tests/state.c
    src/tests/matrix.c
    src/tests/texture.c
    src/tests/buffer.c
    src/tests/draw.c
    src/tests/fbo.c
    src/tests/depth.c
    src/tests/state_flag_version.c
    src/tests/texture_cache.c
    src/tests/thread_stress.c
    src/tests/point_sprite.c
    src/tests/fixed_point.c
    src/tests/all_calls.c
    src/tests/extensions.c
    src/tests/util.c
)

find_package(Python3 COMPONENTS Interpreter REQUIRED)
set(AUTOGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/tests_autogen.in)
set(AUTOGEN_C ${CMAKE_CURRENT_BINARY_DIR}/autogen.c)
add_custom_command(OUTPUT ${AUTOGEN_C}
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/gen_autogen_tests.py ${AUTOGEN_IN} ${AUTOGEN_C}
    DEPENDS ${AUTOGEN_IN} ${CMAKE_CURRENT_SOURCE_DIR}/gen_autogen_tests.py)
list(APPEND CONFORMANCE_SOURCES ${AUTOGEN_C})

add_executable(renderer_conformance ${CONFORMANCE_SOURCES})

# Include directories
 target_include_directories(renderer_conformance PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
 )

# Link with renderer library
 target_link_libraries(renderer_conformance PRIVATE renderer_lib pthread m)

set_target_properties(renderer_conformance PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
