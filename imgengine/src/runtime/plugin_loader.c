
// ./src/runtime/plugin_loader.c


#include "api/v1/img_plugin_api.h"
#include "runtime/plugin_loader.h"
#include "pipeline/jump_table.h"

#include <dlfcn.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>

static int load_plugin(const char *path)
{
    void *handle = dlopen(path, RTLD_NOW);
    if (!handle)
    {
        printf("dlopen failed: %s\n", dlerror());
        return -1;
    }

    img_plugin_getter_fn getter =
        (img_plugin_getter_fn)dlsym(handle, IMG_PLUGIN_EXPORT_SYMBOL);

    if (!getter)
    {
        printf("dlsym failed: %s\n", dlerror());
        return -1;
    }

    const img_plugin_descriptor_t *desc = getter();

    // 🔥 ABI CHECK
    if (desc->abi_version != IMG_PLUGIN_ABI_VERSION)
    {
        printf("ABI mismatch for plugin: %s\n", desc->name);
        return -1;
    }

    // 🔥 Register into jump table
    img_register_op(
        desc->op_code,
        desc->single_exec,
        desc->batch_exec);

    printf("Loaded plugin: %s\n", desc->name);

    return 0;
}

int img_plugin_load_all(const char *directory)
{
    DIR *dir = opendir(directory);
    if (!dir)
        return -1;

    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL)
    {
        if (strstr(entry->d_name, ".so"))
        {
            char fullpath[512];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", directory, entry->d_name);

            load_plugin(fullpath);
        }
    }

    closedir(dir);
    return 0;
}