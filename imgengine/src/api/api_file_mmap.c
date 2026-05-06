// ./src/api/api_file_mmap.c
// src/api/api_file_mmap.c

#define _GNU_SOURCE

#include "core/result.h"

#include <stddef.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

img_result_t load_file_mmap(const char *path, uint8_t **data, size_t *size) {
    if (!path || !data || !size)
        return IMG_ERR_SECURITY;

    int fd = open(path, O_RDONLY);
    if (fd < 0)
        return IMG_ERR_IO;

    struct stat st;
    if (fstat(fd, &st) != 0 || st.st_size <= 0) {
        close(fd);
        return IMG_ERR_IO;
    }

    void *ptr = mmap(NULL, (size_t)st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    if (ptr == MAP_FAILED)
        return IMG_ERR_IO;

    *data = (uint8_t *)ptr;
    *size = (size_t)st.st_size;
    return IMG_SUCCESS;
}
