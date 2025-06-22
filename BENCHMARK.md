# Benchmark Results

The `benchmark` executable exercises a collection of small scenes to
measure renderer performance.  The table below lists the scenes and the
frames-per-second (FPS) results observed on the reference machine.  Your
numbers may vary depending on hardware and compiler options.

Test system: Ubuntu 24.04, Intel Xeon Platinum 8370C (5 cores), 10 GiB RAM.
Built with GCC 13.3 using `-std=gnu11 -O3 -ftree-vectorize`.

| Benchmark Scene            | FPS |
|----------------------------|-----|
| Triangle Strip (100)       | 179211 FPS |
| Triangle Strip (1000)      | 54289 FPS |
| Triangle Strip (10000)     | 6306 FPS |
| Textured Quad              | 359712 FPS |
| Lit Cube (lighting on)     | 336700 FPS |
| Lit Cube (lighting off)    | 288184 FPS |
| FBO operations             | 100000000 FPS |
| Pipeline Test              | 33333333 FPS |
| Spinning Gears             | 177620 FPS |
| Spinning Cubes             | 57264 MP/s |
| Multitexture Demo          | 1101 MP/s |
| Alpha Blend Demo           | 25000000 FPS |
| Fill Rate Suite            | N/A |
| Stress Test (1M cubes)     | 47 FPS |
