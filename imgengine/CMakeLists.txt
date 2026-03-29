cmake_minimum_required(VERSION 3.16)
project(imgengine C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 6-Year Tip: This is the official CMake way to handle Shared vs Static
option(BUILD_SHARED_LIBS "Build shared library" ON)

# =========================
# SOURCES
# =========================
set(LIB_SOURCES
    # core 
    src/core/memory_pool.c
    src/core/context.c
    src/core/image.c
    src/core/lbit_avx2.c

    # ops 
    src/ops/crop.c
    src/ops/resize.c
    src/ops/border.c

    # layouts 
    src/layout/grid_layout.c
    src/layout/auto_fit.c
    src/layout/crop_marks.c

    # io
    src/io/stb_impl.c
    src/io/image_io.c       # <--- ADD THIS

    # plugins 
    src/plugins/pdf_plugin.c
    src/plugins/bleed_plugin.c
    src/plugins/crop_plugin.c
    src/plugins/plugin_registry.c
    src/plugins/registers_all.c

    # pipelines 
    src/pipeline/plugin_runner.c

    # apis 
    src/api/api.c
)

# =========================
# TARGETS
# =========================

# Build the main library
add_library(imgengine ${LIB_SOURCES})

# Critical for shared libraries to work with Python/FFI
set_target_properties(imgengine PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(imgengine
    PUBLIC ${PROJECT_SOURCE_DIR}/include
    PRIVATE src third_party
)

# Compiler Flags
if(NOT MSVC)
    target_compile_options(imgengine PRIVATE 
        -O3 -mavx2 -mfma -march=native
        -Wall -Wextra -Wpedantic
    )
    target_link_libraries(imgengine PRIVATE m)
endif()

# Build the CLI
add_executable(imgengine_cli cli/main.c)
target_link_libraries(imgengine_cli PRIVATE imgengine)
