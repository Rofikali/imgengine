// ./src/runtime/rpc_server_io.c
#include "runtime/rpc_server_internal.h"

#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>

int img_rpc_server_recv_all(int fd, void *buf, size_t size) {
    uint8_t *p = (uint8_t *)buf;
    size_t done = 0;

    while (done < size) {
        ssize_t rc = recv(fd, p + done, size - done, 0);
        if (rc < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        if (rc == 0)
            return -1;
        done += (size_t)rc;
    }

    return 0;
}

int img_rpc_server_send_all(int fd, const void *buf, size_t size) {
    const uint8_t *p = (const uint8_t *)buf;
    size_t done = 0;

    while (done < size) {
        ssize_t rc = send(fd, p + done, size - done, 0);
        if (rc < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        if (rc == 0)
            return -1;
        done += (size_t)rc;
    }

    return 0;
}
