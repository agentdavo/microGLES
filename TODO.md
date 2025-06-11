# TODO Progress Log

## 2025-06-11 - Implementation of review checklist tasks

- [x] TLS thread-id fix in gl_thread.c
- [x] Portable get_cycles() implementation
- [x] Logger flush on shutdown
- [x] thread_pool_wait_timeout() API
- [x] Begin phase A of state unification

## 2025-06-12 - Address additional checklist feedback
- [x] Expand local task queue and adopt LIFO popping
- [x] Improve idle policy with short sleep
- [x] Dynamic memory tracker table with strdup'd type strings
- [x] Logger drop counter, µs timestamps and optional file output
- [x] Thread pool initialized before GL state in benchmark main

## 2025-06-13 - State convergence cleanup
- Removed duplicated texture and light enable flags from GLState
- Introduced SET_BOOL/SET_FLOAT4 macros for state updates
- Updated enable/disable functions to rely on RenderContext
- Completed phase B: moved depth/stencil/blend, raster and array state into
  RenderContext with version counters and updated pipeline guards.

## 2025-06-14 - Finalize API split
- Removed legacy gl_functions.c implementations; file now asserts "All API moved"
- Added new source files gl_api_lighting.c and gl_api_vertex_array.c for lighting and client array entry points
- Expanded gl_api_texture.c and gl_api_pixels.c with missing API such as glTexEnv* and glClearColor
- Updated CMakeLists.txt to list all gl_api_*.c files exactly once

## 2025-06-15 - Address review checklist
- Deleted obsolete gl_functions.c and removed from build
- Implemented glPixelStorei, glLogicOp and point parameter APIs
- Moved polygon offset to depth/stencil module
- Relocated fixed-point wrappers next to their float variants
- Pruned public headers for gl_api_* files and cleaned CMake lists
\n## 2025-06-16 - Extension modularisation
- Split gl_extensions.c into dedicated files under src/extensions.
- Added extension registry utility and updated CMake.

## 2025-06-17 - Complete extension split
- Added OES_EGL_image, OES_EGL_image_external, OES_required_internalformat and
  OES_blend_equation_separate modules.
- Updated build files accordingly.
