// ./api/v1/img_api.h

#ifndef IMGENGINE_API_H
#define IMGENGINE_API_H

#include <stdint.h>
#include <stddef.h>
#include "api/v1/img_error.h"
#include "api/v1/img_job.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /*
     * Opaque engine handle.
     * Owns: worker threads, slab pools, jump tables, plugin registry.
     */
    typedef struct img_engine img_engine_t;
    typedef struct img_pipeline_desc img_pipeline_desc_t;
    typedef struct img_buffer img_buffer_t;

    /*
     * img_api_init()
     *
     * Initialize engine with `workers` worker threads.
     * Returns NULL on failure.
     * Thread-safe: call once per process.
     */
    img_engine_t *img_api_init(uint32_t workers);

    /*
     * img_api_shutdown()
     *
     * Drain queues, stop workers, free all resources.
     * Do not call concurrently with img_api_run_job().
     */
    void img_api_shutdown(img_engine_t *engine);

    /*
     * img_api_run_job()
     *
     * Full print pipeline:
     *   decode input → resize → grid layout → bleed →
     *   crop marks → border → encode output
     *
     * input_path:  path to JPEG or PNG source image
     * output_path: path for output (JPEG, PNG, or PDF by extension)
     * job:         print layout parameters (see img_job_t)
     *
     * Returns IMG_SUCCESS on success, IMG_ERR_* on failure.
     * Thread-safe: multiple calls may run concurrently.
     */
    img_result_t img_api_run_job(
        img_engine_t *engine,
        const char *input_path,
        const char *output_path,
        const img_job_t *job);

    /*
     * img_api_run_job_raw()
     *
     * Same render pipeline as img_api_run_job(), but returns the encoded
     * raster bytes in memory instead of writing them to disk. The caller
     * frees *output with img_encoded_free().
     *
     * Intended for bindings and async file writers such as io_uring.
     */
    img_result_t img_api_run_job_raw(
        img_engine_t *engine,
        const char *input_path,
        const img_job_t *job,
        uint8_t **output,
        size_t *output_size);

    /*
     * img_api_process_raw()
     *
     * Low-level: decode → no-op pipeline → encode.
     * Used by bench_lat and bindings.
     * Caller frees *output with free().
     */
    img_result_t img_api_process_raw(
        img_engine_t *engine,
        uint8_t *input,
        size_t input_size,
        uint8_t **output,
        size_t *output_size);

    /*
     * img_api_process_fast()
     *
     * Low-level kernel entrypoint:
     *   validate input → decode → execute precompiled pipeline
     *
     * Intended for benchmark harnesses and advanced bindings that already
     * own a compiled pipeline descriptor and output buffer.
     */
    img_result_t img_api_process_fast(
        img_engine_t *engine,
        const uint8_t *input,
        size_t input_size,
        img_pipeline_desc_t *pipe,
        img_buffer_t *out_buf);

    /*
     * img_api_release_raw_buffer()
     *
     * Release a decoded/raw buffer previously produced by img_api_process_fast()
     * or other API decode helpers.
     */
    void img_api_release_raw_buffer(
        img_engine_t *engine,
        img_buffer_t *buf);

    void img_encoded_free(uint8_t *ptr);

#ifdef __cplusplus
}
#endif

#endif /* IMGENGINE_API_H */
