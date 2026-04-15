// include/api/io_vtable.h  (NEW FILE)

// ================================================================
// STEP 5 OF 6: Fix api/ depending on io/ (3 violations)
// api/api.c includes io/decoder and io/encoder directly.
// Fix: register decode/encode functions at init time via function
// pointers. api/ calls them through a thin io_vtable.
// ================================================================

#ifndef IMGENGINE_API_IO_VTABLE_H
#define IMGENGINE_API_IO_VTABLE_H

#include <stddef.h>
#include <stdint.h>
#include "core/result.h"

/*
 * io_vtable_t — I/O function pointer table.
 *
 * Registered at engine startup by io/ layer.
 * api/ calls I/O through these pointers — no direct include of io/.
 *
 * This breaks the api/ → io/ dependency:
 *   OLD: api/api.c #include "io/decoder/decoder_entry.h"  (violation)
 *   NEW: api/api.c calls g_io_vtable.decode()             (no violation)
 */

/* forward declarations */
typedef struct img_ctx img_ctx_t;
typedef struct img_buffer img_buffer_t;

typedef img_result_t (*img_decode_fn)(
    img_ctx_t *ctx,
    const uint8_t *input,
    size_t size,
    img_buffer_t *out);

typedef img_result_t (*img_encode_fn)(
    img_ctx_t *ctx,
    img_buffer_t *buf,
    uint8_t **out_data,
    size_t *out_size);

typedef img_result_t (*img_encode_pdf_fn)(
    const img_buffer_t *buf,
    const char *path,
    uint32_t dpi);

typedef struct
{
    img_decode_fn decode;
    img_encode_fn encode;
    img_encode_pdf_fn encode_pdf;
} img_io_vtable_t;

/* Global vtable — set by img_io_register() at startup */
extern img_io_vtable_t g_io_vtable;

/* Called once from io/ layer during engine init */
void img_io_register(
    img_decode_fn decode,
    img_encode_fn encode,
    img_encode_pdf_fn encode_pdf);

#endif /* IMGENGINE_API_IO_VTABLE_H */
