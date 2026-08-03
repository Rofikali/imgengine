// ./src/api/api_job_load_input.c
#include "api/api_job_internal.h"
img_result_t load_file_mmap(const char *path, uint8_t **data, size_t *size);

img_result_t img_api_load_input(const char *input_path, uint8_t **file_data, size_t *file_size) {
    if (!input_path || !file_data || !file_size)
        return IMG_ERR_SECURITY;

    img_result_t r = load_file_mmap(input_path, file_data, file_size);
    if (r != IMG_SUCCESS)
        return r;

    if (*file_size == 0) {
        img_api_release_input(*file_data, *file_size);
        *file_data = NULL;
        *file_size = 0;
        return IMG_ERR_IO;
    }

    return IMG_SUCCESS;
}
