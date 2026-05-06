// ./src/api/api_init_sandbox.c
#include "api/api_init_internal.h"
#include "security/sandbox.h"

int img_api_init_enter_sandbox(void) { return img_security_enter_sandbox() ? 0 : -1; }
