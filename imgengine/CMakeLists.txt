cmake_minimum_required(VERSION 3.16)

project(imgengine C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(IMGENGINE_BUILD_SHARED "Build shared library" OFF)

# =========================
# SOURCES
# =========================

set(IMGENGINE_PUBLIC_HEADERS
    include/imgengine/api.h
    include/imgengine/context.h
    include/imgengine/memory_pool.h
    include/imgengine/image.h
    include/imgengine/error.h
    include/imgengine/io.h
    include/imgengine/crop.h
    include/imgengine/resize.h
    include/imgengine/border.h
    include/imgengine/layout.h
)

set(IMGENGINE_PRIVATE_SOURCES

    # core
    src/core/memory_pool.c
    src/core/context.c
    src/core/image.c
    src/core/lbit_avx2.c   # ✅ ADD THIS

    # ops
    src/ops/crop.c
    src/ops/resize.c
    src/ops/border.c

    # layout
    src/layout/grid_layout.c
    src/layout/auto_fit.c

    # io
    src/io/stb_impl.c

    # api
    src/api/api.c
)

# =========================
# LIBRARY
# =========================

if(IMGENGINE_BUILD_SHARED)
    add_library(imgengine SHARED
        ${IMGENGINE_PUBLIC_HEADERS}
        ${IMGENGINE_PRIVATE_SOURCES}
    )
else()
    add_library(imgengine STATIC
        ${IMGENGINE_PUBLIC_HEADERS}
        ${IMGENGINE_PRIVATE_SOURCES}
    )
endif()

# =========================
# INCLUDE STRUCTURE
# =========================

target_include_directories(imgengine
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include   # exposed to users

    PRIVATE
        ${PROJECT_SOURCE_DIR}/src        # internal only
        ${PROJECT_SOURCE_DIR}/third_party
)

if (MSVC)
    target_compile_options(imgengine PRIVATE /W4)
else()
    target_compile_options(imgengine PRIVATE 
        -Wall -Wextra -Wpedantic
        -O3
        -mavx2
        -mfma
        -march=native
        -fopenmp
    )

    target_link_libraries(imgengine PRIVATE m gomp)
endif()

# =========================
# CLI
# =========================

add_executable(imgengine_cli
    cli/main.c
)

target_link_libraries(imgengine_cli PRIVATE imgengine m)

# CLI needs public headers too
target_include_directories(imgengine_cli
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

# =========================
# WARNINGS
# =========================

if (MSVC)
    target_compile_options(imgengine PRIVATE /W4)
else()
    target_compile_options(imgengine PRIVATE -Wall -Wextra -Wpedantic)
endif()