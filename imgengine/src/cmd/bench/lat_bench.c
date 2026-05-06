// ./src/cmd/bench/lat_bench.c

#define _GNU_SOURCE

#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <turbojpeg.h>

#include "api/v1/img_api.h"
#include "api/v1/img_error.h"
#include "api/api_benchmark_internal.h"
#include "api/api_internal.h"
#include "api/api_process_fast_internal.h"
#include "api/api_process_raw_internal.h"
#include "core/context_internal.h"
#include "core/cpu_caps.h"
#include "io/encoder/encoder_entry.h"
#include "pipeline/job_presets.h"

#define BENCH_ITERATIONS 1000
#define BENCH_WARMUP 100
#define BENCH_SWEEP_ITERATIONS 100
#define BENCH_SWEEP_WARMUP 20

typedef enum {
    BENCH_INPUT_FORMAT_ENCODED = 0,
    BENCH_INPUT_FORMAT_RAW_RGB24 = 1,
} bench_input_format_t;

typedef struct {
    double avg_ms;
    double p50_ms;
    double p95_ms;
    double p99_ms;
    double min_ms;
    double max_ms;
    double throughput;
    uint64_t avg_ns;
    uint64_t p50_ns;
    uint64_t p95_ns;
    uint64_t p99_ns;
    uint64_t min_ns;
    uint64_t max_ns;
} bench_stats_t;

static int compare_u64(const void *lhs, const void *rhs) {
    const uint64_t a = *(const uint64_t *)lhs;
    const uint64_t b = *(const uint64_t *)rhs;

    if (a < b)
        return -1;
    if (a > b)
        return 1;
    return 0;
}

static uint64_t monotonic_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

static uint64_t percentile_ns(const uint64_t *sorted, size_t count, double pct) {
    if (!sorted || count == 0)
        return 0;

    double rank = pct * (double)(count - 1);
    size_t idx = (size_t)(rank + 0.5);
    if (idx >= count)
        idx = count - 1;
    return sorted[idx];
}

static bench_stats_t compute_stats(const uint64_t *samples, size_t count) {
    bench_stats_t stats = {0};
    uint64_t *sorted = malloc(count * sizeof(*sorted));
    if (!sorted || !samples || count == 0) {
        free(sorted);
        return stats;
    }

    memcpy(sorted, samples, count * sizeof(*sorted));
    qsort(sorted, count, sizeof(*sorted), compare_u64);

    uint64_t total_ns = 0;
    for (size_t i = 0; i < count; i++)
        total_ns += samples[i];

    stats.avg_ns = (uint64_t)((double)total_ns / (double)count);
    stats.p50_ns = percentile_ns(sorted, count, 0.50);
    stats.p95_ns = percentile_ns(sorted, count, 0.95);
    stats.p99_ns = percentile_ns(sorted, count, 0.99);
    stats.min_ns = sorted[0];
    stats.max_ns = sorted[count - 1];
    stats.avg_ms = (double)stats.avg_ns / 1e6;
    stats.p50_ms = (double)stats.p50_ns / 1e6;
    stats.p95_ms = (double)stats.p95_ns / 1e6;
    stats.p99_ms = (double)stats.p99_ns / 1e6;
    stats.min_ms = (double)stats.min_ns / 1e6;
    stats.max_ms = (double)stats.max_ns / 1e6;
    stats.throughput = (stats.avg_ns > 0u) ? (1e9 / (double)stats.avg_ns) : 0.0;

    free(sorted);
    return stats;
}

static void print_stats(const char *label, const uint64_t *samples, size_t count,
                        bench_stats_t *stats_out) {
    bench_stats_t stats = compute_stats(samples, count);

    if (stats_out)
        *stats_out = stats;

    printf("%s\n", label);
    printf("  avg:  %.3f ms (%" PRIu64 " ns)\n", stats.avg_ms, stats.avg_ns);
    printf("  p50:  %.3f ms (%" PRIu64 " ns)\n", stats.p50_ms, stats.p50_ns);
    printf("  p99:  %.3f ms (%" PRIu64 " ns)\n", stats.p99_ms, stats.p99_ns);
    printf("  min:  %.3f ms (%" PRIu64 " ns)\n", stats.min_ms, stats.min_ns);
    printf("  max:  %.3f ms (%" PRIu64 " ns)\n", stats.max_ms, stats.max_ns);
    printf("  thr:  %.0f ops/sec\n", stats.throughput);
}

