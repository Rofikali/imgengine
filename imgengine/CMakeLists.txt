cmake_minimum_required(VERSION 3.10)

# ============================================================
#  imgengine — High-Performance Kernel-Grade Image Engine
#  Principal Engineer (L10)
# ============================================================

project(imgengine C)

# ------------------------------------------------------------
# 1. GLOBAL STANDARDS
# ------------------------------------------------------------
set(CMAKE_C_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ------------------------------------------------------------
# 2. COMPILER FLAGS
# ------------------------------------------------------------
set(IMG_C_FLAGS
    "-O3 -march=native -flto -fno-plt -fstrict-aliasing"
    # "-Wl,--undefined=__start_img_plugins,--undefined=__stop_img_plugins"
    # "-Wl,--undefined=__start_img_plugins"
    # "-Wl,--undefined=__stop_img_plugins"
)
string(JOIN " " IMG_C_FLAGS ${IMG_C_FLAGS})

set(IMG_HARDEN_FLAGS "-fstack-protector-strong -D_FORTIFY_SOURCE=2")
set(IMG_SAN_FLAGS    "-fsanitize=address,undefined -fno-omit-frame-pointer")

option(ENABLE_SANITIZER "Enable ASAN/UBSAN" ON)

if(ENABLE_SANITIZER)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${IMG_C_FLAGS} ${IMG_HARDEN_FLAGS} ${IMG_SAN_FLAGS}")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${IMG_C_FLAGS} ${IMG_HARDEN_FLAGS}")
endif()

# ------------------------------------------------------------
# 3. ARCHITECTURE DETECTION
# ------------------------------------------------------------
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64")
    set(ARCH_DIR "src/arch/x86_64")
    add_definitions(-DARCH_X86_64)
    message(STATUS "[CORE] Target Architecture: x86_64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
    set(ARCH_DIR "src/arch/aarch64")
    add_definitions(-DARCH_AARCH64)
    message(STATUS "[CORE] Target Architecture: aarch64")
else()
    set(ARCH_DIR "src/arch/scalar")
    add_definitions(-DARCH_SCALAR)
    message(STATUS "[CORE] Target Architecture: Generic Scalar")
endif()

# ------------------------------------------------------------
# 4. INCLUDE PATHS
# ------------------------------------------------------------
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/api/v1
    ${CMAKE_SOURCE_DIR}/third_party
)

# ------------------------------------------------------------
# 5. EXTERNAL DEPENDENCIES
# ------------------------------------------------------------
find_package(Threads REQUIRED)

find_path(TURBOJPEG_INCLUDE_DIR NAMES turbojpeg.h)
find_library(TURBOJPEG_LIB NAMES turbojpeg)
if(TURBOJPEG_LIB)
    message(STATUS "[IO] turbojpeg found")
else()
    message(WARNING "[IO] turbojpeg not found (JPEG disabled)")
endif()

find_library(NUMA_LIB numa)
find_library(URING_LIB uring)

# ------------------------------------------------------------
# 6. LIBRARY SOURCE LIST (EXPLICIT — no GLOB_RECURSE on src/)
#
# Rules:
#   - src/cmd/** EXCLUDED — those are executables, never library
#   - Each subdirectory listed once, explicitly
#   - No duplicate files across lists
# ------------------------------------------------------------

# --- API ---
set(SOURCES_API
    src/api/api.c
)

