// ./include/pipeline/job_presets.h
#ifndef IMGENGINE_PIPELINE_JOB_PRESETS_H
#define IMGENGINE_PIPELINE_JOB_PRESETS_H

#include <string.h>

#include "pipeline/job.h"

static inline const char *img_job_template_name(img_job_template_t template_id) {
    switch (template_id) {
    case IMG_JOB_TEMPLATE_PASSPORT_45X35:
        return "passport-45x35";
    case IMG_JOB_TEMPLATE_PASSPORT_38X35:
        return "passport-38x35";
    case IMG_JOB_TEMPLATE_PRINTREADY_6X6:
        return "printready-6x6";
    case IMG_JOB_TEMPLATE_CUSTOM:
    default:
        return "custom";
    }
}

static inline const char *img_job_template_description(img_job_template_t template_id) {
    switch (template_id) {
    case IMG_JOB_TEMPLATE_PASSPORT_45X35:
        return "4.5 x 3.5 cm, 6x6 grid, border 2px, bleed 1px, crop 5px";
    case IMG_JOB_TEMPLATE_PASSPORT_38X35:
        return "3.8 x 3.5 cm, 6x6 grid, border 2px, bleed 1px, crop 5px";
    case IMG_JOB_TEMPLATE_PRINTREADY_6X6:
        return "6x6 print sheet, border 2px, bleed 1px, crop 5px";
    case IMG_JOB_TEMPLATE_CUSTOM:
    default:
        return "user-defined layout";
    }
}

static inline int img_job_template_lookup(const char *name, img_job_template_t *template_id) {
    if (!name || !template_id)
        return -1;

    if (strcmp(name, "passport-45x35") == 0 || strcmp(name, "passport-45x35-printready") == 0) {
        *template_id = IMG_JOB_TEMPLATE_PASSPORT_45X35;
        return 0;
    }

    if (strcmp(name, "passport-38x35") == 0 || strcmp(name, "passport-38x35-printready") == 0) {
        *template_id = IMG_JOB_TEMPLATE_PASSPORT_38X35;
        return 0;
    }

    if (strcmp(name, "printready-6x6") == 0) {
        *template_id = IMG_JOB_TEMPLATE_PRINTREADY_6X6;
        return 0;
    }

    return -1;
}

#endif /* IMGENGINE_PIPELINE_JOB_PRESETS_H */