static void print_benchmark_note(bench_input_format_t input_format) {
    printf("  note:       prepared benchmark excludes final encode/output\n");
    if (input_format == BENCH_INPUT_FORMAT_RAW_RGB24)
        printf("  note:       raw-rgb24 mode also bypasses decode by contract\n");
    printf("  note:       final render-cache hits are disabled for honest timings\n");
}

static const char *subsamp_name(int subsamp) {
    switch (subsamp) {
    case TJSAMP_420:
        return "4:2:0";
    case TJSAMP_422:
        return "4:2:2";
    case TJSAMP_440:
        return "4:4:0";
    case TJSAMP_GRAY:
        return "GRAY";
    case TJSAMP_444:
    default:
        return "4:4:4";
    }
}

static const char *cpu_caps_name(cpu_caps_t caps) {
    if (img_cpu_has_avx512(caps))
        return "AVX512";
    if (img_cpu_has_avx2(caps))
        return "AVX2";
    if (img_cpu_has_neon(caps))
        return "NEON";
    return "SCALAR";
}

static const char *resize_dispatch_name(cpu_caps_t caps) {
    if (img_cpu_has_avx512(caps))
        return "AVX2 resize on AVX512 CPU (current fallback)";
    if (img_cpu_has_avx2(caps))
        return "AVX2";
    if (img_cpu_has_neon(caps))
        return "SCALAR resize on NEON CPU (NEON path not wired)";
    return "SCALAR";
}

static void print_simd_profile(const img_engine_t *engine) {
    cpu_caps_t caps = engine ? engine->caps : 0;
    printf("  cpu caps:   %s (0x%x)\n", cpu_caps_name(caps), (unsigned int)caps);
    printf("  resize op:  %s\n", resize_dispatch_name(caps));
}

static void print_score_line(const char *label, bool pass, double measured, double target,
                             const char *unit) {
    printf("  %-30s %s  measured=%.3f%s target=%s%.3f%s\n", label, pass ? "PASS" : "FAIL", measured,
           unit, pass ? "<= " : "<= ", target, unit);
}

static void print_throughput_score_line(const char *label, bool pass, double measured,
                                        double target) {
    printf("  %-30s %s  measured=%.0f ops/sec target=>= %.0f ops/sec\n", label,
           pass ? "PASS" : "FAIL", measured, target);
}

static void print_rfc_scorecard(bench_input_format_t input_format,
                                const bench_stats_t *render_stats,
                                const bench_stats_t *decode_stats,
                                const bench_stats_t *encode_stats,
                                const bench_stats_t *cold_stats) {
    const double render_target_ms = 2.0;
    const double throughput_target = 100000.0;

    printf("\nRFC scorecard\n");
    printf("  target reference: docs/v2.0 RFC.md -> < 2 ms latency, 100K+ ops/sec\n");

    if (render_stats) {
        print_score_line("prepared render avg", render_stats->avg_ms <= render_target_ms,
                         render_stats->avg_ms, render_target_ms, " ms");
        print_score_line("prepared render p99", render_stats->p99_ms <= render_target_ms,
                         render_stats->p99_ms, render_target_ms, " ms");
        print_throughput_score_line("prepared render throughput",
                                    render_stats->throughput >= throughput_target,
                                    render_stats->throughput, throughput_target);
    }

    if (decode_stats)
        print_score_line("decode ingress avg", decode_stats->avg_ms <= render_target_ms,
                         decode_stats->avg_ms, render_target_ms, " ms");

    if (encode_stats)
        print_score_line("encode-only avg", encode_stats->avg_ms <= render_target_ms,
                         encode_stats->avg_ms, render_target_ms, " ms");

    if (cold_stats) {
        print_score_line("cold path avg", cold_stats->avg_ms <= render_target_ms,
                         cold_stats->avg_ms, render_target_ms, " ms");
        print_throughput_score_line("cold path throughput",
                                    cold_stats->throughput >= throughput_target,
                                    cold_stats->throughput, throughput_target);
    }

    if (input_format == BENCH_INPUT_FORMAT_ENCODED && render_stats && decode_stats && cold_stats) {
        double other_ms = cold_stats->avg_ms - decode_stats->avg_ms - render_stats->avg_ms;
        if (other_ms < 0.0)
            other_ms = 0.0;
        double total_ms = cold_stats->avg_ms > 0.0 ? cold_stats->avg_ms : 1.0;
        double decode_pct = (decode_stats->avg_ms / total_ms) * 100.0;
        double render_pct = (render_stats->avg_ms / total_ms) * 100.0;
        double other_pct = (other_ms / total_ms) * 100.0;

        printf("  bottleneck summary               decode=%.3f ms, render=%.3f ms, "
               "encode+other~=%.3f ms\n",
               decode_stats->avg_ms, render_stats->avg_ms, other_ms);
        printf(
            "  time share                       decode=%.1f%% render=%.1f%% encode+other~=%.1f%%\n",
            decode_pct, render_pct, other_pct);
    }
}

