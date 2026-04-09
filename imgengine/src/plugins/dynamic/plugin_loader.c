#include "api/v1/img_plugin_api.h"
#include "pipeline/dispatch/jump_table.h"

#include <dlfcn.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>

typedef const img_plugin_descriptor_t *(*plugin_getter_fn)(void);

static int load_plugin(const char *path)
{
    void *handle = dlopen(path, RTLD_NOW);
    if (!handle)
    {
        printf("dlopen failed: %s\n", dlerror());
        return -1;
    }

    plugin_getter_fn getter =
        (plugin_getter_fn)dlsym(handle, "img_get_plugin");

    if (!getter)
    {
        printf("dlsym failed\n");
        return -1;
    }

    const img_plugin_descriptor_t *desc = getter();

    if (!desc)
        return -1;

    /*
     * 🔥 ABI CHECK
     */
    if (desc->abi_version != IMG_PLUGIN_ABI_VERSION)
    {
        printf("ABI mismatch: %s\n", desc->name);
        return -1;
    }

    img_register_op(desc->op_code, desc->single_exec);

    printf("✅ Loaded plugin: %s\n", desc->name);

    return 0;
}

int img_plugin_load_all(const char *dir)
{
    DIR *d = opendir(dir);
    if (!d)
        return -1;

    struct dirent *ent;

    while ((ent = readdir(d)) != NULL)
    {
        if (strstr(ent->d_name, ".so"))
        {
            char path[512];
            snprintf(path, sizeof(path), "%s/%s", dir, ent->d_name);

            load_plugin(path);
        }
    }

    closedir(d);
    return 0;
}

// #include "api/v1/img_plugin_api.h"

// static void my_op(img_ctx_t *ctx, img_buffer_t *buf, void *params)
// {
//     (void)ctx;
//     (void)params;

//     buf->width += 10;
// }

// static img_plugin_descriptor_t plugin = {
//     .name = "my_plugin",
//     .abi_version = IMG_PLUGIN_ABI_VERSION,
//     .op_code = 42,
//     .capabilities = IMG_CAP_SINGLE,
//     .single_exec = my_op,
// };

// /*
//  * 🔥 REQUIRED EXPORT SYMBOL
//  */
// const img_plugin_descriptor_t *img_get_plugin(void)
// {
//     return &plugin;
// }