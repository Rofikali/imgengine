cmake_minimum_required(VERSION 3.16)
project(imgengine C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "-O3 -march=native -Wall -Wextra -pthread")

# ============================================================
# 🔥 INCLUDE PATHS
# ============================================================

include_directories(
    api/v1
    core
    pipeline
    runtime
    memory
    io
    plugins
    observability
    security
    arch
    cold
    cmd
)

# ============================================================
# 🔥 CORE (A)
# ============================================================

file(GLOB CORE_SRC
    core/context/*.c
    core/buffer/*.c
    core/pipeline/*.c
    core/engine/*.c
    core/config/*.c
)

# ============================================================
# 🔥 MEMORY (B)
# ============================================================

file(GLOB MEMORY_SRC
    memory/slab/*.c
    memory/arena/*.c
    memory/numa/*.c
    memory/hugepage/*.c
)

# ============================================================
# 🔥 SECURITY (C)
# ============================================================

file(GLOB SECURITY_SRC
    security/sandbox/*.c
    security/validation/*.c
    security/fuzz/*.c
)

# ============================================================
# 🔥 ARCH (D)
# ============================================================

file(GLOB ARCH_SRC
    arch/*.c
)

# ============================================================
# 🔥 PIPELINE (E + G)
# ============================================================

file(GLOB PIPELINE_SRC
    pipeline/exec/*.c
    pipeline/batch/*.c
    pipeline/threaded/*.c
    pipeline/dispatch/*.c
)

# ============================================================
# 🔥 RUNTIME (F)
# ============================================================

file(GLOB RUNTIME_SRC
    runtime/worker/*.c
    runtime/scheduler/*.c
    runtime/queue/*.c
    runtime/dispatch/*.c
    runtime/affinity/*.c
    runtime/cluster/*.c
)

# ============================================================
# 🔥 PLUGINS (H)
# ============================================================

file(GLOB PLUGIN_SRC
    plugins/builtin/*.c
    plugins/dynamic/*.c
)

# ============================================================
# 🔥 OBSERVABILITY (I)
# ============================================================

file(GLOB OBS_SRC
    observability/binlog/*.c
    observability/tracing/*.c
    observability/metrics/*.c
    observability/logger/*.c
    observability/profiler/*.c
)

# ============================================================
# 🔥 IO (K)
# ============================================================

file(GLOB IO_SRC
    io/decoder/*.c
    io/encoder/*.c
    io/vfs/*.c
    io/remote/*.c
)

# ============================================================
# 🔥 API (J)
# ============================================================

file(GLOB API_SRC
    src/api/*.c
)

# ============================================================
# 🔥 BUILD LIBRARY
# ============================================================

add_library(imgengine STATIC
    ${CORE_SRC}
    ${MEMORY_SRC}
    ${SECURITY_SRC}
    ${ARCH_SRC}
    ${PIPELINE_SRC}
    ${RUNTIME_SRC}
    ${PLUGIN_SRC}
    ${OBS_SRC}
    ${IO_SRC}
    ${API_SRC}
)

target_link_libraries(imgengine pthread dl)

# ============================================================
# 🔥 CLI
# ============================================================

add_executable(imgengine_cli
    cmd/imgengine/main.c
    cmd/imgengine/args.c
    cmd/imgengine/io_uring_engine.c
)

target_link_libraries(imgengine_cli imgengine uring)

# ============================================================
# 🔥 BENCH
# ============================================================

add_executable(lat_bench
    cmd/bench/lat_bench.c
)

target_link_libraries(lat_bench imgengine)