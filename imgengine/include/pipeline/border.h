// ./include/pipeline/border.h

// ================================================================
// FILE 3: include/pipeline/border.h  (NEW FILE)
// ================================================================

#ifndef IMGENGINE_PIPELINE_BORDER_H
#define IMGENGINE_PIPELINE_BORDER_H

#include "core/buffer.h"
#include "pipeline/layout.h"
#include "api/v1/img_error.h"
#include <stdint.h>

img_result_t img_draw_borders(
    img_buffer_t *canvas,
    const img_layout_t *layout,
    uint32_t border_px);

#endif /* IMGENGINE_PIPELINE_BORDER_H */
