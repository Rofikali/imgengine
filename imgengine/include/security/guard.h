#ifndef IMGENGINE_SECURITY_GUARD_H
#define IMGENGINE_SECURITY_GUARD_H

#define IMG_REQUIRE(cond, err) \
    do { if (!(cond)) return (err); } while (0)

#define IMG_ASSERT(cond) \
    do { if (!(cond)) __builtin_trap(); } while (0)

#endif