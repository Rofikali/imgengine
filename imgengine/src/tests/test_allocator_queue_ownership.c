#include "memory/arena.h"
#include "memory/slab.h"
#include "memory/slab_internal.h"
#include "runtime/queue_mpmc.h"
#include "runtime/queue_spsc.h"
#include "runtime/scheduler.h"
#include "runtime/worker.h"
#include "runtime/worker_internal.h"

#include "core/context_internal.h"

#include <pthread.h>
#include <sched.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum {
    SLAB_BLOCKS = 32,
    SLAB_THREADS = 4,
    SLAB_ITERATIONS = 2000,
    MPMC_PRODUCERS = 2,
    MPMC_CONSUMERS = 2,
    MPMC_PER_PRODUCER = 1000,
};

typedef struct {
    img_slab_pool_t *pool;
    atomic_bool *start;
    atomic_int *in_use;
    atomic_int *failures;
} slab_thread_context_t;

typedef struct {
    img_mpmc_queue_t *queue;
    atomic_bool *start;
    atomic_int *producers_done;
    atomic_int *completed;
    atomic_int *seen;
    atomic_int *failures;
    int producer_index;
} mpmc_thread_context_t;

#if defined(IMGENGINE_ALLOCATOR_FAILURE_TEST)
static atomic_int fail_malloc = 0;
static atomic_int fail_calloc = 0;
static atomic_int fail_calloc_after = -1;
static atomic_int fail_aligned_alloc = 0;

void *__real_malloc(size_t size);
void *__real_calloc(size_t count, size_t size);
void *__real_aligned_alloc(size_t alignment, size_t size);

void *__wrap_malloc(size_t size) {
    if (atomic_exchange_explicit(&fail_malloc, 0, memory_order_acq_rel))
        return NULL;
    return __real_malloc(size);
}

void *__wrap_calloc(size_t count, size_t size) {
    if (atomic_exchange_explicit(&fail_calloc, 0, memory_order_acq_rel))
        return NULL;
    int remaining = atomic_load_explicit(&fail_calloc_after, memory_order_acquire);
    if (remaining >= 0 &&
        atomic_compare_exchange_strong_explicit(&fail_calloc_after, &remaining, remaining - 1,
                                                memory_order_acq_rel, memory_order_acquire) &&
        remaining == 0)
        return NULL;
    return __real_calloc(count, size);
}

void *__wrap_aligned_alloc(size_t alignment, size_t size) {
    if (atomic_exchange_explicit(&fail_aligned_alloc, 0, memory_order_acq_rel))
        return NULL;
    return __real_aligned_alloc(alignment, size);
}

static void fail_next(atomic_int *target) {
    atomic_store_explicit(target, 1, memory_order_release);
}

static void fail_calloc_on_call(int call_index) {
    atomic_store_explicit(&fail_calloc_after, call_index, memory_order_release);
}
#endif

static void *slab_worker(void *opaque) {
    slab_thread_context_t *context = opaque;
    while (!atomic_load_explicit(context->start, memory_order_acquire))
        sched_yield();

    for (int iteration = 0; iteration < SLAB_ITERATIONS; iteration++) {
        uint8_t *block = img_slab_alloc(context->pool);
        if (!block) {
            atomic_fetch_add_explicit(context->failures, 1, memory_order_relaxed);
            continue;
        }
        size_t index = ((size_t)(block - context->pool->memory)) / context->pool->block_size;
        int expected = 0;
        if (index >= SLAB_BLOCKS ||
            !atomic_compare_exchange_strong_explicit(&context->in_use[index], &expected, 1,
                                                     memory_order_acq_rel, memory_order_acquire)) {
            atomic_fetch_add_explicit(context->failures, 1, memory_order_relaxed);
        }
        block[0] = (uint8_t)iteration;
        atomic_store_explicit(&context->in_use[index], 0, memory_order_release);
        img_slab_free(context->pool, block);
    }
    return NULL;
}

static void *mpmc_producer(void *opaque) {
    mpmc_thread_context_t *context = opaque;
    while (!atomic_load_explicit(context->start, memory_order_acquire))
        sched_yield();

    for (int index = 0; index < MPMC_PER_PRODUCER; index++) {
        uintptr_t value = (uintptr_t)(context->producer_index * MPMC_PER_PRODUCER + index + 1);
        while (img_mpmc_push(context->queue, (void *)value) != 0)
            sched_yield();
    }
    atomic_fetch_add_explicit(context->producers_done, 1, memory_order_release);
    return NULL;
}

