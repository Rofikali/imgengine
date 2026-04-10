
// ./src/memory/hugepage.c

#include "memory/hugepage.h"
#include <sys/mman.h>
#include <stdio.h>

void *img_hugepage_alloc(size_t size)
{
#ifdef MAP_HUGETLB
    void *ptr = mmap(NULL, size,
                     PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB,
                     -1, 0);

    if (ptr == MAP_FAILED)
    {
        perror("HugePage mmap failed");
        return NULL;
    }

    return ptr;
#else
    return NULL;
#endif
}