// ./src/runtime/rpc_client_open.c
#include "runtime/rpc_client_internal.h"

#include <sys/socket.h>

int img_rpc_client_open(void) { return socket(AF_INET, SOCK_STREAM, 0); }