static void *mpmc_consumer(void *opaque) {
    mpmc_thread_context_t *context = opaque;
    const int total = MPMC_PRODUCERS * MPMC_PER_PRODUCER;
    while (!atomic_load_explicit(context->start, memory_order_acquire))
        sched_yield();

    while (atomic_load_explicit(context->completed, memory_order_acquire) < total) {
        void *entry = img_mpmc_pop(context->queue);
        if (!entry) {
            if (atomic_load_explicit(context->producers_done, memory_order_acquire) ==
                    MPMC_PRODUCERS &&
                atomic_load_explicit(context->completed, memory_order_acquire) == total)
                break;
            sched_yield();
            continue;
        }
        uintptr_t value = (uintptr_t)entry;
        if (value == 0 || value > (uintptr_t)total) {
            atomic_fetch_add_explicit(context->failures, 1, memory_order_relaxed);
        } else {
            int prior =
                atomic_fetch_add_explicit(&context->seen[value - 1], 1, memory_order_acq_rel);
            if (prior != 0)
                atomic_fetch_add_explicit(context->failures, 1, memory_order_relaxed);
        }
        atomic_fetch_add_explicit(context->completed, 1, memory_order_release);
    }
    return NULL;
}

static int test_slab_reuse_and_permitted_concurrency(void) {
    img_slab_pool_t *pool = img_slab_create(SLAB_BLOCKS * 64u, 64u);
    atomic_bool start = false;
    atomic_int in_use[SLAB_BLOCKS];
    atomic_int failures = 0;
    pthread_t threads[SLAB_THREADS];
    slab_thread_context_t context = {
        .pool = pool,
        .start = &start,
        .in_use = in_use,
        .failures = &failures,
    };

    if (!pool)
        return 1;
    for (size_t index = 0; index < SLAB_BLOCKS; index++)
        atomic_init(&in_use[index], 0);
    uint8_t *first = img_slab_alloc(pool);
    if (!first) {
        img_slab_destroy(pool);
        return 1;
    }
    img_slab_free(pool, first);
    if (img_slab_alloc(pool) != first) {
        fprintf(stderr, "slab did not reuse its most recently released block\n");
        img_slab_destroy(pool);
        return 1;
    }
    img_slab_free(pool, first);

    for (size_t index = 0; index < SLAB_THREADS; index++) {
        if (pthread_create(&threads[index], NULL, slab_worker, &context) != 0) {
            img_slab_destroy(pool);
            return 1;
        }
    }
    atomic_store_explicit(&start, true, memory_order_release);
    for (size_t index = 0; index < SLAB_THREADS; index++)
        pthread_join(threads[index], NULL);

    int result = atomic_load_explicit(&failures, memory_order_acquire) != 0;
    img_slab_destroy(pool);
    return result;
}

static int test_arena_reset_and_reuse(void) {
    img_arena_t *arena = img_arena_create(256);
    if (!arena)
        return 1;

    void *first = img_arena_alloc_aligned(arena, 32, 64);
    void *second = img_arena_alloc(arena, 64);
    if (!first || !second || img_arena_alloc(arena, 256)) {
        img_arena_destroy(arena);
        return 1;
    }
    img_arena_reset(arena);
    void *reused = img_arena_alloc_aligned(arena, 32, 64);
    int result = reused != first;
    img_arena_destroy(arena);
    return result;
}

