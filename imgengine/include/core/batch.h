// ./include/core/batch.h
// include/core/batch.h  (NEW FILE — move from pipeline/batch.h)

#ifndef IMGENGINE_CORE_BATCH_H
#define IMGENGINE_CORE_BATCH_H

#include <stdint.h>

/* forward declaration */
typedef struct img_buffer img_buffer_t;

/*
 * img_batch_t — array of buffer pointers for batch processing.
 *
 * Moved from pipeline/batch.h to core/batch.h because:
 *   arch/ (index 2) needs img_batch_t for fused batch kernels
 *   pipeline/ (index 4) defines batch.h — arch/ can't include up
 *
 * core/ (index 0) is the correct home: it's a plain data struct
 * with no dependencies beyond img_buffer_t (also in core/).
 */
#define IMG_BATCH_MAX 64

typedef struct img_batch {
    img_buffer_t *buffers[IMG_BATCH_MAX];
    uint32_t count;
} img_batch_t;

#endif /* IMGENGINE_CORE_BATCH_H */