/* runtime/worker.c */

#include "runtime/worker.h"
#include "runtime/queue_mpmc.h"
#include "runtime/affinity.h"
#include "pipeline/jump_table.h"
#include "security/sandbox.h"
#include "runtime/task.h"
#include "io/decoder/decoder_entry.h"
#include "io/encoder/encoder_entry.h"
#include "hot/pipeline_exec.h"

#include <stdlib.h>

void *img_worker_loop(void *arg)
{
    img_worker_t *self = (img_worker_t *)arg;

    img_pin_thread_to_core(self->cpu_id);
    img_security_enter_sandbox();
    img_pipeline_bind_thread();

    while (self->active)
    {
        img_task_t *task = img_mpmc_pop(self->task_queue);

        if (!task)
        {
            __builtin_ia32_pause();
            continue;
        }

        img_buffer_t buf;

        // 🔥 1. DECODE
        if (img_decode_to_buffer(self->ctx,
                                 task->input_data,
                                 task->input_size,
                                 &buf) != 0)
        {
            free(task);
            continue;
        }

        // 🔥 2. PROCESS (SIMD PIPELINE)
        // img_execute_pipeline(self->ctx,
        //                      &buf,
        //                      task->ops,
        //                      task->params,
        //                      task->op_count);


        // 🔥 3. ENCODE
        img_encode_from_buffer(self->ctx,
                               &buf,
                               &task->output_data,
                               &task->output_size);

        // 🔥 4. FREE SLAB (IMPORTANT)
        img_slab_free(self->ctx->pool, buf.data);

        // (later: push result to output queue)

        free(task);
    }

    return NULL;
}
