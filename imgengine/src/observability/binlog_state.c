// ./src/observability/binlog_state.c
#define _GNU_SOURCE

#include "observability/binlog.h"

img_binlog_t g_binlog;

static __thread uint32_t tls_cpu_id = 0;

void img_binlog_set_cpu(uint32_t cpu) { tls_cpu_id = cpu; }

uint32_t img_binlog_tls_cpu_id(void) { return tls_cpu_id; }
