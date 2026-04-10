
// pipeline/pipeline_signature.h

#ifndef IMGENGINE_PIPELINE_SIGNATURE_H
#define IMGENGINE_PIPELINE_SIGNATURE_H

#include <stdint.h>
#include "core/opcodes.h"

/* Forward Declarations (Opaque Handles) */
typedef struct img_pipeline_desc img_pipeline_desc_t;

/*
 * 🔥 BITMASK SIGNATURE (MAX 32 OPS)
 */
typedef uint32_t img_pipeline_sig_t;

/*
 * 🔥 OP BITS
 */
#define SIG_OP_GRAYSCALE (1u << 0)
#define SIG_OP_BRIGHTNESS (1u << 1)
#define SIG_OP_RESIZE (1u << 2)

/*
 * 🔥 BUILD SIGNATURE
 */
static inline img_pipeline_sig_t
img_pipeline_build_signature(const img_pipeline_desc_t *p)
{
    img_pipeline_sig_t sig = 0;

    for (uint32_t i = 0; i < p->count; i++)
    {
        switch (p->ops[i].op_code)
        {
        case OP_GRAYSCALE:
            sig |= SIG_OP_GRAYSCALE;
            break;
        case OP_BRIGHTNESS:
            sig |= SIG_OP_BRIGHTNESS;
            break;
        case OP_RESIZE:
            sig |= SIG_OP_RESIZE;
            break;
        }
    }

    return sig;
}

#endif