// ./src/cmd/bench/cli_bench_deleted.c
#define _GNU_SOURCE

#include "api/api_process_fast_internal.h"
#include "api/v1/img_api.h"

#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

typedef struct {
    int index;
    bool success;
    double seconds;
    char *stderr_text;
} bench_result_t;

typedef struct {
    char binary_path[PATH_MAX];
    char input_file[PATH_MAX];
    char output_dir[PATH_MAX];
    char prepared_input_file[PATH_MAX];
    char raw_width_text[32];
    char raw_height_text[32];
    char raw_stride_text[32];
    int runs;
    int workers;
    bool prepare_raw_rgb24;
    char **extra_args;
    int extra_argc;
    char **command_args;
    int command_argc;
} bench_config_t;

typedef struct {
    bench_config_t *config;
    bench_result_t *results;
    int next_index;
    pthread_mutex_t index_mutex;
} bench_context_t;

static int detect_worker_count(void) {
    long value = sysconf(_SC_NPROCESSORS_ONLN);
    if (value < 1 || value > INT32_MAX)
        return 1;
    return (int)value;
}

static double monotonic_seconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + ((double)ts.tv_nsec / 1000000000.0);
}

static void print_usage(const char *prog) {
    printf("Usage: %s [options] [-- extra imgengine_cli args...]\n", prog);
    printf("\n");
    printf("Options:\n");
    printf("  --binary <path>       CLI binary path\n");
    printf("  --input <path>        Input image path\n");
    printf("  --output-dir <path>   Output directory\n");
    printf("  --runs <n>            Number of jobs (default: 1000)\n");
    printf("  --workers <n>         Number of worker threads (default: auto-detected)\n");
    printf("  --prepare-raw-rgb24   Decode input once, then benchmark CLI raw-rgb24 path\n");
    printf("  --help                Show this help\n");
    printf("\n");
    printf("Defaults mirror imgengine/scripts/bench_mark.py.\n");
}

static int parse_int_arg(const char *text, const char *name) {
    char *end = NULL;
    long value;

    errno = 0;
    value = strtol(text, &end, 10);
    if (errno != 0 || end == text || (end && *end != '\0') || value <= 0 || value > INT32_MAX) {
        fprintf(stderr, "imgengine_cli_bench: invalid %s '%s'\n", name, text ? text : "");
        return -1;
    }

    return (int)value;
}

static int ensure_directory_recursive(const char *path) {
    char tmp[PATH_MAX];
    size_t len;

    if (!path || !path[0])
        return -1;

    len = strlen(path);
    if (len >= sizeof(tmp))
        return -1;

    memcpy(tmp, path, len + 1);

    if (tmp[len - 1] == '/')
        tmp[len - 1] = '\0';

    for (char *p = tmp + 1; *p; ++p) {
        if (*p != '/')
            continue;

        *p = '\0';
        if (mkdir(tmp, 0755) != 0 && errno != EEXIST)
            return -1;
        *p = '/';
    }

    if (mkdir(tmp, 0755) != 0 && errno != EEXIST)
        return -1;

    return 0;
}

static int build_output_path(char *out, size_t out_size, const char *dir, int index) {
    int written = snprintf(out, out_size, "%s/photo_%d.jpg", dir, index);
    if (written < 0 || (size_t)written >= out_size)
        return -1;
    return 0;
}

static char *read_text_file(const char *path) {
    FILE *fp;
    long size;
    size_t read_size;
    char *buffer;

    fp = fopen(path, "rb");
    if (!fp)
        return NULL;

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return NULL;
    }

    size = ftell(fp);
    if (size < 0) {
        fclose(fp);
        return NULL;
    }

    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return NULL;
    }

    buffer = calloc((size_t)size + 1u, 1u);
    if (!buffer) {
        fclose(fp);
        return NULL;
    }

    read_size = fread(buffer, 1, (size_t)size, fp);
    fclose(fp);

    if (read_size != (size_t)size) {
        free(buffer);
        return NULL;
    }

    return buffer;
}

