
# microGLES — CPU-only OpenGL ES 1.1 Renderer

**microGLES** is a small, self-contained implementation of the fixed-function OpenGL ES 1.1 pipeline. It is designed for:

- **Conformance**: Run official ES 1.1 functional tests on machines with no GPU or broken drivers.
- **Benchmarking**: Measure raw CPU raster throughput on embedded boards.
- **Reference/Teaching**: Study a clean, fully-commented code path that mirrors the classic GL pipeline step-by-step.

No SIMD intrinsics or platform APIs are required; only a C11 toolchain is needed.

---

## Feature Matrix

| Area                | Status                                                                 |
|---------------------|----------------------------------------------------------------------|
| **Fixed-function core** | ✔ Matrix stacks, lighting (8 lights), fog, 2-unit texturing, alpha-test, depth & stencil, blending, scissor, point/line primitives |
| **Extensions**      | ✔ `OES_framebuffer_object`, `OES_draw_texture`, `OES_point_sprite`, `OES_point_size_array`, `OES_matrix_palette` (stubs for others) |
| **Utilities**       | ✔ `load_ktx_texture()` helper for KTX image loading                  |
| **Framebuffer**     | ✔ RGBA8 + 32-bit float depth, atomic CAS writes, morton-swizzled layout |
| **Threading**       | ✔ Lock-free MPMC queue, built-in command buffer recorder, per-stage profiling (`--profile`) |
| **Pipeline**        | ✔ 16×16 tiled fragment stage, 4×4 texture block cache                |
| **State model**     | ✔ Versioned `RenderContext`; worker threads clone only dirtied chunks. RenderContext holds all dynamic flags (see `docs/migration/state.md`) |
| **Diagnostics**     | ✔ Early-init memory tracker, async logger, built-in perf counters    |
| **Tooling**         | ✔ Release + ASAN builds, style check (`clang-format`), benchmarks, conformance harness |
| **API Coverage**    | ✔ All 145 OpenGL ES 1.1 entry points implemented |

Note: Rasterization now discards fragments outside the scissor box when `GL_SCISSOR_TEST` is enabled.

---

## Project Layout

```
include/
  └─ Khronos GLES & GLEXT headers
src/
  ├─ api/        # One file per GL 1.1 chapter (gl_api_xxx.c)
  ├─ pipeline/   # Vertex → primitive → raster → fragment → framebuffer
  ├─ util/       # Logger, memory-tracker, math helpers
docs/            # HTML docs & migration notes
benchmark/       # Perf micro-benchmarks
conformance/     # Spec-validation tests
```

All public symbols are declared by `#include <GLES/gl.h>` and `<GLES/glext.h>`. No additional headers are installed.

---

## Building

Tested on GCC 12+, Clang 15+, Linux/macOS/MinGW. Requires CMake ≥ 3.20 and a C11-capable libc.

### Release

```bash
cmake -S . -B build -DCMAKE_C_FLAGS="-std=gnu11 -O3 -ftree-vectorize"
cmake --build build
# Produces bin/* executables
./build/bin/benchmark
./build/bin/renderer_conformance
```
Set `MICROGLES_THREADS` to specify the number of worker threads (defaults to the
number of online CPUs).

To gather per-stage timings at runtime, pass `--profile` to the benchmark or conformance executables. The stress_test program also accepts `--profile` for analyzing the million-cube scene. Pass `--stream-fb` to stress_test to pipe the framebuffer as raw RGBA to stdout for tools like `ffmpeg`. Use `--x11-window --width=640 --height=480` to display the framebuffer in an X11 window. The command buffer recorder is always enabled, so no extra build flags are required.

### Debug / Sanitizer

```bash
cmake -S . -B build_debug \
      -DCMAKE_C_FLAGS="-std=gnu11 -Og -g -fsanitize=undefined,address"
cmake --build build_debug
ASAN_OPTIONS=halt_on_error=1 ./build_debug/bin/renderer_conformance
```

Auto-format:

```bash
cmake --build build --target format  # Runs clang-format on all .c/.h
cmake --build build --target doc    # Generate HTML docs under docs/release
```

Developers may build with `-Werror` to surface warnings as errors:

```bash
cmake -S . -B build -DCMAKE_C_FLAGS="-std=gnu11 -O3 -ftree-vectorize -Werror"
cmake --build build
```

### Debugging with Valgrind and GDB

Use these tools to catch memory errors and inspect crashes:

```bash
valgrind ./build/bin/renderer_conformance
gdb --args ./build/bin/renderer_conformance
```

