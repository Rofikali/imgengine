// ./include/pipeline/pipeline_signature.h

// include/pipeline/pipeline_signature.h

#ifndef IMGENGINE_PIPELINE_SIGNATURE_H
#define IMGENGINE_PIPELINE_SIGNATURE_H

#include <stdint.h>
#include "core/opcodes.h"
#include "pipeline/pipeline_types.h" /* REQUIRED: inline fn accesses p->ops[i].op_code */

/*
 * BITMASK SIGNATURE
 * Max 32 ops supported (uint32_t bitmask)
 * ABI-STABLE — never reorder bits
 */
typedef uint32_t img_pipeline_sig_t;

/* OP BITMASK FLAGS */
#define SIG_OP_GRAYSCALE (1u << 0)
#define SIG_OP_BRIGHTNESS (1u << 1)
#define SIG_OP_RESIZE (1u << 2)

/*
 * img_pipeline_build_signature()
 *
 * Cold path: called once per pipeline compile.
 * Maps op_code array → bitmask for O(1) dispatch lookup.
 *
 * Inline justified: trivial loop, called from cold path only,
 * avoids function call overhead at compile boundary.
 */
static inline img_pipeline_sig_t img_pipeline_build_signature(const img_pipeline_desc_t *p) {
    img_pipeline_sig_t sig = 0;

    if (__builtin_expect(!p || p->count == 0, 0))
        return 0;

    for (uint32_t i = 0; i < p->count; i++) {
        switch (p->ops[i].op_code) {
        case OP_GRAYSCALE:
            sig |= SIG_OP_GRAYSCALE;
            break;
        case OP_BRIGHTNESS:
            sig |= SIG_OP_BRIGHTNESS;
            break;
        case OP_RESIZE:
            sig |= SIG_OP_RESIZE;
            break;
        default:
            /* unknown op: ignore, do not crash */
            break;
        }
    }

    return sig;
}

#endif /* IMGENGINE_PIPELINE_SIGNATURE_H */

// // pipeline/pipeline_signature.h

// #ifndef IMGENGINE_PIPELINE_SIGNATURE_H
// #define IMGENGINE_PIPELINE_SIGNATURE_H

// #include <stdint.h>
// #include "core/opcodes.h"

// /* Forward Declarations (Opaque Handles) */
// typedef struct img_pipeline_desc img_pipeline_desc_t;

// /*
//  * 🔥 BITMASK SIGNATURE (MAX 32 OPS)
//  */
// typedef uint32_t img_pipeline_sig_t;

// /*
//  * 🔥 OP BITS
//  */
// #define SIG_OP_GRAYSCALE (1u << 0)
// #define SIG_OP_BRIGHTNESS (1u << 1)
// #define SIG_OP_RESIZE (1u << 2)

// /*
//  * 🔥 BUILD SIGNATURE
//  */
// static inline img_pipeline_sig_t
// img_pipeline_build_signature(const img_pipeline_desc_t *p)
// {
//     img_pipeline_sig_t sig = 0;

//     for (uint32_t i = 0; i < p->count; i++)
//     {
//         switch (p->ops[i].op_code)
//         {
//         case OP_GRAYSCALE:
//             sig |= SIG_OP_GRAYSCALE;
//             break;
//         case OP_BRIGHTNESS:
//             sig |= SIG_OP_BRIGHTNESS;
//             break;
//         case OP_RESIZE:
//             sig |= SIG_OP_RESIZE;
//             break;
//         }
//     }

//     return sig;
// }

// #endif