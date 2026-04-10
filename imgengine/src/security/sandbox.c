
// ./src/security/sandbox.c



#include "security/sandbox.h"

#include <linux/seccomp.h>
#include <linux/filter.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stddef.h>

// 🔥 allow minimal syscalls only
#define ALLOW(syscall)                                  \
    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, syscall, 0, 1), \
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW)

bool img_security_enter_sandbox(void)
{
    struct sock_filter filter[] = {

        BPF_STMT(BPF_LD | BPF_W | BPF_ABS,
                 offsetof(struct seccomp_data, nr)),

        // ================================
        // SAFE SYSCALLS
        // ================================
        ALLOW(SYS_read),
        ALLOW(SYS_write),
        ALLOW(SYS_exit),
        ALLOW(SYS_exit_group),
        ALLOW(SYS_futex),
        ALLOW(SYS_clock_gettime),

        // memory
        ALLOW(SYS_mmap),
        ALLOW(SYS_munmap),
        ALLOW(SYS_brk),

        // ================================
        // DEFAULT DENY
        // ================================
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL),
    };

    struct sock_fprog prog = {
        .len = (unsigned short)(sizeof(filter) / sizeof(filter[0])),
        .filter = filter,
    };

    // 🔥 NO PRIVILEGE ESCALATION
    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0))
        return false;

    // 🔥 APPLY FILTER
    if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &prog))
        return false;

    return true;
}