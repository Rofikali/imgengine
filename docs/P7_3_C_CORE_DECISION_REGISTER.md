# P7.3 C-Core Decision Register

| ID | Decision | Classification | Evidence / rationale | Next gate |
| --- | --- | --- | --- | --- |
| D-01 | Retain C scheduler as execution plane | KEEP | P6 characterization and current architecture; Rust owns lifecycle/admission | scheduler stress and shutdown evidence only if requirements change |
| D-02 | Keep ABI v1 frozen at seven symbols | KEEP | installed-header, export, compatibility, consumer and FFI verification | future layout needs a separately versioned capability decision |
| D-03 | Keep decoder/encoder integration in C | KEEP | codec/native path and real-image verifier | decoder resource-policy evidence |
| D-04 | Preserve slab/arena ownership contract after allocator audit | KEEP / HARDEN | P7.4-C2 source trace and normal/sanitizer/authoritative evidence cover permitted slab reuse/concurrency, arena reset/reuse, queue ownership, and initialization rollback. Arena concurrency and destruction with live borrows remain explicitly unsupported. | retain C2 regression; any expansion of arena concurrency or destruction semantics needs a separate contract and gate |
| D-05 | Keep P7.2.8 private layout capability frozen | KEEP | independent pre-JPEG oracle, sanitizer and Ubuntu verifier | no ABI/Rust/HTTP exposure without approval |
| D-06 | Do not reuse mmap-owning decode for borrowed bytes | KEEP / QUARANTINE | borrowed ABI/P7.2.8 decode uses the non-owning vtable contract; legacy file-job input remains explicitly mmap-owned and is released only by its legacy owner path | retain source trace and ownership regression; do not adapt legacy mmap release for borrowed callers |
| D-07 | Do not use float FIT or partial grid semantics for new plans | QUARANTINE | frozen checked FIT/exact completion contract | legacy retirement decision |
| D-08 | SIMD-001 scalar-equivalence correction | KEEP (verified fallback) | independent RGB oracle reproduced the divergence; AVX2/fused entry points now use scalar-equivalent RGB-luma behavior and passed focused sanitizer plus full Ubuntu verification | a future optimized RGB-deinterleaving kernel requires a new oracle/performance gate |
| D-09 | Do not set production resource limits from C audit | RESEARCH | code safety differs from product policy | measured P6/P7 policy proposal |
| D-10 | No repo-local skill yet | RESEARCH | workflows are being established; no stable repeated procedure demonstrated | reassess after P7.3 remediation cycles |
| D-11 | Scheduler shutdown resolves every accepted task before queue destruction | KEEP (verified hardening) | P7.4-C closes admission under one lock, drains accepted work before joins, rejects residual never-started work terminally, and uses C11 acquire/release lifecycle state. Focused, normal/ASAN, Rust/API, and authoritative Ubuntu evidence passed. | retain regression; future allocator/queue stress must preserve the same terminal-ownership invariant |
| D-12 | Reject unrepresentable MPMC capacities before allocation | KEEP (verified hardening) | C2 reproduced unchecked power-of-two rounding as a bounded timeout at `SIZE_MAX`; the narrow representability guard prevents wraparound and validates aligned cell storage. Focused, subsystem, and authoritative Ubuntu evidence passed. | retain regression; future queue changes preserve representability checks |
| D-13 | P7.5 found untested plugin grayscale dispatch overriding the P7.4-A-corrected kernel | BLOCKED | Engine startup calls plugin registration after hardware registration; the `OP_GRAYSCALE` plugin uses byte-wise `value * 77 >> 8`, not the documented RGB-luma formula. This is a demonstrated P7.4-A coverage gap, not an ABI or scheduler issue. | separate, minimal dispatch-wide grayscale oracle/remediation gate; preserve P7.4-A evidence and do not broaden into unrelated SIMD optimization |

## Recommended remediation order

1. MUST-FIX before public capability expansion: fully trace borrowed-input ABI
   route and document/enforce non-owning decode ownership; close SIMD-001 with
   scalar-reference evidence; establish scheduler queue/shutdown stress evidence.
2. SHOULD-FIX: allocator capacity/reuse/concurrency matrix; native logging
   redaction review; legacy/commented-code reachability inventory.
3. Remain quarantined: mmap decode wrapper, float FIT, partial grid placement.
4. Possible P7.4 scope: narrowly scoped SIMD equivalence and legacy ownership
   hardening only—no ABI expansion or architecture rewrite.
