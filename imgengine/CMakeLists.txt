cmake_minimum_required(VERSION 3.10)

# ============================================================
#  Title: High-Performance, Kernel-Grade Image Processing Engine
#  Author: Principal Engineer (L7)
#  Status: Final Draft
#  Date: 2026-03-30
# ============================================================

project(imgengine C)

# ------------------------------------------------------------
# 1. GLOBAL STANDARDS
# ------------------------------------------------------------
set(CMAKE_C_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ------------------------------------------------------------
# 2. PRINCIPAL-LEVEL FLAGS (Performance + Security)
# ------------------------------------------------------------

# Core performance flags (Google/Amazon-grade)
# set(IMG_C_FLAGS "-O3 -march=native -flto  -fno-plt -fstrict-aliasing -luring -lpthread -Wl,--whole-archive")
set(IMG_C_FLAGS "-O3 -march=native -flto  -fno-plt -fstrict-aliasing -luring -lpthread -Wl,--undefined=__start_img_plugins -Wl,--undefined=__stop_img_plugins")


# -Wl,--undefined=__start_img_plugins
# -Wl,--undefined=__stop_img_plugins

# Hardening flags (optional but recommended for SaaS)
set(IMG_HARDEN_FLAGS "-fstack-protector-strong -D_FORTIFY_SOURCE=2")

# Sanitizers (DEV MODE ONLY)
set(IMG_SAN_FLAGS "-fsanitize=address,undefined -fno-omit-frame-pointer")

# Toggle sanitizers easily
option(ENABLE_SANITIZER "Enable ASAN/UBSAN" ON)

if(ENABLE_SANITIZER)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${IMG_C_FLAGS} ${IMG_HARDEN_FLAGS} ${IMG_SAN_FLAGS}")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${IMG_C_FLAGS} ${IMG_HARDEN_FLAGS}")
endif()

# ------------------------------------------------------------
# 3. ARCHITECTURE DETECTION (Silicon Awareness)
# ------------------------------------------------------------

if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64")
    set(ARCH_DIR "arch/x86_64")
    add_definitions(-DARCH_X86_64)
    message(STATUS "[CORE] Target Architecture: x86_64")

elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
    set(ARCH_DIR "arch/aarch64")
    add_definitions(-DARCH_AARCH64)
    message(STATUS "[CORE] Target Architecture: aarch64")

else()
    set(ARCH_DIR "arch/scalar")
    add_definitions(-DARCH_SCALAR)
    message(STATUS "[CORE] Target Architecture: Generic Scalar")
endif()

# ------------------------------------------------------------
# 4. INCLUDE PATHS (Clean Layered Architecture)
# ------------------------------------------------------------

include_directories(
    .
    api/v1

    # Internal Contracts
    include

    # Core Engine
    src/
    src/
    src/

    # Execution System
    pipeline
    runtime
    memory

    # Extensions
    plugins
    io
    security
    observability

    # third_party
    third_party/
)


# ------------------------------------------------------------
# 5. EXTERNAL DEPENDENCIES
# ------------------------------------------------------------

find_package(Threads REQUIRED)

# JPEG Turbo (optional but recommended)
find_path(TURBOJPEG_INCLUDE_DIR NAMES turbojpeg.h)
find_library(TURBOJPEG_LIB NAMES turbojpeg)

if(TURBOJPEG_LIB)
    message(STATUS "[IO] turbojpeg found")
else()
    message(WARNING "[IO] turbojpeg not found (JPEG disabled)")
endif()

# ------------------------------------------------------------
# 6. SOURCE COLLECTION (Layered + Modular)
# ------------------------------------------------------------

file(GLOB_RECURSE CORE_SOURCES
    "src/*.c"
)

file(GLOB_RECURSE HOT_SOURCES
    "src/*.c"
)

file(GLOB API_SOURCES
    "src/api/*.c"
)

file(GLOB_RECURSE COLD_SOURCES
    "src/*.c"
)

file(GLOB_RECURSE PIPELINE_SOURCES
    "pipeline/*.c"
)

file(GLOB_RECURSE RUNTIME_SOURCES
    "runtime/*.c"
)

file(GLOB_RECURSE MEMORY_SOURCES
    "memory/*.c"
)

file(GLOB_RECURSE PLUGIN_SOURCES
    "plugins/*.c"
)

file(GLOB_RECURSE IO_SOURCES
    "io/*.c"
)

file(GLOB_RECURSE SECURITY_SOURCES
    "security/*.c"
)

file(GLOB_RECURSE OBSERVABILITY_SOURCES
    "observability/*.c"
)

file(GLOB_RECURSE ARCH_SOURCES
    "${ARCH_DIR}/*.c"
)

# Combine all
set(ENGINE_SOURCES
    ${ENGINE_SOURCES}
    ${API_SOURCES}
    ${CORE_SOURCES}
    ${HOT_SOURCES}
    ${COLD_SOURCES}
    ${PIPELINE_SOURCES}
    ${RUNTIME_SOURCES}
    ${MEMORY_SOURCES}
    ${PLUGIN_SOURCES}
    ${IO_SOURCES}
    ${SECURITY_SOURCES}
    ${OBSERVABILITY_SOURCES}
    ${ARCH_SOURCES}
)

# ------------------------------------------------------------
# 7. BUILD TARGETS
# ------------------------------------------------------------

# ============================================================
# A. SHARED LIBRARY (Your SaaS Core Engine)
# ============================================================

add_library(plugin_resize SHARED src/plugins/plugin_resize.c)

target_include_directories(plugin_resize PRIVATE ${CMAKE_SOURCE_DIR})

add_library(imgengine SHARED ${ENGINE_SOURCES})

target_include_directories(imgengine PRIVATE ${TURBOJPEG_INCLUDE_DIR})

target_link_libraries(imgengine
    PRIVATE
    Threads::Threads
    m
)

# Link turbojpeg if available
if(TURBOJPEG_LIB)
    target_link_libraries(imgengine PRIVATE ${TURBOJPEG_LIB})
endif()

# Sanitizer link (if enabled)
if(ENABLE_SANITIZER)
    target_link_libraries(imgengine PRIVATE "-fsanitize=address,undefined")
endif()


find_library(NUMA_LIB numa)

target_link_libraries(imgengine PRIVATE ${NUMA_LIB})

# ============================================================
# B. CLI TOOL (Production Interface)
# ============================================================

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmd/imgengine/main.c")
    add_executable(imgengine_cli
        cmd/imgengine/main.c
        cmd/imgengine/args.c
    )

    target_link_libraries(imgengine_cli PRIVATE imgengine)
endif()

# ============================================================
# C. LATENCY BENCH (P99 Measurement)
# ============================================================

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmd/bench/lat_bench.c")
    add_executable(bench_lat
        cmd/bench/lat_bench.c
    )

    target_link_libraries(bench_lat PRIVATE imgengine)
endif()

# ============================================================
# D. THROUGHPUT BENCH (Optional Future)
# ============================================================

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmd/bench/thr_bench.c")
    add_executable(bench_thr
        cmd/bench/thr_bench.c
    )

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