static int parse_u32_arg(const char *text, uint32_t *out, const char *name) {
    char *end = NULL;
    unsigned long value;

    if (!text || !out)
        return -1;

    value = strtoul(text, &end, 10);
    if (!end || *end != '\0' || value > UINT32_MAX) {
        fprintf(stderr, "bench_lat: invalid %s '%s'\n", name, text ? text : "");
        return -1;
    }

    *out = (uint32_t)value;
    return 0;
}

static const char *bench_input_format_name(bench_input_format_t format) {
    switch (format) {
    case BENCH_INPUT_FORMAT_RAW_RGB24:
        return "raw-rgb24";
    case BENCH_INPUT_FORMAT_ENCODED:
    default:
        return "encoded";
    }
}

static int benchmark_raw(img_engine_t *engine, const uint8_t *input, size_t input_size,
                         bench_stats_t *stats_out) {
    uint64_t *samples = calloc(BENCH_ITERATIONS, sizeof(*samples));
    if (!samples)
        return 1;

    for (int i = 0; i < BENCH_WARMUP; i++) {
        uint8_t *out = NULL;
        size_t out_size = 0;
        img_result_t r = img_api_process_raw(engine, (uint8_t *)input, input_size, &out, &out_size);
        if (r != IMG_SUCCESS) {
            fprintf(stderr, "bench_lat: raw warmup failed: %s\n", img_result_name(r));
            free(samples);
            img_encoded_free(out);
            return 1;
        }
        img_encoded_free(out);
    }

    for (int i = 0; i < BENCH_ITERATIONS; i++) {
        uint8_t *out = NULL;
        size_t out_size = 0;

        const uint64_t start_ns = monotonic_ns();
        img_result_t r = img_api_process_raw(engine, (uint8_t *)input, input_size, &out, &out_size);
        const uint64_t end_ns = monotonic_ns();

        if (r != IMG_SUCCESS) {
            fprintf(stderr, "bench_lat: raw iteration %d failed: %s\n", i, img_result_name(r));
            free(samples);
            img_encoded_free(out);
            return 1;
        }

        samples[i] = end_ns - start_ns;
        img_encoded_free(out);
    }

    print_stats("Cold path (decode + encode)", samples, BENCH_ITERATIONS, stats_out);
    free(samples);
    return 0;
}

