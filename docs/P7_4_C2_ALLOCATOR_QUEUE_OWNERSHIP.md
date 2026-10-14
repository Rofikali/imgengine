# P7.4-C2 Allocator, Slab, and Queue Ownership Stress Gate

## Status

**CLOSED — VERIFIED.** Focused, subsystem, sanitizer, and authoritative Ubuntu
24.04 evidence passed. This document records source contracts and executed
evidence only. It does not establish product resource limits or production
readiness.

## Source-backed ownership model

| Boundary | Contract | Synchronization / owner | Teardown rule |
| --- | --- | --- | --- |
| Slab pool | `img_slab_alloc()` transfers one fixed block to the caller; `img_slab_free()` / `img_slab_recycle()` returns that exact block to the same pool. Invalid, interior, and duplicate frees are ignored. | Pool mutex serializes allocation/free and the allocation bitmap. | Pool destroy is externally synchronized: no borrower or allocator may remain. |
| Arena | `img_arena_alloc*()` returns a borrow into one arena; individual release is unsupported. `reset()` invalidates all earlier allocations and rewinds the offset. | Thread-confined / externally synchronized; source has no lock or atomic field. | Destroy invalidates all arena allocations; callers must not retain them. |
| SPSC queue | Push transfers a task pointer to exactly one consumer; pop transfers it to the consumer. | One producer and one consumer only. Scheduler admission locking serializes producers before a selected worker queue. | Destroy occurs after scheduler close/drain or residual rejection. |
| MPMC queue | Push transfers a pointer; successful pop transfers it to the popper. | Bounded Vyukov queue with atomic sequence/head/tail. | Destroy is externally synchronized after all producers/consumers stop and residual task pointers are resolved. |
| Scheduler task | `READY -> QUEUED -> RUNNING -> DONE`, or teardown `REJECTED`. | Scheduler admission lock plus C11 task-state acquire/release. | P7.4-C closes admission, drains workers, and rejects residual accepted tasks before queue release. |

`img_slab_create()` rolls back allocation bitmap, NUMA memory, and pool storage
on partial initialization failure. `img_scheduler_init()` rolls back worker
queues, MPMC queue, worker storage, and its mutex on later-stage failure.
The C2 test uses linker wrapping in its own test binary to exercise the
meaningful allocator failure points in slab, arena, SPSC, and MPMC setup;
production allocator behavior is unchanged.

## Findings

### C-003 — High: MPMC capacity rounding could not reject an unrepresentable size

Before remediation, `img_mpmc_init(SIZE_MAX)` called unchecked
next-power-of-two growth in `core/align.h`. The capacity counter wrapped to
zero and the loop did not terminate. The bounded pre-fix Docker probe timed
out after three seconds with exit 124.

The narrow fix validates representable power-of-two capacity and aligned cell
storage before allocation. The regression confirms prompt rejection. It does
not alter queue topology, ABI v1, or normal capacity semantics.

### C-004 - Moderate: SPSC construction did not reject a failed backing allocation

Before remediation, `img_queue_create()` returned a queue object even if its
`calloc` backing storage failed; later push/pop could dereference null storage.
It now rejects invalid shift width and frees the wrapper when backing allocation
fails. The C2 test-binary allocator wrapper deterministically exercises this
`calloc` failure and confirms that no partial queue object escapes.

## Evidence table

