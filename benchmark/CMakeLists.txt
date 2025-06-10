# Benchmark build configuration

set(BENCH_SOURCES
    src/main.c
    src/benchmark.c
    src/triangle_strip.c
    src/textured_quad.c
    src/lit_cube.c
    src/fbo_benchmark.c
    src/spinning_gears.c
    src/spinning_cubes.c
    src/multitexture_demo.c
)

add_executable(benchmark ${BENCH_SOURCES})

# Include directories
 target_include_directories(benchmark PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
 )

# Link with renderer library
 target_link_libraries(benchmark PRIVATE renderer_lib Threads::Threads m)

set_target_properties(benchmark PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
