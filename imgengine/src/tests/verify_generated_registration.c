// Simple verification executable to ensure generated pipelines register into jump table.
#include <stdio.h>
#include <stdlib.h>
#include "pipeline/generated.h"
#include "pipeline/jump_table.h"
#include "core/opcodes.h"

int main(void)
{
    /* Register the generated pipelines explicitly */
    register_generated_pipelines();

    /* Basic sanity checks for a couple of known opcodes */
    if (g_jump_table[OP_CUSTOM_BASE + 0] == NULL) {
        fprintf(stderr, "[verify] OP_CUSTOM_BASE+0 not registered\n");
        return 2;
    }
    if (g_jump_table[OP_CUSTOM_BASE + 4] == NULL) {
        fprintf(stderr, "[verify] OP_CUSTOM_BASE+4 not registered\n");
        return 2;
    }

    printf("[verify] generated pipelines registered OK\n");
    return 0;
}
