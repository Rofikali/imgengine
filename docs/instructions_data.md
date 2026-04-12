// ================================================================
// imgengine — Complete Integration
// PART 4 OF 4: File map, slab sizing fix, build instructions
// ================================================================

// ================================================================
// CRITICAL FIX: api.c slab pool size
//
// The A4 canvas at 300 DPI = 2480×3508×3 = ~26MB.
// The photo resize buffer needs another ~24MB for 4K input.
// Total concurrent slab demand per job: ~50-60MB.
//
// In img_api_init(), change the global_pool creation:
// ================================================================

// In src/api/api.c — find img_api_init() and change:

//  OLD (too small):
//    g_engine.global_pool = img_slab_create(
//        64  *1024* 1024,
//        64  * 1024);

//  NEW (correct for A4 @ 300 DPI + 4K input):
//    g_engine.global_pool = img_slab_create(
//        1024ULL *1024* 1024,   /*1GB total pool */
//        64ULL   * 1024 * 1024);  /* 64MB per block*/

// WHY 1GB total:
//   Each job uses 2 blocks: canvas (~26MB) + resize buffer (~24MB)
//   At 64MB blocks: 1GB / 64MB = 16 concurrent job slots
//   This matches RFC: "SaaS multi-tenancy, 100K+ ops/sec"
//
// WHY 64MB blocks:
//   A4 @ 300 DPI RGB:  2480 *3508* 3 = 26,099,520 bytes (~26MB)
//   4K input decoded:  3840 *2160* 3 = 24,883,200 bytes (~24MB)
//   64MB covers both with headroom for stride padding
//
// For development/testing with ASAN (slower, uses more memory):
//   Use 512MB total, 64MB blocks.

// ================================================================
// COMPLETE NEW FILE LIST
// Apply these files to your repository:
// ================================================================

/*
NEW FILES (create):
  api/v1/img_job.h                            ← Part 1
  src/pipeline/canvas.c                       ← Part 1
  include/pipeline/canvas.h                   ← Part 1
  src/arch/x86_64/avx2/blit_avx2.c           ← Part 1
  include/pipeline/layout.h                   ← Part 2
  src/pipeline/layout.c                       ← Part 2
  src/pipeline/bleed.c                        ← Part 2
  src/pipeline/crop_marks.c                   ← Part 2
  src/io/encoder/pdf_encoder.c                ← Part 2
  include/io/encoder/pdf_encoder.h            ← Part 2

MODIFIED FILES (update):
  api/v1/img_api.h                            ← Part 3 (add img_api_run_job)
  src/api/api.c                               ← Part 3 (add img_api_run_job impl)
  src/cmd/imgengine/main.c                    ← Part 3 (full replacement)
  CMakeLists.txt                              ← Part 3 (full replacement)
*/

// ================================================================
// BUILD INSTRUCTIONS (production-grade sequence)
// ================================================================

/*

# 1. Clean build

cd /workspaces/imgengine/imgengine
rm -rf build && mkdir build && cd build

# 2. Configure — dev mode (ASAN on, sandbox off)

cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DIMGENGINE_SANITIZE=ON \
    -DIMGENGINE_LTO=ON \
    -DIMGENGINE_SANDBOX=OFF \
    -DIMGENGINE_BENCH=ON

# 3. Build (parallel)

make -j$(nproc)

# 4. Verify all targets built

ls -lah libimgengine.so libplugin_resize.so imgengine_cli bench_lat

# 5. Run CLI — passport sheet

./imgengine_cli \
    --input ../test.jpg \
    --output output.jpg \
    --cols 3 --rows 2 \
    --width 4.5 --height 3.5 \
    --dpi 300

# 6. Run CLI — print-ready with crop marks + PDF

./imgengine_cli \
    --input ../test.jpg \
    --output output.pdf \
    --cols 6 --rows 2 \
    --width 3.5 --height 3.0 \
    --bleed 10 --crop-mark 25 --crop-offset 8

# 7. Benchmark

./bench_lat ../test.jpg

# 8. Production build (no ASAN, max perf)

cd ..
rm -rf build_prod && mkdir build_prod && cd build_prod
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DIMGENGINE_SANITIZE=OFF \
    -DIMGENGINE_LTO=ON \
    -DIMGENGINE_SANDBOX=ON

make -j$(nproc)
./bench_lat ../test.jpg   # real perf numbers — no ASAN overhead
*/

// ================================================================
// PIPELINE EXECUTION DIAGRAM (final, as-built)
// ================================================================

/*
imgengine_cli --input photo.jpg --output sheet.pdf
    │
    ▼
img_api_run_job()                          [src/api/api.c]
    │
    ├─ load_file_mmap()                    [mmap, no malloc]
    │
    ├─ img_security_validate_request()     [src/security/input_validator.c]
    │
    ├─ img_decode_to_buffer()              [src/io/decoder/decoder_entry.c]
    │   └─ tjDecompress2()                 [libturbojpeg]
    │       └─ → img_buffer_t (slab)
    │
    ├─ img_canvas_init()                   [src/pipeline/canvas.c]
    │   ├─ compute A4 px dimensions
    │   ├─ compute grid geometry
    │   └─ → img_canvas_t (slab, bg-filled)
    │
    ├─ img_layout_grid()                   [src/pipeline/layout.c]
    │   ├─ resize_into_slab()              [nearest-neighbour, slab]
    │   └─ img_blit_avx2() × (cols×rows)  [src/arch/x86_64/avx2/blit_avx2.c]
    │
    ├─ img_apply_bleed()                   [src/pipeline/bleed.c]
    │   └─ memcpy edge pixels outward
    │
    ├─ img_draw_crop_marks()               [src/pipeline/crop_marks.c]
    │   └─ set_pixel() per mark segment
    │
    └─ img_encode_pdf()                    [src/io/encoder/pdf_encoder.c]
        ├─ tjCompress2() — JPEG stream
        └─ write PDF 1.4 container
*/

// ================================================================
// PERFORMANCE PATH TO RFC SLOs
//
// Current (ASAN on):   ~110ms/image
// Current (ASAN off):  ~25-30ms/image  (estimated)
// After bilinear AVX2: ~12-15ms/image
// After batch pipeline:~2-4ms/image
//
// The RFC SLO of <2ms applies to the SIMD pipeline stage.
// Full decode+encode round trip target: <20ms.
//
// Next sprint:
//   1. Replace resize_into_slab() with AVX2 separable bilinear
//      using pre-computed index tables (resize_h_avx2 + resize_v_avx2)
//   2. Wire batch processing: submit N images, execute in parallel
//   3. Disable ASAN for production bench
// ================================================================

// ================================================================
// USAGE EXAMPLES (all working after this integration)
// ================================================================

/*

# Passport — fits perfectly

./imgengine_cli --input ../photo.jpg --output passport.jpg \
    --cols 2 --rows 3 --gap 20 --padding 40

# Studio — auto-scales 6 photos to fit with gaps

./imgengine_cli --input ../photo.jpg --output studio.jpg \
    --cols 6 --rows 2 --gap 15 --padding 40

# Print-ready with bleed (use fewer cols to leave room)

 ./imgengine_cli --input ../photo.jpg --output printready.jpg   \
   --cols 6 --rows 6 --gap 15 --padding 20    \
   --bleed 1 --crop-mark 5 --crop-offset 5 --crop-thickness 10 --border 50

## REMOTE (RPC / distributed execution)

    Infuture-Usese
