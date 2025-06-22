# microGLES — Contributor & AI-Agent Guide  

_Last updated 2025-06-11_

## 1 Golden Requirements (“**MUST**”)

| # | Requirement | Notes |
|---|-------------|-------|
| 1 | **OpenGL ES 1.1 conformance is non-negotiable.** | Run `renderer_conformance` (CTest) before every commit. |
| 2 | **Builds must succeed** in release and debug/sanitizer modes. | See build flags below. |
| 3 | **clang-format** via the `format` target *before* committing. |
| 4 | **Memory & threading**: use `gl_memory_tracker`, `gl_thread` wrappers only. |
| 5 | **Zero leaks** and **zero GL errors** after conformance run. |
| 6 | **No public headers added** beyond Khronos `<GLES/*.h>`. |
| 7 | **Commit messages** follow “<72 char summary> + wrapped body”. |
| 8 | **Every GLES 1.1 entry point must exist.** | 145 stubs/impls required. |

### Build invocations

```bash
cmake -S . -B build         -DCMAKE_C_FLAGS="-std=gnu11 -O3 -ftree-vectorize"
cmake --build build

cmake -S . -B build_debug   -DCMAKE_C_FLAGS="-std=gnu11 -Og -g -fsanitize=undefined,address"
cmake --build build_debug
````

Run:

```bash
./build/bin/benchmark
./build/bin/renderer_conformance     # CI hooked via CTest
```


## 2 Strong Recommendations (“**SHOULD**”)

* Verify C11 built-ins (`alignas`, `_Atomic`, `_Generic`, `__builtin_expect`) on your tool-chain.
* Keep modules narrow—one GL spec chapter per `gl_api_*.c`, one Khronos extension per `gl_ext_*.c`.
* Prefer `CHECK_OK`, `CHECK_GLError` macros in new tests.
* Document tricky paths (atomic pixel ops, work-stealing logic) inline.


## 3 File-Structure Snapshot (possibly)

```
src/
├─ extensions/               gl_ext_OES_draw_texture.c, …
├─ pipeline/                 vertex → primitive → raster → fragment → framebuffer
conformance/                 table-driven tests + golden RGBAs
benchmark/                   perf demos
```

## 4 Commit Checklist (quick)

1. Release & sanitizer builds succeed.
2. `renderer_conformance` passes (0 leaks, 0 GL errors).
3. Benchmarks show ≤ 5 % perf regression (or explain).
4. `clang-format` shows no diff.
5. Update `README.md` and `CONFORMANCE.md` if behaviour changes.
6. Push changes and open a merge request.
