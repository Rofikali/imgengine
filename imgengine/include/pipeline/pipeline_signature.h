// pipeline/pipeline_signature.h

#ifndef IMGENGINE_PIPELINE_SIGNATURE_H
#define IMGENGINE_PIPELINE_SIGNATURE_H

#include <stdint.h>
#include "core/opcodes.h"
#include "pipeline/pipeline_types.h"

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

static inline uint64_t make_sig(uint32_t *ops, uint32_t count)
{
    uint64_t sig = 0;

    for (uint32_t i = 0; i < count; i++)
    {
        sig |= ((uint64_t)ops[i] << (i * 8));
    }

    return sig;
}

#endif