| ID | Invariant | Source path | Test or experiment | Expected | Actual | Sanitizer | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C2-01 | slab allocation/release reuses the same block | `slab_hot.c` | focused ownership test | LIFO released block reused | PASS | focused ASAN/UBSAN/leak + full Ubuntu PASS | PASS |
| C2-02 | permitted slab concurrent alloc/free has no duplicate live block | `slab_hot.c`, `slab_internal.h` | four threads x 2,000 deterministic operations | no duplicate ownership or allocation loss | PASS | focused ASAN/UBSAN/leak + full Ubuntu PASS | PASS |
| C2-03 | arena reset reuses its base and invalidates prior borrows by contract | `arena_alloc*.c`, `arena_reset.c` | focused reset/reuse test | first aligned allocation repeats after reset | PASS | focused ASAN/UBSAN/leak + full Ubuntu PASS | PASS |
| C2-04 | arena allocation is not treated as thread-safe | `arena.h`, `arena_*.c` | source trace | explicitly thread-confined/external synchronization | no concurrent test claimed | N/A | PASS (contract) |
| C2-05 | SPSC pointer ownership and wraparound are preserved | `queue_{create,push,pop}.c` | deterministic fill/pop/wraparound test | FIFO transfer, full/empty behavior | PASS | focused ASAN/UBSAN/leak + full Ubuntu PASS | PASS |
| C2-06 | MPMC supports permitted multi-producer/multi-consumer transfer | `queue_mpmc_{init,push,pop}.c` | 2 producers + 2 consumers, 2,000 unique deterministic entries | each entry observed exactly once | PASS | focused ASAN/UBSAN/leak + full Ubuntu PASS | PASS |
| C2-07 | unrepresentable MPMC capacity returns rather than loops | `queue_mpmc_init.c`, `core/align.h` | pre-fix three-second timeout, post-fix regression | prompt rejection | pre-fix exit 124; post-fix PASS | focused ASAN/UBSAN/leak + full Ubuntu PASS | PASS |
| C2-08 | partial allocator initialization rolls back safely | slab/arena/SPSC/MPMC creation sources | test-binary failure injection for malloc/calloc/aligned allocation | clean null failure without exposed partial object | PASS | focused ASAN/UBSAN/leak + full Ubuntu PASS | PASS |
| C2-09 | accepted tasks are resolved across drain/shutdown | scheduler sources | preserved `test_scheduler_shutdown` | completed or terminally rejected before release | PASS | focused ASAN/UBSAN/leak + full Ubuntu PASS | PASS |
| C2-10 | decode/render/encode failure cleanup | API/runtime stage sources | native, API, and real-image suites | no leaked owned resources | PASS | ASAN/UBSAN/leak + full Ubuntu PASS | PASS |
| C2-11 | create/destroy lifecycle repetition | allocator/scheduler lifecycle sources | existing plus C2 focused tests | no leak or stale pointer finding | PASS | focused ASAN/UBSAN/leak + full Ubuntu PASS | PASS |
| C2-12 | worker teardown does not release caller-owned task input | worker loop and scheduler teardown sources | direct worker task using a caller-owned slab block; shutdown/drain then caller write/free | task reaches terminal state; caller block remains valid and releasable by its owner | PASS | focused ASAN/UBSAN/leak + full Ubuntu PASS | PASS |

## Executed verification

- Focused normal: Ubuntu 24.04 Docker build of
  `test_allocator_queue_ownership` with deterministic allocator failure
  injection: PASS.
- Focused sanitizer: Ubuntu 24.04 Docker build with ASAN, UBSAN, and leak
  detection of `test_allocator_queue_ownership`, `test_scheduler_shutdown`,
  and `test_safety_boundaries`: PASS.
- Subsystem evidence: the native CTest/regression and sanitizer suites, plus
  affected FFI, supervisor, and API tests: PASS.
- Authoritative evidence: the unchanged
  `IMGENGINE_VERIFY_JOBS=2 IMGENGINE_FUZZ_RUNS=5000 bash scripts/verify_linux.sh`
  Ubuntu 24.04 gate: PASS, including both 5,000-run fuzz gates, real-image,
  sandbox, native sanitizer, Rust/FFI, supervisor, and API checks.

## Explicit non-claims and gaps

- No product-level memory, queue, image, or concurrency limit is inferred from
  allocator implementation capacity.
- Arena concurrent allocation, reset, or destroy is not supported and is not
  tested as such.
- Slab/queue destruction while borrowers or producers remain is not supported;
  teardown must be externally synchronized.
- Failure injection covers allocation calls reached by the C2 test binary. It
  does not claim exhaustive injection of every process-wide allocator call or
  NUMA-runtime-specific failure mode.
