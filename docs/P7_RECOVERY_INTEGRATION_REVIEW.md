# P7 Recovery Integration Review

## Scope and provenance

This review records the exact uncommitted P7 payload recovered in the linked
worktree `E:\imgengine-p7-frontend-api-boundary` on 2026-10-14.  The payload
was found intact in that worktree; it was not reconstructed from memory or
from a partial patch.  A rescue copy and a tar archive retaining Linux build
symlinks exist outside the repository at `E:\IMGENGINE_RESCUE_20261014-223437`.

The worktree is based directly on `origin/main` commit `3e369e8d`.  It is
therefore integrated as a reviewed recovery branch rather than transplanted
across unrelated history.  This document records review classification only;
verification results are added only after commands actually run.

No recovered implementation, test, build, or historical evidence path contains
credentials, local environment configuration, generated build output, or a
rescue archive. This recovery report alone names its Windows discovery and
rescue locations for audit traceability; those paths do not affect the build.
The private layout entry point is intentionally not exported through ABI v1.

## Path-by-path classification

| Path | Classification and purpose | Contract / dependency impact | Commit decision |
| --- | --- | --- | --- |
| `imgengine/CMakeLists.txt` | Build registration for private layout and P7.4 tests | Registers tests; no ABI symbol addition. Recovery removes one trailing-space gate violation only. | Commit |
| `imgengine/include/api/api_internal.h` | Internal borrowed-decoder ownership contract | Internal API clarification; decoder callers depend on it. | Commit |
| `imgengine/include/api/api_job_internal.h` | Legacy mmap-wrapper ownership contract | Internal historical quarantine boundary; no public ABI change. | Commit |
| `imgengine/include/imgengine/api/v1/imgengine.h` | Public borrowed-input lifetime documentation | Documentation-only ABI ownership clarification; no signature change. | Commit |
| `imgengine/include/io/io_vtable.h` | Decoder-vtable borrowed-input contract | Internal decoder implementations depend on it. | Commit |
| `imgengine/include/runtime/scheduler.h` | Scheduler close/admission lifecycle interface | Internal scheduler behavior and tests depend on it; no ABI v1 export. | Commit |
| `imgengine/include/runtime/task.h` | Atomic task states and private layout task payload | Internal task ownership state machine; no public ABI exposure. | Commit |
| `imgengine/include/runtime/worker.h` | Atomic worker state / join tracking | Internal worker lifecycle only. | Commit |
| `imgengine/src/api/api_init_workers.c` | External queue attachment and worker-start rollback | Scheduler initialization safety; internal only. | Commit |
| `imgengine/src/api/api_task_submit.c` | Submission through scheduler admission boundary | Preserves scheduler ownership linearization; internal only. | Commit |
| `imgengine/src/arch/x86_64/avx2/color_avx2.c` | SIMD-001 safe scalar fallback | Retains dispatch but makes RGB grayscale scalar-equivalent. | Commit |
| `imgengine/src/pipeline/pipeline_fuse_exec_avx2.c` | Fused SIMD-001 safe scalar fallback | Retains dispatch with scalar-equivalent semantics. | Commit |
| `imgengine/src/pipeline/pipeline_fuse_exec_scalar.c` | Correct pixel-aware RGB luma scalar reference behavior | Native rendering correctness; regression-tested. | Commit |
| `imgengine/src/runtime/queue_create.c` | SPSC capacity/rollback validation | Internal allocation safety. | Commit |
| `imgengine/src/runtime/queue_mpmc_destroy.c` | MPMC teardown zeroing | Internal cleanup safety. | Commit |
| `imgengine/src/runtime/queue_mpmc_init.c` | MPMC overflow-safe allocation | Internal allocation safety. | Commit |
| `imgengine/src/runtime/scheduler_destroy.c` | Close, drain/reject, join, and teardown ordering | Internal scheduler ownership; no scheduler redesign. | Commit |
| `imgengine/src/runtime/scheduler_init.c` | Partial-init rollback and lifecycle initialization | Internal scheduler ownership. | Commit |
| `imgengine/src/runtime/scheduler_submit.c` | Admission linearization and terminal-state wait semantics | Internal scheduler ownership; regression-tested. | Commit |
| `imgengine/src/runtime/worker_control.c` | Atomic stop and one-time join | Internal worker teardown. | Commit |
| `imgengine/src/runtime/worker_init.c` | Worker queue reuse and start tracking | Internal worker lifecycle. | Commit |
| `imgengine/src/runtime/worker_loop.c` | Queue draining, terminal publication, private layout execution | Internal execution lifecycle; depends on scheduler/task contracts. | Commit |
| `imgengine/src/tests/test_resize_kernel_equivalence.c` | Independent grayscale/SIMD regression oracle | Test only; validates SIMD-001. | Commit |
| `docs/P7_2_0_B_PRODUCT_LAYOUT_DOMAIN.md` | Product/layout domain archaeology | Planning documentation; no implementation contract addition. | Commit |
| `docs/P7_2_1_NATIVE_LAYOUT_ARCHAEOLOGY.md` | Native layout source trace | Audit documentation. | Commit |
| `docs/P7_2_2_EXACT_LAYOUT_SEMANTICS_AND_EVIDENCE_PLAN.md` | Frozen layout semantics and evidence plan | Planning/evidence documentation. | Commit |
| `docs/P7_2_3_RENDER_PLAN_CONTRACT.md` | RenderPlan contract design | Planned contract; does not expose it through ABI/HTTP. | Commit |
| `docs/P7_2_4_NATIVE_LAYOUT_CAPABILITY_DESIGN.md` | Private capability design | Design documentation. | Commit |
| `docs/P7_2_5_CAPABILITY_IMPLEMENTATION_READINESS.md` | Capability readiness review | Audit documentation. | Commit |
| `docs/P7_2_6_MINIMAL_FIT_CAPABILITY_DECISION.md` | FIT capability decision | Decision documentation. | Commit |
| `docs/P7_2_7_C_PRIVATE_IMPLEMENTATION_PLAN.md` | Private implementation plan | Design documentation. | Commit |
| `docs/P7_2_8_C_PRIVATE_LAYOUT_RENDER_IMPLEMENTATION.md` | Private layout implementation record | Documents C-private, non-ABI/non-HTTP scope. | Commit |
| `docs/P7_2_8_IMPLEMENTATION_AUDIT.md` | P7.2.8 audit | Audit documentation. | Commit |
| `docs/P7_3_C_CORE_DECISION_REGISTER.md` | P7.3 decisions including D-13 | Evidence/decision record. | Commit |
| `docs/P7_3_C_CORE_INVENTORY.md` | C-core inventory | Audit documentation. | Commit |
| `docs/P7_3_C_CORE_PRINCIPAL_AUDIT.md` | Principal C-core audit | Audit documentation. | Commit |
| `docs/P7_3_C_CORE_QUARANTINE_REGISTER.md` | Historical quarantine including Q-07 | Preserves legacy classification; no deletion. | Commit |
| `docs/P7_3_PRINCIPAL_REVIEW.md` | P7.3 review | Audit documentation. | Commit |
| `docs/P7_4_B_BORROWED_INPUT_OWNERSHIP.md` | Borrowed-input evidence | Documents ownership invariant and tests. | Commit |
| `docs/P7_4_C2_ALLOCATOR_QUEUE_OWNERSHIP.md` | Allocator/queue stress evidence | Documents evidence scope and results only when run. | Commit |
| `docs/P7_4_C_SCHEDULER_SHUTDOWN_REMEDIATION.md` | C-001/C-002 remediation record | Documents internal scheduler safety remediation. | Commit |
| `docs/P7_4_EVIDENCE_REGISTER.md` | Cross-gate evidence register | Central P7.4 audit record. | Commit |
| `docs/P7_4_SIMD_EVIDENCE.md` | SIMD-001 evidence | Documents correctness fallback and oracle. | Commit |
| `imgengine/.dockerignore` | Docker verification-context exclusion rules | Reusable build infrastructure; excludes only generated artifacts. | Commit |
| `imgengine/include/pipeline/layout_render_private.h` | Private layout-render interface | Private C interface; not installed/exported ABI. | Commit |
| `imgengine/src/pipeline/layout_render_private.c` | Private synchronous layout-render implementation | Internal scheduler/decoder/render integration; no Rust/HTTP surface. | Commit |
| `imgengine/src/tests/test_allocator_queue_ownership.c` | Allocator, slab, arena, queue stress/failure tests | Test only; uses linker wrapping where supported. | Commit |
| `imgengine/src/tests/test_borrowed_input_ownership.c` | Borrowed decoder ownership regression | Test only; validates non-release on success/failure. | Commit |
| `imgengine/src/tests/test_layout_render_private.c` | Private FIT/layout render tests | Test only; validates private capability. | Commit |
| `imgengine/src/tests/test_scheduler_shutdown.c` | Shutdown/admission linearization regression | Test only; validates C-001/C-002 remediation. | Commit |
| `imgengine/tests/abi/test_borrowed_input_ownership.c` | Public ABI borrowed-input ownership test | ABI test only; no new public symbol. | Commit |

