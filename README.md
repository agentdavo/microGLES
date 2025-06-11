# microGLES — Software OpenGL ES 1.1 Renderer

*Minimal-footprint, fully-conformant, CPU-only implementation of the OpenGL ES 1.1 fixed-function pipeline.*

microGLES targets three use-cases:

1. **Conformance** – pass the ES 1.1 test suite on any CPU with no GPU driver.
2. **Benchmarking** – measure raw triangle / fragment throughput on embedded boards.
3. **Education** – expose a clean, comment-rich reference of the classic GL pipeline.

---

## Features

| Category | Highlights |
|----------|------------|
| **Core ES 1.1** | Full matrix stacks, lighting (8 lights), texturing (2 units), fog, alpha test, stencil, blending, scissor, point & line rasterisation |
| **Extensions** | `OES_framebuffer_object`, `OES_draw_texture`, `OES_point_sprite`, `OES_point_size_array`, `OES_matrix_palette` (stubs for others) |
| **Framebuffer** | In-memory color + depth (atomic 32-bit RGBA / 32-bit depth) with wrap-around swizzle for cache-friendly writes |
| **Threading** | Lock-free MPMC task queue, work-stealing thread pool, stage-tagged profiling |
| **State** | Versioned `RenderContext`; worker threads copy **only** modified chunks |
| **Diagnostics** | Memory-leak tracker, sanitizer-clean, thread-safe logger (printf-style with timestamps) |
| **Tests** |   *Benchmark* suite for perf, *Conformance* suite for spec compliance |

---

## Directory Layout

```

include/            Khronos GLES + GLEXT headers
src/
├─ api/             gl\_api\_\*.c  ← each file = one spec chapter
├─ pipeline/        vertex → primitive → raster → fragment → framebuffer
├─ util/            logger, memory tracker, math helpers
benchmark/          Performance micro-benchmarks
conformance/        Functional test harness

````

> **No additional public headers** are shipped; user code only includes  
> `<GLES/gl.h>` and `<GLES/glext.h>`.

---

## Build & Test

> **Toolchain:** GCC 12 + , Clang 15 +  (C11).  
> **Host OS:** Linux / macOS; Windows via MinGW & CMake ≥ 3.20.

### Release

```bash
cmake -S . -B build \
      -DCMAKE_C_FLAGS="-std=gnu11 -O3 -ftree-vectorize"
cmake --build build           # builds renderer + tests
./build/bin/benchmark
./build/bin/conformance
````

### Debug / Sanitizer

```bash
cmake -S . -B build_debug \
      -DCMAKE_C_FLAGS="-std=gnu11 -Og -g -fsanitize=undefined,address"
cmake --build build_debug
ASAN_OPTIONS=halt_on_error=1 ./build_debug/bin/conformance
```

Formatting:

```bash
cmake --build build --target format     # clang-format auto-fix
```

---

## Quick Start (C)

```c
GLContext *ctx = context_create(4 /*threads*/);

Framebuffer *fb = framebuffer_create(ctx, 256, 256);
context_bind_default_fbo(ctx, fb);

/* 1× red triangle */
glEnable(GL_DEPTH_TEST);
glVertexPointer(3, GL_FLOAT, 0, verts);
glColorPointer (3, GL_FLOAT, 0, colors);
glEnableClientState(GL_VERTEX_ARRAY);
glEnableClientState(GL_COLOR_ARRAY);
glDrawArrays(GL_TRIANGLES, 0, 3);

framebuffer_write_bmp(fb, "triangle.bmp");

framebuffer_destroy(fb);
context_destroy(ctx);
```

---

## Architecture in 60 s

```
┌─────────────────────┐
│  gl_api_*.c (API)   │  ← validates params, builds GLDrawCall
└─────────┬───────────┘
          ▼
┌─────────────────────┐
│ gl_vertex_fetch.c   │  ← copies arrays, applies model/view/texture
│ gl_vertex.c         │     lighting, viewport transform
└─────────┬───────────┘
          ▼
┌─────────────────────┐
│ gl_primitive.c      │  ← assemble + cull + (optional) clip
└─────────┬───────────┘
          ▼
┌─────────────────────┐             thread-pool tiles
│ gl_raster.c         │─┬─► RasterJob{16×16} ──► worker N
└─────────────────────┘ │
                        └─► worker M …
```

* Each worker shades fragments → `gl_fragment.c` → atomic `framebuffer_set_pixel`.
* Depth/stencil pixel ops are lock-free CAS loops; color blending uses
  per-tile ownership to avoid races.

---

## Contributing

1. **Follow `AGENTS.md`.** Short version: run both builds, all tests, and
   `clang-format` before you push.
2. Keep modules narrow: one GL chapter per `src/api/gl_api_*.c`.
3. Public behaviour change ⇒ update this README + `include/CHANGES.md`.

---

## License

MIT — see [`LICENSE`](LICENSE) for full text.
