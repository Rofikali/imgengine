
// pipeline/hardware_registry.h

#ifndef IMGENGINE_PIPELINE_HW_REGISTRY_H
#define IMGENGINE_PIPELINE_HW_REGISTRY_H

#include "arch/cpu_caps.h"

// 🔥 Use caps, not arch enum
void img_hw_register_kernels(cpu_caps_t caps);

#endif
