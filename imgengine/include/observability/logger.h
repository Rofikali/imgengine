// ./include/observability/logger.h

// observability/logger.h

#ifndef IMGENGINE_LOGGER_H
#define IMGENGINE_LOGGER_H

#include <stdint.h>

typedef enum { LOG_DEBUG = 0, LOG_INFO, LOG_WARN, LOG_ERROR } img_log_level_t;

void img_logger_init(void);
void img_logger_shutdown(void);

void img_log_write(img_log_level_t level, const char *fmt, ...);

#endif