static char *copy_string_or_fallback(const char *text) {
    const char *fallback = "unknown error";
    size_t len = strlen(text ? text : fallback);
    char *copy = malloc(len + 1u);

    if (!copy)
        return NULL;

    memcpy(copy, text ? text : fallback, len + 1u);
    return copy;
}

static uint8_t *read_binary_file(const char *path, size_t *size_out) {
    FILE *fp = fopen(path, "rb");
    long size;
    uint8_t *buffer;
    size_t read_size;

    if (!fp)
        return NULL;

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return NULL;
    }

    size = ftell(fp);
    if (size <= 0) {
        fclose(fp);
        return NULL;
    }

    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return NULL;
    }

    buffer = malloc((size_t)size);
    if (!buffer) {
        fclose(fp);
        return NULL;
    }

    read_size = fread(buffer, 1, (size_t)size, fp);
    fclose(fp);
    if (read_size != (size_t)size) {
        free(buffer);
        return NULL;
    }

    *size_out = (size_t)size;
    return buffer;
}

static int write_binary_file(const char *path, const uint8_t *data, size_t size) {
    FILE *fp = fopen(path, "wb");
    if (!fp)
        return -1;

    if (fwrite(data, 1, size, fp) != size) {
        fclose(fp);
        return -1;
    }

    return fclose(fp) == 0 ? 0 : -1;
}

static int prepare_raw_rgb24_input(bench_config_t *config) {
    size_t encoded_size = 0;
    uint8_t *encoded = read_binary_file(config->input_file, &encoded_size);
    img_buffer_t decoded = {0};
    img_engine_t *engine = NULL;
    char temp_template[] = "/tmp/imgengine_cli_bench_raw_XXXXXX";
    int temp_fd = -1;
    char **combined_args = NULL;

    if (!encoded) {
        fprintf(stderr, "imgengine_cli_bench: failed to read input for raw preparation\n");
        return -1;
    }

    engine = img_api_init(1);
    if (!engine) {
        free(encoded);
        fprintf(stderr, "imgengine_cli_bench: failed to init engine for raw preparation\n");
        return -1;
    }

    if (img_api_process_fast_decode(engine, encoded, encoded_size, &decoded) != IMG_SUCCESS) {
        img_api_shutdown(engine);
        free(encoded);
        fprintf(stderr, "imgengine_cli_bench: failed to decode input for raw preparation\n");
        return -1;
    }

    temp_fd = mkstemp(temp_template);
    if (temp_fd < 0) {
        img_api_release_raw_buffer(engine, &decoded);
        img_api_shutdown(engine);
        free(encoded);
        fprintf(stderr, "imgengine_cli_bench: failed to create temp raw input file\n");
        return -1;
    }
    close(temp_fd);

    if (write_binary_file(temp_template, decoded.data,
                          (size_t)decoded.stride * (size_t)decoded.height) != 0) {
        unlink(temp_template);
        img_api_release_raw_buffer(engine, &decoded);
        img_api_shutdown(engine);
        free(encoded);
        fprintf(stderr, "imgengine_cli_bench: failed to write temp raw input file\n");
        return -1;
    }

    snprintf(config->prepared_input_file, sizeof(config->prepared_input_file), "%s", temp_template);
    snprintf(config->raw_width_text, sizeof(config->raw_width_text), "%u", decoded.width);
    snprintf(config->raw_height_text, sizeof(config->raw_height_text), "%u", decoded.height);
    snprintf(config->raw_stride_text, sizeof(config->raw_stride_text), "%u", decoded.stride);

    combined_args = calloc((size_t)(8 + config->extra_argc), sizeof(char *));
    if (!combined_args) {
        unlink(config->prepared_input_file);
        img_api_release_raw_buffer(engine, &decoded);
        img_api_shutdown(engine);
        free(encoded);
        fprintf(stderr, "imgengine_cli_bench: failed to allocate raw CLI args\n");
        return -1;
    }

    combined_args[0] = "--input-format";
    combined_args[1] = "raw-rgb24";
    combined_args[2] = "--input-width";
    combined_args[3] = config->raw_width_text;
    combined_args[4] = "--input-height";
    combined_args[5] = config->raw_height_text;
    combined_args[6] = "--input-stride";
    combined_args[7] = config->raw_stride_text;
    for (int i = 0; i < config->extra_argc; ++i)
        combined_args[8 + i] = config->extra_args[i];

    snprintf(config->input_file, sizeof(config->input_file), "%s", config->prepared_input_file);
    config->command_args = combined_args;
    config->command_argc = 8 + config->extra_argc;

    img_api_release_raw_buffer(engine, &decoded);
    img_api_shutdown(engine);
    free(encoded);
    return 0;
}

