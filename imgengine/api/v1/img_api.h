
// api/v1/img_api.h

#ifndef IMGENGINE_API_H
#define IMGENGINE_API_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // 🔥 Opaque types (kernel-style)
    typedef struct img_engine img_engine_t;
    typedef struct img_buffer img_buffer_t;
    typedef struct img_pipeline img_pipeline_t;

    // Lifecycle
    img_engine_t *img_api_init(uint32_t workers);
    void img_api_shutdown(img_engine_t *engine);

    // 🔥 Zero-copy processing (preferred)
    int img_api_process(
        img_engine_t *engine,
        img_buffer_t *input,
        img_buffer_t **output);

    // 🔥 Fast raw API (for bindings)
    int img_api_process_raw(
        img_engine_t *engine,
        uint8_t *input,
        size_t input_size,
        uint8_t **output,
        size_t *output_size);

#ifdef __cplusplus
}
#endif

#endif