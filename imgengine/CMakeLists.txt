cmake_minimum_required(VERSION 3.16)

# ============================================================
#  imgengine — High-Performance Kernel-Grade Image Engine
#  Unified build: core library + CLI + bench + plugin
# ============================================================

project(imgengine
    VERSION   1.0.0
    LANGUAGES C
)

# ── Standards ──────────────────────────────────────────────
set(CMAKE_C_STANDARD            11)
set(CMAKE_C_STANDARD_REQUIRED   ON)
set(CMAKE_C_EXTENSIONS          OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS   ON)   # for clangd / analyzer

# ── Architecture detection ─────────────────────────────────
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64")
    set(IMGENGINE_ARCH      "x86_64")
    set(IMGENGINE_ARCH_DIR  "src/arch/x86_64")
    add_definitions(-DARCH_X86_64)
    message(STATUS "[ARCH] x86_64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
    set(IMGENGINE_ARCH      "aarch64")
    set(IMGENGINE_ARCH_DIR  "src/arch/aarch64")
    add_definitions(-DARCH_AARCH64)
    message(STATUS "[ARCH] aarch64")
else()
    set(IMGENGINE_ARCH      "scalar")
    set(IMGENGINE_ARCH_DIR  "src/arch/scalar")
    add_definitions(-DARCH_SCALAR)
    message(STATUS "[ARCH] scalar fallback")
endif()

# ── Feature flags ──────────────────────────────────────────
option(IMGENGINE_SANITIZE   "Enable ASAN + UBSAN"          ON)
option(IMGENGINE_LTO        "Enable link-time optimization" ON)
option(IMGENGINE_SANDBOX    "Enable seccomp sandbox"        OFF)
option(IMGENGINE_BENCH      "Build latency benchmark"       ON)

# ── Compiler flags ─────────────────────────────────────────
set(IMGENGINE_COMMON_FLAGS
    -O3
    -march=native
    -fno-plt
    -fstrict-aliasing
    -fstack-protector-strong
    -D_FORTIFY_SOURCE=2
    -Wall
    -Wextra
    -Wshadow
    -Wformat=2
    -Werror=implicit-function-declaration
    -Werror=incompatible-pointer-types
)

if(IMGENGINE_LTO)
    list(APPEND IMGENGINE_COMMON_FLAGS -flto)
    # list(APPEND IMGENGINE_COMMON_FLAGS -flto -Werror) This line must be uncommented in future
    set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} -flto")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -flto")
endif()

if(IMGENGINE_SANITIZE)
    list(APPEND IMGENGINE_COMMON_FLAGS
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
    )
endif()

if(IMGENGINE_SANDBOX)
    add_definitions(-DIMG_SANDBOX_ENABLED)
endif()

# ── Dependencies ───────────────────────────────────────────
find_package(Threads REQUIRED)

find_path(TURBOJPEG_INCLUDE NAMES turbojpeg.h)
find_library(TURBOJPEG_LIB  NAMES turbojpeg)
if(NOT TURBOJPEG_LIB)
    message(FATAL_ERROR "[IO] libturbojpeg not found — required")
endif()
message(STATUS "[IO] turbojpeg: ${TURBOJPEG_LIB}")

find_library(NUMA_LIB  numa)
find_library(URING_LIB uring)
find_library(MATH_LIB  m)

# ── Include paths ──────────────────────────────────────────
set(IMGENGINE_INCLUDES
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/api/v1
    ${CMAKE_SOURCE_DIR}/third_party
    ${TURBOJPEG_INCLUDE}
)

# ── Library source lists (explicit — no GLOB_RECURSE) ──────

set(SOURCES_API
    src/api/api.c
)

set(SOURCES_CORE
    src/core/batch_builder.c
    src/core/buffer_lifecycle.c
    src/core/config.c
    src/core/context.c
    src/core/img_pipeline.c
)

set(SOURCES_HOT
    src/hot/batch_exec.c
    src/hot/pipeline_exec.c
)

set(SOURCES_COLD
    src/cold/debug.c
    src/cold/error.c
    src/cold/pipeline_compiled.c
    src/cold/validation.c
)

set(SOURCES_PIPELINE
    src/pipeline/canvas.c
    src/pipeline/layout.c
    src/pipeline/bleed.c
    src/pipeline/crop_marks.c
    src/pipeline/engine.c
    src/pipeline/fused_dispatch.c
    src/pipeline/fused_kernels.c
    src/pipeline/fused_registry.c
    src/pipeline/hardware_registry.c
    src/pipeline/jump_table.c
    src/pipeline/pipeline_build.c
    src/pipeline/pipeline_fuse.c
    src/pipeline/border.c
)

set(SOURCES_MEMORY
    src/memory/arena.c
    src/memory/hugepage.c
    src/memory/numa.c
    src/memory/slab.c
)

set(SOURCES_RUNTIME
    src/runtime/affinity.c
    src/runtime/cluster_registry.c
    src/runtime/exec_router.c
    src/runtime/plugin_loader.c
    src/runtime/queue_mpmc.c
    src/runtime/queue_spsc.c
    src/runtime/rpc_client.c
    # src/runtime/rpc_server.c
    src/runtime/scheduler.c
    src/runtime/worker.c
)

set(SOURCES_IO
    src/io/decoder/decoder_entry.c
    src/io/decoder/png_decoder.c
    src/io/decoder/stb_bridge.c
    src/io/decoder/streaming_decoder.c
    src/io/encoder/encoder_entry.c
    src/io/encoder/jpeg_encoder.c
    src/io/encoder/pdf_encoder.c
    src/io/vfs/http_stream.c
    src/io/vfs/memory_stream.c
    src/io/vfs/posix_io.c
    src/io/io_register.c
    # src/io/vfs/s3_adapter.c
)

