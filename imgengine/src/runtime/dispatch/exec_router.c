#include "runtime/dispatch/exec_router.h"
#include "pipeline/dispatch/jump_table.h"

img_exec_mode_t img_exec_route(uint32_t op)
{
    switch (op)
    {
        case OP_RESIZE:
            return IMG_EXEC_LOCAL;

        default:
            return IMG_EXEC_REMOTE;
    }
}