#ifndef IMGENGINE_API_V1_IMGENGINE_H
#define IMGENGINE_API_V1_IMGENGINE_H

#include <stddef.h>
#include <stdint.h>

#if defined(_WIN32)
#if defined(IMGENGINE_BUILDING_LIBRARY)
#define IMGENGINE_API __declspec(dllexport)
#else
#define IMGENGINE_API __declspec(dllimport)
#endif
#else
#define IMGENGINE_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define IMGENGINE_ABI_VERSION 1u

typedef struct imgengine_engine imgengine_engine_t;
typedef uint32_t imgengine_status_t;

enum {
    IMGENGINE_STATUS_OK = 0u,
    IMGENGINE_STATUS_INVALID_ARGUMENT = 1u,
    IMGENGINE_STATUS_INVALID_IMAGE = 2u,
    IMGENGINE_STATUS_RESOURCE_LIMIT = 3u,
    IMGENGINE_STATUS_OUT_OF_MEMORY = 4u,
    IMGENGINE_STATUS_UNSUPPORTED = 5u,
    IMGENGINE_STATUS_UNAVAILABLE = 6u,
    IMGENGINE_STATUS_INTERNAL_ERROR = 7u,
};

typedef struct imgengine_engine_options {
    uint32_t struct_size;
    uint32_t worker_count;
    uint64_t reserved[4];
} imgengine_engine_options_t;

typedef struct imgengine_output {
    uint8_t *data;
    size_t size;
} imgengine_output_t;

IMGENGINE_API uint32_t imgengine_abi_version(void);
IMGENGINE_API const char *imgengine_status_message(imgengine_status_t status);
IMGENGINE_API imgengine_status_t imgengine_engine_create(
    const imgengine_engine_options_t *options, imgengine_engine_t **out_engine);
IMGENGINE_API void imgengine_engine_destroy(imgengine_engine_t *engine);
IMGENGINE_API imgengine_status_t imgengine_capability_supported(const char *identifier,
                                                                 uint32_t *out_supported);
IMGENGINE_API imgengine_status_t imgengine_process_encoded_image_to_jpeg(
    imgengine_engine_t *engine, const uint8_t *input, size_t input_size,
    imgengine_output_t *output);
IMGENGINE_API void imgengine_output_release(imgengine_output_t *output);

#ifdef __cplusplus
}
#endif

#endif
