#define _GNU_SOURCE
#include "memory/hugepage/hugepage.h"

#include <sys/mman.h>

void *img_hugepage_alloc(size_t size)
{
    return mmap(NULL,
                size,
                PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB,
                -1,
                0);
}

void img_hugepage_free(void *ptr, size_t size)
{
    if (ptr)
        munmap(ptr, size);
}