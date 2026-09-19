#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <imgengine/api/v1/imgengine.h>

static int read_file(const char *path, uint8_t **data, size_t *size) {
    FILE *file = fopen(path, "rb");
    if (!file)
        return -1;
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return -1;
    }
    const long length = ftell(file);
    if (length < 0 || fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return -1;
    }
    *size = (size_t)length;
    *data = *size == 0 ? NULL : malloc(*size);
    if (*size != 0 && !*data) {
        fclose(file);
        return -1;
    }
    if (*size != 0 && fread(*data, 1, *size, file) != *size) {
        free(*data);
        *data = NULL;
        fclose(file);
        return -1;
    }
    fclose(file);
    return 0;
}

static int write_file(const char *path, const imgengine_output_t *output) {
    FILE *file = fopen(path, "wb");
    if (!file)
        return -1;
    const int failed = fwrite(output->data, 1, output->size, file) != output->size;
    fclose(file);
    return failed ? -1 : 0;
}

static uint64_t elapsed_ns(const struct timespec *start, const struct timespec *end) {
    return (uint64_t)(end->tv_sec - start->tv_sec) * UINT64_C(1000000000) +
           (uint64_t)(end->tv_nsec - start->tv_nsec);
}

int main(int argc, char **argv) {
    if (argc != 5) {
        fprintf(stderr, "usage: %s INPUT OUTPUT EXPECTED_STATUS REPETITIONS\n", argv[0]);
        return 2;
    }
    const unsigned long expected = strtoul(argv[3], NULL, 10);
    const unsigned long repetitions = strtoul(argv[4], NULL, 10);
    if (expected > UINT32_MAX || repetitions == 0)
        return 2;

    uint32_t capability_supported = 0;
    if (imgengine_abi_version() != IMGENGINE_ABI_VERSION ||
        imgengine_capability_supported("image.jpeg_encode", &capability_supported) !=
            IMGENGINE_STATUS_OK ||
        capability_supported != 1) {
        fprintf(stderr, "required public capability is unavailable\n");
        return 1;
    }

    uint8_t *input = NULL;
    size_t input_size = 0;
    if (read_file(argv[1], &input, &input_size) != 0) {
        fprintf(stderr, "unable to read input %s: %s\n", argv[1], strerror(errno));
        return 2;
    }

    imgengine_engine_options_t options = {0};
    options.struct_size = sizeof(options);
    options.worker_count = 1;
    imgengine_engine_t *engine = NULL;
    const imgengine_status_t create_status = imgengine_engine_create(&options, &engine);
    if (create_status != IMGENGINE_STATUS_OK) {
        fprintf(stderr, "engine creation failed: %s\n", imgengine_status_message(create_status));
        free(input);
        return 1;
    }

    imgengine_status_t status = IMGENGINE_STATUS_INTERNAL_ERROR;
    imgengine_output_t output = {0};
    struct timespec start = {0};
    struct timespec end = {0};
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (unsigned long iteration = 0; iteration < repetitions; iteration++) {
        imgengine_output_release(&output);
        status = imgengine_process_encoded_image_to_jpeg(engine, input, input_size, &output);
        if (status != expected)
            break;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);

    int result = 0;
    if (status != expected) {
        fprintf(stderr, "expected status %lu, received %u (%s)\n", expected, status,
                imgengine_status_message(status));
        result = 1;
    } else if (status == IMGENGINE_STATUS_OK) {
        if (output.size < 2 || output.data[0] != 0xff || output.data[1] != 0xd8 ||
            write_file(argv[2], &output) != 0) {
            fprintf(stderr, "valid JPEG output was not produced\n");
            result = 1;
        }
    } else if (output.data != NULL || output.size != 0) {
        fprintf(stderr, "failed operation returned owned output\n");
        result = 1;
    }

    printf("status=%u input_bytes=%zu output_bytes=%zu repetitions=%lu elapsed_ns=%llu\n", status,
           input_size, output.size, repetitions, (unsigned long long)elapsed_ns(&start, &end));
    imgengine_output_release(&output);
    imgengine_engine_destroy(engine);
    free(input);
    return result;
}
