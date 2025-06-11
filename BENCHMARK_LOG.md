=================================================================
==7632==ERROR: AddressSanitizer: heap-use-after-free on address 0x7f3398235220 at pc 0x55ea4c89c35d bp 0x7f3393efbac0 sp 0x7f3393efbab0
WRITE of size 4 at 0x7f3398235220 thread T-1
    #0 0x55ea4c89c35c in framebuffer_clear /workspace/microGLES/src/pipeline/gl_framebuffer.c:51
    #1 0x55ea4c89cb46 in clear_task_func /workspace/microGLES/src/pipeline/gl_framebuffer.c:75
    #2 0x55ea4c8980ec in worker_thread_main /workspace/microGLES/src/gl_thread.c:164
    #3 0x7f339899b87a in start_thread nptl/pthread_create.c:444
    #4 0x7f3398a28c3b in clone3 ../sysdeps/unix/sysv/linux/x86_64/clone3.S:78

0x7f3398235220 is located 43552 bytes inside of 262144-byte region [0x7f339822a800,0x7f339826a800)
freed by thread T0 here:
    #0 0x7f339930c4d8 in free ../../../../src/libsanitizer/asan/asan_malloc_linux.cpp:52
    #1 0x55ea4c86b66a in tracked_free /workspace/microGLES/src/gl_utils.c:26

previously allocated by thread T0 here:
    #0 0x7f339930d9c7 in malloc ../../../../src/libsanitizer/asan/asan_malloc_linux.cpp:69
    #1 0x55ea4c8bc0e2 in memory_tracker_allocate /workspace/microGLES/src/gl_memory_tracker.c:59

SUMMARY: AddressSanitizer: heap-use-after-free /workspace/microGLES/src/pipeline/gl_framebuffer.c:51 in framebuffer_clear
Shadow bytes around the buggy address:
  0x7f3398234f80: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  0x7f3398235000: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  0x7f3398235080: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  0x7f3398235100: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  0x7f3398235180: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
=>0x7f3398235200: fd fd fd fd[fd]fd fd fd fd fd fd fd fd fd fd fd
  0x7f3398235280: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  0x7f3398235300: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  0x7f3398235380: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  0x7f3398235400: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  0x7f3398235480: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
Shadow byte legend (one shadow byte represents 8 application bytes):
  Addressable:           00
  Partially addressable: 01 02 03 04 05 06 07 
  Heap left redzone:       fa
  Freed heap region:       fd
  Stack left redzone:      f1
  Stack mid redzone:       f2
  Stack right redzone:     f3
  Stack after return:      f5
  Stack use after scope:   f8
  Global redzone:          f9
  Global init order:       f6
  Poisoned by user:        f7
  Container overflow:      fc
  Array cookie:            ac
  Intra object redzone:    bb
  ASan internal:           fe
  Left alloca redzone:     ca
  Right alloca redzone:    cb
==7632==ABORTING
