// src/io/io_register.c  (NEW FILE)

// ================================================================
// Wires concrete implementations into the vtable.
// Called from img_engine_init() or img_api_init().
// ================================================================

#define _GNU_SOURCE

#include "api/io_vtable.h"
#include "io/decoder/decoder_entry.h"
#include "io/encoder/encoder_entry.h"
#include "io/encoder/pdf_encoder.h"
#include "core/result.h"

img_io_vtable_t g_io_vtable = {0};

void img_io_register(
    img_decode_fn decode,
    img_encode_fn encode,
    img_encode_pdf_fn encode_pdf)
{
    g_io_vtable.decode = decode;
    g_io_vtable.encode = encode;
    g_io_vtable.encode_pdf = encode_pdf;
}

/*
 * img_io_register_defaults()
 *
 * Register the standard JPEG/PNG decoder and encoder.
 * Called once from img_api_init() after slab pools are ready.
 * After this call, api/ can call I/O without including io/ headers.
 */
void img_io_register_defaults(void)
{
    img_io_register(
        (img_decode_fn)img_decode_to_buffer,
        (img_encode_fn)img_encode_from_buffer,
        img_encode_pdf);
}
