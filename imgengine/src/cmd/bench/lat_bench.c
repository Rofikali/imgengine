/* cmd/bench/lat_bench.c */

#include <x86intrin.h>
#include <stdio.h>
#include "api/v1/img_types.h"
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

        img_api_process_fast(
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

// #include <x86intrin.h>
// #include <stdio.h>
// #include "api/v1/img_core.h"

// int main()
// {
//     img_engine_t engine = img_api_init(1); // Pin to Core 0
//     const char *test_img = "tests/samples/4k_test.jpg";

//     printf("[BENCH] Starting 1000 iteration stress test...\n");

//     for (int i = 0; i < 1000; i++)
//     {
//         uint64_t start = __rdtsc();

//         // Full Hot Path Execution
//         img_api_process_fast(engine, test_img, "/dev/null", 1920, 1080);

//         uint64_t end = __rdtsc();
//         // Log cycles to a result array for P99 calculation...
//     }

//     img_api_shutdown(engine);
//     return 0;
// }
