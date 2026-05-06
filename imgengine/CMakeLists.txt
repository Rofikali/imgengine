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
option(IMGENGINE_SANITIZE   "Enable ASAN + UBSAN"          OFF)
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
    list(APPEND IMGENGINE_COMMON_FLAGS -flto -Werror)
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

# 1. Try to find the library on the system first
find_path(TURBOJPEG_INCLUDE NAMES turbojpeg.h)
find_library(TURBOJPEG_LIB  NAMES turbojpeg)

if(NOT TURBOJPEG_LIB OR NOT TURBOJPEG_INCLUDE)
    message(STATUS "[IO] libturbojpeg not found — downloading and building from source...")
    
    include(FetchContent)
    FetchContent_Declare(
      libjpeg-turbo
      GIT_REPOSITORY https://github.com
      GIT_TAG        3.0.2  # Use a stable version tag
    )
    
    # 2. Disable unnecessary parts of libjpeg-turbo to speed up your build
    set(WITH_SIMD ON CACHE BOOL "Enable SIMD" FORCE)
    set(ENABLE_STATIC ON CACHE BOOL "Build static" FORCE)
    set(ENABLE_SHARED OFF CACHE BOOL "Build shared" FORCE)

    FetchContent_MakeAvailable(libjpeg-turbo)

    # 3. Set variables so the rest of your script works
    # Note: libjpeg-turbo exports a target named 'turbojpeg-static' or 'turbojpeg'
    set(TURBOJPEG_LIB turbojpeg-static)
    # The include path is handled automatically by the target, but for your logic:
    get_target_property(TURBOJPEG_INCLUDE turbojpeg-static INTERFACE_INCLUDE_DIRECTORIES)
else()
    message(STATUS "[IO] Found system turbojpeg: ${TURBOJPEG_LIB}")
endif()

# ── Linkage ────────────────────────────────────────────────
# Now you can use ${TURBOJPEG_LIB} safely in target_link_libraries


find_library(NUMA_LIB  numa)
find_library(URING_LIB uring)
find_library(MATH_LIB  m)

# ── Include paths ──────────────────────────────────────────
set(IMGENGINE_INCLUDES
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/api/v1
    ${CMAKE_SOURCE_DIR}/third_party
    ${TURBOJPEG_INCLUDE}
)

# ── Library source lists (explicit — no GLOB_RECURSE) ──────

set(SOURCES_API
    src/api/api.c
    src/api/api_decode_secure.c
    src/api/api_encoded.c
    src/api/api_file_mmap.c
    src/api/api_init.c
    src/api/api_init_engine.c
    src/api/api_init_sandbox.c
    src/api/api_init_rollback.c
    src/api/api_init_rollback_workers.c
    src/api/api_init_workers.c
    src/api/api_init_task_queue.c
    src/api/api_init_worker_prepare.c
    src/api/api_init_reset_engine.c
    src/api/api_init_reset_worker_state.c
    src/api/api_init_reset_task_state.c
    src/api/api_init_reset_engine_state.c
    src/api/api_lifecycle.c
    src/api/api_job_begin.c
    src/api/api_job_decode_input.c
    src/api/api_job_finish_stage.c
    src/api/api_job_init_ctx.c
    src/api/api_job_load_input.c
    src/api/api_job_release_input.c
    src/api/api_job_prepare_render_stage.c
    src/api/api_job_finish_output.c
    src/api/api_job_finish_output_file.c
    src/api/api_job_finish_output_pdf.c
    src/api/api_job_postfx.c
    src/api/api_job_render_output.c
    src/api/api_job_run_rgb24.c
    src/api/api_job_write_buffer.c
    src/api/api_job_run.c
    src/api/api_job_run_raw.c
    src/api/api_pdf_output.c
    src/api/api_task_submit.c
    src/api/api_process_common.c
    src/api/api_process_fast.c
    src/api/api_process_fast_decode.c
    src/api/api_process_fast_pipeline.c
    src/api/api_process_fast_validate.c
    src/api/api_process_raw.c
    src/api/api_process_raw_decode.c
    src/api/api_process_raw_encode.c
    src/api/api_process_raw_free_buffer.c
    src/api/api_process_raw_prepare.c
    src/api/api_process_raw_execute.c
    src/api/api_process_raw_validate.c
    src/api/api_shutdown.c
    src/api/api_shutdown_globals.c
    src/api/api_shutdown_workers.c
    src/api/api_state.c
    src/api/api_process.c
    src/api/api_benchmark_hot.c
    src/api/api_job.c
)

