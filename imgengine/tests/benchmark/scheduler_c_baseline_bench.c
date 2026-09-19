#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <time.h>

#include "imgengine/api/v1/imgengine.h"

typedef struct {
    const uint8_t *input;
    size_t input_size;
    imgengine_engine_t *engine;
    imgengine_status_t status;
    size_t output_size;
    uint64_t elapsed_ns;
} benchmark_task_t;

typedef struct {
    uint64_t p50_ns;
    uint64_t p95_ns;
    uint64_t average_ns;
} latency_stats_t;

static uint64_t monotonic_ns(void) {
    struct timespec timestamp;
    if (clock_gettime(CLOCK_MONOTONIC, &timestamp) != 0)
        return 0;
    return (uint64_t)timestamp.tv_sec * UINT64_C(1000000000) + (uint64_t)timestamp.tv_nsec;
}

static int compare_u64(const void *left, const void *right) {
    const uint64_t a = *(const uint64_t *)left;
    const uint64_t b = *(const uint64_t *)right;
    return (a > b) - (a < b);
}

static latency_stats_t latency_stats(const uint64_t *samples, size_t count) {
    latency_stats_t stats = {0};
    uint64_t *sorted = NULL;
    uint64_t total = 0;

    if (!samples || count == 0)
        return stats;
    sorted = malloc(count * sizeof(*sorted));
    if (!sorted)
        return stats;
    memcpy(sorted, samples, count * sizeof(*sorted));
    qsort(sorted, count, sizeof(*sorted), compare_u64);
    for (size_t index = 0; index < count; ++index)
        total += samples[index];
    stats.average_ns = total / count;
    stats.p50_ns = sorted[(count - 1) / 2];
    stats.p95_ns = sorted[((count - 1) * 95 + 99) / 100];
    free(sorted);
    return stats;
}

static uint64_t timeval_ns(struct timeval value) {
    return (uint64_t)value.tv_sec * UINT64_C(1000000000) + (uint64_t)value.tv_usec * 1000u;
}

