// ./include/api/api_init_internal.h
#ifndef IMGENGINE_API_INIT_INTERNAL_H
#define IMGENGINE_API_INIT_INTERNAL_H

#include "api/api_internal.h"
#include "arch/cpu_caps.h"

int img_api_init_enter_sandbox(void);
int img_api_init_prepare_engine(uint32_t workers);
void img_api_init_rollback(uint32_t initialized_workers);
void img_api_init_rollback_workers(uint32_t initialized_workers);
int img_api_init_prepare_worker(uint32_t worker_id);
int img_api_init_prepare_task_queue(void);
int img_api_init_boot_workers(uint32_t workers);
void img_api_init_reset_engine(void);
void img_api_init_reset_worker_state(void);
void img_api_init_reset_task_state(void);
void img_api_init_reset_engine_state(void);

#endif /* IMGENGINE_API_INIT_INTERNAL_H */