# --- ARCH --- (replace your current SOURCES_ARCH block)
set(SOURCES_ARCH
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

# --- CORE ---
set(SOURCES_CORE
    src/core/batch_builder.c
    src/core/buffer_lifecycle.c
    src/core/config.c
    src/core/context.c
    src/core/img_pipeline.c
    src/core/init.c
)

# --- HOT PATH ---
set(SOURCES_HOT
    src/hot/batch_exec.c
    src/hot/pipeline_exec.c
)

# --- COLD PATH ---
set(SOURCES_COLD
    src/cold/debug.c
    src/cold/error.c
    src/cold/pipeline_compiled.c
    src/cold/validation.c
)

# --- PIPELINE ---
set(SOURCES_PIPELINE
    src/pipeline/batch_exec.c
    src/pipeline/engine.c
    src/pipeline/fused_dispatch.c
    src/pipeline/fused_kernels.c
    src/pipeline/fused_registry.c
    src/pipeline/hardware_registry.c
    src/pipeline/jump_table.c
    src/pipeline/pipeline_build.c
    src/pipeline/pipeline_fuse.c
)

# --- MEMORY ---
set(SOURCES_MEMORY
    src/memory/arena.c
    src/memory/hugepage.c
    src/memory/numa.c
    src/memory/slab.c
)

# --- RUNTIME ---
set(SOURCES_RUNTIME
    src/runtime/affinity.c
    src/runtime/cluster_registry.c
    src/runtime/exec_router.c
    src/runtime/plugin_loader.c
    src/runtime/queue_mpmc.c
    src/runtime/queue_spsc.c
    src/runtime/rpc_client.c
    src/runtime/rpc_server.c
    src/runtime/scheduler.c
    src/runtime/worker.c
)

# --- IO ---
set(SOURCES_IO
    src/io/decoder/decoder_entry.c
    src/io/decoder/png_decoder.c
    src/io/decoder/stb_bridge.c
    src/io/decoder/streaming_decoder.c
    src/io/encoder/encoder_entry.c
    src/io/encoder/jpeg_encoder.c
    src/io/vfs/http_stream.c
    src/io/vfs/memory_stream.c
    src/io/vfs/posix_io.c
    src/io/vfs/s3_adapter.c
)

# --- SECURITY ---
set(SOURCES_SECURITY
    src/security/fuzz_hooks.c
    src/security/input_validator.c
    src/security/sandbox.c
)

# --- OBSERVABILITY ---
set(SOURCES_OBSERVABILITY
    src/observability/binlog.c
    src/observability/logger.c
    src/observability/metrics.c
    src/observability/profiler.c
    src/observability/tracing.c
)

# --- PLUGINS ---
set(SOURCES_PLUGINS
    src/plugins/plugin_crop.c
    src/plugins/plugin_grayscale.c
    src/plugins/plugin_registry.c
)

# --- ARCH ---
# file(GLOB_RECURSE SOURCES_ARCH "${ARCH_DIR}/*.c")

# --- THIRD PARTY ---
set(SOURCES_THIRD_PARTY
    src/third_party/stb/stb_image_impl.c
)

# --- TOOLS (internal tools, NOT cmd executables) ---
set(SOURCES_TOOLS
    src/tools/binlog_decode.c
)

# --- COMBINED LIBRARY SOURCES ---
set(ENGINE_SOURCES
    ${SOURCES_API}
    ${SOURCES_CORE}
    ${SOURCES_HOT}
    ${SOURCES_COLD}
    ${SOURCES_PIPELINE}
    ${SOURCES_MEMORY}
    ${SOURCES_RUNTIME}
    ${SOURCES_IO}
    ${SOURCES_SECURITY}
    ${SOURCES_OBSERVABILITY}
    ${SOURCES_PLUGINS}
    ${SOURCES_ARCH}
    ${SOURCES_THIRD_PARTY}
)

# ------------------------------------------------------------
# 7. BUILD TARGETS
# ------------------------------------------------------------

# --- Plugin (dynamic, loadable at runtime) ---
add_library(plugin_resize SHARED src/plugins/plugin_resize.c)
target_include_directories(plugin_resize PRIVATE ${CMAKE_SOURCE_DIR})

# --- Core shared library ---
add_library(imgengine SHARED ${ENGINE_SOURCES})

target_include_directories(imgengine PRIVATE
    ${TURBOJPEG_INCLUDE_DIR}
)

target_link_libraries(imgengine PRIVATE
    Threads::Threads
    m
)

if(TURBOJPEG_LIB)
    target_link_libraries(imgengine PRIVATE ${TURBOJPEG_LIB})
endif()

if(NUMA_LIB)
    target_link_libraries(imgengine PRIVATE ${NUMA_LIB})
endif()

if(URING_LIB)
    target_link_libraries(imgengine PRIVATE ${URING_LIB})
endif()

if(ENABLE_SANITIZER)
    target_link_libraries(imgengine PRIVATE "-fsanitize=address,undefined")
endif()

# --- CLI executable (NOT in library) ---
add_executable(imgengine_cli
    src/cmd/imgengine/main.c
    src/cmd/imgengine/args.c
    src/cmd/imgengine/io_uring_engine.c
)
target_link_libraries(imgengine_cli PRIVATE imgengine)
if(URING_LIB)
    target_link_libraries(imgengine_cli PRIVATE ${URING_LIB})
endif()
if(ENABLE_SANITIZER)
    target_link_libraries(imgengine_cli PRIVATE "-fsanitize=address,undefined")
endif()

# --- Latency bench executable (NOT in library) ---
add_executable(bench_lat
    src/cmd/bench/lat_bench.c
)
target_link_libraries(bench_lat PRIVATE imgengine)
if(ENABLE_SANITIZER)
    target_link_libraries(bench_lat PRIVATE "-fsanitize=address,undefined")
endif()

# --- Throughput bench (optional) ---
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/cmd/bench/thr_bench.c")
    add_executable(bench_thr src/cmd/bench/thr_bench.c)
    target_link_libraries(bench_thr PRIVATE imgengine)
endif()

# ------------------------------------------------------------
# 8. BUILD SUMMARY
# ------------------------------------------------------------
message(STATUS "======================================")
message(STATUS " ImgEngine Build Configuration")
message(STATUS "--------------------------------------")
message(STATUS " Architecture : ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS " SIMD Layer   : ${ARCH_DIR}")
message(STATUS " Sanitizers   : ${ENABLE_SANITIZER}")
message(STATUS "======================================")