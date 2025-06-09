When modifying code in this folder:

1. Format all `.c` and `.h` files using `clang-format -i` before committing.
2. Ensure the project still builds using CMake:
   ```bash
   cmake -S . -B build
   cmake --build build
   ```
3. Update README.md in this folder if the public API or build steps change
4. Benchmark sources under `benchmark/` follow the same rules.
