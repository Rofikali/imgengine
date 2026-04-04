/* memory/hugepage.h */
#ifndef IMGENGINE_MEMORY_HUGEPAGE_H
#define IMGENGINE_MEMORY_HUGEPAGE_H

#include <stddef.h>

/**
 * @brief Allocate memory using HugePages (2MB pages)
 */
void *img_hugepage_alloc(size_t size);

#endif