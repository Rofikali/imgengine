// ./src/startup/engine_init_sandbox.c
#define _GNU_SOURCE

#include "startup/engine_init_internal.h"
#include "security/sandbox.h"

#include <stdlib.h>

void img_engine_init_sandbox(void) {
    if (!img_security_enter_sandbox())
        abort();
}