static int run_once(const bench_config_t *config, int index, bench_result_t *result) {
    char output_file[PATH_MAX];
    char stderr_template[] = "/tmp/imgengine_cli_bench_stderr_XXXXXX";
    char *stderr_text = NULL;
    char **argv = NULL;
    int stderr_fd = -1;
    pid_t pid;
    int status = 0;
    int arg_count;
    int argi = 0;
    double start_time;
    double end_time;

    if (build_output_path(output_file, sizeof(output_file), config->output_dir, index) != 0) {
        result->index = index;
        result->success = false;
        result->seconds = 0.0;
        result->stderr_text = copy_string_or_fallback("failed to build output path");
        return -1;
    }

    stderr_fd = mkstemp(stderr_template);
    if (stderr_fd < 0) {
        result->index = index;
        result->success = false;
        result->seconds = 0.0;
        result->stderr_text = copy_string_or_fallback("failed to create temporary stderr file");
        return -1;
    }

    arg_count = 1 + 4 + config->command_argc + 1;
    argv = calloc((size_t)arg_count, sizeof(char *));
    if (!argv) {
        close(stderr_fd);
        unlink(stderr_template);
        result->index = index;
        result->success = false;
        result->seconds = 0.0;
        result->stderr_text = copy_string_or_fallback("failed to allocate argv");
        return -1;
    }

    argv[argi++] = (char *)config->binary_path;
    argv[argi++] = "--input";
    argv[argi++] = (char *)config->input_file;
    argv[argi++] = "--output";
    argv[argi++] = output_file;

    for (int i = 0; i < config->command_argc; ++i)
        argv[argi++] = config->command_args[i];

    argv[argi] = NULL;

    start_time = monotonic_seconds();
    pid = fork();
    if (pid == 0) {
        int null_fd = open("/dev/null", O_WRONLY);
        if (null_fd >= 0) {
            dup2(null_fd, STDOUT_FILENO);
            close(null_fd);
        }

        dup2(stderr_fd, STDERR_FILENO);
        close(stderr_fd);

        execv(config->binary_path, argv);
        dprintf(STDERR_FILENO, "execv failed: %s\n", strerror(errno));
        _exit(127);
    }

    if (pid < 0) {
        close(stderr_fd);
        unlink(stderr_template);
        free(argv);
        result->index = index;
        result->success = false;
        result->seconds = 0.0;
        result->stderr_text = copy_string_or_fallback(strerror(errno));
        return -1;
    }

    close(stderr_fd);

    while (waitpid(pid, &status, 0) < 0) {
        if (errno != EINTR)
            break;
    }

    end_time = monotonic_seconds();
    stderr_text = read_text_file(stderr_template);
    unlink(stderr_template);

    result->index = index;
    result->seconds = end_time - start_time;
    result->success = WIFEXITED(status) && WEXITSTATUS(status) == 0;
    result->stderr_text = stderr_text ? stderr_text : copy_string_or_fallback("");

    free(argv);
    return result->success ? 0 : -1;
}

static void *worker_main(void *opaque) {
    bench_context_t *ctx = (bench_context_t *)opaque;

    for (;;) {
        int index;

        pthread_mutex_lock(&ctx->index_mutex);
        index = ctx->next_index++;
        pthread_mutex_unlock(&ctx->index_mutex);

        if (index > ctx->config->runs)
            break;

        run_once(ctx->config, index, &ctx->results[index - 1]);

        if (ctx->results[index - 1].success)
            printf("[OK] %d | %.4fs\n", index, ctx->results[index - 1].seconds);
        else
            printf("[ERR] %d | %s\n", index,
                   ctx->results[index - 1].stderr_text ? ctx->results[index - 1].stderr_text : "");

        fflush(stdout);
    }

    return NULL;
}

