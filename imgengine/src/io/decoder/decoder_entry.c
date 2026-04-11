// ./src/io/decoder/decoder_entry.c

// ./src/io/decoder/decoder_entry.c


#include "io/decoder/decoder_entry.h"

#include <turbojpeg.h>
#include <stdint.h>
#include <stddef.h>

#include "core/buffer.h"
#include "memory/slab.h"

#include "security/bounds_check.h"
#include "security/input_validator.h"

#include "arch/arch_interface.h"

// ============================================================
// 🔥 INTERNAL SAFE MULTIPLY (overflow guard)
// ============================================================

static inline int safe_mul(size_t a, size_t b, size_t *out)
{
    if (a == 0 || b == 0)
    {
        *out = 0;
        return 1;
    }

    if (a > SIZE_MAX / b)
        return 0;

    *out = a * b;
    return 1;
}

// ============================================================
// 🔥 SECURE JPEG DECODE ENTRY
// ============================================================

int img_decode_to_buffer(
    img_ctx_t *ctx,
    const uint8_t *input,
    size_t size,
    img_buffer_t *out)
{
    if (!ctx || !input || size == 0 || !out)
        return IMG_ERR_SECURITY;

    // ========================================================
    // 🔥 1. BOUNDS CHECK (WHOLE INPUT)
    // ========================================================

    if (!img_bounds_check(input, size, input, size))
        return IMG_ERR_SECURITY;

    // ========================================================
    // 🔥 2. INIT DECODER
    // ========================================================

    tjhandle tj = tjInitDecompress();
    if (!tj)
        return IMG_ERR_NOMEM;

    int w = 0, h = 0, subsamp = 0, cs = 0;

    // ========================================================
    // 🔥 3. READ HEADER (UNTRUSTED)
    // ========================================================

    if (tjDecompressHeader3(
            tj,
            input,
            size,
            &w,
            &h,
            &subsamp,
            &cs) != 0)
    {
        tjDestroy(tj);
        return IMG_ERR_FORMAT;
    }

    // ========================================================
    // 🔥 4. VALIDATE DIMENSIONS (CRITICAL)
    // ========================================================

    img_result_t sec = img_security_validate_request(
        (uint32_t)w,
        (uint32_t)h,
        size);

    if (sec != IMG_SUCCESS)
    {
        tjDestroy(tj);
        return sec;
    }

    // ========================================================
    // 🔥 5. SIZE CALCULATION (SAFE)
    // ========================================================

    const size_t ch = 3;

    size_t stride = 0;
    size_t required = 0;

    if (!safe_mul((size_t)w, ch, &stride))
    {
        tjDestroy(tj);
        return IMG_ERR_SECURITY;
    }

    if (!safe_mul(stride, (size_t)h, &required))
    {
        tjDestroy(tj);
        return IMG_ERR_SECURITY;
    }

    // ========================================================
    // 🔥 6. SLAB LIMIT CHECK
    // ========================================================

    size_t block = img_slab_block_size(ctx->local_pool);

    if (required > block)
    {
        tjDestroy(tj);
        return IMG_ERR_NOMEM;
    }

    // ========================================================
    // 🔥 7. ALLOCATE (SAFE ZONE)
    // ========================================================

    uint8_t *mem = img_slab_alloc(ctx->local_pool);
    if (!mem)
    {
        tjDestroy(tj);
        return IMG_ERR_NOMEM;
    }

    // ========================================================
    // 🔥 8. DECODE (CONTROLLED WRITE)
    // ========================================================

    if (tjDecompress2(
            tj,
            input,
            size,
            mem,
            w,
            (int)stride,
            h,
            TJPF_RGB,
            TJFLAG_FASTDCT | TJFLAG_FASTUPSAMPLE) != 0)
    {
        img_slab_free(ctx->local_pool, mem);
        tjDestroy(tj);
        return IMG_ERR_FORMAT;
    }

    // ========================================================
    // 🔥 9. FINAL BOUNDS ASSERT (DEFENSE-IN-DEPTH)
    // ========================================================

    if (!img_bounds_check(mem, required, mem, block))
    {
        img_slab_free(ctx->local_pool, mem);
        tjDestroy(tj);
        return IMG_ERR_SECURITY;
    }

    // ========================================================
    // 🔥 10. OUTPUT (ZERO-COPY)
    // ========================================================

    out->data = mem;
    out->width = (uint32_t)w;
    out->height = (uint32_t)h;
    out->channels = (uint32_t)ch;
    out->stride = (uint32_t)stride;

    tjDestroy(tj);
    return IMG_SUCCESS;
}
