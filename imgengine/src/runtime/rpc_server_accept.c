// ./src/runtime/rpc_server_accept.c
#include "runtime/rpc_server_internal.h"

#include <sys/socket.h>

int img_rpc_server_accept_client(int server_fd) { return accept(server_fd, NULL, NULL); }