static int benchmark_hot(img_engine_t *engine, const uint8_t *input, size_t input_size,
                         img_job_template_t preset_template, bench_stats_t *stats_out) {
    img_hot_bench_state_t state;
    uint64_t *samples = calloc(BENCH_ITERATIONS, sizeof(*samples));
    if (!samples) {
        fprintf(stderr, "bench_lat: hot benchmark allocation failed\n");
        free(samples);
        return 1;
    }

    img_result_t r =
        img_api_hot_bench_init_with_template(engine, input, input_size, preset_template, &state);
    if (r != IMG_SUCCESS) {
        fprintf(stderr, "bench_lat: hot setup failed: %s\n", img_result_name(r));
        free(samples);
        return 1;
    }

    for (int i = 0; i < BENCH_WARMUP; i++) {
        r = img_api_hot_bench_step(&state);
        if (r != IMG_SUCCESS) {
            fprintf(stderr, "bench_lat: hot warmup failed: %s\n", img_result_name(r));
            free(samples);
            img_api_hot_bench_destroy(engine, &state);
            return 1;
        }
    }

    for (int i = 0; i < BENCH_ITERATIONS; i++) {
        const uint64_t start_ns = monotonic_ns();
        r = img_api_hot_bench_step(&state);
        const uint64_t end_ns = monotonic_ns();
        if (r != IMG_SUCCESS) {
            fprintf(stderr, "bench_lat: hot iteration %d failed: %s\n", i, img_result_name(r));
            free(samples);
            img_api_hot_bench_destroy(engine, &state);
            return 1;
        }

        samples[i] = end_ns - start_ns;
    }

    if (preset_template != IMG_JOB_TEMPLATE_CUSTOM) {
        printf("  preset:   %s\n", img_job_template_name(preset_template));
        fflush(stdout);
    }

    print_stats("Prepared render stage (decoded input, final cache disabled)", samples,
                BENCH_ITERATIONS, stats_out);
    fflush(stdout);

    free(samples);
    img_api_hot_bench_destroy(engine, &state);
    return 0;
}

static int benchmark_hot_rgb24(img_engine_t *engine, const uint8_t *input, uint32_t width,
                               uint32_t height, uint32_t stride, img_job_template_t preset_template,
                               bench_stats_t *stats_out) {
    img_hot_bench_state_t state;
    uint64_t *samples = calloc(BENCH_ITERATIONS, sizeof(*samples));
    if (!samples) {
        fprintf(stderr, "bench_lat: raw hot benchmark allocation failed\n");
        return 1;
    }

    img_result_t r = img_api_hot_bench_init_rgb24_with_template(engine, input, width, height,
                                                                stride, preset_template, &state);
    if (r != IMG_SUCCESS) {
        fprintf(stderr, "bench_lat: raw hot setup failed: %s\n", img_result_name(r));
        free(samples);
        return 1;
    }

    for (int i = 0; i < BENCH_WARMUP; i++) {
        r = img_api_hot_bench_step(&state);
        if (r != IMG_SUCCESS) {
            fprintf(stderr, "bench_lat: raw hot warmup failed: %s\n", img_result_name(r));
            free(samples);
            img_api_hot_bench_destroy(engine, &state);
            return 1;
        }
    }

    for (int i = 0; i < BENCH_ITERATIONS; i++) {
        const uint64_t start_ns = monotonic_ns();
        r = img_api_hot_bench_step(&state);
        const uint64_t end_ns = monotonic_ns();
        if (r != IMG_SUCCESS) {
            fprintf(stderr, "bench_lat: raw hot iteration %d failed: %s\n", i, img_result_name(r));
            free(samples);
            img_api_hot_bench_destroy(engine, &state);
            return 1;
        }

        samples[i] = end_ns - start_ns;
    }

    if (preset_template != IMG_JOB_TEMPLATE_CUSTOM) {
        printf("  preset:   %s\n", img_job_template_name(preset_template));
        fflush(stdout);
    }

    print_stats("Prepared render stage (raw ingress, final cache disabled)", samples,
                BENCH_ITERATIONS, stats_out);
    fflush(stdout);

    free(samples);
    img_api_hot_bench_destroy(engine, &state);
    return 0;
}

