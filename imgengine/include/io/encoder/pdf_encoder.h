// ./include/io/encoder/pdf_encoder.h

// ================================================================
// FILE 11: include/io/encoder/pdf_encoder.h  (NEW FILE)
// ================================================================

#ifndef IMGENGINE_PDF_ENCODER_H
#define IMGENGINE_PDF_ENCODER_H

#include "core/buffer.h"
#include "core/result.h"
#include <stdint.h>

img_result_t img_encode_pdf(const img_buffer_t *buf, const char *output_path, uint32_t dpi);

#endif /* IMGENGINE_PDF_ENCODER_H */
