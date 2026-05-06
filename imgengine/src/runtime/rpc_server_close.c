// ./src/runtime/rpc_server_close.c
#include "runtime/rpc_server_internal.h"

#include <unistd.h>

void img_rpc_server_close(int server_fd) {
    if (server_fd >= 0)
        close(server_fd);
}
