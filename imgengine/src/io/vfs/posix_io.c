
// ./src/io/vfs/posix_io.c


#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "api/v1/img_error.h"
#include <stdint.h>

img_result_t img_vfs_mmap_file(
    const char *path,
    uint8_t **data,
    size_t *size)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0)
        return IMG_ERR_IO;

    off_t sz = lseek(fd, 0, SEEK_END);

    void *ptr = mmap(NULL, sz, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    if (ptr == MAP_FAILED)
        return IMG_ERR_IO;

    *data = (uint8_t *)ptr;
    *size = sz;

    return IMG_SUCCESS;
}