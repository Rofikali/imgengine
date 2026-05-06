// ./src/startup/engine_init_compute.c
#define _GNU_SOURCE

#include "startup/engine_init_internal.h"
#include "pipeline/jump_table.h"

void img_hw_register_kernels(cpu_caps_t caps);
void img_fused_init(cpu_caps_t caps);

void img_engine_init_compute(cpu_caps_t caps) {
    img_jump_table_init(caps);
    img_hw_register_kernels(caps);
    img_fused_init(caps);
}
