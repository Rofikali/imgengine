// ./src/security/sandbox.c
// src/security/sandbox.c

#include "security/sandbox.h"
#include "security/sandbox_internal.h"

#include <linux/seccomp.h>
#include <sys/syscall.h>

/*
 * SANDBOX POLICY
 *
 * The seccomp filter is the last line of defense against
 * decoder exploits that attempt syscalls (shell, network, etc).
 *
 * DEV MODE: disabled by default to allow full engine startup.
 *           Enable IMG_SANDBOX_ENABLED=1 in production.
 *
 * PRODUCTION: enable only AFTER engine is fully initialized.
 *             Call img_security_enter_sandbox() after:
 *               - slab pools allocated (mmap done)
 *               - threads created (clone done)
 *               - files opened (openat done)
 *               - io_uring initialized (io_uring_setup done)
 *             Then lock down. Decoder runs under the filter.
 */

#ifdef IMG_SANDBOX_ENABLED

bool img_security_enter_sandbox(void) {
    struct sock_fprog prog = {0};
    if (!img_sandbox_build_filter(&prog))
        return false;
    return img_sandbox_apply_filter(&prog);
}

#else /* IMG_SANDBOX_ENABLED not set */

/*
 * DEV MODE: sandbox is a no-op.
 *
 * To enable in production:
 *   cmake .. -DIMG_SANDBOX_ENABLED=1
 * or:
 *   add_definitions(-DIMG_SANDBOX_ENABLED) in CMakeLists.txt
 *
 * IMPORTANT: call img_security_enter_sandbox() AFTER full engine
 * initialization — after threads, slabs, io_uring, and file handles
 * are all set up. The filter locks down the decoder only.
 */
bool img_security_enter_sandbox(void) { return true; /* no-op in dev mode */ }

#endif /* IMG_SANDBOX_ENABLED */