static void report_results(const bench_result_t *results, int count, double total_seconds) {
    int success = 0;
    int failed = 0;
    double success_seconds = 0.0;
    double avg_seconds = 0.0;
    double throughput = 0.0;

    for (int i = 0; i < count; ++i) {
        if (results[i].success) {
            success++;
            success_seconds += results[i].seconds;
        } else {
            failed++;
        }
    }

    if (success > 0)
        avg_seconds = success_seconds / (double)success;

    if (total_seconds > 0.0)
        throughput = (double)success / total_seconds;

    printf("\n==================================================\n");
    printf("BENCHMARK RESULT\n");
    printf("==================================================\n");
    printf("Total Runs     : %d\n", count);
    printf("Success        : %d\n", success);
    printf("Failed         : %d\n", failed);
    printf("Total Time     : %.4fs\n", total_seconds);
    printf("Avg Time       : %.4fs\n", avg_seconds);
    printf("Throughput     : %.2f ops/sec\n", throughput);
    printf("==================================================\n");
}

static int derive_default_base_dir(const char *argv0, char *out, size_t out_size) {
    char resolved[PATH_MAX];
    char dir_a[PATH_MAX];
    char dir_b[PATH_MAX];
    char *dir1;
    char *dir2;

    if (!realpath(argv0, resolved))
        return -1;

    snprintf(dir_a, sizeof(dir_a), "%s", resolved);
    dir1 = dirname(dir_a);

    snprintf(dir_b, sizeof(dir_b), "%s", dir1);
    dir2 = dirname(dir_b);

    if (snprintf(out, out_size, "%s", dir2) >= (int)out_size)
        return -1;

    return 0;
}

static void set_default_extra_args(bench_config_t *config) {
    static char *defaults[] = {
        "--cols",  "6",  "--rows",      "3",  "--gap",         "15", "--padding",        "20",
        "--bleed", "10", "--crop-mark", "25", "--crop-offset", "8",  "--crop-thickness", "2",
    };

    config->extra_args = defaults;
    config->extra_argc = (int)(sizeof(defaults) / sizeof(defaults[0]));
    config->command_args = config->extra_args;
    config->command_argc = config->extra_argc;
}

static int init_config_defaults(bench_config_t *config, const char *argv0) {
    char base_dir[PATH_MAX];

    memset(config, 0, sizeof(*config));
    config->runs = 1000;
    config->workers = detect_worker_count();

    if (derive_default_base_dir(argv0, base_dir, sizeof(base_dir)) != 0) {
        fprintf(stderr, "imgengine_cli_bench: failed to derive base directory from '%s'\n", argv0);
        return -1;
    }

    if (snprintf(config->binary_path, sizeof(config->binary_path), "%s/build/imgengine_cli",
                 base_dir) >= (int)sizeof(config->binary_path) ||
        snprintf(config->input_file, sizeof(config->input_file), "%s/photo.jpg", base_dir) >=
            (int)sizeof(config->input_file) ||
        snprintf(config->output_dir, sizeof(config->output_dir), "%s/outputs", base_dir) >=
            (int)sizeof(config->output_dir)) {
        fprintf(stderr, "imgengine_cli_bench: failed to initialize default paths\n");
        return -1;
    }

    set_default_extra_args(config);
    return 0;
}