set(SOURCES_STARTUP
    src/startup/engine_init.c
)

set(SOURCES_SECURITY
    src/security/fuzz_hooks.c
    src/security/input_validator.c
    src/security/sandbox.c
)

set(SOURCES_OBSERVABILITY
    src/observability/binlog.c
    src/observability/logger.c
    src/observability/metrics.c
    src/observability/profiler.c
    src/observability/tracing.c
)

set(SOURCES_PLUGINS
    src/plugins/plugin_crop.c
    src/plugins/plugin_grayscale.c
    src/plugins/plugin_registry.c
)

set(SOURCES_ARCH_X86
    src/arch/x86_64/avx2/blit_avx2.c
    src/arch/x86_64/avx2/color_avx2.c
    src/arch/x86_64/avx2/fused_resize_color_norm_avx2.c
    src/arch/x86_64/avx2/resize_avx2.c
    src/arch/x86_64/avx2/resize_fused_avx2.c
    src/arch/x86_64/avx2/resize_h_avx2.c
    src/arch/x86_64/avx2/resize_v_avx2.c
    src/arch/x86_64/avx512/avx512.c
    src/arch/x86_64/scalar/color_scalar.c
    src/arch/x86_64/scalar/resize_scalar.c
    src/arch/x86_64/cpu_detect.c
)

set(SOURCES_THIRD_PARTY
    src/third_party/stb/stb_image_impl.c
)

# ── Combine all library sources ────────────────────────────
set(IMGENGINE_LIB_SOURCES
    ${SOURCES_API}
    ${SOURCES_CORE}
    ${SOURCES_HOT}
    ${SOURCES_COLD}
    ${SOURCES_PIPELINE}
    ${SOURCES_MEMORY}
    ${SOURCES_RUNTIME}
    ${SOURCES_IO}
    ${SOURCES_STARTUP}
    ${SOURCES_SECURITY}
    ${SOURCES_OBSERVABILITY}
    ${SOURCES_PLUGINS}
    ${SOURCES_ARCH_X86}
    ${SOURCES_THIRD_PARTY}
)

# ── Shared library ─────────────────────────────────────────
add_library(imgengine SHARED ${IMGENGINE_LIB_SOURCES})

target_include_directories(imgengine PUBLIC ${IMGENGINE_INCLUDES})
target_compile_options(imgengine PRIVATE ${IMGENGINE_COMMON_FLAGS})

target_link_libraries(imgengine
    PUBLIC  ${TURBOJPEG_LIB}
    PRIVATE Threads::Threads
)
if(MATH_LIB)
    target_link_libraries(imgengine PRIVATE ${MATH_LIB})
endif()
if(NUMA_LIB)
    target_link_libraries(imgengine PRIVATE ${NUMA_LIB})
endif()
if(IMGENGINE_SANITIZE)
    target_link_libraries(imgengine PRIVATE -fsanitize=address,undefined)
endif()

# ── Dynamic plugin (loadable) ──────────────────────────────
add_library(plugin_resize SHARED src/plugins/plugin_resize.c)
target_include_directories(plugin_resize PRIVATE ${IMGENGINE_INCLUDES})
target_compile_options(plugin_resize PRIVATE ${IMGENGINE_COMMON_FLAGS})

# ── CLI executable ─────────────────────────────────────────
add_executable(imgengine_cli
    src/cmd/imgengine/main.c
    src/cmd/imgengine/args.c
    src/cmd/imgengine/job_builder.c
    src/cmd/imgengine/io_uring_engine.c
)
target_include_directories(imgengine_cli PRIVATE ${IMGENGINE_INCLUDES})
target_compile_options(imgengine_cli PRIVATE ${IMGENGINE_COMMON_FLAGS})
target_link_libraries(imgengine_cli PRIVATE imgengine)
if(URING_LIB)
    target_link_libraries(imgengine_cli PRIVATE ${URING_LIB})
endif()
if(IMGENGINE_SANITIZE)
    target_link_libraries(imgengine_cli PRIVATE -fsanitize=address,undefined)
endif()

# ── Latency benchmark ──────────────────────────────────────
if(IMGENGINE_BENCH)
    add_executable(bench_lat src/cmd/bench/lat_bench.c)
    target_include_directories(bench_lat PRIVATE ${IMGENGINE_INCLUDES})
    target_compile_options(bench_lat PRIVATE ${IMGENGINE_COMMON_FLAGS})
    target_link_libraries(bench_lat PRIVATE imgengine)
    if(IMGENGINE_SANITIZE)
        target_link_libraries(bench_lat PRIVATE -fsanitize=address,undefined)
    endif()
endif()

# ── Build summary ──────────────────────────────────────────
message(STATUS "")
message(STATUS "======================================")
message(STATUS " imgengine ${PROJECT_VERSION} build config")
message(STATUS "--------------------------------------")
message(STATUS " Architecture : ${IMGENGINE_ARCH}")
message(STATUS " Sanitizers   : ${IMGENGINE_SANITIZE}")
message(STATUS " LTO          : ${IMGENGINE_LTO}")
message(STATUS " Sandbox      : ${IMGENGINE_SANDBOX}")
message(STATUS " Bench        : ${IMGENGINE_BENCH}")
message(STATUS "======================================")
message(STATUS "")
