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
    src/alpha_blend_demo.c
    src/milestone1.c
    src/milestone2.c
    src/texture_stream.c
    src/toggle_blend.c
    src/fill_rate.c
    src/pipeline_test.c
)

add_executable(benchmark ${BENCH_SOURCES})

add_executable(stage_logging_demo
    src/stage_logging_demo.c
)

target_include_directories(stage_logging_demo PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(stage_logging_demo PRIVATE renderer_lib pthread m)

set_target_properties(stage_logging_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

add_executable(stress_test
    src/stress_test.c
    src/benchmark.c
    src/stress_main.c
)

add_executable(perf_monitor
    src/perf_monitor.c
)

target_include_directories(perf_monitor PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(perf_monitor PRIVATE renderer_lib ${EXTRA_LIBS} m)

set_target_properties(perf_monitor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

target_include_directories(stress_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(stress_test PRIVATE renderer_lib pthread m)

set_target_properties(stress_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Include directories
 target_include_directories(benchmark PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
 )

# Link with renderer library
 target_link_libraries(benchmark PRIVATE renderer_lib pthread m)

set_target_properties(benchmark PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
