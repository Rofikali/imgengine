#define _POSIX_C_SOURCE 200809L

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <imgengine/api/v1/imgengine.h>
#include <imgengine/api/v2/layout.h>

static const uint8_t k_png[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48,
    0x44, 0x52, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x08, 0x06, 0x00, 0x00,
    0x00, 0x1f, 0x15, 0xc4, 0x89, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x44, 0x41, 0x54, 0x08,
    0xd7, 0x63, 0xf8, 0xcf, 0xc0, 0xf0, 0x1f, 0x00, 0x05, 0x00, 0x01, 0xff, 0x89, 0x99,
    0x3d, 0x1d, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82,
};

typedef struct process_thread {
    imgengine_engine_t *engine;
    int failed;
} process_thread_t;

static int process_valid_image(imgengine_engine_t *engine) {
    imgengine_output_t output = {0};
    const imgengine_status_t status =
        imgengine_process_encoded_image_to_jpeg(engine, k_png, sizeof(k_png), &output);
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

static int layout_valid_image(imgengine_engine_t *engine) {
    const imgengine_layout_rect_v2_t placement = {
        .x = 1,
        .y = 1,
        .width = 4,
        .height = 4,
    };
    const imgengine_layout_request_v2_t request = {
        .struct_size = sizeof(request),
        .version = IMGENGINE_LAYOUT_RENDER_V2_VERSION,
        .input = k_png,
        .input_size = sizeof(k_png),
        .canvas_width = 6,
        .canvas_height = 6,
        .background_r = 255,
        .background_g = 255,
        .background_b = 255,
        .placements = &placement,
        .placement_count = 1,
        .expected_count = 1,
    };
    imgengine_layout_output_v2_t output = {0};
    const imgengine_status_t status = imgengine_layout_render_v2(engine, &request, &output);
    if (status != IMGENGINE_STATUS_OK || output.size < 2 || output.data[0] != 0xff ||
        output.data[1] != 0xd8 || output.canvas_width != 6 || output.canvas_height != 6 ||
        output.expected_count != 1 || output.planned_count != 1 || output.executed_count != 1) {
        fprintf(stderr, "layout rendering failed: %s\n", imgengine_status_message(status));
        imgengine_layout_output_release(&output);
        return 1;
    }
    imgengine_layout_output_release(&output);
    imgengine_layout_output_release(&output);
    return output.data != NULL || output.size != 0;
}

static int layout_output_is_empty(const imgengine_layout_output_v2_t *output) {
    return !output->data && output->size == 0 && output->canvas_width == 0 &&
           output->canvas_height == 0 && output->expected_count == 0 &&
           output->planned_count == 0 && output->executed_count == 0;
}

static int layout_bounded_output_contract(imgengine_engine_t *engine) {
    static const uint8_t malformed[] = {0x00, 0x01, 0x02, 0x03};
    const imgengine_layout_rect_v2_t placement = {.x = 0, .y = 0, .width = 6, .height = 6};
    const imgengine_layout_request_v2_t request = {
        .struct_size = sizeof(request),
        .version = IMGENGINE_LAYOUT_RENDER_V2_VERSION,
        .input = k_png,
        .input_size = sizeof(k_png),
        .canvas_width = 6,
        .canvas_height = 6,
        .background_r = 255,
        .background_g = 255,
        .background_b = 255,
        .placements = &placement,
        .placement_count = 1,
        .expected_count = 1,
    };
    imgengine_layout_output_v2_t output = {(uint8_t *)"not-owned", 9, 1, 1, 1, 1, 1};
    if (imgengine_layout_render_v2_bounded(engine, &request, 0, &output) !=
            IMGENGINE_STATUS_INVALID_ARGUMENT ||
        !layout_output_is_empty(&output))
        return 1;
    output = (imgengine_layout_output_v2_t){(uint8_t *)"not-owned", 9, 1, 1, 1, 1, 1};
    if (imgengine_layout_render_v2_bounded(engine, &request, 1, &output) !=
            IMGENGINE_STATUS_RESOURCE_LIMIT ||
        !layout_output_is_empty(&output))
        return 1;
    imgengine_layout_request_v2_t malformed_request = request;
    malformed_request.input = malformed;
    malformed_request.input_size = sizeof(malformed);
    output = (imgengine_layout_output_v2_t){(uint8_t *)"not-owned", 9, 1, 1, 1, 1, 1};
    if (imgengine_layout_render_v2_bounded(engine, &malformed_request, 16u * 1024u * 1024u,
                                           &output) != IMGENGINE_STATUS_INVALID_IMAGE ||
        !layout_output_is_empty(&output))
        return 1;
    if (imgengine_layout_render_v2_bounded(engine, &request, 16u * 1024u * 1024u, &output) !=
            IMGENGINE_STATUS_OK ||
        !output.data || output.size == 0 || output.size >= 16u * 1024u * 1024u ||
        output.data[0] != 0xff || output.data[1] != 0xd8) {
        imgengine_layout_output_release(&output);
        return 1;
    }
    imgengine_layout_output_release(&output);
    return !layout_output_is_empty(&output);
}

static int layout_resource_limits_are_enforced(imgengine_engine_t *engine) {
    const imgengine_layout_rect_v2_t placement = {
        .x = 0,
        .y = 0,
        .width = 1,
        .height = 1,
    };
    imgengine_layout_output_v2_t output = {0};
    imgengine_layout_request_v2_t request = {
        .canvas_width = 4097,
        .canvas_height = 1,
        .struct_size = sizeof(request),
        .version = IMGENGINE_LAYOUT_RENDER_V2_VERSION,
        .input = k_png,
        .input_size = sizeof(k_png),
        .placements = &placement,
        .placement_count = 1,
        .expected_count = 1,
    };

    /* Oversized dimension must be rejected before allocating or scheduling. */
    output.data = (uint8_t *)"sentinel";
    output.size = 99;
    if (imgengine_layout_render_v2(engine, &request, &output) != IMGENGINE_STATUS_RESOURCE_LIMIT ||
        output.data != NULL || output.size != 0)
        return 1;

    /* Dimensions individually within bounds can still exceed the pixel budget. */
    request.canvas_width = 2001;
    request.canvas_height = 2000;
    output.data = (uint8_t *)"sentinel";
    output.size = 99;
    if (imgengine_layout_render_v2(engine, &request, &output) != IMGENGINE_STATUS_RESOURCE_LIMIT ||
        output.data != NULL || output.size != 0)
        return 1;

    /* The quadratic overlap validator must never receive an unbounded list. */
    imgengine_layout_rect_v2_t placements[65] = {{0}};
    request.canvas_width = 65;
    request.canvas_height = 1;
    request.placements = placements;
    request.placement_count = 65;
    request.expected_count = 65;
    output.data = (uint8_t *)"sentinel";
    output.size = 99;
    if (imgengine_layout_render_v2(engine, &request, &output) != IMGENGINE_STATUS_RESOURCE_LIMIT ||
        output.data != NULL || output.size != 0)
        return 1;

    return 0;
}

static void *run_process(void *argument) {
    process_thread_t *thread = argument;
    thread->failed = process_valid_image(thread->engine);
    return NULL;
}

static int malformed_input_is_silent(imgengine_engine_t *engine) {
    static const uint8_t malformed[] =
        "P75-redaction-probe /private/request/secret-image.jpg X-API-Key: not-a-real-secret";
    FILE *capture = NULL;
    int saved_stderr = -1;
    int failed = 0;
    char diagnostics[4096] = {0};

    capture = tmpfile();
    if (!capture || (saved_stderr = dup(STDERR_FILENO)) < 0) {
        fprintf(stderr, "could not capture ABI diagnostics\n");
        if (capture)
            fclose(capture);
        return 1;
    }

    fflush(stderr);
    if (dup2(fileno(capture), STDERR_FILENO) < 0) {
        fprintf(stderr, "could not redirect ABI diagnostics\n");
        close(saved_stderr);
        fclose(capture);
        return 1;
    }

    for (unsigned int index = 0; index < 32u; index++) {
        imgengine_output_t output = {(uint8_t *)"not-owned", 9};
        if (imgengine_process_encoded_image_to_jpeg(engine, malformed, sizeof(malformed) - 1u,
                                                    &output) != IMGENGINE_STATUS_INVALID_IMAGE ||
            output.data != NULL || output.size != 0) {
            failed = 1;
        }
        imgengine_output_release(&output);
    }

    fflush(stderr);
    if (dup2(saved_stderr, STDERR_FILENO) < 0)
        failed = 1;
    close(saved_stderr);
    rewind(capture);
    const size_t bytes = fread(diagnostics, 1, sizeof(diagnostics) - 1u, capture);
    const int read_failed = ferror(capture);
    fclose(capture);

    if (failed || read_failed || (bytes != 0 && strstr(diagnostics, "[API]") != NULL)) {
        fprintf(stderr, "ABI processing emitted diagnostics for malformed input\n");
        return 1;
    }
    return 0;
}

int main(void) {
    if (imgengine_abi_version() != IMGENGINE_ABI_VERSION) {
        fprintf(stderr, "unexpected ABI version\n");
        return 1;
    }

    uint32_t supported = 0;
    if (imgengine_capability_supported("image.jpeg_encode", &supported) != IMGENGINE_STATUS_OK ||
        supported != 1 ||
        imgengine_capability_supported("document.ocr", &supported) != IMGENGINE_STATUS_OK ||
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
    if (imgengine_engine_create(&options, &second_engine) != IMGENGINE_STATUS_UNAVAILABLE ||
        second_engine) {
        fprintf(stderr, "process scope was not enforced\n");
        imgengine_engine_destroy(engine);
        return 1;
    }

    if (malformed_input_is_silent(engine) != 0) {
        imgengine_engine_destroy(engine);
        return 1;
    }

    if (process_valid_image(engine) != 0) {
        imgengine_engine_destroy(engine);
        return 1;
    }

    if (layout_valid_image(engine) != 0 || layout_resource_limits_are_enforced(engine) != 0) {
        fprintf(stderr, "layout ABI-v2 resource-limit regression failed\n");
        imgengine_engine_destroy(engine);
        return 1;
    }
    if (layout_bounded_output_contract(engine) != 0) {
        fprintf(stderr, "bounded layout output contract failed\n");
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
