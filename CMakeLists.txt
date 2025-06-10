# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(OpenGLES_Renderer VERSION 1.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Define source files
set(SOURCE_FILES
    src/gl_framebuffer_object.c
    src/gl_state.c
    src/gl_texture.c
    src/gl_utils.c
    src/gl_functions.c
    src/gl_extensions.c
    src/gl_init.c
    src/framebuffer.c
    src/matrix_utils.c
    src/memory_tracker.c
    src/logger.c
)
set(RENDERER_MAIN src/main.c)

# Define header files (for IDE visibility, optional)
set(HEADER_FILES
    src/gl_framebuffer_object.h
    src/gl_state.h
    src/gl_texture.h
    src/gl_utils.h
    src/matrix_utils.h
    src/framebuffer.h
    src/memory_tracker.h
    src/logger.h
    src/gl_extensions.h
    src/gl_init.h
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

# Find pthread
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Link libraries
target_link_libraries(renderer PRIVATE
    renderer_lib
    ${GLESv1_CM_LIBRARY}
    ${EGL_LIBRARY}
    Threads::Threads
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

add_subdirectory(benchmark)
add_subdirectory(conformance)
