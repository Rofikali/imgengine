#include <stdio.h>

#include "api/v1/img_api.h"

int main(void) {
    img_engine_t *engine = img_api_init(1);
    if (!engine) {
        fprintf(stderr, "sandbox engine initialization failed\n");
        return 1;
    }

    img_api_shutdown(engine);
    puts("[sandbox] engine lifecycle passed");
    return 0;
}