static int test_queue_boundaries_and_spsc_reuse(void) {
    img_mpmc_queue_t mpmc = {0};
    if (img_mpmc_init(&mpmc, SIZE_MAX) == 0) {
        fprintf(stderr, "unrepresentable MPMC capacity was accepted\n");
        img_mpmc_destroy(&mpmc);
        return 1;
    }
    img_mpmc_destroy(&mpmc);
    if (img_queue_create(31) != NULL)
        return 1;

    img_queue_t *queue = img_queue_create(2);
    int values[3] = {1, 2, 3};
    if (!queue || !img_queue_push(queue, &values[0]) || !img_queue_push(queue, &values[1]) ||
        !img_queue_push(queue, &values[2]) || img_queue_push(queue, &values[0]) ||
        img_queue_pop(queue) != &values[0] || !img_queue_push(queue, &values[0]) ||
        img_queue_pop(queue) != &values[1] || img_queue_pop(queue) != &values[2] ||
        img_queue_pop(queue) != &values[0] || img_queue_pop(queue) != NULL) {
        img_queue_destroy(queue);
        return 1;
    }
    img_queue_destroy(queue);
    return 0;
}

static int test_mpmc_permitted_concurrency(void) {
    img_mpmc_queue_t queue = {0};
    atomic_bool start = false;
    atomic_int producers_done = 0;
    atomic_int completed = 0;
    atomic_int failures = 0;
    atomic_int seen[MPMC_PRODUCERS * MPMC_PER_PRODUCER];
    pthread_t producers[MPMC_PRODUCERS];
    pthread_t consumers[MPMC_CONSUMERS];
    mpmc_thread_context_t contexts[MPMC_PRODUCERS + MPMC_CONSUMERS] = {0};

    if (img_mpmc_init(&queue, 64) != 0)
        return 1;
    for (size_t index = 0; index < sizeof(seen) / sizeof(seen[0]); index++)
        atomic_init(&seen[index], 0);
    for (int index = 0; index < MPMC_PRODUCERS; index++) {
        contexts[index] = (mpmc_thread_context_t){
            .queue = &queue,
            .start = &start,
            .producers_done = &producers_done,
            .completed = &completed,
            .seen = seen,
            .failures = &failures,
            .producer_index = index,
        };
        if (pthread_create(&producers[index], NULL, mpmc_producer, &contexts[index]) != 0) {
            img_mpmc_destroy(&queue);
            return 1;
        }
    }
    for (int index = 0; index < MPMC_CONSUMERS; index++) {
        contexts[MPMC_PRODUCERS + index] = contexts[0];
        if (pthread_create(&consumers[index], NULL, mpmc_consumer,
                           &contexts[MPMC_PRODUCERS + index]) != 0) {
            img_mpmc_destroy(&queue);
            return 1;
        }
    }
    atomic_store_explicit(&start, true, memory_order_release);
    for (int index = 0; index < MPMC_PRODUCERS; index++)
        pthread_join(producers[index], NULL);
    for (int index = 0; index < MPMC_CONSUMERS; index++)
        pthread_join(consumers[index], NULL);

    int result = atomic_load_explicit(&failures, memory_order_acquire) != 0 ||
                 atomic_load_explicit(&completed, memory_order_acquire) !=
                     MPMC_PRODUCERS * MPMC_PER_PRODUCER;
    for (size_t index = 0; !result && index < sizeof(seen) / sizeof(seen[0]); index++)
        result = atomic_load_explicit(&seen[index], memory_order_acquire) != 1;
    img_mpmc_destroy(&queue);
    return result;
}

static int test_allocation_failure_rollback(void) {
#if !defined(IMGENGINE_ALLOCATOR_FAILURE_TEST)
    return 0;
#else
    img_mpmc_queue_t mpmc = {0};

    fail_next(&fail_malloc);
    if (img_slab_create(256, 64) != NULL)
        return 1;
    fail_next(&fail_calloc);
    if (img_slab_create(256, 64) != NULL)
        return 1;

    fail_next(&fail_malloc);
    if (img_arena_create(64) != NULL)
        return 1;
    fail_next(&fail_aligned_alloc);
    if (img_arena_create(64) != NULL)
        return 1;

    fail_next(&fail_malloc);
    if (img_queue_create(2) != NULL)
        return 1;
    fail_next(&fail_calloc);
    if (img_queue_create(2) != NULL)
        return 1;

    fail_next(&fail_aligned_alloc);
    if (img_mpmc_init(&mpmc, 4) == 0) {
        img_mpmc_destroy(&mpmc);
        return 1;
    }
    img_mpmc_destroy(&mpmc);

    img_scheduler_t scheduler = {0};
    fail_next(&fail_malloc);
    if (img_scheduler_init(&scheduler, 1) == 0) {
        img_scheduler_destroy(&scheduler);
        return 1;
    }
    fail_next(&fail_aligned_alloc);
    if (img_scheduler_init(&scheduler, 1) == 0) {
        img_scheduler_destroy(&scheduler);
        return 1;
    }
    fail_next(&fail_malloc);
    if (img_scheduler_init(&scheduler, 1) == 0) {
        img_scheduler_destroy(&scheduler);
        return 1;
    }
    fail_next(&fail_calloc);
    if (img_scheduler_init(&scheduler, 1) == 0) {
        img_scheduler_destroy(&scheduler);
        return 1;
    }
    fail_calloc_on_call(1);
    if (img_scheduler_init(&scheduler, 1) == 0) {
        img_scheduler_destroy(&scheduler);
        return 1;
    }
    atomic_store_explicit(&fail_calloc_after, -1, memory_order_release);
    return 0;
#endif
}

