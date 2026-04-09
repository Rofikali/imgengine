// cmd/bench/lat_bench.c

#include <x86intrin.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "api/v1/img_api.h"

#define ITERATIONS 1000

static inline uint64_t rdtsc(void)
{
    return __rdtsc();
}

int main()
{
    img_engine_t *engine = img_api_init(1);
    if (!engine)
    {
        printf("engine init failed\n");
        return 1;
    }

    int fd = open("tests/samples/4k_test.jpg", O_RDONLY);
    if (fd < 0)
    {
        perror("open");
        return 1;
    }

    size_t size = lseek(fd, 0, SEEK_END);

    uint8_t *input = mmap(NULL, size,
                          PROT_READ,
                          MAP_PRIVATE,
                          fd, 0);

    if (!input)
        return 1;

    uint64_t total_cycles = 0;

    for (int i = 0; i < ITERATIONS; i++)
    {
        uint8_t *out = NULL;
        size_t out_size = 0;

        uint64_t start = rdtsc();

        img_api_process_raw(
            engine,
            input,
            size,
            &out,
            &out_size);

        uint64_t end = rdtsc();

        total_cycles += (end - start);
    }

    double avg = (double)total_cycles / ITERATIONS;

    printf("Avg cycles: %.2f\n", avg);

    munmap(input, size);
    close(fd);

    img_api_shutdown(engine);

    return 0;
}