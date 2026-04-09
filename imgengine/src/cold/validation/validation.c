#include "cold/validation/validation.h"
#include "core/pipeline/opcodes.h"

int img_validate_pipeline_safety(
    const img_pipeline_desc_t *p)
{
    if (!p)
        return 0;

    for (uint32_t i = 0; i < p->count; i++)
    {
        uint32_t op = p->ops[i].op_code;

        // 🔥 Example policy:
        // forbid unknown ops
        if (op == 0 || op >= OP_MAX)
            return 0;

        // 🔥 future: restrict dangerous ops
    }

    return 1;
}