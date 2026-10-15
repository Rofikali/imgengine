# P7.4 Evidence Register

| ID | Workstream | Owner | Risk | Claim | Evidence Level | Artifact | Command/Test | Expected | Actual | Decision | Status |
| -- | ---------- | ----- | ---- | ----- | -------------- | -------- | ------------ | -------- | ------ | -------- | ------ |
| SIMD-001-E01 | SIMD-001 | OWNER-C | correctness | Scalar RGB luma is the reference semantic | E3 | `P7_4_SIMD_EVIDENCE.md` | `test_resize_kernel_equivalence` | Scalar equals oracle | PASS | fix decision pending authorization | PASS |
| SIMD-001-E02 | SIMD-001 | OWNER-C | correctness | AVX2/fused paths match RGB luma | E3 | `P7_4_SIMD_EVIDENCE.md` | deterministic oracle matrix | scalar/AVX2/fused match | pre-fix divergence reproduced; corrected paths match | MINIMAL CORRECTNESS FIX | PASS |
| SIMD-001-E03 | SIMD-001 | OWNER-SEC / OWNER-OPS | memory safety | Corrected paths have no sanitizer/leak finding | E4 | `P7_4_SIMD_EVIDENCE.md` | focused ASAN/UBSAN/leak command | no finding | PASS | pending full gate | PASS |
| SIMD-001-E04 | SIMD-001 | OWNER-OPS | reproducibility | Corrected tree passes the complete Ubuntu gate | E5 | `P7_4_SIMD_EVIDENCE.md` | `IMGENGINE_VERIFY_JOBS=2 IMGENGINE_FUZZ_RUNS=5000 bash scripts/verify_linux.sh` | all canonical stages pass | exit 0; format, normal/ASAN CTest, ABI/FFI/Rust, real images, sandbox, fuzz passed | correction verified | PASS |
| OWN-001-E01 | P7.4-B | OWNER-C | ownership | Borrowed decoder input is never released by native decode | E3 | `P7_4_B_BORROWED_INPUT_OWNERSHIP.md` | `test_borrowed_input_ownership` | valid and malformed mappings remain live and byte-identical | focused normal test passed | invariant enforced at boundary | PASS |
| OWN-001-E02 | P7.4-B | OWNER-C | queued lifetime | Queued private render receives decoded slab-owned data, not the encoded borrow | E3 | `P7_4_B_BORROWED_INPUT_OWNERSHIP.md` | guarded-input queued render case | render succeeds while encoded mapping remains caller-owned | focused normal test passed | no encoded borrow crosses queue | PASS |
| OWN-001-E03 | P7.4-B | OWNER-SEC | negative release detection | Guard detects legacy mmap release without post-release dereference | E3 | `P7_4_B_BORROWED_INPUT_OWNERSHIP.md` | `mincore` guard plus legacy release case | mapping becomes non-live only through legacy owner release | focused normal test passed | regression mechanism validated | PASS |
| OWN-001-E04 | P7.4-B | OWNER-SEC / OWNER-OPS | memory safety | Ownership regression passes with ASAN, UBSAN, and leak detection | E4 | `P7_4_B_BORROWED_INPUT_OWNERSHIP.md` | focused sanitizer command | no finding | focused sanitizer test passed | full gate pending | PASS |
| OWN-001-E05 | P7.4-B | OWNER-OPS | reproducibility | Corrected tree passes complete Ubuntu verification | E5 | `P7_4_B_BORROWED_INPUT_OWNERSHIP.md` | `IMGENGINE_VERIFY_JOBS=2 IMGENGINE_FUZZ_RUNS=5000 bash scripts/verify_linux.sh` | all canonical stages pass | exit 0; format, normal/ASAN CTest, ABI/FFI/Rust/API, real images, sandbox, and both fuzz targets passed | invariant verified | PASS |
| SCH-001-E01 | P7.4-C | OWNER-C | shutdown ownership | C-001: accepted queued tasks retain a terminal completion path through teardown | E5 | `P7_4_C_SCHEDULER_SHUTDOWN_REMEDIATION.md` | `test_scheduler_shutdown`; full Ubuntu verifier | every accepted task executes or becomes terminally rejected before queue release | PASS: close/drain joins workers; residual queues publish `REJECTED` before release | narrow remediation verified | PASS |
| SCH-001-E02 | P7.4-C | OWNER-C | memory model | C-002: task/worker lifecycle coordination has defined C11 synchronization | E5 | `P7_4_C_SCHEDULER_SHUTDOWN_REMEDIATION.md` | atomic source review; normal/ASAN tests; full Ubuntu verifier | acquire/release synchronization protects task terminal state and worker stop/drain state | PASS: task and worker lifecycle fields use C11 atomics with terminal release/acquire publication | narrow remediation verified | PASS |
| C2-001-E01 | P7.4-C2 | OWNER-C | queue capacity | C-003: unrepresentable MPMC capacity is rejected without non-terminating rounding | E5 | `P7_4_C2_ALLOCATOR_QUEUE_OWNERSHIP.md` | pre-fix timeout + focused regression + full Ubuntu verifier | prompt rejection | pre-fix exit 124; fixed normal/ASAN and authoritative Ubuntu PASS | narrow fix verified | PASS |
| C2-001-E02 | P7.4-C2 | OWNER-C | allocator/queue ownership | slab, arena, SPSC, MPMC, worker teardown, and shutdown contracts are source-traced and tested | E5 | `P7_4_C2_ALLOCATOR_QUEUE_OWNERSHIP.md` | deterministic reuse/concurrency/failure-injection/worker-teardown regression + full Ubuntu verifier | no duplicate, loss, premature release, or leak finding in permitted use | focused/subsystem ASAN and authoritative Ubuntu PASS | C2 verified | PASS |

