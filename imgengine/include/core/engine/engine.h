#ifndef IMGENGINE_ENGINE_H
#define IMGENGINE_ENGINE_H

#include <stdint.h>

typedef struct img_engine img_engine_t;

img_engine_t *img_engine_create(uint32_t workers);
void img_engine_destroy(img_engine_t *e);

#endif