// ./src/cmd/bench/lat_bench.c

#define _GNU_SOURCE

#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#include "api/v1/img_api.h"
#include "api/v1/img_error.h"
#include "api/api_benchmark_internal.h"
#include "api/api_internal.h"
#include "api/api_process_fast_internal.h"
#include "pipeline/job_presets.h"

#define BENCH_ITERATIONS 1000
#define BENCH_WARMUP 100

static int compare_u64(const void *lhs, const void *rhs)
{
    const uint64_t a = *(const uint64_t *)lhs;
    const uint64_t b = *(const uint64_t *)rhs;

    if (a < b)
        return -1;
    if (a > b)
        return 1;
    return 0;
}

static uint64_t monotonic_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

static uint64_t percentile_ns(const uint64_t *sorted, size_t count, double pct)
{
    if (!sorted || count == 0)
        return 0;

    double rank = pct * (double)(count - 1);
    size_t idx = (size_t)(rank + 0.5);
    if (idx >= count)
        idx = count - 1;
    return sorted[idx];
}

static void print_stats(
    const char *label,
    const uint64_t *samples,
    size_t count)
{
    uint64_t *sorted = malloc(count * sizeof(*sorted));
    if (!sorted)
        return;

    memcpy(sorted, samples, count * sizeof(*sorted));
    qsort(sorted, count, sizeof(*sorted), compare_u64);

    uint64_t total_ns = 0;
    for (size_t i = 0; i < count; i++)
        total_ns += samples[i];

    const double avg_ns = (double)total_ns / (double)count;
    const uint64_t p50_ns = percentile_ns(sorted, count, 0.50);
    const uint64_t p99_ns = percentile_ns(sorted, count, 0.99);
    const uint64_t min_ns = sorted[0];
    const uint64_t max_ns = sorted[count - 1];
    const double throughput = (avg_ns > 0.0) ? (1e9 / avg_ns) : 0.0;

    printf("%s\n", label);
    printf("  avg:  %.3f ms (%.0f ns)\n", avg_ns / 1e6, avg_ns);
    printf("  p50:  %.3f ms (%" PRIu64 " ns)\n", (double)p50_ns / 1e6, p50_ns);
    printf("  p99:  %.3f ms (%" PRIu64 " ns)\n", (double)p99_ns / 1e6, p99_ns);
    printf("  min:  %.3f ms (%" PRIu64 " ns)\n", (double)min_ns / 1e6, min_ns);
    printf("  max:  %.3f ms (%" PRIu64 " ns)\n", (double)max_ns / 1e6, max_ns);
    printf("  thr:  %.0f ops/sec\n", throughput);

    free(sorted);
}

static int benchmark_raw(img_engine_t *engine, const uint8_t *input, size_t input_size)
{
    uint64_t *samples = calloc(BENCH_ITERATIONS, sizeof(*samples));
    if (!samples)
        return 1;

    for (int i = 0; i < BENCH_WARMUP; i++)
    {
        uint8_t *out = NULL;
        size_t out_size = 0;
        img_result_t r = img_api_process_raw(
            engine, (uint8_t *)input, input_size, &out, &out_size);
        if (r != IMG_SUCCESS)
        {
            fprintf(stderr, "bench_lat: raw warmup failed: %s\n", img_result_name(r));
            free(samples);
            img_encoded_free(out);
            return 1;
        }
        img_encoded_free(out);
    }

    for (int i = 0; i < BENCH_ITERATIONS; i++)
    {
        uint8_t *out = NULL;
        size_t out_size = 0;

        const uint64_t start_ns = monotonic_ns();
        img_result_t r = img_api_process_raw(
            engine, (uint8_t *)input, input_size, &out, &out_size);
        const uint64_t end_ns = monotonic_ns();

        if (r != IMG_SUCCESS)
        {
            fprintf(stderr, "bench_lat: raw iteration %d failed: %s\n",
                    i, img_result_name(r));
            free(samples);
            img_encoded_free(out);
            return 1;
        }

        samples[i] = end_ns - start_ns;
        img_encoded_free(out);
    }

    print_stats("Cold path (decode + encode)", samples, BENCH_ITERATIONS);
    free(samples);
    return 0;
}

static int benchmark_hot(
    img_engine_t *engine,
    const uint8_t *input,
    size_t input_size,
    img_job_template_t preset_template)
{
    img_hot_bench_state_t state;
    uint64_t *samples = calloc(BENCH_ITERATIONS, sizeof(*samples));
    if (!samples)
    {
        fprintf(stderr, "bench_lat: hot benchmark allocation failed\n");
        free(samples);
        return 1;
    }

    img_result_t r = img_api_hot_bench_init_with_template(
        engine, input, input_size, preset_template, &state);
    if (r != IMG_SUCCESS)
    {
        fprintf(stderr, "bench_lat: hot setup failed: %s\n", img_result_name(r));
        free(samples);
        return 1;
    }

    for (int i = 0; i < BENCH_WARMUP; i++)
    {
        r = img_api_hot_bench_step(&state);
        if (r != IMG_SUCCESS)
        {
            fprintf(stderr, "bench_lat: hot warmup failed: %s\n", img_result_name(r));
            free(samples);
            img_api_hot_bench_destroy(engine, &state);
            return 1;
        }
    }

    for (int i = 0; i < BENCH_ITERATIONS; i++)
    {
        const uint64_t start_ns = monotonic_ns();
        r = img_api_hot_bench_step(&state);
        const uint64_t end_ns = monotonic_ns();
        if (r != IMG_SUCCESS)
        {
            fprintf(stderr, "bench_lat: hot iteration %d failed: %s\n",
                    i, img_result_name(r));
            free(samples);
            img_api_hot_bench_destroy(engine, &state);
            return 1;
        }

        samples[i] = end_ns - start_ns;
    }

    if (preset_template != IMG_JOB_TEMPLATE_CUSTOM)
    {
        printf("  preset:   %s\n", img_job_template_name(preset_template));
        fflush(stdout);
    }

    print_stats("Passport render stage (runtime + pipeline + arch)", samples, BENCH_ITERATIONS);
    fflush(stdout);

    free(samples);
    img_api_hot_bench_destroy(engine, &state);
    return 0;
}

