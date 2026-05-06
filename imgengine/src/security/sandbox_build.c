// ./src/security/sandbox_build.c
#define _GNU_SOURCE

#include "security/sandbox_internal.h"

#include <linux/seccomp.h>
#include <stddef.h>
#include <sys/syscall.h>

#define ALLOW(syscall_nr)                                                                          \
    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, syscall_nr, 0, 1),                                         \
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW)

/*
 * Keep the policy in a separate unit so the public entrypoint stays small.
 * The analyzer treats this as the policy owner, not the syscall installer.
 */
bool img_sandbox_build_filter(struct sock_fprog *prog) {
    static struct sock_filter filter[] = {
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS, offsetof(struct seccomp_data, nr)),
        ALLOW(SYS_read),
        ALLOW(SYS_write),
        ALLOW(SYS_readv),
        ALLOW(SYS_writev),
        ALLOW(SYS_pread64),
        ALLOW(SYS_pwrite64),
        ALLOW(SYS_openat),
        ALLOW(SYS_close),
        ALLOW(SYS_fstat),
        ALLOW(SYS_lseek),
        ALLOW(SYS_stat),
        ALLOW(SYS_mmap),
        ALLOW(SYS_munmap),
        ALLOW(SYS_mprotect),
        ALLOW(SYS_mremap),
        ALLOW(SYS_brk),
        ALLOW(SYS_clone),
        ALLOW(SYS_clone3),
        ALLOW(SYS_futex),
        ALLOW(SYS_set_robust_list),
        ALLOW(SYS_get_robust_list),
        ALLOW(SYS_sched_getaffinity),
        ALLOW(SYS_sched_setaffinity),
        ALLOW(SYS_io_uring_setup),
        ALLOW(SYS_io_uring_enter),
        ALLOW(SYS_io_uring_register),
        ALLOW(SYS_clock_gettime),
        ALLOW(SYS_clock_nanosleep),
        ALLOW(SYS_nanosleep),
        ALLOW(SYS_exit),
        ALLOW(SYS_exit_group),
        ALLOW(SYS_getpid),
        ALLOW(SYS_gettid),
        ALLOW(SYS_tgkill),
        ALLOW(SYS_getcpu),
        ALLOW(SYS_rt_sigaction),
        ALLOW(SYS_rt_sigprocmask),
        ALLOW(SYS_rt_sigreturn),
        ALLOW(SYS_sigaltstack),
        ALLOW(SYS_prctl),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL),
    };

    prog->len = (unsigned short)(sizeof(filter) / sizeof(filter[0]));
    prog->filter = filter;
    return true;
}
