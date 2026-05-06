// ./src/io/io_register.c
// src/io/io_register.c  (NEW FILE)

// ================================================================
// Wires concrete implementations into the vtable.
// Called from img_engine_init() or img_api_init().
// ================================================================

#define _GNU_SOURCE

#include <stdlib.h>
#include <strings.h>

#include "io/io_vtable.h"
#include "io/decoder/decoder_entry.h"
#include "io/decoder/decoder_dispatch.h"
#include "io/encoder/encoder_entry.h"
#include "io/encoder/pdf_encoder.h"

img_io_vtable_t g_io_vtable = {0};

void img_io_register(img_decode_fn decode, img_encode_fn encode, img_encode_pdf_fn encode_pdf) {
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
void img_io_register_defaults(void) {
    /* Allow override via env var before we pick default strategy */
    const char *env = getenv("IMGENGINE_DECODE_STRATEGY");
    if (env) {
        if (strcasecmp(env, "stream") == 0)
            img_io_set_decode_strategy(IMG_DECODE_STRATEGY_STREAM);
        else if (strcasecmp(env, "bulk") == 0)
            img_io_set_decode_strategy(IMG_DECODE_STRATEGY_BULK);
        else
            img_io_set_decode_strategy(IMG_DECODE_STRATEGY_AUTO);
    }

    img_io_register((img_decode_fn)img_decode_dispatch, (img_encode_fn)img_encode_from_buffer,
                    img_encode_pdf);
}
