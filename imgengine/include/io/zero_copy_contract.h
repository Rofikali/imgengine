// ./include/io/zero_copy_contract.h

// include/io/zero_copy_contract.h

#ifndef IMGENGINE_ZERO_COPY_CONTRACT_H
#define IMGENGINE_ZERO_COPY_CONTRACT_H

/*
================= L8 MEMORY CONTRACT =================

✔ Decoder → writes directly into slab
✔ Pipeline → operates in-place
✔ Batch → references slab buffers only
✔ Encoder → reads directly

================= HARD RULES =================

❌ malloc/free in hot path
❌ memcpy in pipeline
❌ unbounded allocations
❌ hidden ownership

================= REQUIRED =================

✔ slab block size >= max image size
✔ explicit ownership transfer
✔ bounds check before every decode

*/

#endif