#ifndef IMGENGINE_KERNEL_ADAPTER_H
#define IMGENGINE_KERNEL_ADAPTER_H

#include "pipeline/kernel_types.h"

/*
 * 🔥 RAW ADAPT (ZERO COST)
 */
#define IMG_ADAPT(fn) ((img_kernel_fn)(fn))

#endif