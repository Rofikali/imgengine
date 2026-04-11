// ./api/v1/img_plugin_api.h

// api/v1/img_plugin_abi.h

#ifndef IMGENGINE_PLUGIN_API_H
#define IMGENGINE_PLUGIN_API_H

#include <stdint.h>

/*
 * 🔥 ABI VERSION
 */
#define IMG_PLUGIN_ABI_VERSION 1

/*
 * 🔥 CAPABILITIES
 */
#define IMG_CAP_SINGLE (1u << 0)
#define IMG_CAP_BATCH (1u << 1)
#define IMG_CAP_ZERO_COPY (1u << 3)

/*
 * 🔥 FUNCTION TYPES
 */
typedef struct img_ctx img_ctx_t;
typedef struct img_buffer img_buffer_t;
typedef struct img_batch img_batch_t;

typedef void (*img_op_fn)(img_ctx_t *, img_buffer_t *);
typedef void (*img_batch_op_fn)(img_ctx_t *, img_batch_t *);

/*
 * 🔥 PLUGIN DESCRIPTOR
 */
typedef struct img_plugin_descriptor
{
    const char *name;
    uint32_t abi_version;
    uint32_t op_code;
    uint32_t capabilities;

    img_op_fn single_exec;
    img_batch_op_fn batch_exec;

} img_plugin_descriptor_t;

/*
 * 🔥 ELF SECTION MAGIC
 */
#if defined(__GNUC__)
#define IMG_PLUGIN_SECTION __attribute__((section("img_plugins"), used))
#else
#define IMG_PLUGIN_SECTION
#endif

/*
 * 🔥 DECLARE PLUGIN (ZERO COST)
 */
#define IMG_REGISTER_PLUGIN(desc)          \
    static const img_plugin_descriptor_t * \
        __img_plugin_##desc IMG_PLUGIN_SECTION = &desc

#endif
