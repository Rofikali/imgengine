// ./src/api/api.c

#include "api/v1/img_api.h"
#include "api/v1/img_buffer_utils.h"
#include "api/io_vtable.h"
#include "core/context_internal.h"
#include "runtime/worker.h"
#include "runtime/task.h"
#include "runtime/queue_mpmc.h"

#include "memory/slab.h"
#include "memory/arena.h"

#include "arch/cpu_caps.h"

#include "pipeline/jump_table.h"
#include "pipeline/pipeline_compiled.h"

#include "hot/pipeline_exec.h"

#include "security/input_validator.h"
#include "security/sandbox.h"

#include "cold/validation.h"

#include "pipeline/canvas.h"
#include "pipeline/layout.h"

#include <string.h> /* strrchr */
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>

// 🔥 MISSING DECLARATION FIX
void img_hw_register_kernels(cpu_caps_t caps);

// ============================================================
// 🔥 GLOBAL ENGINE
// ============================================================

static img_mpmc_queue_t g_task_queue;
static img_engine_t g_engine;
static img_worker_t g_workers[64];

// ============================================================
// 🔥 FORWARD
// ============================================================

// ================================================================
// FILE 12: src/api/api.c  — ADD img_api_run_job() to existing file
// This is the full pipeline: decode → layout → bleed →
// crop marks → encode (JPEG or PDF by extension).
// Add this block AFTER existing functions in api.c.
// ================================================================

/* forward declarations for pipeline stages */
// img_result_t img_apply_bleed(img_buffer_t *, const img_layout_t *, uint32_t);
// img_result_t img_draw_crop_marks(img_buffer_t *, const img_layout_t *, const img_job_t *);

/* NEW — correct 3-arg signature */
img_result_t img_apply_bleed(img_buffer_t *, const img_layout_t *, uint32_t);
img_result_t img_draw_borders(img_buffer_t *, const img_layout_t *, uint32_t);
img_result_t img_draw_crop_marks(img_buffer_t *, const img_layout_t *, const img_job_t *);

/*
 * detect_output_format()
 *
 * Returns 1 if output path ends with .pdf (case-insensitive), 0 otherwise.
 */
static int is_pdf_output(const char *path)
{
    const char *dot = strrchr(path, '.');
    if (!dot)
        return 0;
    return (dot[1] == 'p' || dot[1] == 'P') &&
           (dot[2] == 'd' || dot[2] == 'D') &&
           (dot[3] == 'f' || dot[3] == 'F') &&
           dot[4] == '\0';
}

/*
 * load_file_mmap()
 *
 * Memory-map a file. Returns IMG_SUCCESS on success.
 * Caller must munmap(*data, *size) when done.
 */
static img_result_t load_file_mmap(
    const char *path,
    uint8_t **data,
    size_t *size)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0)
        return IMG_ERR_IO;

    off_t file_size = lseek(fd, 0, SEEK_END);
    if (file_size <= 0)
    {
        close(fd);
        return IMG_ERR_IO;
    }

    void *ptr = mmap(NULL, (size_t)file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    if (ptr == MAP_FAILED)
        return IMG_ERR_IO;

    *data = (uint8_t *)ptr;
    *size = (size_t)file_size;
    return IMG_SUCCESS;
}

/*
 * img_api_run_job()
 *
 * Full print pipeline:
 *
 *  1. Validate job ABI version
 *  2. mmap input file
 *  3. Security validate
 *  4. Decode JPEG/PNG → img_buffer_t (slab)
 *  5. Allocate A4 canvas (slab), fill background
 *  6. Compute grid geometry
 *  7. Resize photo → grid cell size (slab)
 *  8. Blit photo N×M times onto canvas (AVX2)
 *  9. Apply bleed (in-place)
 * 10. Draw crop marks (in-place)
 * 11. Encode canvas → JPEG or PDF
 * 12. Free all slab allocations
 *
 * Thread-safe: uses per-call arena and global slab pool.
 * Zero heap allocation in hot path (steps 7-10).
 */
