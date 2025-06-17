add_library(lv_lib_uGLES STATIC
    main.c
)

# Include directories for microGLES and allow external LVGL include path
# Users must supply LVGL headers when enabling this target

set_target_properties(lv_lib_uGLES PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

target_include_directories(lv_lib_uGLES PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(lv_lib_uGLES PUBLIC renderer_lib pthread m)
