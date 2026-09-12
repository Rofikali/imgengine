#include <stdint.h>
#include <stdio.h>

#include "core/result.h"
#include "memory/arena.h"
#include "memory/slab.h"
#include "runtime/queue_mpmc.h"
#include "runtime/scheduler.h"
#include "security/input_validator.h"

static int expect_result(const char *name, img_result_t actual, img_result_t expected) {
    if (actual == expected)
        return 0;
    fprintf(stderr, "%s: expected %s, got %s\n", name, img_result_name(expected),
            img_result_name(actual));
    return 1;
}

static int test_input_validation(void) {
    int failures = 0;

    failures += expect_result("minimal valid", img_security_validate_request(1, 1, 1), IMG_SUCCESS);
    failures +=
        expect_result("zero width", img_security_validate_request(0, 1, 1), IMG_ERR_SECURITY);
    failures +=
        expect_result("zero file", img_security_validate_request(1, 1, 0), IMG_ERR_SECURITY);
    failures += expect_result("dimension cap", img_security_validate_request(16385, 1, SIZE_MAX),
                              IMG_ERR_SECURITY);
    failures += expect_result("compression ratio", img_security_validate_request(1000, 1, 2),
                              IMG_ERR_SECURITY);
    failures += expect_result("compression ratio boundary",
                              img_security_validate_request(1000, 1, 3), IMG_SUCCESS);
    failures += expect_result("large file arithmetic",
                              img_security_validate_request(1, 1, SIZE_MAX), IMG_SUCCESS);
    return failures;
}

static int test_slab_lifecycle(void) {
    if (img_slab_create(0, 64) != NULL || img_slab_create(64, 0) != NULL ||
        img_slab_create(64, 128) != NULL || img_slab_create(SIZE_MAX, 64) != NULL) {
        fprintf(stderr, "slab rejected invalid geometry incorrectly\n");
        return 1;
    }

    img_slab_pool_t *pool = img_slab_create(1024, 256);
    if (!pool || img_slab_block_size(pool) != 256) {
        fprintf(stderr, "slab creation failed\n");
        img_slab_destroy(pool);
        return 1;
    }

    uint8_t *blocks[4] = {0};
    for (size_t index = 0; index < 4; index++) {
        blocks[index] = img_slab_alloc(pool);
        if (!blocks[index]) {
            fprintf(stderr, "slab exhausted too early\n");
            img_slab_destroy(pool);
            return 1;
        }
    }
    if (img_slab_alloc(pool) != NULL) {
        fprintf(stderr, "slab exceeded configured capacity\n");
        img_slab_destroy(pool);
        return 1;
    }
    for (size_t index = 0; index < 4; index++)
        img_slab_free(pool, blocks[index]);
    uint8_t foreign_block[256] = {0};
    img_slab_free(pool, foreign_block);
    img_slab_free(pool, blocks[0]);
    img_slab_free(pool, blocks[0] + 1);
    for (size_t index = 0; index < 4; index++) {
        if (!img_slab_alloc(pool)) {
            fprintf(stderr, "slab did not recycle a released block\n");
            img_slab_destroy(pool);
            return 1;
        }
    }
    img_slab_destroy(pool);
    return 0;
}

static int test_arena_boundaries(void) {
    if (img_arena_create(0) != NULL || img_arena_create(SIZE_MAX) != NULL) {
        fprintf(stderr, "arena accepted invalid size\n");
        return 1;
    }

    img_arena_t *arena = img_arena_create(128);
    if (!arena || !img_arena_alloc(arena, 64) || img_arena_alloc(arena, SIZE_MAX) ||
        img_arena_alloc_aligned(arena, 1, 3) || img_arena_alloc_aligned(arena, 1, 256) ||
        img_arena_alloc_aligned(arena, SIZE_MAX, 64)) {
        fprintf(stderr, "arena boundary validation failed\n");
        img_arena_destroy(arena);
        return 1;
    }
    img_arena_destroy(arena);
    return 0;
}

static int test_scheduler_overflow_queue(void) {
    img_scheduler_t scheduler = {0};
    img_task_t tasks[1024] = {0};

    if (img_scheduler_init(&scheduler, 1) != 0) {
        fprintf(stderr, "scheduler initialization failed\n");
        return 1;
    }

    int failures = 0;
    for (size_t index = 0; index < 1024; index++) {
        if (img_scheduler_submit(&scheduler, &tasks[index]) != 0) {
            fprintf(stderr, "scheduler rejected task %zu\n", index);
            failures = 1;
            break;
        }
    }
    if (!failures && img_mpmc_pop(&scheduler.global_queue) != &tasks[1023]) {
        fprintf(stderr, "scheduler did not route overflow task to global queue\n");
        failures = 1;
    }

    img_scheduler_destroy(&scheduler);
    return failures;
}

int main(void) {
    int failures = test_input_validation() + test_slab_lifecycle() + test_arena_boundaries() +
                   test_scheduler_overflow_queue();
    if (failures != 0)
        return 1;
    printf("[safety] input and slab boundary tests passed\n");
    return 0;
}