static int read_file(const char *path, uint8_t **data, size_t *size) {
    FILE *file = NULL;
    long file_size = 0;
    uint8_t *buffer = NULL;

    if (!path || !data || !size)
        return -1;
    *data = NULL;
    *size = 0;
    file = fopen(path, "rb");
    if (!file)
        return -1;
    if (fseek(file, 0, SEEK_END) != 0 || (file_size = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return -1;
    }
    buffer = malloc((size_t)file_size);
    if (!buffer || fread(buffer, 1, (size_t)file_size, file) != (size_t)file_size) {
        free(buffer);
        fclose(file);
        return -1;
    }
    fclose(file);
    *data = buffer;
    *size = (size_t)file_size;
    return 0;
}

static void run_task(benchmark_task_t *task) {
    imgengine_output_t output = {0};
    const uint64_t started = monotonic_ns();
    task->status = imgengine_process_encoded_image_to_jpeg(task->engine, task->input,
                                                           task->input_size, &output);
    task->elapsed_ns = monotonic_ns() - started;
    task->output_size = output.size;
    imgengine_output_release(&output);
}

static void *run_task_thread(void *argument) {
    run_task(argument);
    return NULL;
}

static int parse_positive(const char *text, size_t *value) {
    char *end = NULL;
    unsigned long parsed = 0;

    if (!text || !value)
        return -1;
    errno = 0;
    parsed = strtoul(text, &end, 10);
    if (errno != 0 || !end || *end != '\0' || parsed == 0)
        return -1;
    *value = (size_t)parsed;
    return 0;
}

int main(int argc, char **argv) {
    const char *input_path = NULL;
    const char *fixture = "unnamed";
    size_t iterations = 0;
    size_t warmup = 0;
    size_t concurrency = 1;
    uint8_t *input = NULL;
    size_t input_size = 0;
    imgengine_engine_t *engine = NULL;
    imgengine_engine_options_t options = {0};
    benchmark_task_t *tasks = NULL;
    pthread_t *threads = NULL;
    uint64_t *wall_samples = NULL;
    size_t successful = 0;
    size_t failed = 0;
    size_t cleanup_success = 0;
    uint64_t output_bytes = 0;
    struct rusage before = {0};
    struct rusage after = {0};
    uint64_t started = 0;
    uint64_t elapsed = 0;
    int exit_code = 1;

    for (int index = 1; index < argc; ++index) {
        if (strcmp(argv[index], "--input") == 0 && index + 1 < argc)
            input_path = argv[++index];
        else if (strcmp(argv[index], "--fixture") == 0 && index + 1 < argc)
            fixture = argv[++index];
        else if (strcmp(argv[index], "--iterations") == 0 && index + 1 < argc &&
                 parse_positive(argv[++index], &iterations) == 0)
            continue;
        else if (strcmp(argv[index], "--warmup") == 0 && index + 1 < argc) {
            char *end = NULL;
            warmup = (size_t)strtoul(argv[++index], &end, 10);
            if (!end || *end != '\0')
                goto cleanup;
        } else if (strcmp(argv[index], "--concurrency") == 0 && index + 1 < argc &&
                   parse_positive(argv[++index], &concurrency) == 0)
            continue;
        else {
            fprintf(stderr,
                    "usage: %s --input PATH --fixture NAME --iterations N --warmup N "
                    "[--concurrency N]\n",
                    argv[0]);
            goto cleanup;
        }
    }
    if (!input_path || iterations == 0)
        goto cleanup;
    if (read_file(input_path, &input, &input_size) != 0) {
        fprintf(stderr, "unable to read benchmark input\n");
        goto cleanup;
    }

    options.struct_size = sizeof(options);
    options.worker_count = 1;
    if (imgengine_engine_create(&options, &engine) != IMGENGINE_STATUS_OK) {
        fprintf(stderr, "unable to create C baseline engine\n");
        goto cleanup;
    }
    benchmark_task_t warmup_task = {.input = input, .input_size = input_size, .engine = engine};
    for (size_t index = 0; index < warmup; ++index)
        run_task(&warmup_task);

    tasks = calloc(concurrency, sizeof(*tasks));
    threads = calloc(concurrency, sizeof(*threads));
    wall_samples = calloc(iterations, sizeof(*wall_samples));
    if (!tasks || !threads || !wall_samples)
        goto cleanup;
    if (getrusage(RUSAGE_SELF, &before) != 0)
        goto cleanup;
    started = monotonic_ns();

    for (size_t offset = 0; offset < iterations;) {
        const size_t batch =
            (iterations - offset < concurrency) ? iterations - offset : concurrency;
        if (batch == 1) {
            tasks[0] =
                (benchmark_task_t){.input = input, .input_size = input_size, .engine = engine};
            run_task(&tasks[0]);
        } else {
            for (size_t index = 0; index < batch; ++index) {
                tasks[index] =
                    (benchmark_task_t){.input = input, .input_size = input_size, .engine = engine};
                if (pthread_create(&threads[index], NULL, run_task_thread, &tasks[index]) != 0) {
                    for (size_t joined = 0; joined < index; ++joined)
                        pthread_join(threads[joined], NULL);
                    goto cleanup;
                }
            }
            for (size_t index = 0; index < batch; ++index)
                pthread_join(threads[index], NULL);
        }
        for (size_t index = 0; index < batch; ++index) {
            benchmark_task_t *task = &tasks[index];
            if (task->status == IMGENGINE_STATUS_OK) {
                wall_samples[successful] = task->elapsed_ns;
                successful++;
                cleanup_success++;
                output_bytes += task->output_size;
            } else {
                failed++;
            }
        }
        offset += batch;
    }
    elapsed = monotonic_ns() - started;
    if (getrusage(RUSAGE_SELF, &after) != 0)
        goto cleanup;

    const latency_stats_t stats = latency_stats(wall_samples, successful);
    printf("{\"schema_version\":1,\"path\":\"c_abi_direct\",\"fixture\":\"%s\","
           "\"input_bytes\":%zu,\"iterations\":%zu,\"warmup\":%zu,\"concurrency\":%zu,"
           "\"queue_capacity\":0,\"queue_depth\":0,\"max_queue_depth\":0,"
           "\"queue_rejection_count\":0,\"successful_completions\":%zu,"
           "\"failed_completions\":%zu,\"cleanup_success_count\":%zu,\"cleanup_failure_count\":0,"
           "\"output_bytes_total\":%" PRIu64 ",\"wall_latency_p50_ns\":%" PRIu64 ","
           "\"wall_latency_p95_ns\":%" PRIu64 ",\"processing_latency_p50_ns\":null,"
           "\"processing_latency_p95_ns\":null,\"throughput_ops_per_sec\":%.6f,"
           "\"cpu_user_ns\":%" PRIu64 ",\"cpu_system_ns\":%" PRIu64 ","
           "\"peak_rss_kib\":%ld}\n",
           fixture, input_size, iterations, warmup, concurrency, successful, failed,
           cleanup_success, output_bytes, stats.p50_ns, stats.p95_ns,
           elapsed ? (double)iterations * 1e9 / (double)elapsed : 0.0,
           timeval_ns(after.ru_utime) - timeval_ns(before.ru_utime),
           timeval_ns(after.ru_stime) - timeval_ns(before.ru_stime), after.ru_maxrss);
    exit_code = 0;

cleanup:
    imgengine_engine_destroy(engine);
    free(wall_samples);
    free(threads);
    free(tasks);
    free(input);
    return exit_code;
}
