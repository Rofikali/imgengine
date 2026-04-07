/* cmd/bench/lat_bench.c */

#include <x86intrin.h>
#include <stdio.h>
#include "api/v1/img_api.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

int main()
{
    img_engine_t *engine = img_api_init(1);

    int fd = open("tests/samples/4k_test.jpg", O_RDONLY);
    size_t size = lseek(fd, 0, SEEK_END);

    uint8_t *input = mmap(NULL, size,
                          PROT_READ,
                          MAP_PRIVATE,
                          fd, 0);

    for (int i = 0; i < 1000; i++)
    {
        uint8_t *out;
        size_t out_size;

        uint64_t start = __rdtsc();

        img_api_process_raw(
            engine,
            input,
            size,
            &out,
            &out_size);

        uint64_t end = __rdtsc();
    }

    munmap(input, size);
    img_api_shutdown(engine);

    return 0;
}
