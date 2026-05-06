// ./include/api/v1/img_error.h

// ================================================================
// Keeps backward compatibility for any external consumer.
// ================================================================

#ifndef IMGENGINE_API_ERROR_H
#define IMGENGINE_API_ERROR_H

/*
 * img_result_t lives in core/result.h.
 * This header re-exports it for API consumers.
 * Internal code must include "core/result.h" directly.
 */
#include "core/result.h"

#endif /* IMGENGINE_API_ERROR_H */