## Explicit exclusions

No generated build directory, Docker image, Cargo target directory, rescue
archive, local environment file, or temporary Dockerfile is in this recovery
payload. The temporary P7.5 Dockerfiles and grayscale plugin change are in the
separate canonical checkout and are not part of this historical P7 recovery.

## Verification gate

The recovery must pass the unchanged Ubuntu 24.04 verifier:
`docker build -f imgengine/Dockerfile.verify -t imgengine-verify:ubuntu24 imgengine`
followed by a bind-mounted invocation from the repository root,
`docker run --rm -v "${PWD}/imgengine:/workspace" -w /workspace
imgengine-verify:ubuntu24 bash scripts/verify_linux.sh`.
Its result is recorded in the integration report only after completion.

## Recovery verification actually executed

The following commands completed successfully against the recovered worktree
in an Ubuntu 24.04 Docker container on 2026-10-14.  Build and Cargo outputs
were redirected to `/tmp` inside the container so preserved worktree build
artifacts were not overwritten.

| Gate | Command / scope | Actual result |
| --- | --- | --- |
| Focused native recovery | CMake Debug build, `format-check`, and the six recovered CTest targets | PASS: 6/6 (`resize_kernel_equivalence`, private layout, internal/ABI borrowed input, scheduler shutdown, allocator/queue) |
| Authoritative Ubuntu 24.04 | `IMGENGINE_VERIFY_BUILD_ROOT=/tmp/p7-recovery-authoritative CARGO_TARGET_DIR=/tmp/p7-recovery-cargo IMGENGINE_VERIFY_JOBS=2 IMGENGINE_FUZZ_RUNS=5000 bash scripts/verify_linux.sh` | PASS (exit 0) |
| Normal native | 13 registered CTests plus security, geometry, progressive, and output-format regressions | PASS |
| Rust and API | Rust FFI smoke/tests, supervisor tests, and API tests | PASS; API suite reported 11 passing tests |
| Real-image verification | Public ABI C/Rust fixtures, including JPEG, PNG, progressive, CMYK, malformed, and oversized-dimension inputs | PASS |
| Sanitizers | ASAN/UBSAN/leak-enabled CTest and C-only real-image verification | PASS |
| Sandbox | Configured sandbox lifecycle test | PASS |
| Fuzz | `fuzz_input_validator -runs=5000` and `fuzz_decoder -runs=5000` | PASS |

The container emitted pre-existing compiler warnings in logger formatting and
the grayscale plugin declaration, but no gate failed and no warning suppression
or behavior change was added for recovery.