---

## Quick Start

```c
#include <GLES/gl.h>
#include "gl_init.h"
#include "gl_logger.h"
#include "gl_memory_tracker.h"
#include "gl_thread.h"

int main(void)
{
    /* Infrastructure */
    logger_init(NULL, LOG_LEVEL_INFO);
    memory_tracker_init();
    thread_pool_init_from_env(); /* uses MICROGLES_THREADS or the
                                   number of online CPUs */
#ifdef ENABLE_PROFILE
    thread_profile_start(); /* Optional: per-stage timings */
#endif

    /* GL context + default FBO */
    GLContext *ctx = context_create(); /* Wraps global RenderContext */
    Framebuffer *fb = framebuffer_create(ctx, 256, 256);
    context_bind_default_fbo(ctx, fb);

    /* Draw a cyan triangle */
    GLfloat verts[] = { -0.5f, -0.5f, 0, 0.5f, -0.5f, 0, 0, 0.5f, 0 };
    GLfloat colors[] = { 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1 };
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, verts);
    glColorPointer(4, GL_FLOAT, 0, colors);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    /* Flush & dump */
    thread_pool_wait();
    framebuffer_write_rgba(fb, "tri.rgba");

    /* Teardown */
    framebuffer_destroy(fb);
    context_destroy(ctx);
#ifdef ENABLE_PROFILE
    thread_profile_stop();
#endif
    thread_pool_shutdown();
    memory_tracker_report(); /* Should show 0 leaks */
    logger_shutdown();
}
```

Compile with `-DENABLE_PROFILE` or run any program with `--profile` to record per-stage timings. Without the flag, tasks still execute but no profiling counters are recorded.
Set `MICROGLES_THREADS` to override the default thread count (number of online
CPUs).

## Profiling

microGLES includes a lightweight profiler integrated into the thread pool. When
profiling is enabled (either by compiling with `-DENABLE_PROFILE` or passing
`--profile` on the command line) each worker thread measures the cycle count
spent in every pipeline stage while processing a command buffer. At shutdown
`thread_profile_report()` prints a table showing task counts, average time per
task (microseconds), maximum queue depth, longest task time and cache statistics
for the vertex, primitive, raster, fragment, framebuffer and steal stages. The
data pinpoints bottlenecks—e.g.
excessive fragment time may suggest better texture caching or smaller tile
size—allowing refinement of math routines and thread counts.

The conversion from cycle counts to microseconds is platform dependent.
Systems without a cycle counter treat the value as nanoseconds. When
`__builtin_readcyclecounter` is available, the profiler calibrates CPU
frequency on first use so reported times are approximations.

### Linking as a Static Library

Build only the library if you want to embed microGLES in another project:

```bash
cmake -S . -B build
cmake --build build --target renderer_lib
```

Then link `build/lib/librenderer_lib.a` and include the `src/` headers. Use the
initialization helpers from the Quick Start section (e.g., `context_create`,
`framebuffer_create`). When compiled with X11, running an app with
`--x11-window` will display the framebuffer.
---

## Internal Pipeline

```
API thread
┌──────────────────────┐
│ gl_api_draw.c        │ Validate + build GLDrawCall
└──────────────────────┘
        │ Submit
        ▼ Thread pool workers
┌──────────────────────┐ VertexFetch: Copy arrays → SoA
│ gl_vertex_fetch.c    │ Transform + lighting
└─▲──────────────┬─────┘
  │              │
  │              ▼
  │ gl_primitive.c   Assemble, cull, clip
  │              │
  │              ▼
  │ gl_raster.c      Edge functions, emit 16×16 tile jobs
  │              │
  │              ▼
  │ gl_fragment.c    Shade tile buffer, texturing, fog, blend
  │              │
  ▼              ▼ gl_framebuffer.c (atomic depth/stencil/color)
```

Each stage increments its own profiling counters (`thread_profile_report()`).

---

## Contributing

- Follow **AGENTS.md** (build, test, clang-format, conformance pass).
- Keep each `gl_api_*.c` focused on a single spec chapter.
- Prefer `MT_ALLOC/MT_FREE` and `thread_pool_submit()` over raw `malloc/thrd_create`.
- Document non-obvious algorithms (e.g., atomic depth CAS loop) inline.
- See [`docs/PORTABILITY.md`](docs/PORTABILITY.md) for cross-platform guidelines.

Pull requests that break conformance or style are rejected automatically.

---

## License

MIT — see [`LICENSE`](LICENSE).
