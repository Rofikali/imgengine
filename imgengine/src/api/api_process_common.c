// ./src/api/api_process_common.c
#include "api/api_internal.h"

#include "memory/arena.h"
#include "memory/slab.h"
#include "runtime/template_registry.h"

#include <pthread.h>
#include <stdlib.h>

typedef struct {
    img_slab_pool_t *pool;
    img_arena_t *arena;
} img_api_thread_ctx_t;

static pthread_key_t g_api_thread_ctx_key;
static pthread_once_t g_api_thread_ctx_once = PTHREAD_ONCE_INIT;

static void img_api_thread_ctx_destroy(void *ptr) {
    img_api_thread_ctx_t *thread_ctx = (img_api_thread_ctx_t *)ptr;
    if (!thread_ctx)
        return;

    if (thread_ctx->arena)
        img_arena_destroy(thread_ctx->arena);
    if (thread_ctx->pool)
        img_slab_destroy(thread_ctx->pool);

    free(thread_ctx);
}

static void img_api_thread_ctx_init_key(void) {
    (void)pthread_key_create(&g_api_thread_ctx_key, img_api_thread_ctx_destroy);
}

static img_api_thread_ctx_t *img_api_get_thread_ctx(void) {
    pthread_once(&g_api_thread_ctx_once, img_api_thread_ctx_init_key);

    img_api_thread_ctx_t *thread_ctx =
        (img_api_thread_ctx_t *)pthread_getspecific(g_api_thread_ctx_key);
    if (thread_ctx)
        return thread_ctx;

    thread_ctx = (img_api_thread_ctx_t *)calloc(1, sizeof(*thread_ctx));
    if (!thread_ctx)
        return NULL;

    thread_ctx->pool = img_slab_create(128 * 1024 * 1024, 32 * 1024 * 1024);
    if (!thread_ctx->pool)
        goto fail;

    thread_ctx->arena = img_arena_create(1024 * 1024);
    if (!thread_ctx->arena)
        goto fail;

    if (pthread_setspecific(g_api_thread_ctx_key, thread_ctx) != 0)
        goto fail;

    return thread_ctx;

fail:
    img_api_thread_ctx_destroy(thread_ctx);
    return NULL;
}

void img_api_make_ctx(const img_engine_t *engine, img_ctx_t *ctx) {
    img_ctx_bind_engine(engine, ctx);

    img_api_thread_ctx_t *thread_ctx = img_api_get_thread_ctx();
    if (!thread_ctx)
        return;

    img_arena_reset(thread_ctx->arena);
    ctx->local_pool = thread_ctx->pool;
    ctx->scratch_arena = thread_ctx->arena;
}

void img_api_release_thread_ctx(void) {
    pthread_once(&g_api_thread_ctx_once, img_api_thread_ctx_init_key);

    img_api_thread_ctx_t *thread_ctx =
        (img_api_thread_ctx_t *)pthread_getspecific(g_api_thread_ctx_key);
    if (!thread_ctx)
        return;

    pthread_setspecific(g_api_thread_ctx_key, NULL);
    img_api_thread_ctx_destroy(thread_ctx);
}

img_result_t img_api_resolve_template_job(const img_engine_t *engine,
                                          img_job_template_t template_id, img_job_t *job_out) {
    return img_runtime_resolve_template_job(engine, template_id, job_out);
}
