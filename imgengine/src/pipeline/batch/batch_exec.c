#include "pipeline/batch/batch.h"
#include "pipeline/dispatch/jump_table.h"

void img_batch_execute(
    img_ctx_t *ctx,
    uint32_t op_code,
    img_batch_t *batch,
    void *params)
{
    if (!batch) return;

    for (uint32_t i = 0; i < batch->count; i++)
    {
        g_jump_table[op_code](
            ctx,
            batch->items[i],
            params);
    }
}