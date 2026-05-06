// ./include/api/io_vtable.h
// include/api/io_vtable.h  (NEW FILE)

// ================================================================
// STEP 5 OF 6: Fix api/ depending on io/ (3 violations)
// api/api.c includes io/decoder and io/encoder directly.
// Fix: register decode/encode functions at init time via function
// pointers. api/ calls them through a thin io_vtable.
// ================================================================

#ifndef IMGENGINE_API_IO_VTABLE_H
#define IMGENGINE_API_IO_VTABLE_H

/* Public compatibility shim for the I/O vtable. */
#include "io/io_vtable.h"

#endif /* IMGENGINE_API_IO_VTABLE_H */
