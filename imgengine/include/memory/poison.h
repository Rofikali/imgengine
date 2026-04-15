// include/memory/poison.h  (NEW FILE — correct spelling)

// ================================================================
// STEP 4 OF 6: Move poison.h to memory/
// Fixes: memory depending on security (3 violations)
// ASAN poison is a memory debugging primitive, not a security one.
// ================================================================

#ifndef IMGENGINE_MEMORY_POISON_H
#define IMGENGINE_MEMORY_POISON_H

/*
 * Memory poison macros — ASAN integration.
 *
 * Moved from security/poision.h to memory/poison.h because:
 *   - memory/ (index 1) must not depend on security/ (index 2)
 *   - Poison is a memory debugging primitive, not a security policy
 *   - security/ can still use these macros by including memory/poison.h
 *
 * Note: old file was security/poision.h (typo). New file corrects spelling.
 * Old file kept as a compatibility shim below.
 */

#if defined(__has_feature)
#if __has_feature(address_sanitizer)
#include <sanitizer/asan_interface.h>
#define IMG_POISON_MEMORY(addr, size) ASAN_POISON_MEMORY_REGION(addr, size)
#define IMG_UNPOISON_MEMORY(addr, size) ASAN_UNPOISON_MEMORY_REGION(addr, size)
#endif
#endif

#ifndef IMG_POISON_MEMORY
#define IMG_POISON_MEMORY(addr, size) ((void)(addr), (void)(size))
#define IMG_UNPOISON_MEMORY(addr, size) ((void)(addr), (void)(size))
#endif

#define img_poison_block(ptr, size) IMG_POISON_MEMORY(ptr, size)
#define img_unpoison_block(ptr, size) IMG_UNPOISON_MEMORY(ptr, size)

#endif /* IMGENGINE_MEMORY_POISON_H */