set(SOURCES_CORE
    src/core/batch_builder.c
    src/core/buffer_lifecycle.c
    src/core/config.c
    src/core/context.c
    src/core/context_create.c
    src/core/context_destroy.c
    # src/core/img_pipeline.c -- MOVED to src/pipeline/pipeline_executor.c
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
    src/pipeline/canvas_alloc_buffer.c
    src/pipeline/canvas_fill_background.c
    src/pipeline/canvas_free_buffer.c
    src/pipeline/canvas_geometry.c
    src/pipeline/layout_grid.c
    src/pipeline/layout_grid_alloc_cells.c
    src/pipeline/layout_grid_place_cells.c
    src/pipeline/layout_grid_prepare_scaled.c
    src/pipeline/layout_prepare_fill.c
    src/pipeline/layout_prepare_fill_commit.c
    src/pipeline/layout_prepare_fit.c
    src/pipeline/layout_prepare_fit_commit.c
    src/pipeline/layout_resize.c
    src/pipeline/layout_resize_bilinear.c
    src/pipeline/layout_resize_resample_bilinear.c
    src/pipeline/layout_resize_validate_alloc.c
    src/pipeline/layout.c
    src/pipeline/bleed.c
    src/pipeline/crop_marks.c
    src/pipeline/engine.c
    src/pipeline/fused_dispatch.c
    src/pipeline/jump_table_init.c
    src/pipeline/jump_table_register.c
    src/pipeline/jump_table_reset.c
    src/pipeline/jump_table_select_resize_avx2.c
    src/pipeline/jump_table_select_resize_avx512.c
    src/pipeline/jump_table_select_resize_scalar.c
    src/pipeline/jump_table_select_resize.c
    src/pipeline/fused_kernels.c
    src/pipeline/fused_registry.c
    src/pipeline/hardware_registry.c
    src/pipeline/jump_table.c
    src/pipeline/pipeline_fuse_exec_avx2.c
    src/pipeline/pipeline_fuse_exec_scalar.c
    src/pipeline/pipeline_fuse_execute_batch.c
    src/pipeline/pipeline_build.c
    src/pipeline/pipeline_fuse.c
    src/pipeline/pipeline_fuse_state.c
    src/pipeline/border.c
    src/pipeline/dispatch.c
    src/pipeline/pipeline_fuse_exec.c
    src/pipeline/pipeline_fuse_select.c
    src/pipeline/pipeline_executor.c
)

# Option: enable/disable DSL -> C codegen
option(IMGENGINE_ENABLE_DSL_CODEGEN "Enable DSL -> C codegen for pipelines" ON)

if(IMGENGINE_ENABLE_DSL_CODEGEN)
    # Auto-generate pipelines from DSL (creates src/pipeline/generated.c)
    # corebases present in /include/pipeline/job_presets.h and job_templates.h 
    add_custom_command(
        OUTPUT ${CMAKE_SOURCE_DIR}/src/pipeline/generated.c
        COMMENT "Generate pipeline source from DSL"
    )
    list(APPEND SOURCES_PIPELINE ${CMAKE_SOURCE_DIR}/src/pipeline/generated.c)
else()
    # If codegen disabled but generated.c exists in tree, include it so builds don't break
    if(EXISTS ${CMAKE_SOURCE_DIR}/src/pipeline/generated.c)
        list(APPEND SOURCES_PIPELINE ${CMAKE_SOURCE_DIR}/src/pipeline/generated.c)
    endif()