static int benchmark_decode_only(img_engine_t *engine, const uint8_t *input, size_t input_size,
                                 bench_stats_t *stats_out) {
    img_prepared_decoder_t decoder;
    uint64_t *samples = calloc(BENCH_ITERATIONS, sizeof(*samples));
    if (!samples) {
        fprintf(stderr, "bench_lat: decode benchmark allocation failed\n");
        return 1;
    }

    img_result_t r = img_api_prepare_decoder(engine, &decoder);
    if (r != IMG_SUCCESS) {
        fprintf(stderr, "bench_lat: decode prepare failed: %s\n", img_result_name(r));
        free(samples);
        return 1;
    }

    for (int i = 0; i < BENCH_WARMUP; i++) {
        img_buffer_t out = {0};
        r = img_api_decode_prepared(&decoder, input, input_size, &out);
        if (r != IMG_SUCCESS) {
            fprintf(stderr, "bench_lat: decode warmup failed: %s\n", img_result_name(r));
            free(samples);
            img_api_release_raw_buffer(engine, &out);
            img_api_decoder_destroy(&decoder);
            return 1;
        }
        img_api_release_raw_buffer(engine, &out);
    }

    for (int i = 0; i < BENCH_ITERATIONS; i++) {
        img_buffer_t out = {0};

        const uint64_t start_ns = monotonic_ns();
        r = img_api_decode_prepared(&decoder, input, input_size, &out);
        const uint64_t end_ns = monotonic_ns();

        if (r != IMG_SUCCESS) {
            fprintf(stderr, "bench_lat: decode iteration %d failed: %s\n", i, img_result_name(r));
            free(samples);
            img_api_release_raw_buffer(engine, &out);
            img_api_decoder_destroy(&decoder);
            return 1;
        }

        samples[i] = end_ns - start_ns;
        img_api_release_raw_buffer(engine, &out);
    }

    print_stats("Prepared decode ingress", samples, BENCH_ITERATIONS, stats_out);
    fflush(stdout);
    free(samples);
    img_api_decoder_destroy(&decoder);
    return 0;
}

static int benchmark_encode_only(img_engine_t *engine, const uint8_t *input, size_t input_size,
                                 bench_stats_t *stats_out) {
    img_buffer_t decoded = {0};
    uint64_t *samples = calloc(BENCH_ITERATIONS, sizeof(*samples));
    img_ctx_t ctx = {0};
    if (!samples)
        return 1;

    img_result_t r = decode_image_secure(engine, input, input_size, &decoded);
    if (r != IMG_SUCCESS) {
        fprintf(stderr, "bench_lat: encode benchmark decode setup failed: %s\n",
                img_result_name(r));
        free(samples);
        return 1;
    }

    img_ctx_bind_engine(engine, &ctx);

    for (int i = 0; i < BENCH_WARMUP; i++) {
        uint8_t *out = NULL;
        size_t out_size = 0;
        r = img_api_process_raw_encode(&ctx, &decoded, &out, &out_size);
        if (r != IMG_SUCCESS) {
            fprintf(stderr, "bench_lat: encode warmup failed: %s\n", img_result_name(r));
            free(samples);
            img_encoded_free(out);
            img_api_release_raw_buffer(engine, &decoded);
            return 1;
        }
        img_encoded_free(out);
    }

    for (int i = 0; i < BENCH_ITERATIONS; i++) {
        uint8_t *out = NULL;
        size_t out_size = 0;
        const uint64_t start_ns = monotonic_ns();
        r = img_api_process_raw_encode(&ctx, &decoded, &out, &out_size);
        const uint64_t end_ns = monotonic_ns();
        if (r != IMG_SUCCESS) {
            fprintf(stderr, "bench_lat: encode iteration %d failed: %s\n", i, img_result_name(r));
            free(samples);
            img_encoded_free(out);
            img_api_release_raw_buffer(engine, &decoded);
            return 1;
        }
        samples[i] = end_ns - start_ns;
        img_encoded_free(out);
    }

    print_stats("Prepared encode only (decoded source frame)", samples, BENCH_ITERATIONS,
                stats_out);
    fflush(stdout);
    free(samples);
    img_api_release_raw_buffer(engine, &decoded);
    return 0;
}