img_result_t img_api_run_job(
    img_engine_t *engine,
    const char *input_path,
    const char *output_path,
    const img_job_t *job)
{
    if (!engine || !input_path || !output_path || !job)
        return IMG_ERR_SECURITY;

    if (job->abi_version != IMG_JOB_ABI_VERSION)
        return IMG_ERR_INTERNAL;

    /* ── 1. Load input ── */
    uint8_t *file_data = NULL;
    size_t file_size = 0;

    img_result_t r = load_file_mmap(input_path, &file_data, &file_size);
    if (r != IMG_SUCCESS)
    {
        fprintf(stderr, "[JOB] cannot open input '%s': %s\n",
                input_path, img_result_name(r));
        return r;
    }

    /* ── 2. Security gate ── */
    r = img_security_validate_request(16384, 16384, file_size);
    if (r != IMG_SUCCESS)
    {
        fprintf(stderr, "[JOB] security reject: %s\n", img_result_name(r));
        munmap(file_data, file_size);
        return r;
    }

    /* ── 3. Per-job context ── */
    img_ctx_t ctx = {0};
    ctx.thread_id = 0;
    ctx.caps = engine->caps;
    ctx.local_pool = engine->global_pool;

    /* Scratch arena for cells array (~few KB) */
    img_arena_t *arena = img_arena_create(1 * 1024 * 1024);
    if (!arena)
    {
        munmap(file_data, file_size);
        return IMG_ERR_NOMEM;
    }

    /* ── 4. Decode ── */
    img_buffer_t photo = {0};

    // r = (img_result_t)img_decode_to_buffer(&ctx, file_data, file_size, &photo);

    r = (img_result_t)g_io_vtable.decode(&ctx, file_data, file_size, &photo);
    munmap(file_data, file_size);

    if (r != IMG_SUCCESS)
    {
        fprintf(stderr, "[JOB] decode failed: %s\n", img_result_name(r));
        img_arena_destroy(arena);
        return r;
    }

    /* ── 5. Allocate A4 canvas ── */
    img_canvas_t canvas = {0};
    r = img_canvas_init(&canvas, engine->global_pool, job);
    if (r != IMG_SUCCESS)
    {
        fprintf(stderr, "[JOB] canvas alloc failed: %s\n", img_result_name(r));
        img_slab_free(engine->global_pool, photo.data);
        img_arena_destroy(arena);
        return r;
    }

    /* ── 6-8. Grid layout + blit ── */
    img_layout_t layout = {0};
    r = img_layout_grid(&canvas, &photo, job, &layout, arena, engine->global_pool);
    img_slab_free(engine->global_pool, photo.data);

    if (r != IMG_SUCCESS)
    {
        fprintf(stderr, "[JOB] layout failed: %s\n", img_result_name(r));
        img_canvas_free(&canvas, engine->global_pool);
        img_arena_destroy(arena);
        return r;
    }

    /* 11. Border — drawn AFTER bleed, AFTER crop marks */
    /*     Border is inside the photo so it draws on top of bleed */
    if (job->border_px > 0)
    {
        r = img_draw_borders(&canvas.buf, &layout, job->border_px);
        if (r != IMG_SUCCESS)
        {
            fprintf(stderr, "[JOB] border failed: %s\n", img_result_name(r));
            img_canvas_free(&canvas, engine->global_pool);
            img_arena_destroy(arena);
            return r;
        }
    }

    /* ── 9. Bleed ── */
    r = img_apply_bleed(&canvas.buf, &layout, job->bleed_px);
    if (r != IMG_SUCCESS)
    {
        fprintf(stderr, "[JOB] bleed failed: %s\n", img_result_name(r));
        img_canvas_free(&canvas, engine->global_pool);
        img_arena_destroy(arena);
        return r;
    }

    /* ── 10. Crop marks ── */
    r = img_draw_crop_marks(&canvas.buf, &layout, job);
    if (r != IMG_SUCCESS)
    {
        fprintf(stderr, "[JOB] crop marks failed: %s\n", img_result_name(r));
        img_canvas_free(&canvas, engine->global_pool);
        img_arena_destroy(arena);
        return r;
    }

    /* ── 11. Encode output ── */
    if (is_pdf_output(output_path))
    {
        r = g_io_vtable.encode_pdf(&canvas.buf, output_path, job->dpi);
    }
    else
    {
        /* JPEG output */
        uint8_t *out = NULL;
        size_t out_size = 0;

        r = (img_result_t)g_io_vtable.encode(
            &ctx, &canvas.buf, &out, &out_size);

        if (r == IMG_SUCCESS && out)
        {
            FILE *f = fopen(output_path, "wb");
            if (f)
            {
                fwrite(out, 1, out_size, f);
                fclose(f);
            }
            else
            {
                r = IMG_ERR_IO;
            }
            free(out);
        }
    }

    if (r != IMG_SUCCESS)
        fprintf(stderr, "[JOB] encode failed: %s\n", img_result_name(r));

    /* ── 12. Free ── */
    img_canvas_free(&canvas, engine->global_pool);
    img_arena_destroy(arena);

    return r;
}

/*
 * decode_image_secure()
 *
 * Internal bridge to the hardened JPEG decoder.
 * Returns img_result_t — never raw -1.
 */
img_result_t decode_image_secure(
    const uint8_t *input,
    size_t size,
    img_buffer_t *out_buf)
{
    img_ctx_t ctx = {0};
    ctx.thread_id = 0;
    ctx.caps = g_engine.caps;
    ctx.local_pool = g_engine.global_pool;

    int rc = g_io_vtable.decode(&ctx, input, size, out_buf);

    /*
     * img_decode_to_buffer returns img_result_t cast to int.
     * Cast back — no information lost.
     */
    return (img_result_t)rc;
}

/*
 * img_api_process_raw()
 *
 * Full pipeline: decode → (no-op pipeline) → encode.
 * Every error path returns a named code with a log line.
 */
