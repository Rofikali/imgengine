// ./tests/fuzz/target_decoder.c

// #include "api/v1/img_core.h"
// #include "security/input_validator.h"
// #include <stdint.h>
// #include <stddef.h>

// int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
// {
//     // 1. Setup a minimal engine (1 worker)
//     static img_engine_t engine = NULL;
//     if (!engine)
//     {
//         engine = img_api_init(1);
//     }

//     // 2. Fuzz the Memory Stream (The primary attack vector)
//     // This will try millions of combinations of 'data' to crash the engine
//     img_api_process_mem(engine, data, size, 256, 256);

//     return 0;
// }
