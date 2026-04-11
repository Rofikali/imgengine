// ./include/runtime/task.h
// include/runtime/task.h

#ifndef IMGENGINE_RUNTIME_TASK_H
#define IMGENGINE_RUNTIME_TASK_H

#include <stdint.h>

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

/*
 * Task state
 */
typedef enum
{
    IMG_TASK_READY = 0,
    IMG_TASK_RUNNING = 1,
    IMG_TASK_DONE = 2,
} img_task_state_t;

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
typedef struct __attribute__((aligned(64)))
{
    img_buffer_t *buffer;
    img_pipeline_runtime_t *pipeline; /* compiled — NOT img_pipeline_desc_t */

    uint32_t op_index;
    img_task_state_t state;

    int status;

} img_task_t;

#endif /* IMGENGINE_RUNTIME_TASK_H */
