// pipeline/hardware_registry.h

#ifndef IMGENGINE_PIPELINE_HW_REGISTRY_H
#define IMGENGINE_PIPELINE_HW_REGISTRY_H

#include "arch/cpu_caps.h"

// 🔥 Use caps, not arch enum
void img_hw_register_kernels(cpu_caps_t caps);

#endif

// #ifndef IMGENGINE_PIPELINE_HW_REGISTRY_H
// #define IMGENGINE_PIPELINE_HW_REGISTRY_H

// #include "arch/cpu_caps.h"

// void img_registry_init_hardware(cpu_arch_t arch);

// #endif