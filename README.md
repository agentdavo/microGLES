#microGLES - Software OpenGL ES 1.1 Renderer

microGLES is a minimal OpenGL ES 1.1 renderer designed primarily for testing and benchmarking purposes. The project implements a subset of the API with a focus on framebuffer objects, texture management and matrix utilities. A simple software framebuffer allows the renderer to operate on systems without GPU hardware.

## Features

- Framebuffer object support with color and depth buffers
- Texture creation, uploading and basic filtering
- Support for common extensions: OES_draw_texture, OES_framebuffer_object,
  OES_point_sprite, OES_point_size_array, OES_matrix_palette and more
- Matrix utility functions for building transformations
- Memory tracking with leak reporting
- Thread–safe logging system
- Lightweight thread pool and lock-free task queue
- Stage-tagged profiling of pipeline tasks
- Global render context with thread-local error management
- Versioned render context allows threads to copy only modified state
- Modular pipeline stages separating vertex, primitive, raster, and fragment logic
- Benchmarks measuring common rendering workloads
- Conformance tests validating API behaviour

## Directory Layout

```
benchmark/    Performance benchmarks
conformance/  Functional tests
include/      OpenGL ES headers
src/          Renderer implementation
src/pipeline/ Pipeline stage modules
logs/         Log files
```

The repository ships the standard Khronos headers. Extension prototypes are
available through `GLES/glext.h` and no separate `gl_extensions.h` is needed.

## Building

The project uses CMake. A recent GCC or Clang toolchain with C11 support is required.

### Release build

```bash
cmake -S . -B build -DCMAKE_C_FLAGS="-std=gnu11 -O3 -ftree-vectorize"
cmake --build build
```

### Debug/sanitizer build

```bash
cmake -S . -B build_debug -DCMAKE_C_FLAGS="-std=gnu11 -Og -g -fsanitize=undefined,address"
cmake --build build_debug
```

This will produce the `renderer`, `benchmark` and `conformance` executables in `build/bin/` or `build_debug/bin/` respectively.

## Quick Start

```c
if (!logger_init(LOG_LEVEL_DEBUG) || !memory_tracker_init())
    return -1;
thread_pool_init(4);
thread_profile_start();
context_init();
InitGLState(&gl_state);
Framebuffer *fb = GL_init_with_framebuffer(256, 256);
// rendering commands here ...
GL_cleanup_with_framebuffer(fb);
CleanupGLState(&gl_state);
thread_profile_stop();
thread_pool_shutdown();
memory_tracker_shutdown();
logger_shutdown();
```

Benchmarks and conformance tests follow a similar pattern and already use the helper functions above.
## Architecture

microGLES organises its renderer around a global `RenderContext` and a lock-free thread pool.
All state is stored in `gl_types.h` with atomic version counters so worker
threads only copy changed data. The pool in `gl_thread.c` executes tasks from a
multiple-producer, multiple-consumer queue with work stealing.
Pipeline stages live in `src/pipeline/` (vertex, primitive, raster, fragment and
framebuffer) and operate on thread-local snapshots of `RenderContext` while
writing results to a software framebuffer.

Each stage submits jobs to the pool:
* **gl_vertex_array.c** – transforms vertices and applies lighting for calls
  such as `glDrawArrays`.
* **gl_primitive.c** – builds primitives and performs culling.
* **gl_raster.c** – converts primitives into fragments.
* **gl_fragment.c** – shades fragments using texture state, including
  extension behaviour from `gl_extensions.c`.
* **gl_framebuffer.c** – writes pixels and clears buffers using atomic memory
  operations for thread-safe access.

Core ES 1.1 entry points live in `gl_functions.c` and extensions in
`gl_extensions.c`;
both enqueue work so multiple cores can share the 320×240 framebuffer
	workload efficiently.

`GL_init_with_framebuffer()` sets up the context and framebuffer; `GL_cleanup_with_framebuffer()` tears them down.

## Contributing

Contributions are welcome! Please format all source files with `clang-format -i` and ensure the project builds successfully before opening a pull request.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
