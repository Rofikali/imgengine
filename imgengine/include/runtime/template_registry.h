// ./include/runtime/template_registry.h
#ifndef IMGENGINE_RUNTIME_TEMPLATE_REGISTRY_H
#define IMGENGINE_RUNTIME_TEMPLATE_REGISTRY_H

#include <stdint.h>

#include "core/result.h"
#include "pipeline/job.h"

typedef struct img_engine img_engine_t;

#define IMG_TEMPLATE_REGISTRY_CAP_PREPARED 0x1u
#define IMG_TEMPLATE_REGISTRY_CAP_COMPILED 0x2u

typedef struct {
    img_job_template_t template_id;
    img_job_t job;
    uint32_t caps;
} img_template_entry_t;

typedef struct {
    uint32_t count;
    img_template_entry_t entries[4];
} img_template_registry_t;

img_template_registry_t *img_template_registry_create(void);
void img_template_registry_destroy(img_template_registry_t *registry);

const img_template_entry_t *img_template_registry_lookup(const img_engine_t *engine,
                                                         img_job_template_t template_id);

img_result_t img_runtime_resolve_template_job(const img_engine_t *engine,
                                              img_job_template_t template_id, img_job_t *job_out);

#endif
