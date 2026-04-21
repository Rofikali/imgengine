// ./src/api/api_init.c

#include "api/v1/img_api.h"
#include "api/api_init_internal.h"
#include "io/io_vtable.h"

void img_io_register_defaults(void);

img_engine_t *img_api_init(uint32_t workers)
{
    if (img_api_init_enter_sandbox() != 0)
        return NULL;

    if (img_api_init_prepare_engine(workers) != 0)
        return NULL;

    img_io_register_defaults();

    if (img_api_init_boot_workers(workers) != 0)
        return NULL;

    return &g_engine;
}
