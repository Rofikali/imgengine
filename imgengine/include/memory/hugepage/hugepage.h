#ifndef IMGENGINE_HUGEPAGE_H
#define IMGENGINE_HUGEPAGE_H

#include <stddef.h>

void *img_hugepage_alloc(size_t size);
void img_hugepage_free(void *ptr, size_t size);

#endif