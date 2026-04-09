

#ifndef IMGENGINE_PIPELINE_EXEC_H
#define IMGENGINE_PIPELINE_EXEC_H

#include <stdint.h>

typedef struct img_ctx img_ctx_t;
typedef struct img_buffer img_buffer_t;

/*
 * 🔥 RUNTIME OP
 */
typedef struct
{
    uint32_t op_code;
    void *params;

} img_runtime_op_t;

/*
 * 🔥 COMPILED PIPELINE
 */
typedef struct
{
    img_runtime_op_t *ops;
    uint32_t count;

} img_pipeline_runtime_t;

/*
 * 🔥 HOT EXECUTION ENTRY
 */
void img_pipeline_execute_hot(
    img_ctx_t *ctx,
    img_pipeline_runtime_t *pipe,
    img_buffer_t *buf);

#endif