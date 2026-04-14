# imgengine — Claude Session Context

## What This Project Is

High-performance kernel-grade image processing engine in C.
L10 principal engineer level. Open source. 50-year codebase.
See RFC: docs/RFC_v1.0_FINAL.md

## Architecture

- Slab allocator, arena, NUMA-aware memory
- Jump table dispatch, fused SIMD kernels (AVX2/AVX-512/NEON)
- Lock-free SPSC/MPMC queues, pinned worker threads
- Plugin system (ELF section registration)
- Full print pipeline: decode → resize → grid layout → bleed → crop marks → border → encode

## Current Branch

with_claude_from_main

## Build

cd build && cmake .. -DIMGENGINE_SANITIZE=ON && make -j$(nproc)

## Working Commands

./imgengine_cli --input ../photo.jpg --output out.jpg \
    --cols 6 --rows 6 --gap 15 --padding 20 \
    --bleed 5 --crop-mark 10 --crop-offset 5 --border 2

## Key Files Changed This Session

- src/pipeline/canvas.c      — grid layout geometry
- src/pipeline/layout.c      — resize + blit grid
- src/pipeline/bleed.c       — edge pixel extension
- src/pipeline/crop_marks.c  — ISO print marks
- src/pipeline/border.c      — photo border (new)
- src/api/api.c              — img_api_run_job() pipeline
- include/pipeline/canvas.h  — img_canvas_t, img_job_t
- CMakeLists.txt             — explicit source lists, feature flags

## ABI Rules (DO NOT BREAK)

img_kernel_fn:        (img_ctx_t*, img_buffer_t*, void*)
img_single_kernel_fn: (img_ctx_t*, img_buffer_t*)
img_fused_kernel_fn:  (img_ctx_t*, img_batch_t*, void*)

## Next Work

- Bilinear resize (replace nearest-neighbour in layout.c)
- PDF engine
- Video/audio (future)
- Performance: target <2ms pipeline stage (currently ~25ms without ASAN)
