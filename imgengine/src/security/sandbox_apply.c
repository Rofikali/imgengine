// ./src/security/sandbox_apply.c
#define _GNU_SOURCE

#include "security/sandbox_internal.h"

#include <linux/seccomp.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>

bool img_sandbox_apply_filter(const struct sock_fprog *prog) {
    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) != 0) {
        perror("prctl PR_SET_NO_NEW_PRIVS");
        return false;
    }

    if (syscall(SYS_seccomp, SECCOMP_SET_MODE_FILTER, SECCOMP_FILTER_FLAG_TSYNC, prog) != 0) {
        perror("seccomp TSYNC");
        return false;
    }

    return true;
}