img_result_t img_api_process_raw(
    img_engine_t *engine,
    uint8_t *input,
    size_t input_size,
    uint8_t **output,
    size_t *output_size)
{
    if (!engine || !input || !output || !output_size)
        return IMG_ERR_SECURITY;

    /* Security gate — validate dimensions and size before touching data */
    img_result_t sec = img_security_validate_request(4096, 4096, input_size);
    if (sec != IMG_SUCCESS)
    {
        fprintf(stderr, "[API] security validation failed: %d\n", sec);
        return sec;
    }

    /* Decode */
    img_buffer_t buf = {0};

    img_ctx_t ctx = {0};
    ctx.thread_id = 0;
    ctx.caps = engine->caps;
    ctx.local_pool = engine->global_pool;

    img_result_t dec = (img_result_t)g_io_vtable.decode(
        &ctx, input, input_size, &buf);

    if (dec != IMG_SUCCESS)
    {
        fprintf(stderr, "[API] decode failed: %d (%s)\n",
                dec, img_result_name(dec));
        return dec;
    }

    /* Encode */
    img_result_t enc = (img_result_t)g_io_vtable.encode(
        &ctx, &buf, output, output_size);

    img_slab_free(engine->global_pool, buf.data);

    if (enc != IMG_SUCCESS)
    {
        fprintf(stderr, "[API] encode failed: %d (%s)\n",
                enc, img_result_name(enc));
        return enc;
    }

    return IMG_SUCCESS;
}

// ============================================================
// 🔥 FAST PATH
// ============================================================

img_result_t img_api_process_fast(
    img_engine_t *engine,
    const uint8_t *input,
    size_t input_size,
    img_pipeline_desc_t *pipe,
    img_buffer_t *out_buf)
{
    if (!engine || !input || !pipe || !out_buf)
        return IMG_ERR_SECURITY;

    img_result_t sec = img_security_validate_request(
        4096, 4096,
        input_size);

    if (sec != IMG_SUCCESS)
        return sec;

    img_result_t res = decode_image_secure(
        input,
        input_size,
        out_buf);

    if (res != IMG_SUCCESS)
        return res;

    if (!img_validate_pipeline_safety(pipe))
        return IMG_ERR_SECURITY;

    img_pipeline_compiled_t compiled;

    if (img_pipeline_compile(pipe, &compiled) != 0)
        return IMG_ERR_FORMAT;

    img_ctx_t ctx = {0};
    ctx.thread_id = 0;
    ctx.caps = engine->caps;

    img_pipeline_execute_hot(
        &ctx,
        (img_pipeline_runtime_t *)&compiled,
        out_buf);

    return IMG_SUCCESS;
}

// ============================================================
// 🔥 INIT
// ============================================================

img_engine_t *img_api_init(uint32_t workers)
{
    if (!img_security_enter_sandbox())
        return NULL;

    if (workers == 0 || workers > 64)
        return NULL;

    memset(&g_engine, 0, sizeof(g_engine));

    g_engine.worker_count = workers;
    g_engine.workers = g_workers;

    // 🔥 CPU DETECT
    g_engine.caps = img_cpu_detect_caps();

    // 🔥 GLOBAL SLAB
    g_engine.global_pool = img_slab_create(
        512 * 1024 * 1024, // 512MB total pool (supports ~16 concurrent 4K images)
        32 * 1024 * 1024); // 32MB per block  (fits 4K RGB: 3840*2160*3 = 24MB)

    //    g_engine.global_pool = img_slab_create(
    //        1024ULL * 1024 * 1024,   /* 1GB total pool */
    //        64ULL   * 1024 * 1024);  /* 64MB per block  */

    if (!g_engine.global_pool)
        return NULL;

    // 🔥 DISPATCH INIT
    img_jump_table_init(g_engine.caps);
    img_hw_register_kernels(g_engine.caps);
    static img_ctx_t g_worker_ctxs[64];
    /* 🔥 ZERO-COST PLUGIN LOAD */
    // img_plugins_init_all();

    for (uint32_t i = 0; i < workers; i++)
    {
        g_worker_ctxs[i].thread_id = i;
        g_worker_ctxs[i].caps = g_engine.caps;
        g_worker_ctxs[i].local_pool = g_engine.global_pool;

        img_worker_init(
            &g_workers[i],
            i,
            NULL,
            &g_worker_ctxs[i]);
    }

    // 🔥 OPTIONAL GLOBAL QUEUE (for external submissions only)
    img_mpmc_init(&g_task_queue, 1024);

    return &g_engine;
}

// ============================================================
// 🔥 TASK SUBMIT
// ============================================================

int img_submit_task(img_task_t *task)
{
    if (!task)
        return 0;

    return img_mpmc_push(&g_task_queue, task);
}

// ============================================================
// 🔥 SHUTDOWN
// ============================================================

void img_api_shutdown(img_engine_t *engine)
{
    if (!engine)
        return;

    for (uint32_t i = 0; i < engine->worker_count; i++)
    {
        img_worker_stop(&engine->workers[i]);
        img_worker_join(&engine->workers[i]);
    }

    if (engine->global_pool)
        img_slab_destroy(engine->global_pool);
}

// ============================================================
// 🔥 FREE
// ============================================================

void img_encoded_free(uint8_t *ptr)
{
    if (ptr)
        free(ptr);
}
