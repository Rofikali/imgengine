// ./include/pipeline/plugin_abi.h
//  include/pipeline/plugin_abi.h  (NEW FILE)

#ifndef IMGENGINE_PIPELINE_PLUGIN_ABI_H
#define IMGENGINE_PIPELINE_PLUGIN_ABI_H

#include <stdint.h>

/*
 * Plugin ABI — moved from api/v1/img_plugin_api.h to pipeline/plugin_abi.h
 *
 * The plugin descriptor is a pipeline concept (registers into jump table).
 * runtime/ (plugin_loader) and plugins/ both need it.
 * Neither should depend on api/ just for this struct.
 *
 * api/v1/img_plugin_api.h re-exports this for external plugin authors.
 */

#define IMG_PLUGIN_ABI_VERSION 1

#define IMG_CAP_SINGLE (1u << 0)
#define IMG_CAP_BATCH (1u << 1)
#define IMG_CAP_ZERO_COPY (1u << 3)

typedef struct img_ctx img_ctx_t;
typedef struct img_buffer img_buffer_t;
typedef struct img_batch img_batch_t;

typedef void (*img_op_fn)(img_ctx_t *, img_buffer_t *, void *);
typedef void (*img_batch_op_fn)(img_ctx_t *, img_batch_t *, void *);

typedef struct img_plugin_descriptor {
    const char *name;
    uint32_t abi_version;
    uint32_t op_code;
    uint32_t capabilities;
    img_op_fn single_exec;
    img_batch_op_fn batch_exec;
} img_plugin_descriptor_t;

typedef const img_plugin_descriptor_t *(*img_plugin_getter_fn)(void);

#define IMG_PLUGIN_EXPORT_SYMBOL "img_get_plugin_descriptor"

#if defined(__GNUC__)
#define IMG_PLUGIN_SECTION __attribute__((section("img_plugins"), used))
#else
#define IMG_PLUGIN_SECTION
#endif

#define IMG_REGISTER_PLUGIN(desc)                                                                  \
    static const img_plugin_descriptor_t *__img_plugin_##desc IMG_PLUGIN_SECTION = &desc

#endif /* IMGENGINE_PIPELINE_PLUGIN_ABI_H */