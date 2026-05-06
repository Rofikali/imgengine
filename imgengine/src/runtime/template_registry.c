// ./src/runtime/template_registry.c
#include "runtime/template_registry.h"

#include "core/context_internal.h"

#include <stdlib.h>

static void img_template_registry_seed_entry(img_template_registry_t *registry, uint32_t index,
                                             img_job_template_t template_id) {
    img_template_entry_t *entry = &registry->entries[index];
    img_job_defaults(&entry->job);
    img_job_apply_template(&entry->job, template_id);
    entry->template_id = template_id;
    entry->caps = IMG_TEMPLATE_REGISTRY_CAP_PREPARED;
}

img_template_registry_t *img_template_registry_create(void) {
    img_template_registry_t *registry = (img_template_registry_t *)calloc(1, sizeof(*registry));
    if (!registry)
        return NULL;

    registry->count = 3;
    img_template_registry_seed_entry(registry, 0, IMG_JOB_TEMPLATE_PASSPORT_45X35);
    img_template_registry_seed_entry(registry, 1, IMG_JOB_TEMPLATE_PASSPORT_38X35);
    img_template_registry_seed_entry(registry, 2, IMG_JOB_TEMPLATE_PRINTREADY_6X6);

    return registry;
}

void img_template_registry_destroy(img_template_registry_t *registry) { free(registry); }

const img_template_entry_t *img_template_registry_lookup(const img_engine_t *engine,
                                                         img_job_template_t template_id) {
    if (!engine || !engine->user_data)
        return NULL;

    const img_template_registry_t *registry = (const img_template_registry_t *)engine->user_data;

    for (uint32_t i = 0; i < registry->count; i++) {
        if (registry->entries[i].template_id == template_id)
            return &registry->entries[i];
    }

    return NULL;
}

img_result_t img_runtime_resolve_template_job(const img_engine_t *engine,
                                              img_job_template_t template_id, img_job_t *job_out) {
    if (!job_out)
        return IMG_ERR_SECURITY;

    if (template_id == IMG_JOB_TEMPLATE_CUSTOM) {
        img_job_defaults(job_out);
        return IMG_SUCCESS;
    }

    const img_template_entry_t *entry = img_template_registry_lookup(engine, template_id);
    if (!entry)
        return IMG_ERR_FORMAT;

    *job_out = entry->job;
    return IMG_SUCCESS;
}