static int benchmark_encode_variant(img_engine_t *engine, const uint8_t *input, size_t input_size,
                                    int quality, int subsamp, int iterations, int warmup,
                                    bench_stats_t *stats_out) {
    img_buffer_t decoded = {0};
    uint64_t *samples = calloc((size_t)iterations, sizeof(*samples));
    img_ctx_t ctx = {0};
    char label[128];
    if (!samples)
        return 1;

    img_result_t r = decode_image_secure(engine, input, input_size, &decoded);
    if (r != IMG_SUCCESS) {
        fprintf(stderr, "bench_lat: encode variant decode setup failed: %s\n", img_result_name(r));
        free(samples);
        return 1;
    }

    img_ctx_bind_engine(engine, &ctx);

    for (int i = 0; i < warmup; i++) {
        uint8_t *out = NULL;
        size_t out_size = 0;
        if (img_encode_from_buffer_ex(&ctx, &decoded, &out, &out_size, quality, subsamp) != 0) {
            fprintf(stderr, "bench_lat: encode variant warmup failed (q=%d %s)\n", quality,
                    subsamp_name(subsamp));
            free(samples);
            img_encoded_free(out);
            img_api_release_raw_buffer(engine, &decoded);
            return 1;
        }
        img_encoded_free(out);
    }

    for (int i = 0; i < iterations; i++) {
        uint8_t *out = NULL;
        size_t out_size = 0;
        const uint64_t start_ns = monotonic_ns();
        if (img_encode_from_buffer_ex(&ctx, &decoded, &out, &out_size, quality, subsamp) != 0) {
            fprintf(stderr, "bench_lat: encode variant iteration failed (q=%d %s)\n", quality,
                    subsamp_name(subsamp));
            free(samples);
            img_encoded_free(out);
            img_api_release_raw_buffer(engine, &decoded);
            return 1;
        }
        const uint64_t end_ns = monotonic_ns();
        samples[i] = end_ns - start_ns;
        img_encoded_free(out);
    }

    snprintf(label, sizeof(label), "Encode sweep q=%d subsamp=%s (%d iters)", quality,
             subsamp_name(subsamp), iterations);
    print_stats(label, samples, (size_t)iterations, stats_out);
    fflush(stdout);

    free(samples);
    img_api_release_raw_buffer(engine, &decoded);
    return 0;
}

static int benchmark_encode_sweep(img_engine_t *engine, const uint8_t *input, size_t input_size) {
    struct {
        int quality;
        int subsamp;
    } variants[] = {
        {85, TJSAMP_444},
        {85, TJSAMP_420},
        {75, TJSAMP_420},
        {92, TJSAMP_444},
    };

    printf("\nEncode sweep (%d iterations + %d warmup each)\n", BENCH_SWEEP_ITERATIONS,
           BENCH_SWEEP_WARMUP);

    for (size_t i = 0; i < sizeof(variants) / sizeof(variants[0]); i++) {
        if (benchmark_encode_variant(engine, input, input_size, variants[i].quality,
                                     variants[i].subsamp, BENCH_SWEEP_ITERATIONS,
                                     BENCH_SWEEP_WARMUP, NULL) != 0)
            return 1;
    }

    return 0;
}

static int benchmark_rgb24_output(img_engine_t *engine, const uint8_t *input, size_t input_size,
                                  uint32_t width, uint32_t height, uint32_t stride,
                                  img_job_template_t preset_template, bench_stats_t *stats_out) {
    img_job_t job;
    uint64_t *samples = calloc(BENCH_ITERATIONS, sizeof(*samples));
    if (!samples)
        return 1;

    if (img_api_resolve_template_job(engine, preset_template, &job) != IMG_SUCCESS) {
        img_job_defaults(&job);
        img_job_apply_template(&job, preset_template);
    }

    for (int i = 0; i < BENCH_WARMUP; i++) {
        uint8_t *out = NULL;
        size_t out_size = 0;
        img_result_t r = img_api_run_job_rgb24_raw(engine, input, input_size, width, height, stride,
                                                   &job, &out, &out_size);
        if (r != IMG_SUCCESS) {
            fprintf(stderr, "bench_lat: raw-rgb24 warmup failed: %s\n", img_result_name(r));
            free(samples);
            img_encoded_free(out);
            return 1;
        }
        img_encoded_free(out);
    }

    for (int i = 0; i < BENCH_ITERATIONS; i++) {
        uint8_t *out = NULL;
        size_t out_size = 0;

        const uint64_t start_ns = monotonic_ns();
        img_result_t r = img_api_run_job_rgb24_raw(engine, input, input_size, width, height, stride,
                                                   &job, &out, &out_size);
        const uint64_t end_ns = monotonic_ns();

        if (r != IMG_SUCCESS) {
            fprintf(stderr, "bench_lat: raw-rgb24 iteration %d failed: %s\n", i,
                    img_result_name(r));
            free(samples);
            img_encoded_free(out);
            return 1;
        }

        samples[i] = end_ns - start_ns;
        img_encoded_free(out);
    }

    print_stats("Cold path (raw ingress + render + encode)", samples, BENCH_ITERATIONS, stats_out);
    free(samples);
    return 0;
}

