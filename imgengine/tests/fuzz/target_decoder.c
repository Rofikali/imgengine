#include <stddef.h>
#include <stdint.h>

#include "core/buffer.h"
#include "core/context_internal.h"
#include "io/decoder/decoder_dispatch.h"
#include "memory/slab.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (!data || size == 0 || size > (8u * 1024u * 1024u))
        return 0;

    static img_slab_pool_t *pool = NULL;
    if (!pool)
        pool = img_slab_create(8u * 1024u * 1024u, 8u * 1024u * 1024u);
    if (!pool)
        return 0;

    img_ctx_t ctx = {.local_pool = pool};
    img_buffer_t output = {0};
    (void)img_decode_dispatch(&ctx, data, size, &output);
    if (output.data)
        img_slab_free(pool, output.data);
    return 0;
}
