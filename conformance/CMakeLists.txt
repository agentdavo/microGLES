# Conformance test suite

set(CONFORMANCE_SOURCES
    src/main.c
    src/tests.c
)

add_executable(conformance ${CONFORMANCE_SOURCES})

# Include directories
 target_include_directories(conformance PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
 )

# Link with renderer library
 target_link_libraries(conformance PRIVATE renderer_lib Threads::Threads m)

set_target_properties(conformance PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
