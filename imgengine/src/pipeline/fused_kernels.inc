/*
 * 🔥 MACRO-BASED CODEGEN
 */

#define DEFINE_KERNEL(NAME, DO_GRAY, DO_BRIGHT, DO_RESIZE)       \
static void NAME(img_ctx_t *ctx, img_buffer_t *buf)              \
{                                                               \
    img_fused_params_t *p = ctx->fused_params;                  \
                                                                \
    uint8_t *data = buf->data;                                 \
    uint32_t n = buf->width * buf->height * buf->channels;      \
                                                                \
    for (uint32_t i = 0; i < n; i++)                           \
    {                                                           \
        uint8_t px = data[i];                                   \
                                                                \
        if (DO_GRAY)                                            \
            px = (px * 77) >> 8;                                \
                                                                \
        if (DO_BRIGHT)                                          \
        {                                                       \
            uint16_t tmp = px + p->brightness;                  \
            px = (tmp > 255) ? 255 : tmp;                       \
        }                                                       \
                                                                \
        data[i] = px;                                           \
    }                                                           \
                                                                \
    if (DO_RESIZE)                                              \
    {                                                           \
        buf->width  = p->new_w;                                 \
        buf->height = p->new_h;                                 \
    }                                                           \
}