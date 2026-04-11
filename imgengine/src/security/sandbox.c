// src/security/sandbox.c

#include "security/sandbox.h"

#include <linux/seccomp.h>
#include <linux/filter.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>

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

#define ALLOW(syscall_nr)                                  \
    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, syscall_nr, 0, 1), \
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW)

bool img_security_enter_sandbox(void)
{
    struct sock_filter filter[] = {

        /* load syscall number */
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS,
                 offsetof(struct seccomp_data, nr)),

        /* ================================================
         * ALLOWED SYSCALLS — extended for full engine ops
         * ================================================ */

        /* I/O */
        ALLOW(SYS_read),
        ALLOW(SYS_write),
        ALLOW(SYS_readv),
        ALLOW(SYS_writev),
        ALLOW(SYS_pread64),
        ALLOW(SYS_pwrite64),

        /* files (needed for image decode/encode) */
        ALLOW(SYS_openat),
        ALLOW(SYS_close),
        ALLOW(SYS_fstat),
        ALLOW(SYS_lseek),
        ALLOW(SYS_stat),

        /* memory */
        ALLOW(SYS_mmap),
        ALLOW(SYS_munmap),
        ALLOW(SYS_mprotect),
        ALLOW(SYS_mremap),
        ALLOW(SYS_brk),

        /* threads */
        ALLOW(SYS_clone),
        ALLOW(SYS_clone3),
        ALLOW(SYS_futex),
        ALLOW(SYS_set_robust_list),
        ALLOW(SYS_get_robust_list),
        ALLOW(SYS_sched_getaffinity),
        ALLOW(SYS_sched_setaffinity),

        /* io_uring */
        ALLOW(SYS_io_uring_setup),
        ALLOW(SYS_io_uring_enter),
        ALLOW(SYS_io_uring_register),

        /* time / clock */
        ALLOW(SYS_clock_gettime),
        ALLOW(SYS_clock_nanosleep),
        ALLOW(SYS_nanosleep),

        /* process */
        ALLOW(SYS_exit),
        ALLOW(SYS_exit_group),
        ALLOW(SYS_getpid),
        ALLOW(SYS_gettid),
        ALLOW(SYS_tgkill),
        ALLOW(SYS_getcpu),

        /* signals (needed by ASAN/UBSAN runtime) */
        ALLOW(SYS_rt_sigaction),
        ALLOW(SYS_rt_sigprocmask),
        ALLOW(SYS_rt_sigreturn),
        ALLOW(SYS_sigaltstack),

        /* prctl (needed to enter seccomp itself) */
        ALLOW(SYS_prctl),

        /* ================================================
         * DEFAULT DENY — kill process on unknown syscall
         * ================================================ */
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL),
    };

    struct sock_fprog prog = {
        .len = (unsigned short)(sizeof(filter) / sizeof(filter[0])),
        .filter = filter,
    };

    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) != 0)
    {
        perror("prctl PR_SET_NO_NEW_PRIVS");
        return false;
    }

    if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &prog) != 0)
    {
        perror("prctl PR_SET_SECCOMP");
        return false;
    }

    return true;
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
bool img_security_enter_sandbox(void)
{
    return true; /* no-op in dev mode */
}

#endif /* IMG_SANDBOX_ENABLED */

// // ./src/security/sandbox.c

// #include "security/sandbox.h"

// #include <linux/seccomp.h>
// #include <linux/filter.h>
// #include <sys/prctl.h>
// #include <sys/syscall.h>
// #include <unistd.h>
// #include <stddef.h>

// // 🔥 allow minimal syscalls only
// #define ALLOW(syscall)                                  \
//     BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, syscall, 0, 1), \
//         BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW)

// bool img_security_enter_sandbox(void)
// {
//     struct sock_filter filter[] = {

//         BPF_STMT(BPF_LD | BPF_W | BPF_ABS,
//                  offsetof(struct seccomp_data, nr)),

//         // ================================
//         // SAFE SYSCALLS
//         // ================================
//         ALLOW(SYS_read),
//         ALLOW(SYS_write),
//         ALLOW(SYS_exit),
//         ALLOW(SYS_exit_group),
//         ALLOW(SYS_futex),
//         ALLOW(SYS_clock_gettime),

//         // memory
//         ALLOW(SYS_mmap),
//         ALLOW(SYS_munmap),
//         ALLOW(SYS_brk),

//         // ================================
//         // DEFAULT DENY
//         // ================================
//         BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL),
//     };

//     struct sock_fprog prog = {
//         .len = (unsigned short)(sizeof(filter) / sizeof(filter[0])),
//         .filter = filter,
//     };

//     // 🔥 NO PRIVILEGE ESCALATION
//     if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0))
//         return false;

//     // 🔥 APPLY FILTER
//     if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &prog))
//         return false;

//     return true;
// }