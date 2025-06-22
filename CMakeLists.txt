# Minimum CMake version required
cmake_minimum_required(VERSION 3.20)

# Project name and version
project(OpenGLES_Renderer VERSION 1.0 LANGUAGES C)


# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Define source files
set(SOURCE_FILES
    src/gl_api_state.c
    src/gl_api_matrix.c
    src/gl_api_vertex_array.c
    src/gl_api_draw.c
    src/gl_api_texture.c
    src/gl_api_lighting.c
    src/gl_api_depthstencil.c
    src/gl_api_blend.c
    src/gl_api_pixels.c
    src/gl_api_misc.c
    src/extensions/gl_ext_OES_framebuffer_object.c
    src/gl_api_buffer.c
    src/gl_state.c
    src/gl_utils.c
    src/gl_errors.c
    src/extensions/gl_ext_common.c
    src/extensions/gl_ext_OES_draw_texture.c
    src/extensions/gl_ext_OES_matrix_get.c
    src/extensions/gl_ext_OES_point_size_array.c
    src/extensions/gl_ext_OES_point_sprite.c
    src/extensions/gl_ext_OES_egl_image.c
    src/extensions/gl_ext_OES_egl_image_external.c
    src/extensions/gl_ext_OES_required_internalformat.c
    src/extensions/gl_ext_OES_blend_eq_sep.c
    src/extensions/gl_ext_OES_fixed_point.c
    src/fixed_point.c
    src/gl_context.c
    src/gl_thread.c
    src/texture_cache.c
    src/function_profile.c
    src/ktx_loader.c
    src/command_buffer.c
    src/x11_window.c
    src/gl_init.c
    src/pipeline/gl_framebuffer.c
    src/pipeline/gl_vertex.c
    src/pipeline/gl_primitive.c
    src/pipeline/gl_raster.c
    src/pipeline/gl_fragment.c
    src/matrix_utils.c
    src/pool.c
    src/gl_memory_tracker.c
    src/gl_logger.c
)

include(CheckIncludeFile)
check_include_file(threads.h HAVE_THREADS_H)
if(NOT HAVE_THREADS_H)
    list(APPEND SOURCE_FILES src/portable/tinycthread.c)
endif()
include(CheckFunctionExists)
check_function_exists(fmemopen HAVE_FMEMOPEN)
if(NOT HAVE_FMEMOPEN)
    add_definitions(-DNO_FMEMOPEN)
endif()
set(RENDERER_MAIN src/main.c)

# Define header files (for IDE visibility, optional)
set(HEADER_FILES
    src/gl_api_fbo.h
    src/gl_api_vertex_array.h
    src/gl_state.h
    src/gl_utils.h
    src/fixed_point.h
    src/matrix_utils.h
    src/gl_types.h
    src/pipeline/gl_framebuffer.h
    src/pipeline/gl_vertex.h
    src/pipeline/gl_primitive.h
    src/pipeline/gl_raster.h
    src/pipeline/gl_fragment.h
    src/pool.h
    src/gl_memory_tracker.h
    src/gl_logger.h
    src/gl_context.h
    src/gl_thread.h
    src/texture_cache.h
    src/ktx_loader.h
    src/command_buffer.h
    src/gl_init.h
    src/x11_window.h
)

# Create executable
add_library(renderer_lib STATIC ${SOURCE_FILES} ${HEADER_FILES})

add_executable(renderer ${RENDERER_MAIN})

# Include directories
target_include_directories(renderer_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
)

# Locate GLES and EGL libraries if available. The headers are provided in
# the `include/` directory so we avoid picking up desktop variants.
find_library(GLESv1_CM_LIBRARY GLESv1_CM)
find_library(EGL_LIBRARY EGL)
if(NOT GLESv1_CM_LIBRARY OR NOT EGL_LIBRARY)
    message(WARNING "GLESv1_CM or EGL not found. Building without system libs.")
    set(GLESv1_CM_LIBRARY "")
    set(EGL_LIBRARY "")
endif()

