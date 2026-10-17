#define _GNU_SOURCE

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "imgengine/api/v1/imgengine.h"

static const uint8_t k_png_fixture[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44,
    0x52, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x03, 0x00, 0x00, 0x00, 0x25,
    0xdb, 0x56, 0xca, 0x00, 0x00, 0x00, 0x06, 0x50, 0x4c, 0x54, 0x45, 0xff, 0x00, 0x00, 0xff,
    0xff, 0xff, 0x41, 0x1d, 0x34, 0x11, 0x00, 0x00, 0x00, 0x0a, 0x49, 0x44, 0x41, 0x54, 0x08,
    0xd7, 0x63, 0x60, 0x00, 0x00, 0x00, 0x02, 0x00, 0x01, 0xe2, 0x21, 0xbc, 0x33, 0x00, 0x00,
    0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82,
};

int main(void) {
    const long page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0 || sizeof(k_png_fixture) > (size_t)page_size)
        return 1;
    uint8_t *input =
        mmap(NULL, (size_t)page_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (input == MAP_FAILED)
        return 1;
    memcpy(input, k_png_fixture, sizeof(k_png_fixture));

    const imgengine_engine_options_t options = {
        .struct_size = sizeof(options),
        .worker_count = 1,
    };
    imgengine_engine_t *engine = NULL;
    imgengine_output_t output = {0};
    const imgengine_status_t created = imgengine_engine_create(&options, &engine);
    const imgengine_status_t status =
        created == IMGENGINE_STATUS_OK
            ? imgengine_process_encoded_image_to_jpeg(engine, input, sizeof(k_png_fixture), &output)
            : created;
    unsigned char resident = 0;
    const int live = mincore(input, (size_t)page_size, &resident) == 0;
    const int passed = status == IMGENGINE_STATUS_OK && output.data && output.size > 0 && live &&
                       memcmp(input, k_png_fixture, sizeof(k_png_fixture)) == 0;
    imgengine_output_release(&output);
    imgengine_engine_destroy(engine);
    munmap(input, (size_t)page_size);
    if (!passed) {
        fprintf(stderr, "ABI v1 borrowed input was released or modified\n");
        return 1;
    }
    printf("[abi-ownership] borrowed ABI v1 input remains live and unchanged\n");
    return 0;
}
