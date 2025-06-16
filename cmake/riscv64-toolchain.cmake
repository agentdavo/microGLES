# CPU architecture
set(ARCH "rv64imac")
set(ABI "lp64")
set(CMODEL "medany")
set(ARCH_FLAGS -march=${ARCH} -mabi=${ABI} -mcmodel=${CMODEL})

set(CMAKE_SYSTEM_NAME       "Generic"   CACHE STRING "")
set(CMAKE_SYSTEM_PROCESSOR  "riscv"     CACHE STRING "")

set(TOOLCHAIN_PREFIX    "riscv64-unknown-elf-")

set(CMAKE_C_COMPILER    "${TOOLCHAIN_PREFIX}gcc")
set(CMAKE_ASM_COMPILER  "${TOOLCHAIN_PREFIX}gcc")
set(CMAKE_CXX_COMPILER  "${TOOLCHAIN_PREFIX}g++")
set(CMAKE_AR            "${TOOLCHAIN_PREFIX}ar")
set(CMAKE_LINKER        "${TOOLCHAIN_PREFIX}ld")
set(CMAKE_OBJCOPY       "${TOOLCHAIN_PREFIX}objcopy")
set(CMAKE_OBJDUMP       "${TOOLCHAIN_PREFIX}objdump")
set(CMAKE_SIZE          "${TOOLCHAIN_PREFIX}size")
set(CMAKE_STRIP         "${TOOLCHAIN_PREFIX}ld")

add_compile_options(-O1)
add_compile_options(-Wall -Wextra)
add_compile_options(-Wno-error=coverage-mismatch)
add_compile_options(${ARCH_FLAGS})
add_link_options(-static)
add_link_options(-nostartfiles)
add_link_options(${ARCH_FLAGS})

set(CMAKE_C_FLAGS "${ARCH_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${ARCH_FLAGS}")
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
