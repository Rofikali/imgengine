#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "imgengine/api/v1/imgengine.h"

#include "api/v1/img_api.h"

struct imgengine_engine {
    img_engine_t *native_engine;
    pthread_mutex_t operation_lock;
};

static pthread_mutex_t g_engine_scope_lock = PTHREAD_MUTEX_INITIALIZER;
static bool g_engine_active = false;

static imgengine_status_t imgengine_map_result(img_result_t result) {
    switch (result) {
    case IMG_SUCCESS:
        return IMGENGINE_STATUS_OK;
    case IMG_ERR_NOMEM:
        return IMGENGINE_STATUS_OUT_OF_MEMORY;
    case IMG_ERR_FORMAT:
        return IMGENGINE_STATUS_INVALID_IMAGE;
    case IMG_ERR_SECURITY:
        return IMGENGINE_STATUS_RESOURCE_LIMIT;
    case IMG_ERR_HW_UNSUP:
        return IMGENGINE_STATUS_UNAVAILABLE;
    case IMG_ERR_IO:
    case IMG_ERR_INTERNAL:
    default:
        return IMGENGINE_STATUS_INTERNAL_ERROR;
    }
}

uint32_t imgengine_abi_version(void) { return IMGENGINE_ABI_VERSION; }

const char *imgengine_status_message(imgengine_status_t status) {
    switch (status) {
    case IMGENGINE_STATUS_OK:
        return "ok";
    case IMGENGINE_STATUS_INVALID_ARGUMENT:
        return "invalid_argument";
    case IMGENGINE_STATUS_INVALID_IMAGE:
        return "invalid_image";
    case IMGENGINE_STATUS_RESOURCE_LIMIT:
        return "resource_limit";
    case IMGENGINE_STATUS_OUT_OF_MEMORY:
        return "out_of_memory";
    case IMGENGINE_STATUS_UNSUPPORTED:
        return "unsupported";
    case IMGENGINE_STATUS_UNAVAILABLE:
        return "unavailable";
    default:
        return "internal_error";
    }
}

imgengine_status_t imgengine_engine_create(const imgengine_engine_options_t *options,
                                           imgengine_engine_t **out_engine) {
    if (!out_engine)
        return IMGENGINE_STATUS_INVALID_ARGUMENT;
    *out_engine = NULL;
    if (!options || options->struct_size < sizeof(*options) || options->worker_count == 0 ||
        options->worker_count > 64)
        return IMGENGINE_STATUS_INVALID_ARGUMENT;
    for (size_t index = 0; index < sizeof(options->reserved) / sizeof(options->reserved[0]); index++) {
        if (options->reserved[index] != 0)
            return IMGENGINE_STATUS_INVALID_ARGUMENT;
    }

    pthread_mutex_lock(&g_engine_scope_lock);
    if (g_engine_active) {
        pthread_mutex_unlock(&g_engine_scope_lock);
        return IMGENGINE_STATUS_UNAVAILABLE;
    }

    imgengine_engine_t *engine = calloc(1, sizeof(*engine));
    if (!engine) {
        pthread_mutex_unlock(&g_engine_scope_lock);
        return IMGENGINE_STATUS_OUT_OF_MEMORY;
    }
    if (pthread_mutex_init(&engine->operation_lock, NULL) != 0) {
        free(engine);
        pthread_mutex_unlock(&g_engine_scope_lock);
        return IMGENGINE_STATUS_INTERNAL_ERROR;
    }

    engine->native_engine = img_api_init(options->worker_count);
    if (!engine->native_engine) {
        pthread_mutex_destroy(&engine->operation_lock);
        free(engine);
        pthread_mutex_unlock(&g_engine_scope_lock);
        return IMGENGINE_STATUS_UNAVAILABLE;
    }

    g_engine_active = true;
    *out_engine = engine;
    pthread_mutex_unlock(&g_engine_scope_lock);
    return IMGENGINE_STATUS_OK;
}

void imgengine_engine_destroy(imgengine_engine_t *engine) {
    if (!engine)
        return;

    pthread_mutex_lock(&engine->operation_lock);
    img_api_shutdown(engine->native_engine);
    engine->native_engine = NULL;
    pthread_mutex_unlock(&engine->operation_lock);
    pthread_mutex_destroy(&engine->operation_lock);

    pthread_mutex_lock(&g_engine_scope_lock);
    g_engine_active = false;
    pthread_mutex_unlock(&g_engine_scope_lock);
    free(engine);
}

imgengine_status_t imgengine_capability_supported(const char *identifier, uint32_t *out_supported) {
    if (!identifier || !out_supported)
        return IMGENGINE_STATUS_INVALID_ARGUMENT;
    *out_supported = strcmp(identifier, "image.jpeg_encode") == 0 ? 1u : 0u;
    return IMGENGINE_STATUS_OK;
}

imgengine_status_t imgengine_process_encoded_image_to_jpeg(imgengine_engine_t *engine,
                                                            const uint8_t *input, size_t input_size,
                                                            imgengine_output_t *output) {
    if (!output)
        return IMGENGINE_STATUS_INVALID_ARGUMENT;
    output->data = NULL;
    output->size = 0;
    if (!engine || !input || input_size == 0)
        return IMGENGINE_STATUS_INVALID_ARGUMENT;

    pthread_mutex_lock(&engine->operation_lock);
    uint8_t *encoded = NULL;
    size_t encoded_size = 0;
    const img_result_t result = img_api_process_raw(engine->native_engine, (uint8_t *)input,
                                                    input_size, &encoded, &encoded_size);
    pthread_mutex_unlock(&engine->operation_lock);
    if (result != IMG_SUCCESS) {
        img_encoded_free(encoded);
        return imgengine_map_result(result);
    }
    if (!encoded || encoded_size == 0) {
        img_encoded_free(encoded);
        return IMGENGINE_STATUS_INTERNAL_ERROR;
    }
    output->data = encoded;
    output->size = encoded_size;
    return IMGENGINE_STATUS_OK;
}

void imgengine_output_release(imgengine_output_t *output) {
    if (!output)
        return;
    img_encoded_free(output->data);
    output->data = NULL;
    output->size = 0;
}
