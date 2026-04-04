// include/io/zero_copy_contract.h

#ifndef IMGENGINE_ZERO_COPY_CONTRACT_H
#define IMGENGINE_ZERO_COPY_CONTRACT_H

/*
🔥 SYSTEM GUARANTEES

1. Decoder writes directly into slab memory
2. Pipeline NEVER copies buffers
3. Plugins mutate in-place or swap buffers
4. Encoder reads directly from buffer
5. Network layer MAY copy (boundary only)

🔥 FORBIDDEN

- memcpy in hot path
- malloc in pipeline
- intermediate buffers outside slab

*/

#endif