## P7.4-D cross-gate reconciliation (read-only audit)

This reconciliation records the state inspected after P7.4-A, B, C1, and C2.
It does not add a production-readiness claim, rerun a verifier, or supersede
the historical findings in the P7.3 principal audit.

### Closure matrix

| Gate | Reported status | Evidence inspected | Conclusion | Remaining limitation |
| --- | --- | --- | --- | --- |
| P7.4-A | CLOSED / VERIFIED | `P7_4_SIMD_EVIDENCE.md`; scalar/AVX2/fused source and oracle test registration; E3-E5 rows above | Reconciled. The demonstrated RGB-luma divergence is addressed by a scalar-equivalent fallback behind unchanged entry points. | Optimized SIMD restoration and its performance characterization are deferred. |
| P7.4-B | CLOSED / VERIFIED | `P7_4_B_BORROWED_INPUT_OWNERSHIP.md`; non-owning decoder and legacy mmap-owner sources; ownership-test registration; OWN E3-E5 rows above | Reconciled. Current borrowed paths use `img_api_decode_prepared`; legacy mmap release remains quarantined. | Future decoder-vtable implementations must retain the non-owning contract; legacy file routes are not modernized. |
| P7.4-C1 | CLOSED / VERIFIED | `P7_4_C_SCHEDULER_SHUTDOWN_REMEDIATION.md`; scheduler submit/close/destroy, worker loop, task-state source; shutdown-test registration; SCH E5 rows above | Reconciled. Accepted tasks have a terminal execution or rejection path before queue storage is destroyed under the documented non-concurrent shutdown contract. | No claim is made for concurrent scheduler destruction or native cancellation. |
| P7.4-C2 | CLOSED / VERIFIED | `P7_4_C2_ALLOCATOR_QUEUE_OWNERSHIP.md`; slab, arena, SPSC/MPMC source and focused-test registration; C2 E5 rows above | Reconciled. Permitted allocator/queue ownership, failure rollback, and worker-teardown paths have identified evidence. | Arena concurrent use and destruction with live borrows remain unsupported; bounded tests do not prove every allocator interleaving. |

### Cross-boundary invariant matrix

