// ./src/runtime/rpc_client_close.c
#include "runtime/rpc_client_internal.h"

#include <unistd.h>

void img_rpc_client_close(int sock) {
    if (sock >= 0)
        close(sock);
}