int main(int argc, char **argv) {
    static struct option long_opts[] = {
        {"preset", required_argument, 0, 'p'},
        {"workers", required_argument, 0, 'w'},
        {"input-format", required_argument, 0, 'f'},
        {"input-width", required_argument, 0, 1},
        {"input-height", required_argument, 0, 2},
        {"input-stride", required_argument, 0, 3},
        {"encode-sweep", no_argument, 0, 4},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0},
    };

    img_job_template_t preset_template = IMG_JOB_TEMPLATE_CUSTOM;
    bench_input_format_t input_format = BENCH_INPUT_FORMAT_ENCODED;
    uint32_t input_width = 0;
    uint32_t input_height = 0;
    uint32_t input_stride = 0;
    uint32_t workers = 1;
    bool has_input_width = false;
    bool has_input_height = false;
    bool has_input_stride = false;
    bool run_encode_sweep = false;
    int opt;
    int idx = 0;

    optind = 1;
    while ((opt = getopt_long(argc, argv, "p:f:h", long_opts, &idx)) != -1) {
        switch (opt) {
        case 'p':
            if (img_job_template_lookup(optarg, &preset_template) != 0) {
                fprintf(stderr, "bench_lat: unknown preset '%s' (use %s, %s, %s)\n", optarg,
                        img_job_template_name(IMG_JOB_TEMPLATE_PASSPORT_45X35),
                        img_job_template_name(IMG_JOB_TEMPLATE_PASSPORT_38X35),
                        img_job_template_name(IMG_JOB_TEMPLATE_PRINTREADY_6X6));
                return 1;
            }
            break;
        case 'w':
            if (parse_u32_arg(optarg, &workers, "workers") != 0)
                return 1;
            break;
        case 'f':
            if (strcmp(optarg, "encoded") == 0)
                input_format = BENCH_INPUT_FORMAT_ENCODED;
            else if (strcmp(optarg, "raw-rgb24") == 0)
                input_format = BENCH_INPUT_FORMAT_RAW_RGB24;
            else {
                fprintf(stderr, "bench_lat: unsupported input format '%s'\n", optarg);
                return 1;
            }
            break;
        case 1:
            if (parse_u32_arg(optarg, &input_width, "input-width") != 0)
                return 1;
            has_input_width = true;
            break;
        case 2:
            if (parse_u32_arg(optarg, &input_height, "input-height") != 0)
                return 1;
            has_input_height = true;
            break;
        case 3:
            if (parse_u32_arg(optarg, &input_stride, "input-stride") != 0)
                return 1;
            has_input_stride = true;
            break;
        case 4:
            run_encode_sweep = true;
            break;
        case 'h':
            printf("Usage: %s [--preset <name>] [--workers <n>] [--input-format encoded|raw-rgb24] "
                   "[--input-width <px>] [--input-height <px>] [--input-stride <bytes>] "
                   "[--encode-sweep] [file]\n",
                   argv[0]);
            return 0;
        default:
            fprintf(stderr, "bench_lat: invalid arguments\n");
            return 1;
        }
    }

    const char *path = "tests/samples/4k_test.jpg";
    if (optind < argc) {
        path = argv[optind++];
        if (optind < argc) {
            fprintf(stderr, "bench_lat: unexpected extra arguments\n");
            return 1;
        }
    }

    if (input_format == BENCH_INPUT_FORMAT_RAW_RGB24) {
        uint64_t min_stride;

        if (!has_input_width || !has_input_height) {
            fprintf(stderr, "bench_lat: raw-rgb24 requires --input-width and --input-height\n");
            return 1;
        }

        min_stride = (uint64_t)input_width * 3ull;
        if (!has_input_stride)
            input_stride = (uint32_t)min_stride;
        else if ((uint64_t)input_stride < min_stride) {
            fprintf(stderr, "bench_lat: --input-stride must be at least width * 3 for raw-rgb24\n");
            return 1;
        }
    } else if (has_input_width || has_input_height || has_input_stride) {
        fprintf(stderr, "bench_lat: raw dimension arguments require --input-format raw-rgb24\n");
        return 1;
    }

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr,
                "bench_lat: cannot open '%s' — "
                "create tests/samples/4k_test.jpg or pass path as arg\n",
                path);
        return 1;
    }

    off_t file_size = lseek(fd, 0, SEEK_END);
    if (file_size <= 0) {
        fprintf(stderr, "bench_lat: invalid file size %lld\n", (long long)file_size);
        close(fd);
        return 1;
    }
    lseek(fd, 0, SEEK_SET);

    uint8_t *input = mmap(NULL, (size_t)file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    if (input == MAP_FAILED) {
        perror("bench_lat: mmap failed");
        return 1;
    }

    img_engine_t *engine = img_api_init(workers);
    if (!engine) {
        fprintf(stderr, "bench_lat: engine init failed\n");
        munmap(input, (size_t)file_size);
        return 1;
    }

    printf("\n=== imgengine latency benchmark ===\n");
    printf("  file:       %s (%lld bytes)\n", path, (long long)file_size);
    printf("  format:     %s\n", bench_input_format_name(input_format));
    printf("  workers:    %u\n", workers);
    if (input_format == BENCH_INPUT_FORMAT_RAW_RGB24)
        printf("  frame:      %ux%u stride=%u\n", input_width, input_height, input_stride);
    printf("  iterations: %d (+ %d warmup)\n", BENCH_ITERATIONS, BENCH_WARMUP);
    print_simd_profile(engine);
    print_benchmark_note(input_format);
    fflush(stdout);

    int rc = 0;
    bench_stats_t render_stats = {0};
    bench_stats_t decode_stats = {0};
    bench_stats_t encode_stats = {0};
    bench_stats_t cold_stats = {0};
    if (input_format == BENCH_INPUT_FORMAT_RAW_RGB24) {
        if (benchmark_hot_rgb24(engine, input, input_width, input_height, input_stride,
                                preset_template, &render_stats) != 0)
            rc = 1;
        else {
            printf("Prepared decode ingress\n");
            printf("  skipped: raw-rgb24 input bypasses decoder\n");
            printf("Prepared encode only\n");
            printf("  skipped: raw-rgb24 mode in this benchmark does not build a separate "
                   "encode-only stage\n");
            printf("\n--- entering cold path (raw ingress + encode) ---\n");
            fflush(stdout);
            if (benchmark_rgb24_output(engine, input, (size_t)file_size, input_width, input_height,
                                       input_stride, preset_template, &cold_stats) != 0)
                rc = 1;
        }
    } else if (benchmark_hot(engine, input, (size_t)file_size, preset_template, &render_stats) != 0)
        rc = 1;
    else if (benchmark_decode_only(engine, input, (size_t)file_size, &decode_stats) != 0)
        rc = 1;
    else if (benchmark_encode_only(engine, input, (size_t)file_size, &encode_stats) != 0)
        rc = 1;
    else {
        printf("\n--- entering cold path (decode + encode) ---\n");
        fflush(stdout);
        if (benchmark_raw(engine, input, (size_t)file_size, &cold_stats) != 0)
            rc = 1;
    }

    if (!rc)
        print_rfc_scorecard(input_format, &render_stats, &decode_stats, &encode_stats, &cold_stats);

    if (!rc && run_encode_sweep && input_format == BENCH_INPUT_FORMAT_ENCODED)
        rc = benchmark_encode_sweep(engine, input, (size_t)file_size);

    printf("===================================\n\n");

    img_api_shutdown(engine);
    munmap(input, (size_t)file_size);
    return rc;
}
