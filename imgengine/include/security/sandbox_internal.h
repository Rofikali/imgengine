// ./include/security/sandbox_internal.h
#ifndef IMGENGINE_SECURITY_SANDBOX_INTERNAL_H
#define IMGENGINE_SECURITY_SANDBOX_INTERNAL_H

#include <linux/filter.h>
#include <stdbool.h>

bool img_sandbox_build_filter(struct sock_fprog *prog);
bool img_sandbox_apply_filter(const struct sock_fprog *prog);

#endif /* IMGENGINE_SECURITY_SANDBOX_INTERNAL_H */
