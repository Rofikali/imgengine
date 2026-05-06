// ./include/runtime/task.h
// include/runtime/task.h

#ifndef IMGENGINE_RUNTIME_TASK_H
#define IMGENGINE_RUNTIME_TASK_H

#include <stdatomic.h>
#include <stdint.h>

#include "memory/arena.h"
#include "pipeline/canvas.h"
#include "pipeline/job.h"
#include "pipeline/layout.h"

/*
 * Forward declarations
 *
 * img_pipeline_runtime_t: compiled pipeline (internal, hot path safe)
 * img_pipeline_desc_t:    raw user descriptor (NOT used in hot path)
 *
 * The task carries the COMPILED pipeline only.
 * Compilation happens at submit time (cold path).
 * Execution happens at pop time (hot path).
 */
typedef struct img_buffer img_buffer_t;
typedef struct img_pipeline_runtime img_pipeline_runtime_t;
typedef struct img_engine img_engine_t;

/*
 * Task state
 */
typedef enum {
    IMG_TASK_READY = 0,
    IMG_TASK_RUNNING = 1,
    IMG_TASK_DONE = 2,
} img_task_state_t;

typedef enum {
    IMG_TASK_KIND_PIPELINE = 0,
    IMG_TASK_KIND_RENDER_STAGE = 1,
} img_task_kind_t;

typedef struct {
    img_buffer_t *buffer;
    img_pipeline_runtime_t *pipeline;
} img_pipeline_task_payload_t;

typedef struct {
    img_engine_t *engine;
    img_ctx_t *ctx;
    img_canvas_t *canvas;
    img_layout_t *layout;
    const img_job_t *job;
    const img_buffer_t *photo;
    img_arena_t **arena;
} img_render_task_payload_t;

/*
 * img_task_t
 *
 * Cache-line aligned: 64 bytes.
 * Fits in one cache line — worker pops task and all fields
 * are hot in L1 before first access.
 *
 * FIELDS:
 *   buffer   — image to process (slab-owned, zero-copy)
 *   pipeline — pre-compiled runtime (arena-owned, read-only in hot path)
 *   op_index — current op (for future partial execution / preemption)
 *   state    — task lifecycle state
 *   status   — result code written by worker on completion
 *
 * OWNERSHIP:
 *   buffer   — returned to slab by caller after task->status written
 *   pipeline — owned by arena, valid for lifetime of request
 */
typedef struct __attribute__((aligned(64))) {
    img_task_kind_t kind;
    img_task_state_t state;
    uint32_t op_index;

    int status;
    union {
        img_pipeline_task_payload_t pipeline;
        img_render_task_payload_t render;
    } payload;

} img_task_t;

#endif /* IMGENGINE_RUNTIME_TASK_H */
