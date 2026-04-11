// src/cmd/bench/lat_bench.c

#include <x86intrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "api/v1/img_api.h"
#include "api/v1/img_error.h"

#define BENCH_ITERATIONS 1000
#define BENCH_WARMUP 100

int main(int argc, char **argv)
{
    const char *path = (argc > 1) ? argv[1] : "tests/samples/4k_test.jpg";

    /* OPEN */
    int fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        fprintf(stderr, "bench_lat: cannot open '%s' — "
                        "create tests/samples/4k_test.jpg or pass path as arg\n",
                path);
        return 1;
    }

    /* SIZE */
    off_t file_size = lseek(fd, 0, SEEK_END);
    if (file_size <= 0)
    {
        fprintf(stderr, "bench_lat: invalid file size %lld\n",
                (long long)file_size);
        close(fd);
        return 1;
    }
    lseek(fd, 0, SEEK_SET);

    /* MMAP — check for failure */
    uint8_t *input = mmap(NULL, (size_t)file_size,
                          PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    if (input == MAP_FAILED)
    {
        perror("bench_lat: mmap failed");
        return 1;
    }

    /* ENGINE */
    img_engine_t *engine = img_api_init(1);
    if (!engine)
    {
        fprintf(stderr, "bench_lat: engine init failed\n");
        munmap(input, (size_t)file_size);
        return 1;
    }

    /* WARMUP — not measured */
    for (int i = 0; i < BENCH_WARMUP; i++)
    {
        uint8_t *out = NULL;
        size_t out_size = 0;

        img_result_t r = img_api_process_raw(
            engine, input, (size_t)file_size, &out, &out_size);

        if (r != IMG_SUCCESS)
        {
            fprintf(stderr, "bench_lat: warmup failed: %s\n",
                    img_result_name(r));
            munmap(input, (size_t)file_size);
            img_api_shutdown(engine);
            return 1;
        }

        free(out);
    }

    /* BENCHMARK */
    uint64_t total_cycles = 0;
    uint64_t min_cycles = UINT64_MAX;
    uint64_t max_cycles = 0;

    for (int i = 0; i < BENCH_ITERATIONS; i++)
    {
        uint8_t *out = NULL;
        size_t out_size = 0;

        uint64_t start = __rdtsc();

        img_result_t r = img_api_process_raw(
            engine, input, (size_t)file_size, &out, &out_size);

        uint64_t end = __rdtsc();

        if (r != IMG_SUCCESS)
        {
            fprintf(stderr, "bench_lat: iteration %d failed: %s\n",
                    i, img_result_name(r));
            break;
        }

        uint64_t cycles = end - start;
        total_cycles += cycles;

        if (cycles < min_cycles)
            min_cycles = cycles;
        if (cycles > max_cycles)
            max_cycles = cycles;

        free(out);
    }

    uint64_t avg_cycles = total_cycles / BENCH_ITERATIONS;

    /* Approximate nanoseconds (assumes ~3GHz, adjust if needed) */
    double ns_per_cycle = 1000.0 / 3000.0; /* 1000ns / 3000MHz */
    double avg_ns = (double)avg_cycles * ns_per_cycle;
    double min_ns = (double)min_cycles * ns_per_cycle;
    double max_ns = (double)max_cycles * ns_per_cycle;

    printf("\n=== imgengine latency benchmark ===\n");
    printf("  file:       %s (%lld bytes)\n", path, (long long)file_size);
    printf("  iterations: %d (+ %d warmup)\n", BENCH_ITERATIONS, BENCH_WARMUP);
    printf("  avg:        %.2f ms (%.0f ns)\n", avg_ns / 1e6, avg_ns);
    printf("  min (P0):   %.2f ms (%.0f ns)\n", min_ns / 1e6, min_ns);
    printf("  max (P100): %.2f ms (%.0f ns)\n", max_ns / 1e6, max_ns);
    printf("===================================\n\n");

    munmap(input, (size_t)file_size);
    img_api_shutdown(engine);

    return 0;
}
