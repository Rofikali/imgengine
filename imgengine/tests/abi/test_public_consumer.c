#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <imgengine/api/v1/imgengine.h>

static const uint8_t k_png[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49,
    0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x08, 0x06,
    0x00, 0x00, 0x00, 0x1f, 0x15, 0xc4, 0x89, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x44,
    0x41, 0x54, 0x08, 0xd7, 0x63, 0xf8, 0xcf, 0xc0, 0xf0, 0x1f, 0x00, 0x05, 0x00,
    0x01, 0xff, 0x89, 0x99, 0x3d, 0x1d, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e,
    0x44, 0xae, 0x42, 0x60, 0x82,
};

typedef struct process_thread {
    imgengine_engine_t *engine;
    int failed;
} process_thread_t;

static int process_valid_image(imgengine_engine_t *engine) {
    imgengine_output_t output = {0};
    const imgengine_status_t status = imgengine_process_encoded_image_to_jpeg(
        engine, k_png, sizeof(k_png), &output);
    if (status != IMGENGINE_STATUS_OK || output.size < 2 || output.data[0] != 0xff ||
        output.data[1] != 0xd8) {
        fprintf(stderr, "valid image processing failed: %s\n", imgengine_status_message(status));
        imgengine_output_release(&output);
        return 1;
    }
    imgengine_output_release(&output);
    imgengine_output_release(&output);
    return output.data != NULL || output.size != 0;
}

static void *run_process(void *argument) {
    process_thread_t *thread = argument;
    thread->failed = process_valid_image(thread->engine);
    return NULL;
}

int main(void) {
    if (imgengine_abi_version() != IMGENGINE_ABI_VERSION) {
        fprintf(stderr, "unexpected ABI version\n");
        return 1;
    }

    uint32_t supported = 0;
    if (imgengine_capability_supported("image.jpeg_encode", &supported) != IMGENGINE_STATUS_OK ||
        supported != 1 || imgengine_capability_supported("document.ocr", &supported) != IMGENGINE_STATUS_OK ||
        supported != 0) {
        fprintf(stderr, "capability discovery failed\n");
        return 1;
    }

    imgengine_engine_options_t options = {0};
    options.struct_size = sizeof(options);
    options.worker_count = 1;
    imgengine_engine_t *engine = NULL;
    if (imgengine_engine_create(&options, &engine) != IMGENGINE_STATUS_OK || !engine) {
        fprintf(stderr, "engine create failed\n");
        return 1;
    }

    imgengine_engine_t *second_engine = NULL;
    if (imgengine_engine_create(&options, &second_engine) != IMGENGINE_STATUS_UNAVAILABLE || second_engine) {
        fprintf(stderr, "process scope was not enforced\n");
        imgengine_engine_destroy(engine);
        return 1;
    }

    imgengine_output_t output = {(uint8_t *)"not-owned", 9};
    const uint8_t malformed[] = {0x00, 0xff, 0x01};
    if (imgengine_process_encoded_image_to_jpeg(engine, malformed, sizeof(malformed), &output) !=
            IMGENGINE_STATUS_INVALID_IMAGE ||
        output.data != NULL || output.size != 0) {
        fprintf(stderr, "malformed input contract failed\n");
        imgengine_engine_destroy(engine);
        return 1;
    }

    if (process_valid_image(engine) != 0) {
        imgengine_engine_destroy(engine);
        return 1;
    }

    process_thread_t threads[2] = {{.engine = engine}, {.engine = engine}};
    pthread_t handles[2];
    for (size_t index = 0; index < 2; index++) {
        if (pthread_create(&handles[index], NULL, run_process, &threads[index]) != 0) {
            fprintf(stderr, "thread creation failed\n");
            imgengine_engine_destroy(engine);
            return 1;
        }
    }
    for (size_t index = 0; index < 2; index++)
        pthread_join(handles[index], NULL);
    if (threads[0].failed || threads[1].failed) {
        imgengine_engine_destroy(engine);
        return 1;
    }

    imgengine_engine_destroy(engine);
    puts("[abi] public consumer passed");
    return 0;
}
