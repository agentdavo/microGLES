# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(OpenGLES_Renderer VERSION 1.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Define source files
set(SOURCE_FILES
    src/gl_framebuffer_object.c
    src/gl_state.c
    src/gl_texture.c
    src/gl_utils.c
    src/matrix_utils.c
    src/memory_tracker.c
    src/logger.c
    src/main.c
)

# Define header files (for IDE visibility, optional)
set(HEADER_FILES
    src/gl_framebuffer_object.h
    src/gl_state.h
    src/gl_texture.h
    src/gl_utils.h
    src/matrix_utils.h
    src/memory_tracker.h
    src/logger.h
)

# Create executable
add_executable(renderer ${SOURCE_FILES} ${HEADER_FILES})

# Include directories
target_include_directories(renderer PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
)

# Find OpenGL ES 1.1 (GLESv1_CM)
find_library(GLESv1_CM_LIBRARY NAMES GLESv1_CM GLES_CM)
if(NOT GLESv1_CM_LIBRARY)
    message(FATAL_ERROR "OpenGL ES 1.1 library (GLESv1_CM) not found")
endif()

# Find pthread
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Link libraries
target_link_libraries(renderer PRIVATE
    ${GLESv1_CM_LIBRARY}
    Threads::Threads
    m # Math library
)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Optional: Enable warnings and optimizations
target_compile_options(renderer PRIVATE
    -Wall -Wextra -Wpedantic # Enable warnings
    -O2                      # Enable optimization
)

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