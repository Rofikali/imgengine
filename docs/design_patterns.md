# Design Patterns — Kernel-grade Guidance ( GoF (Gang of Four) patterns )

This document provides concise pattern recommendations, short C sketches, and concrete places in the repository to apply them. These patterns aim to meet the hard rules in the HLD (no heap in hot path, deterministic hot kernels, stable init order, zero-copy contracts).

1) Strategy — runtime algorithm selection

- Problem: multiple interchangeable kernels (AVX2/AVX512/scalar) and algorithm variations.
- Why: allows selecting the best implementation for CPU caps and image shape without branchy hot paths.

Sketch (select at startup; hot path uses direct table lookup):

```c
// img_types.h (docs):
typedef void (*img_kernel_fn)(img_ctx_t*, img_buffer_t*, void*);
extern img_kernel_fn g_jump_table[IMG_MAX_OPS];

// selection (startup)
void img_jump_table_init(cpu_caps_t caps) {
    memset(g_jump_table, 0, sizeof(g_jump_table));
    if (caps & CPU_CAP_AVX2) {
        g_jump_table[OP_RESIZE] = resize_avx2;
    } else {
        g_jump_table[OP_RESIZE] = resize_scalar;
    }
}

// hot path (inlined / minimal):
// img_kernel_fn fn = g_jump_table[op_code];
// fn(ctx, buf, params);
```

Where to apply: `src/pipeline/jump_table.c`, `src/pipeline/hardware_registry.c`.

2) Factory — centralize complex object creation

- Problem: many variants of decoders/encoders or pipelines that require different initialization.
- Why: centralizes allocation/config and makes tests simpler; hides construction details.

Sketch:

```c
typedef struct img_decoder { int (*decode)(struct img_decoder*, const uint8_t*, size_t, img_buffer_t*); void (*destroy)(struct img_decoder*); void *priv; } img_decoder_t;

img_decoder_t *img_decoder_create(enum decoder_mode mode) {
    if (mode == DEC_MODE_STREAMING) return streaming_decoder_create();
    return bulk_decoder_create();
}
```

Where to apply: `src/io/decoder/*` (add `decoder_factory.c` and unit tests under `src/tests`).

3) Observer — low-overhead instrumentation hooks

- Problem: one-to-many updates for metrics, traces, or log sinks.
- Why: decouples observability from core logic; keep hot path cheap with on/off sampling.

Sketch:

```c
typedef void (*img_metrics_cb_t)(const img_metrics_t*);
void img_metrics_register(img_engine_t *e, img_metrics_cb_t cb);
static inline void img_metrics_notify(const img_metrics_t *m) {
    for (size_t i=0;i<metrics_cb_count;i++) metrics_cb[i](m);
}
```

Where to apply: `src/observability/*`, hook into `src/runtime` and `src/pipeline` at coarse-grained points.

4) Decorator — add behavior without changing kernel code

- Problem: add caching, validation, rate-limiting or tracing around kernels.
- Why: enables cross-cutting concerns without touching hot kernel source.

Sketch (wrapper registered at startup):

```c
typedef struct { img_kernel_fn core; void *ctx; } kernel_wrapper_t;

void kernel_cache_wrapper(img_ctx_t *ctx, img_buffer_t *buf, void *params) {
    kernel_wrapper_t *w = params; // or provided via pipeline metadata
    if (cache_hit(ctx, buf)) return; // cheap check
    w->core(ctx, buf, w->ctx);
    cache_store(ctx, buf);
}

// register: img_register_op(OP_CUSTOM, kernel_cache_wrapper, wrapper_ctx);
```

Where to apply: `src/pipeline/generated.c`, `src/runtime/plugin_loader.c`.

5) Singleton (deterministic registries)

- Problem: global registries that must be initialized exactly once (jump tables, IO vtables).
- Why: deterministic startup order and safe shutdown avoid races and ABI mismatches.

Sketch:

```c
// ensure deterministic init in img_api_init()
void img_api_init_all(void) {
    img_slab_init(...);
    img_jump_table_init(detect_cpu_caps());
    img_io_register(...);
}
```