static int benchmark_decode_only(img_engine_t *engine, const uint8_t *input, size_t input_size)
{
    uint64_t *samples = calloc(BENCH_ITERATIONS, sizeof(*samples));
    if (!samples)
    {
        fprintf(stderr, "bench_lat: decode benchmark allocation failed\n");
        return 1;
    }

    for (int i = 0; i < BENCH_WARMUP; i++)
    {
        img_buffer_t out = {0};
        img_result_t r = decode_image_secure(engine, input, input_size, &out);
        if (r != IMG_SUCCESS)
        {
            fprintf(stderr, "bench_lat: decode warmup failed: %s\n", img_result_name(r));
            free(samples);
            img_api_release_raw_buffer(engine, &out);
            return 1;
        }
        img_api_release_raw_buffer(engine, &out);
    }

    for (int i = 0; i < BENCH_ITERATIONS; i++)
    {
        img_buffer_t out = {0};

        const uint64_t start_ns = monotonic_ns();
        img_result_t r = decode_image_secure(engine, input, input_size, &out);
        const uint64_t end_ns = monotonic_ns();

        if (r != IMG_SUCCESS)
        {
            fprintf(stderr, "bench_lat: decode iteration %d failed: %s\n",
                    i, img_result_name(r));
            free(samples);
            img_api_release_raw_buffer(engine, &out);
            return 1;
        }

        samples[i] = end_ns - start_ns;
        img_api_release_raw_buffer(engine, &out);
    }

    print_stats("Decode-only path (API decode helper)", samples, BENCH_ITERATIONS);
    fflush(stdout);
    free(samples);
    return 0;
}

int main(int argc, char **argv)
{
    static struct option long_opts[] = {
        {"preset", required_argument, 0, 'p'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0},
    };

    img_job_template_t preset_template = IMG_JOB_TEMPLATE_CUSTOM;
    int opt;
    int idx = 0;

    optind = 1;
    while ((opt = getopt_long(argc, argv, "p:h", long_opts, &idx)) != -1)
    {
        switch (opt)
        {
        case 'p':
            if (img_job_template_lookup(optarg, &preset_template) != 0)
            {
                fprintf(stderr,
                        "bench_lat: unknown preset '%s' (use %s, %s, %s)\n",
                        optarg,
                        img_job_template_name(IMG_JOB_TEMPLATE_PASSPORT_45X35),
                        img_job_template_name(IMG_JOB_TEMPLATE_PASSPORT_38X35),
                        img_job_template_name(IMG_JOB_TEMPLATE_PRINTREADY_6X6));
                return 1;
            }
            break;
        case 'h':
            printf("Usage: %s [--preset <name>] [file]\n", argv[0]);
            return 0;
        default:
            fprintf(stderr, "bench_lat: invalid arguments\n");
            return 1;
        }
    }

    const char *path = "tests/samples/4k_test.jpg";
    if (optind < argc)
    {
        path = argv[optind++];
        if (optind < argc)
        {
            fprintf(stderr, "bench_lat: unexpected extra arguments\n");
            return 1;
        }
    }

    int fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        fprintf(stderr, "bench_lat: cannot open '%s' — "
                        "create tests/samples/4k_test.jpg or pass path as arg\n",
                path);
        return 1;
    }

    off_t file_size = lseek(fd, 0, SEEK_END);
    if (file_size <= 0)
    {
        fprintf(stderr, "bench_lat: invalid file size %lld\n",
                (long long)file_size);
        close(fd);
        return 1;
    }
    lseek(fd, 0, SEEK_SET);

    uint8_t *input = mmap(NULL, (size_t)file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    if (input == MAP_FAILED)
    {
        perror("bench_lat: mmap failed");
        return 1;
    }

    img_engine_t *engine = img_api_init(1);
    if (!engine)
    {
        fprintf(stderr, "bench_lat: engine init failed\n");
        munmap(input, (size_t)file_size);
        return 1;
    }

    printf("\n=== imgengine latency benchmark ===\n");
    printf("  file:       %s (%lld bytes)\n", path, (long long)file_size);
    printf("  iterations: %d (+ %d warmup)\n", BENCH_ITERATIONS, BENCH_WARMUP);
    fflush(stdout);

    int rc = 0;
    if (benchmark_hot(engine, input, (size_t)file_size, preset_template) != 0)
        rc = 1;
    else if (benchmark_decode_only(engine, input, (size_t)file_size) != 0)
        rc = 1;
    else
    {
        printf("\n--- entering cold path (decode + encode) ---\n");
        fflush(stdout);
        if (benchmark_raw(engine, input, (size_t)file_size) != 0)
            rc = 1;
    }

    printf("===================================\n\n");

    img_api_shutdown(engine);
    munmap(input, (size_t)file_size);
    return rc;
}