# Optional POSIX thread library
if(NOT WIN32)
    list(APPEND EXTRA_LIBS pthread)
endif()
find_package(X11)
if(X11_FOUND)
    target_compile_definitions(renderer_lib PUBLIC HAVE_X11)
    target_include_directories(renderer_lib PUBLIC ${X11_INCLUDE_DIR})
    list(APPEND EXTRA_LIBS ${X11_LIBRARIES})
endif()

# Link libraries
target_link_libraries(renderer PRIVATE
    renderer_lib
    ${GLESv1_CM_LIBRARY}
    ${EGL_LIBRARY}
    ${EXTRA_LIBS}
    m # Math library
)

# Set output directories
set_target_properties(renderer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

# Optional: Enable warnings and optimizations
target_compile_options(renderer_lib PRIVATE -Wall -Wextra -Wpedantic -O2)
target_compile_options(renderer PRIVATE -Wall -Wextra -Wpedantic -O2)
target_compile_definitions(renderer_lib PUBLIC MICROGLES_COMMAND_BUFFER)
target_compile_definitions(renderer PUBLIC MICROGLES_COMMAND_BUFFER)

# Optional: Create logs directory if it doesn't exist
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/logs)

# Optional: Install target
install(TARGETS renderer DESTINATION bin)

# Optional: Add custom target for running the application
add_custom_target(run
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/renderer
    DEPENDS renderer
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running the renderer application"
)

# Formatting target
file(GLOB_RECURSE SOURCES
    "src/*.h" "src/*.c"
    "benchmark/*.h" "benchmark/*.c"
    "conformance/*.h" "conformance/*.c")
list(FILTER SOURCES EXCLUDE REGEX "CMakeLists.txt")
list(FILTER SOURCES EXCLUDE REGEX "\\.md$")
find_program(CLANG_FORMAT clang-format)
if(CLANG_FORMAT)
    add_custom_target(format
        COMMAND ${CLANG_FORMAT} -i ${SOURCES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running clang-format on src, benchmark, and conformance")
endif()

add_subdirectory(benchmark)
add_subdirectory(conformance)

enable_testing()
add_test(
    NAME conformance
    COMMAND renderer_conformance
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/docs/release")
    configure_file(${CMAKE_SOURCE_DIR}/docs/Doxyfile.in
                   ${CMAKE_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generate API documentation with Doxygen")
endif()

# RISC-V cross compilation tasks
add_custom_target(riscv32_build
    COMMAND ${CMAKE_COMMAND} -S ${CMAKE_SOURCE_DIR} -B build_rv32 -DCMAKE_TOOLCHAIN_FILE=${CMAKE_SOURCE_DIR}/cmake/riscv32-toolchain.cmake -DENABLE_PROFILE=ON
    COMMAND ${CMAKE_COMMAND} --build build_rv32 --target benchmark
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Build benchmark for RISC-V 32")

add_custom_target(riscv64_build
    COMMAND ${CMAKE_COMMAND} -S ${CMAKE_SOURCE_DIR} -B build_rv64 -DCMAKE_TOOLCHAIN_FILE=${CMAKE_SOURCE_DIR}/cmake/riscv64-toolchain.cmake -DENABLE_PROFILE=ON
    COMMAND ${CMAKE_COMMAND} --build build_rv64 --target benchmark
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Build benchmark for RISC-V 64")

add_custom_target(run_rv32
    COMMAND qemu-riscv32 ${CMAKE_SOURCE_DIR}/build_rv32/bin/benchmark > ${CMAKE_SOURCE_DIR}/benchmark/RV32.md || true
    DEPENDS riscv32_build
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Run RISC-V32 benchmark via QEMU")

add_custom_target(run_rv64
    COMMAND qemu-riscv64 ${CMAKE_SOURCE_DIR}/build_rv64/bin/benchmark > ${CMAKE_SOURCE_DIR}/benchmark/RV64.md || true
    DEPENDS riscv64_build
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Run RISC-V64 benchmark via QEMU")