static int parse_args(int argc, char **argv, bench_config_t *config) {
    int i = 1;

    while (i < argc) {
        if (strcmp(argv[i], "--") == 0) {
            config->extra_args = &argv[i + 1];
            config->extra_argc = argc - (i + 1);
            config->command_args = config->extra_args;
            config->command_argc = config->extra_argc;
            return 0;
        }
        if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 1;
        }
        if (strcmp(argv[i], "--prepare-raw-rgb24") == 0) {
            config->prepare_raw_rgb24 = true;
            i += 1;
            continue;
        }
        if ((strcmp(argv[i], "--binary") == 0 || strcmp(argv[i], "--input") == 0 ||
             strcmp(argv[i], "--output-dir") == 0 || strcmp(argv[i], "--runs") == 0 ||
             strcmp(argv[i], "--workers") == 0) &&
            (i + 1) >= argc) {
            fprintf(stderr, "imgengine_cli_bench: missing value for %s\n", argv[i]);
            return -1;
        }

        if (strcmp(argv[i], "--binary") == 0) {
            snprintf(config->binary_path, sizeof(config->binary_path), "%s", argv[i + 1]);
            i += 2;
            continue;
        }
        if (strcmp(argv[i], "--input") == 0) {
            snprintf(config->input_file, sizeof(config->input_file), "%s", argv[i + 1]);
            i += 2;
            continue;
        }
        if (strcmp(argv[i], "--output-dir") == 0) {
            snprintf(config->output_dir, sizeof(config->output_dir), "%s", argv[i + 1]);
            i += 2;
            continue;
        }
        if (strcmp(argv[i], "--runs") == 0) {
            config->runs = parse_int_arg(argv[i + 1], "runs");
            if (config->runs <= 0)
                return -1;
            i += 2;
            continue;
        }
        if (strcmp(argv[i], "--workers") == 0) {
            config->workers = parse_int_arg(argv[i + 1], "workers");
            if (config->workers <= 0)
                return -1;
            i += 2;
            continue;
        }

        fprintf(stderr, "imgengine_cli_bench: unknown argument '%s'\n", argv[i]);
        return -1;
    }

    return 0;
}

int main(int argc, char **argv) {
    bench_config_t config;
    bench_context_t ctx;
    bench_result_t *results = NULL;
    pthread_t *threads = NULL;
    double total_start;
    double total_end;
    int parse_rc;
    int exit_code = 0;

    if (init_config_defaults(&config, argv[0]) != 0)
        return 1;

    parse_rc = parse_args(argc, argv, &config);
    if (parse_rc != 0)
        return parse_rc < 0 ? 1 : 0;

    if (config.prepare_raw_rgb24) {
        if (prepare_raw_rgb24_input(&config) != 0)
            return 1;
    }

    if (ensure_directory_recursive(config.output_dir) != 0) {
        fprintf(stderr, "imgengine_cli_bench: failed to create output directory '%s'\n",
                config.output_dir);
        return 1;
    }

    results = calloc((size_t)config.runs, sizeof(*results));
    threads = calloc((size_t)config.workers, sizeof(*threads));
    if (!results || !threads) {
        fprintf(stderr, "imgengine_cli_bench: allocation failed\n");
        free(results);
        free(threads);
        return 1;
    }

    memset(&ctx, 0, sizeof(ctx));
    ctx.config = &config;
    ctx.results = results;
    ctx.next_index = 1;
    pthread_mutex_init(&ctx.index_mutex, NULL);

    printf("Running %d jobs with %d workers...\n\n", config.runs, config.workers);
    printf("binary:     %s\n", config.binary_path);
    printf("input:      %s\n", config.input_file);
    printf("output dir: %s\n\n", config.output_dir);
    if (config.prepare_raw_rgb24) {
        printf("mode:       raw-rgb24 decode-bypass CLI benchmark\n");
        printf("raw frame:  %sx%s stride=%s\n\n", config.raw_width_text, config.raw_height_text,
               config.raw_stride_text);
    }
    fflush(stdout);

    total_start = monotonic_seconds();

    for (int i = 0; i < config.workers; ++i) {
        if (pthread_create(&threads[i], NULL, worker_main, &ctx) != 0) {
            fprintf(stderr, "imgengine_cli_bench: failed to create worker thread\n");
            config.workers = i;
            exit_code = 1;
            break;
        }
    }

    for (int i = 0; i < config.workers; ++i)
        pthread_join(threads[i], NULL);

    total_end = monotonic_seconds();
    report_results(results, config.runs, total_end - total_start);

    for (int i = 0; i < config.runs; ++i)
        free(results[i].stderr_text);

    pthread_mutex_destroy(&ctx.index_mutex);
    if (config.prepare_raw_rgb24 && config.prepared_input_file[0] != '\0')
        unlink(config.prepared_input_file);
    if (config.command_args && config.command_args != config.extra_args)
        free(config.command_args);
    free(results);
    free(threads);
    return exit_code;
}
