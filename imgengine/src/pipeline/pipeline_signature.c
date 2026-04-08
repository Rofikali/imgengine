// pipeline/pipeline_signature.c

#include "pipeline/pipeline_signature.h"
#include "pipeline/pipeline_types.h"

img_pipeline_sig_t
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