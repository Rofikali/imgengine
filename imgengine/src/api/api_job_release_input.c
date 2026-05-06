// ./src/api/api_job_release_input.c
#include "api/api_job_internal.h"

#include <sys/mman.h>

img_result_t img_api_release_input(uint8_t *file_data, size_t file_size) {
    if (!file_data || file_size == 0)
        return IMG_SUCCESS;

    if (munmap(file_data, file_size) != 0)
        return IMG_ERR_IO;

    return IMG_SUCCESS;
}
