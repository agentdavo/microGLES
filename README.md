# microGLES - Software OpenGL ES 1.1 Renderer

microGLES is a minimal OpenGL ES 1.1 renderer designed primarily for testing and benchmarking purposes. The project implements a subset of the API with a focus on framebuffer objects, texture management and matrix utilities. A simple software framebuffer allows the renderer to operate on systems without GPU hardware.

## Features

- Framebuffer object support with color and depth buffers
- Texture creation, uploading and basic filtering
- Matrix utility functions for building transformations
- Memory tracking with leak reporting
- Thread–safe logging system
- Benchmarks measuring common rendering workloads
- Conformance tests validating API behaviour

## Directory Layout

```
benchmark/    Performance benchmarks
conformance/  Functional tests
include/      OpenGL ES headers
src/          Renderer implementation
logs/         Log files
```

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
if (!InitLogger("renderer.log", LOG_LEVEL_DEBUG) || !InitMemoryTracker())
    return -1;
InitGLState(&gl_state);
Framebuffer *fb = GL_init_with_framebuffer(256, 256);
// rendering commands here ...
GL_cleanup_with_framebuffer(fb);
CleanupGLState(&gl_state);
ShutdownMemoryTracker();
ShutdownLogger();
```

Benchmarks and conformance tests follow a similar pattern and already use the helper functions above.

## Contributing

Contributions are welcome! Please format all source files with `clang-format -i` and ensure the project builds successfully before opening a pull request.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
