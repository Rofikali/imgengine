#define _GNU_SOURCE
#include "security/sandbox/sandbox.h"

#include <linux/seccomp.h>
#include <linux/filter.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <stddef.h>
#include <unistd.h>

/*
 * 🔥 ALLOW MACRO
 */
#define ALLOW(syscall)                                  \
    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, syscall, 0, 1), \
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW)

/*
 * 🔥 STRICT SANDBOX
 */
bool img_security_enter_sandbox(void)
{
    struct sock_filter filter[] = {

        BPF_STMT(BPF_LD | BPF_W | BPF_ABS,
                 offsetof(struct seccomp_data, nr)),

        // =========================
        // MINIMAL SYSCALL SET
        // =========================
        // ALLOW(SYS_read),
        // ALLOW(SYS_write),
        // ALLOW(SYS_exit),
        // ALLOW(SYS_exit_group),

        // ADD in sandbox filter

        ALLOW(SYS_openat),
        ALLOW(SYS_close),
        ALLOW(SYS_fstat),
        ALLOW(SYS_lseek),

        ALLOW(SYS_futex),
        ALLOW(SYS_clock_gettime),

        // memory
        ALLOW(SYS_mmap),
        ALLOW(SYS_munmap),
        ALLOW(SYS_brk),

        // io_uring (optional future)
        ALLOW(SYS_io_uring_enter),
        ALLOW(SYS_io_uring_setup),
        ALLOW(SYS_io_uring_register),

        // =========================
        // DEFAULT DENY
        // =========================
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL),
    };

    struct sock_fprog prog = {
        .len = sizeof(filter) / sizeof(filter[0]),
        .filter = filter,
    };

    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0))
        return false;

    if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &prog))
        return false;

    return true;
}