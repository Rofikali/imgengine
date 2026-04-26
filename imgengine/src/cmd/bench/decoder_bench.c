// ./src/cmd/bench/decoder_bench.c
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <inttypes.h>

#include "api/v1/img_api.h"
#include "core/context_internal.h"
#include "core/buffer.h"
#include "io/io_vtable.h"
#include "io/decoder/decoder_dispatch.h"

static uint64_t monotonic_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

static int compare_u64(const void *a, const void *b)
{
    uint64_t x = *(const uint64_t *)a;
    uint64_t y = *(const uint64_t *)b;
    if (x < y) return -1;
    if (x > y) return 1;
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "usage: %s <image.jpg> [auto|bulk|stream] [iterations]\n", argv[0]);
        return 2;
    }

    const char *path = argv[1];
    const char *strategy = (argc >= 3) ? argv[2] : "auto";
    int iters = (argc >= 4) ? atoi(argv[3]) : 1000;
    int warmup = iters / 10;

    FILE *f = fopen(path, "rb");
    if (!f)
    {
        perror("open");
        return 2;
    }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz <= 0)
    {
        fprintf(stderr, "empty file\n");
        fclose(f);
        return 2;
    }

    uint8_t *data = malloc((size_t)sz);
    if (!data) { fclose(f); return 2; }
    if (fread(data, 1, (size_t)sz, f) != (size_t)sz) { free(data); fclose(f); return 2; }
    fclose(f);

    img_engine_t *engine = img_api_init(1);
    if (!engine) { fprintf(stderr, "img_api_init failed\n"); free(data); return 2; }

    if (strcasecmp(strategy, "stream") == 0)
        img_io_set_decode_strategy(IMG_DECODE_STRATEGY_STREAM);
    else if (strcasecmp(strategy, "bulk") == 0)
        img_io_set_decode_strategy(IMG_DECODE_STRATEGY_BULK);
    else
        img_io_set_decode_strategy(IMG_DECODE_STRATEGY_AUTO);

    img_ctx_t ctx = {0};
    img_ctx_bind_engine(engine, &ctx);

    uint64_t *samples = calloc((size_t)iters, sizeof(*samples));
    if (!samples) { img_api_shutdown(engine); free(data); return 2; }

    img_buffer_t buf = {0};

    /* warmup */
    for (int i = 0; i < warmup; i++)
    {
        img_result_t r = g_io_vtable.decode(&ctx, data, (size_t)sz, &buf);
        (void)r;
        img_api_release_raw_buffer(engine, &buf);
    }

    for (int i = 0; i < iters; i++)
    {
        uint64_t t0 = monotonic_ns();
        img_result_t r = g_io_vtable.decode(&ctx, data, (size_t)sz, &buf);
        uint64_t t1 = monotonic_ns();
        samples[i] = t1 - t0;
        if (r != IMG_SUCCESS)
        {
            fprintf(stderr, "decode failed: %s\n", img_result_name(r));
        }
        img_api_release_raw_buffer(engine, &buf);
    }

    qsort(samples, iters, sizeof(*samples), compare_u64);
    uint64_t total = 0;
    for (int i = 0; i < iters; i++) total += samples[i];
    uint64_t avg = total / (uint64_t)iters;
    uint64_t p50 = samples[iters * 50 / 100];
    uint64_t p99 = samples[iters * 99 / 100];
    uint64_t min = samples[0];
    uint64_t max = samples[iters - 1];

    printf("Decoder bench: %s\n", path);
    printf(" strategy: %s\n", strategy);
    printf(" iterations: %d (warmup %d)\n", iters, warmup);
        printf(" avg: %.3f ms (%" PRIu64 " ns)\n", (double)avg / 1e6, avg);
        printf(" p50: %.3f ms (%" PRIu64 " ns)\n", (double)p50 / 1e6, p50);
        printf(" p99: %.3f ms (%" PRIu64 " ns)\n", (double)p99 / 1e6, p99);
        printf(" min: %.3f ms (%" PRIu64 " ns)\n", (double)min / 1e6, min);
        printf(" max: %.3f ms (%" PRIu64 " ns)\n", (double)max / 1e6, max);
        printf(" thr: %.0f ops/sec\n", (avg > 0) ? (1e9 / (double)avg) : 0.0);

    free(samples);
    img_api_shutdown(engine);
    free(data);
    return 0;
}