endif()

set(SOURCES_MEMORY
    src/memory/arena_alloc.c
    src/memory/arena_alloc_aligned.c
    src/memory/arena.c
    src/memory/arena_create.c
    src/memory/hugepage.c
    src/memory/numa.c
    src/memory/slab_create.c
    src/memory/slab.c
    src/memory/slab_block_size.c
    src/memory/slab_destroy.c
    src/memory/slab_hot.c
    src/memory/slab_lifecycle.c
    src/memory/arena_reset.c
    src/memory/arena_destroy.c
)

set(SOURCES_RUNTIME
    src/runtime/affinity.c
    src/runtime/cluster_registry.c
    src/runtime/exec_router.c
    src/runtime/plugin_loader.c
    src/runtime/plugin_loader_descriptor.c
    src/runtime/plugin_loader_range.c
    src/runtime/scheduler_destroy.c
    src/runtime/scheduler_init.c
    src/runtime/scheduler_submit.c
    src/runtime/queue_create.c
    src/runtime/queue_destroy.c
    src/runtime/queue_mpmc.c
    src/runtime/queue_mpmc_destroy.c
    src/runtime/queue_mpmc_init.c
    src/runtime/queue_mpmc_pop.c
    src/runtime/queue_mpmc_push.c
    src/runtime/queue_pop.c
    src/runtime/queue_spsc.c
    src/runtime/queue_push.c
    src/runtime/rpc_client_close.c
    src/runtime/rpc_client_connect.c
    src/runtime/rpc_client_open.c
    src/runtime/rpc_client_recv_response.c
    src/runtime/rpc_client_send.c
    src/runtime/rpc_client_send_request.c
    src/runtime/rpc_server.c
    src/runtime/rpc_server_client.c
    src/runtime/rpc_server_accept.c
    src/runtime/rpc_server_close.c
    src/runtime/rpc_server_io.c
    src/runtime/rpc_server_open.c
    src/runtime/scheduler.c
    src/runtime/benchmark_hot.c
    src/runtime/job_finish_stage.c
    src/runtime/job_postfx.c
    src/runtime/job_prepare_render_stage.c
    src/runtime/job_run_compiled_pipeline.c
    src/runtime/raw_buffer_release.c
    src/runtime/template_registry.c
    src/runtime/worker.c
    src/runtime/worker_control.c
    src/runtime/worker_init.c
    src/runtime/worker_loop.c
)

set(SOURCES_IO
    src/io/decoder/decoder_entry.c
    src/io/decoder/decoder_dispatch.c
    src/io/decoder/png_decoder.c
    src/io/decoder/stb_bridge.c
    src/io/decoder/streaming_decoder.c
    src/io/decoder/streaming_decoder_core.c
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
    src/startup/engine_init_caps.c
    src/startup/engine_init_compute.c
    src/startup/engine_init_reset.c
    src/startup/engine_init_sandbox.c
    src/startup/engine_init_plugins_and_io.c
    src/startup/engine_init_subsystems.c
)

set(SOURCES_SECURITY
    src/security/fuzz_hooks.c
    src/security/input_validator.c
    src/security/sandbox_apply.c
    src/security/sandbox_build.c
    src/security/sandbox.c
)

set(SOURCES_OBSERVABILITY
    src/observability/binlog.c
    src/observability/binlog_lifecycle.c
    src/observability/binlog_state.c
    src/observability/binlog_write.c
    src/observability/logger.c
    src/observability/logger_clear_ring.c
    src/observability/logger_init_ring.c
    src/observability/logger_lifecycle.c
    src/observability/logger_loop.c
    src/observability/logger_ring.c
    src/observability/logger_start_thread.c
    src/observability/logger_stop_thread.c
    src/observability/metrics.c
    src/observability/profiler.c
    src/observability/tracing.c
    src/observability/logger_write.c
)

