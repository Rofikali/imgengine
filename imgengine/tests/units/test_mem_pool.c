// ./tests/units/test_mem_pool.c
// tests/units/mem_pool.c

#include "imgengine/memory_pool.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/**
 * @brief Professional test suite for mem_pool_t
 * Focus: SIMD Alignment, OOM handling, and Offset Reset.
 */

void test_initialization() {
    mem_pool_t pool;
    size_t size = 1024;
    mp_init(&pool, size);

    assert(pool.buffer != NULL);
    assert(pool.size == size);
    assert(pool.offset == 0);

    mp_destroy(&pool);
    printf("✅ PASS: Initialization\n");
}

void test_alignment_and_offset() {
    mem_pool_t pool;
    mp_init(&pool, 1024);

    // AVX2 requires 32-byte alignment for instructions like _mm256_load_si256
    // We test if mp_alloc handles the padding internally.
    void *ptr1 = mp_alloc(&pool, 7);  // Request odd size
    void *ptr2 = mp_alloc(&pool, 12); // Request another small size

    assert(((uintptr_t)ptr1 % 32) == 0);
    assert(((uintptr_t)ptr2 % 32) == 0);

    // Check that ptr2 actually comes after ptr1 + padding
    assert((uintptr_t)ptr2 > (uintptr_t)ptr1);

    mp_destroy(&pool);
    printf("✅ PASS: 32-Byte SIMD Alignment\n");
}

void test_out_of_memory() {
    mem_pool_t pool;
    mp_init(&pool, 64);

    void *ptr1 = mp_alloc(&pool, 32);
    assert(ptr1 != NULL);

    // This should fail because 32 (used) + 32 (padding/alignment) + 40 > 64
    void *ptr2 = mp_alloc(&pool, 40);
    assert(ptr2 == NULL);

    mp_destroy(&pool);
    printf("✅ PASS: OOM Handling\n");
}

void test_reset_logic() {
    mem_pool_t pool;
    mp_init(&pool, 1024);

    mp_alloc(&pool, 100);
    size_t offset_before = pool.offset;
    assert(offset_before > 0);

    mp_reset(&pool);
    assert(pool.offset == 0);

    // Verify we can re-allocate the full size
    void *ptr = mp_alloc(&pool, 1024);
    assert(ptr != NULL);

    mp_destroy(&pool);
    printf("✅ PASS: Pool Reset\n");
}

int main() {
    printf("--- Starting mem_pool_t Unit Tests ---\n");

    test_initialization();
    test_alignment_and_offset();
    test_out_of_memory();
    test_reset_logic();

    printf("--- All Memory Pool Tests Passed ---\n");
    return 0;
}
