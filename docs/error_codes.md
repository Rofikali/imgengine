
// ================================================================
// PATTERN FOR ALL OTHER FILES
//
// Replace every `return -1` in engine code (not third_party) with
// the appropriate named code:
//
//   malloc/slab returns NULL          → return IMG_ERR_NOMEM
//   bad input pointer / size=0        → return IMG_ERR_SECURITY
//   tjDecompress / format parse fail  → return IMG_ERR_FORMAT
//   file open / write / read fail     → return IMG_ERR_IO
//   SIMD path missing on this CPU     → return IMG_ERR_HW_UNSUP
//   internal logic error (shouldn't happen) → return IMG_ERR_INTERNAL
//
// Files to update in order:
//   src/io/decoder/decoder_entry.c   — already uses img_result_t
//   src/io/encoder/encoder_entry.c   — change int return to img_result_t
//   src/io/decoder/streaming_decoder.c — change int return to img_result_t
//   src/cold/pipeline_compiled.c     — change int return to img_result_t
//   src/pipeline/pipeline_build.c    — change int return to img_result_t
//   src/runtime/scheduler.c          — change int return to img_result_t
//   src/runtime/worker.c             — change int return to img_result_t
//
// Third-party (stb_image.h) — DO NOT change. That is not our code.
// ================================================================

// ================================================================
// IMMEDIATE DIAGNOSTIC: why is processing failing right now?
//
// After applying img_result_name() you will see the actual error.
// Most likely causes in order of probability:
//
// 1. IMG_ERR_SECURITY — img_security_validate_request() rejecting
//    the image. The validator uses 4096x4096 as fixed dims — wrong.
//    It should read actual dimensions from the decoded header.
//    Quick fix: pass 0,0 to skip dimension pre-check, let the
//    decoder validate via tjDecompressHeader3 instead.
//
// 2. IMG_ERR_NOMEM — slab block too small for the decoded image.
//    A 4K JPEG decodes to ~50MB uncompressed (3840*2160*3).
//    Default slab block is 256KB (DEFAULT_BLOCK_SIZE in context.c).
//    Fix: increase DEFAULT_BLOCK_SIZE or use global_pool with
//    a large enough block (100MB was your original design intent).
//
// 3. IMG_ERR_FORMAT — turbojpeg rejecting the file. Unlikely for
//    a valid JPEG but possible if the file read was incomplete.
//
// Apply Fix 1-4 above, rebuild, run again.
// The error name in stderr will tell you exactly which case it is.
// ================================================================