set(SOURCES_PLUGINS
    src/plugins/plugin_crop.c
    src/plugins/plugin_grayscale.c
    src/plugins/plugin_registry.c
    src/plugins/plugin_registry_range.c
    src/plugins/plugin_registry_register.c
    src/plugins/plugin_registry_validate.c
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
    src/cmd/imgengine/args_parse_defaults.c
    src/cmd/imgengine/args_parse_finalize.c
    src/cmd/imgengine/args_parse_float.c
    src/cmd/imgengine/args_parse_mode.c
    src/cmd/imgengine/args_parse.c
    src/cmd/imgengine/args_parse_options.c
    src/cmd/imgengine/args_parse_options_io.c
    src/cmd/imgengine/args_parse_options_input.c
    src/cmd/imgengine/args_parse_options_layout.c
    src/cmd/imgengine/args_parse_options_photo.c
    src/cmd/imgengine/args_parse_options_print.c
    src/cmd/imgengine/args_parse_u32.c
    src/cmd/imgengine/args_usage_header.c
    src/cmd/imgengine/args_usage_examples.c
    src/cmd/imgengine/args_usage_layout.c
    src/cmd/imgengine/args_usage_options.c
    src/cmd/imgengine/args_usage_options_input.c
    src/cmd/imgengine/args_usage_options_layout.c
    src/cmd/imgengine/args_usage_options_photo.c
    src/cmd/imgengine/args_usage_options_print.c
    src/cmd/imgengine/args_usage_options_runtime.c
    src/cmd/imgengine/args_usage_section.c
    src/cmd/imgengine/args_usage.c
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

    add_executable(decoder_bench src/cmd/bench/decoder_bench.c)
    target_include_directories(decoder_bench PRIVATE ${IMGENGINE_INCLUDES})
    target_compile_options(decoder_bench PRIVATE ${IMGENGINE_COMMON_FLAGS})
    target_link_libraries(decoder_bench PRIVATE imgengine)
    if(IMGENGINE_SANITIZE)
        target_link_libraries(decoder_bench PRIVATE -fsanitize=address,undefined)
    endif()

    # add_executable(imgengine_cli_bench src/cmd/bench/cli_bench.c)
    # target_include_directories(imgengine_cli_bench PRIVATE ${IMGENGINE_INCLUDES})
    # target_compile_options(imgengine_cli_bench PRIVATE ${IMGENGINE_COMMON_FLAGS})
    # target_link_libraries(imgengine_cli_bench PRIVATE imgengine Threads::Threads)
    # if(IMGENGINE_SANITIZE)
    #     target_link_libraries(imgengine_cli_bench PRIVATE -fsanitize=address,undefined)
    # endif()
endif()
# ── Verify generated pipelines test ─────────────────────────
add_executable(verify_generated_registration src/tests/verify_generated_registration.c)
target_include_directories(verify_generated_registration PRIVATE ${IMGENGINE_INCLUDES})
target_compile_options(verify_generated_registration PRIVATE ${IMGENGINE_COMMON_FLAGS})
target_link_libraries(verify_generated_registration PRIVATE imgengine)
if(IMGENGINE_SANITIZE)
    target_link_libraries(verify_generated_registration PRIVATE -fsanitize=address,undefined)
endif()

# ── Regression test: progressive JPEG ────────────────────────
add_custom_target(regression_progressive
    COMMAND ${CMAKE_COMMAND} -E env BUILD_DIR=${CMAKE_BINARY_DIR} bash ${CMAKE_SOURCE_DIR}/tests/regression/progressive_regression.sh ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/regression_tmp
    DEPENDS imgengine_cli
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

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

# Optional: include clang-format integration (adds `format` and `format-check` targets)
if(EXISTS "${CMAKE_SOURCE_DIR}/cmake/clang-format.cmake")
    include(${CMAKE_SOURCE_DIR}/cmake/clang-format.cmake)
endif()