static int test_repeated_lifecycle(void) {
    for (int iteration = 0; iteration < 50; iteration++) {
        img_slab_pool_t *pool = img_slab_create(256, 64);
        img_arena_t *arena = img_arena_create(128);
        img_scheduler_t scheduler = {0};
        if (!pool || !arena || img_scheduler_init(&scheduler, 1) != 0) {
            img_scheduler_destroy(&scheduler);
            img_arena_destroy(arena);
            img_slab_destroy(pool);
            return 1;
        }
        uint8_t *block = img_slab_alloc(pool);
        if (!block || !img_arena_alloc(arena, 32)) {
            img_scheduler_destroy(&scheduler);
            img_arena_destroy(arena);
            img_slab_destroy(pool);
            return 1;
        }
        img_slab_free(pool, block);
        img_scheduler_destroy(&scheduler);
        img_arena_destroy(arena);
        img_slab_destroy(pool);
    }
    return 0;
}

static int test_worker_teardown_preserves_caller_slab_ownership(void) {
    img_scheduler_t scheduler = {0};
    img_ctx_t context = {0};
    img_slab_pool_t *pool = img_slab_create(256, 64);
    img_task_t task = {0};
    if (!pool || img_scheduler_init(&scheduler, 1) != 0) {
        img_scheduler_destroy(&scheduler);
        img_slab_destroy(pool);
        return 1;
    }
    uint8_t *block = img_slab_alloc(pool);
    img_worker_t *worker = &scheduler.workers[0];
    if (!block) {
        img_scheduler_destroy(&scheduler);
        img_slab_destroy(pool);
        return 1;
    }
    worker->ctx = &context;
    atomic_store_explicit(&worker->running, true, memory_order_release);
    if (pthread_create(&worker->thread, NULL, img_worker_loop, worker) != 0) {
        img_slab_free(pool, block);
        img_scheduler_destroy(&scheduler);
        img_slab_destroy(pool);
        return 1;
    }
    worker->thread_started = 1;

    task.kind = IMG_TASK_KIND_PIPELINE;
    task.payload.pipeline.buffer = (img_buffer_t *)block;
    task.payload.pipeline.pipeline = NULL;
    if (img_scheduler_submit(&scheduler, &task) != 0) {
        img_slab_free(pool, block);
        img_scheduler_destroy(&scheduler);
        img_slab_destroy(pool);
        return 1;
    }
    img_scheduler_destroy(&scheduler);
    if (atomic_load_explicit(&task.state, memory_order_acquire) != IMG_TASK_DONE ||
        img_runtime_wait_task(&task) != IMG_ERR_SECURITY) {
        img_slab_free(pool, block);
        img_slab_destroy(pool);
        return 1;
    }
    block[0] = 0xC2;
    img_slab_free(pool, block);
    img_slab_destroy(pool);
    return 0;
}

int main(void) {
    int failures = test_slab_reuse_and_permitted_concurrency() || test_arena_reset_and_reuse() ||
                   test_queue_boundaries_and_spsc_reuse() || test_mpmc_permitted_concurrency() ||
                   test_allocation_failure_rollback() || test_repeated_lifecycle() ||
                   test_worker_teardown_preserves_caller_slab_ownership();
    if (failures)
        return 1;
    puts("[allocator-queue] ownership, reuse, and permitted concurrency passed");
    return 0;
}