Where to apply: `src/api/api_init.c`, `src/pipeline/jump_table_register.c`, `src/io/io_register.c`.

6) Proxy — layered IO and caching

- Problem: remote IO, auth, retries and caching should not clutter decoder logic.
- Why: allows injecting caching, retries, auth at a single boundary.

Sketch:

```c
ssize_t proxy_decode(const uint8_t *in, size_t in_len, img_buffer_t *out, void *ctx) {
    if (cache_lookup(...)) return cached_len;
    ssize_t r = real_decoder->decode(real_decoder, in, in_len, out);
    if (r>0) cache_store(...);
    return r;
}
```

Where to apply: `src/io/io_register.c`, `src/io/decoder/*`.

7) Command / Pipeline (Chain-of-Responsibility)

- Problem: pipelines must be serializable, replayable, and batchable.
- Why: compact op representation simplifies queuing and batching.

Sketch:

```c
typedef struct { uint16_t op_code; uint16_t flags; uint32_t param_idx; } pipeline_op_t;
void execute_pipeline(img_pipeline_t *p, img_ctx_t *ctx) {
    for (size_t i=0;i<p->n_ops;i++) {
        pipeline_op_t *op = &p->ops[i];
        img_kernel_fn fn = g_jump_table[op->op_code];
        fn(ctx, &p->buf, get_params(op->param_idx));
    }
}
```

Where to apply: `src/pipeline/pipeline_executor.c`, `src/hot/pipeline_exec.c`.

8) Data-Oriented Design (DOD)

- Problem: branchy or scattered memory layouts reduce SIMD and cache effectiveness.
- Why: arranging data for sequential access increases throughput.

Sketch: prefer SoA for per-pixel operations.

```c
// instead of struct { uint8_t r,g,b; } pixels[N];
// use separate arrays for R,G,B channels for SIMD-friendly ops
float *R = malloc(n * sizeof(float));
float *G = malloc(n * sizeof(float));
float *B = malloc(n * sizeof(float));
```

Where to apply: `src/arch/*` kernels such as `resize_avx2`.

9) Lock-free messaging + Work-stealing

- Problem: synchronous global locks throttle throughput.
- Why: SPSC for hot submissions and a light-weight steal API give low-latency and balanced load.

Sketch: a simple SPSC ring buffer for per-worker queues (use atomic ops and release/acquire semantics).

Where to inspect: `src/runtime/scheduler_submit.c`, `src/runtime/worker_loop.c`, `src/runtime/scheduler.c`.

10) Versioned Plugin ABI

- Problem: plugins built against older headers may crash or silently misbehave.
- Why: explicit numeric ABI version prevents subtle runtime failures.

Sketch:

```c
typedef struct { uint32_t abi_version; const char *name; int (*register)(img_engine_t*); } img_plugin_desc_t;
if (desc->abi_version != IMG_PLUGIN_ABI_V1) return -EINVAL;
```

Where to apply: `src/runtime/plugin_loader.c`, `include/pipeline/plugin_abi.h`.

Testing & Microbench ideas

- Isolate decoder: implement `src/cmd/bench/decoder_bench.c` which exercises streaming vs bulk decoder paths, measure p50/p99 and throughput.
- Isolate encoder similarly.
- Add small unit harness under `src/tests/` to assert no heap allocations in instrumented hot paths (use custom malloc hooks or sanitized builds).

Quick commands (example):

```bash
mkdir -p build && cd build
cmake .. -DIMGENGINE_BENCH=ON -DCMAKE_BUILD_TYPE=Release
make -j
./bench_lat ../photo.jpg
```

Next actions recommended (pick order):

- Implement `decoder_factory.c` and a `Strategy` toggle for streaming vs bulk decoder, add `decoder_bench` and measure improvement.
- Add a small `Decorator` wrapper example around one kernel (cache or validation) to demonstrate pattern without touching kernel internals.
- Add CI microbench thresholds for decode/encode regressions.