| ID | Invariant | Paths inspected | Evidence | Status | Gap |
| --- | --- | --- | --- | --- | --- |
| D-INV-01 | Encoded borrowed bytes are not ownership-released by decode or queued work. | ABI/raw and private-render decode -> decoder vtable -> decoded slab buffer -> worker cleanup | OWN-001 E01-E05; non-owning/legacy source trace | VERIFIED | Future vtable additions require the same review. |
| D-INV-02 | A scheduler-accepted task reaches one terminal state before queue destruction. | submit/close -> SPSC or MPMC -> worker or teardown rejection -> waiter | SCH-001 E01-E02; C2-12 | VERIFIED | Shutdown remains explicitly non-concurrent. |
| D-INV-03 | Terminal task status is published with defined synchronization. | task state/status, worker running state, submit/wait | SCH-001 E02; atomic source inspection | VERIFIED | This is not a general race-freedom proof. |
| D-INV-04 | Slab allocation/free is mutex-protected and reuse is only within a live pool. | slab alloc/free -> raw-buffer release -> pool destruction | C2-001 E02 | VERIFIED for permitted use | Destroy with borrowers is unsupported. |
| D-INV-05 | Arena allocations are thread-confined/external-sync borrows invalidated by reset/destroy. | arena allocation/reset/destroy and private render scratch use | C2 source trace and C2-03/C2-04 | SUPPORTED RESTRICTION | No concurrent arena contract or test is claimed. |
| D-INV-06 | Queue initialization rejects impossible capacities and partial initialization does not escape. | SPSC/MPMC initialization and scheduler rollback | C2-001 E01-E02 | VERIFIED | Failure injection is bounded to meaningful test-reachable calls. |
| D-INV-07 | Private renderer owns decoded/canvas/output storage without public ABI exposure. | private render decode -> stack task/wait -> state/canvas/output release | P7.2.8 final re-audit, OWN-001 E02, authoritative-gate records | VERIFIED for private capability | No public ABI/Rust/HTTP contract follows. |

### Residual findings and next decision

| ID | Severity | Classification | Observed fact / affected path | Smallest justified next action |
| --- | --- | --- | --- | --- |
| D-R01 | Moderate | Supported limitation | Arena allocation/reset/destroy is deliberately thread-confined or externally synchronized; live-borrow destruction is unsupported. | Do not expand its contract. If a future feature needs concurrent arena ownership, authorize a separate contract/design gate first. |
| D-R02 | Low | Evidence limitation | SIMD entries are semantically correct through a scalar fallback; performance impact was intentionally not characterized. | Only if optimization is required, run a separate oracle-plus-benchmark gate for a new vector kernel. |
| D-R03 | Low | Quarantined legacy risk | The mmap-owning decode wrapper and legacy file routes remain reachable only through their documented legacy ownership model. | Preserve quarantine; require an adapter/ownership regression before any new caller. |
| D-R04 | Low | Research gap | Historical commented worker alternatives and split queue topology remain archaeology/maintenance concerns, not demonstrated runtime defects. | Separate reachability/cleanup decision; do not delete as part of a correctness gate. |

**P7.4-D status: COMPLETE — READY WITH EXPLICIT LIMITATIONS.** The four P7.4 closure claims
reconcile against the inspected source, registered regressions, and recorded
Ubuntu 24.04 authoritative-gate evidence. The smallest justified next gate is
not public layout exposure or an allocator redesign: it is a separately
authorized native observability/redaction and historical-reachability audit,
with entry criteria that quarantine records remain preserved, no ABI v1 change
is proposed, and any demonstrated defect stops the audit before remediation.

## P7.5 partial audit — blocked finding

| ID | Classification | Source-inspected fact | Affected context | Required next action |
| --- | --- | --- | --- | --- |
| P75-001 | Demonstrated correctness regression / P7.4-A coverage gap | `img_engine_init_subsystems()` performs hardware registration and then plugin registration. `plugin_grayscale.c` registers `OP_GRAYSCALE` with byte-wise `value * 77 >> 8`; compiled pipelines and RPC dispatch call the live jump table. This differs from the RGB-luma reference established by P7.4-A. | Legacy/internal compiled pipeline and RPC operation dispatch. The current ABI-v1 JPEG service path was not shown to construct that pipeline. | Stop before remediation. Authorize a narrow dispatch-wide grayscale oracle/remediation gate, with plugin-route regression evidence and the normal/sanitizer/authoritative escalation required by its scope. |
| P75-002 | Demonstrated observability policy gap; no direct sensitive-data exposure demonstrated | `img_api_process_raw()` is reached by ABI v1 and writes failure classifications to `stderr`. It prints fixed labels and result codes, not input bytes, paths, keys, or headers. | ABI-backed Rust/HTTP native processing can emit process diagnostics to the hosting environment rather than only returning a mapped status. | After P75-001 is resolved, authorize a separate observability policy/remediation decision if service-side native stderr must be eliminated or centrally redacted. |

**P7.5 status: BLOCKED.** The audit stopped after P75-001. No assertion is
made that the complete observability or historical-reachability inventory is
finished, and no remediation was performed.
