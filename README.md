# microGLES — CPU-only OpenGL ES 1.1 Renderer

**microGLES** is a small, self-contained implementation of the fixed-function
OpenGL ES 1.1 pipeline.  
It is aimed at:

* **Conformance** – run the official ES 1.1 functional tests on machines with
  **no GPU** or broken drivers.
* **Benchmarking** – measure raw CPU raster throughput on embedded boards.
* **Reference / Teaching** – study a clean, fully-commented code-path that
  mirrors the classic GL pipeline step-by-step.

No SIMD intrinsics or platform APIs are required; just a C11 tool-chain.

---

## Feature Matrix

| Area | Status |
|------|--------|
| **Fixed-function core** | ✔ Matrix stacks, lighting (8 lights), fog, 2-unit texturing, alpha-test, depth & stencil, blending, scissor, point/line primitives |
| **Extensions** | ✔ `OES_framebuffer_object`, `OES_draw_texture`, `OES_point_sprite`, `OES_point_size_array`, `OES_matrix_palette` *(stubs for others)* |
| **Utilities** | ✔ `load_ktx_texture()` helper for KTX image loading |
| **Framebuffer** | ✔ RGBA8 + 32-bit float depth, atomic CAS writes, morton-swizzled layout |
| **Threading** | ✔ Lock-free MPMC queue, optional command buffer recorder, per-stage profiling (`--profile`) |
| **Pipeline** | ✔ 16×16 tiled fragment stage, 4×4 texture block cache |
| **State model** | ✔ Versioned `RenderContext`; worker threads clone only dirtied chunks. RenderContext now holds all dynamic flags (see docs/migration/state.md) |
| **Diagnostics** | ✔ Early-init memory tracker, async logger, built-in perf counters |
| **Tooling** | ✔ Release + ASAN builds, style check (`clang-format`), benchmarks, conformance harness |

---

## Project Layout

```

include/            <- Khronos GLES & GLEXT headers
src/
├─ api/             <- one file per GL 1.1 chapter  (gl\_api\_xxx.c)
├─ pipeline/        <- vertex → primitive → raster → fragment → framebuffer
├─ util/            <- logger, memory-tracker, math helpers
docs/               <- HTML docs & migration notes
benchmark/          <- perf micro-benchmarks
conformance/        <- spec-validation tests

````

> All public symbols are already declared by `#include <GLES/gl.h>` and
> `<GLES/glext.h>`.  No additional headers are installed.

---

## Building

> Tested on GCC 12 +, Clang 15 +, Linux / macOS / MinGW.  
> Requires CMake ≥ 3.20 and a C11-capable libc.

### Release

```bash
cmake -S . -B build \
      -DCMAKE_C_FLAGS="-std=gnu11 -O3 -ftree-vectorize"
cmake --build build                 # produces bin/* executables
./build/bin/benchmark
./build/bin/renderer_conformance
````

To gather per-stage timings at runtime, pass `--profile` to the
benchmark or conformance executables.

To enable the optional command buffer recorder, configure with:

```bash
cmake -S . -B build_cb \
      -DGL_ENABLE_MICROGLES_COMMAND_BUFFER=ON \
      -DCMAKE_C_FLAGS="-std=gnu11 -O3 -ftree-vectorize"
cmake --build build_cb
```

### Debug / Sanitizer

```bash
cmake -S . -B build_debug \
      -DCMAKE_C_FLAGS="-std=gnu11 -Og -g -fsanitize=undefined,address"
cmake --build build_debug
ASAN_OPTIONS=halt_on_error=1 ./build_debug/bin/renderer_conformance
```

Auto-format:

```bash
cmake --build build --target format    # runs clang-format on all .c/.h
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
    /* infrastructure */
    logger_init(NULL, LOG_LEVEL_INFO);
    memory_tracker_init();
    thread_pool_init(4);            /* 4 worker threads */
#ifdef ENABLE_PROFILE
    thread_profile_start();        /* optional: per-stage timings */
#endif

    /* GL context + default FBO */
    GLContext *ctx = context_create();      /* wraps global RenderContext */
    Framebuffer *fb = framebuffer_create(ctx, 256, 256);
    context_bind_default_fbo(ctx, fb);

    /* draw a cyan triangle */
    GLfloat verts[]  = { -0.5f,-0.5f,0,  0.5f,-0.5f,0,  0,0.5f,0 };
    GLfloat colors[] = {  0,1,1,1,    0,1,1,1,    0,1,1,1 };
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, verts);
    glColorPointer (4, GL_FLOAT, 0, colors);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    /* flush & dump */
    thread_pool_wait();
    framebuffer_write_rgba(fb, "tri.rgba");

    /* teardown */
    framebuffer_destroy(fb);
    context_destroy(ctx);
#ifdef ENABLE_PROFILE
    thread_profile_stop();
#endif
    thread_pool_shutdown();
    memory_tracker_report();        /* should show 0 leaks */
    logger_shutdown();
}
```
Compile with `-DENABLE_PROFILE` or pass `--profile` to the bundled
programs to record per-stage timings.

---

## Internal Pipeline

```
API thread
┌──────────────────────┐
│ gl_api_draw.c        │   validate + build GLDrawCall
└──────────────────────┘
        │  submit
        ▼
Thread pool workers
┌──────────────────────┐  VertexFetch: copy arrays → SoA
│ gl_vertex_fetch.c    │  Transform + lighting
└─▲──────────────┬─────┘
  │              │
  │              ▼
  │      gl_primitive.c       Assemble, cull, clip
  │              │
  │              ▼
  │      gl_raster.c          Edge functions, emit 16×16 tile jobs
  │              │
  │              ▼
  │      gl_fragment.c        Shade tile buffer, texturing, fog, blend
  │              │
  ▼              ▼
 gl_framebuffer.c (atomic depth/stencil/color)
```

Each stage increments its own profiling counters (`thread_profile_report()`).

---

## Contributing

* Follow **`AGENTS.md`** (build, test, clang-format, conformance pass).
* Keep each `gl_api_*.c` focused on a single spec chapter.
* Prefer `MT_ALLOC / MT_FREE` and `thread_pool_submit()` over raw `malloc` /
  `thrd_create`.
* Document non-obvious algorithms (e.g. atomic depth CAS loop) inline.

Pull requests that break conformance or style are rejected automatically.

---

## License

MIT — see [`LICENSE`](LICENSE).
