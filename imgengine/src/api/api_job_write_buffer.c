// ./src/api/api_job_write_buffer.c
#include "api/api_job_internal.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

img_result_t img_write_buffer_file(const char *output_path, const uint8_t *data, size_t size) {
    if (!output_path || !data)
        return IMG_ERR_SECURITY;

    int flags = O_WRONLY | O_CREAT | O_TRUNC;
#ifdef O_CLOEXEC
    flags |= O_CLOEXEC;
#endif

    int fd = open(output_path, flags, 0644);
    if (fd < 0)
        return IMG_ERR_IO;

    size_t written = 0;
    while (written < size) {
        ssize_t rc = write(fd, data + written, size - written);
        if (rc < 0) {
            if (errno == EINTR)
                continue;
            close(fd);
            return IMG_ERR_IO;
        }

        if (rc == 0) {
            close(fd);
            return IMG_ERR_IO;
        }

        written += (size_t)rc;
    }

    if (close(fd) != 0)
        return IMG_ERR_IO;

    return IMG_SUCCESS;
}
