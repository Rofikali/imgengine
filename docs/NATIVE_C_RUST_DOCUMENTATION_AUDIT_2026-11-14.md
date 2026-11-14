# Native C17 and Rust Documentation Audit — 2026-11-14

**Audit type:** Read-only source/document reconciliation, recorded as documentation. **Base SHA:** `e0f8a32044c8c2d9e68ae37299abdedaedf318a7`. **Scope:** all 81 tracked Markdown files under `docs/` inspected for subject and status; deeper review of native/Rust HLD, LLD, architecture, security, observability, ABI, lifecycle, performance and historical evidence. **Excluded:** frontend implementation, Vue/Nuxt/TS, browser Wasm code, and deployment execution. This document does not certify code security, correctness or production readiness. **No builds, sanitizers, fuzzing, benchmarking, CI or runtime testing were executed for this docs change.**

## Evidence classification

| Status | Meaning |
| --- | --- |
| VERIFIED — recorded | Source/contract and dated test evidence at earlier revision; not freshly reproduced at audit SHA |
| IMPLEMENTED | Source or configuration exists; not a full coverage or deployment claim |
| PARTIAL | Component exists but a documented boundary, packaging or release step remains |
| PLANNED | Target/design, no current implementation assertion |
| OPEN / RISK | A question or requirement needing review or measured proof; not necessarily a proven vulnerability |
| HISTORICAL | Document accurately describes older scope, not automatically the current system |
| LEGACY | Older Python/Celery/Redis/Postgres workflow retained separately |
| MIXED / STALE | Document combines phases, outdated wording, or unverified aspirations; requires a scoped status correction |

Do not infer implementation from headings, headers, test names or source filenames. Current public contract must be checked against `imgengine/include/imgengine/api/`, `imgengine/cmake/libimgengine.map`, actual C ABI implementations, Rust safe wrapper, and revision-bound evidence.

## Priority findings: C and Rust only

| ID / priority | Finding based on inspected sources | Evidence | Status | Documentation follow-up (no code authorization) |
| --- | --- | --- | --- | --- |
| NDOC-01 / P1 | HLD calls itself “Production-ready” and describes sub-2-ms render, 100K+ ops/s, zero allocation and AVX-512 execution as general architecture. Those are RFC targets, not proven universal service or shipping guarantees. | [hld.md](hld.md), [NATIVE_BENCHMARKING.md](NATIVE_BENCHMARKING.md), [SCHEDULER_CHARACTERIZATION_REPORT.md](SCHEDULER_CHARACTERIZATION_REPORT.md) | MIXED / STALE | Mark HLD a historical target/specification; use measured controlled benchmark for performance claims. |
| NDOC-02 / P1 | LLD emphasizes internal `include/api/v1` as public. Installed ABI is `include/imgengine/api/v1` and additive `v2` interfaces; its conceptual structures/sketches are not implementation proof. | [lld.md](lld.md), [ABI.md](ABI.md), `imgengine/include/imgengine/api/` | MIXED / STALE | Make installed ABI and provenance explicit; label sketches conceptual. |
| NDOC-03 / P1 | Original PRD and some operational files describe asynchronous durable FastAPI/Redis/Celery/PostgreSQL workflows, while active first-MVP Rust path is synchronous/ephemeral. | [PRD.md](PRD.md), [OBSERVABILITY.md](OBSERVABILITY.md), [SECURITY_OPERATIONS.md](SECURITY_OPERATIONS.md), [SRE.md](SRE.md), [SAAS_CONTRACT.md](SAAS_CONTRACT.md), [architecture.md](architecture.md) | LEGACY / MIXED | Clearly scope old contracts. Do not erase migration/rollback history. |
| NDOC-04 / P1 | C logger/metrics/profiler sources exist, but legacy observability document is not the current native/Rust signal contract. `metrics.c` updates some counters atomically while min/max are plain writes; concurrent reachability and synchronization require focused review. | `imgengine/src/observability/metrics.c`, `logger_write.c`, [P7_5_DIAGNOSTIC_OBSERVABILITY_POLICY.md](P7_5_DIAGNOSTIC_OBSERVABILITY_POLICY.md) | IMPLEMENTED + OPEN / RISK | Source-trace writers/readers, identify data-race risk without claiming a reproduced race; define safe metrics ownership, time units, buffering, redaction and overhead. |
| NDOC-05 / P1 | `security.md` describes decoder execution in a separate non-root networkless process; native seccomp code exists, but universal child-process isolation is not established by source excerpt or Docker config alone. | [security.md](security.md), [threat-model.md](threat-model.md), `imgengine/src/security/sandbox_apply.c`, [P14_PRODUCTION_BASELINE.md](P14_PRODUCTION_BASELINE.md) | PLANNED + PARTIAL | Split target mitigations from verified per-entry-point process/isolation behavior. |
| NDOC-06 / P1 | Encoded-output budget does not cap decoded buffers, canvases, intermediates, Rust copies, aggregate RSS or disk; P14 real host evidence is open. | [P9_3_BOUNDED_OUTPUT_API_DESIGN.md](P9_3_BOUNDED_OUTPUT_API_DESIGN.md), [MVP_RELEASE_CHECKLIST.md](MVP_RELEASE_CHECKLIST.md), [P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md](P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md) | PARTIAL / OPEN | Define native/Rust resource accounting and exact host evidence before capacity claims. |
| NDOC-07 / P1 | Rust safe wrapper is intentionally `!Send + !Sync`; C engine single-active/process scope and operation serialization remain. Client abandonment/deadline is not native cancellation. | [RUST_FFI.md](RUST_FFI.md), [RUST_REQUEST_LIFECYCLE.md](RUST_REQUEST_LIFECYCLE.md), [RUST_SUPERVISOR.md](RUST_SUPERVISOR.md) | VERIFIED — recorded | Preserve ownership, Rust auto-traits, one-engine and cancellation limitations in HLD/LLD. |
| NDOC-08 / P1 | The shared `.so` has explicit export map, installed headers and ABI 1.0–2.3; a public installed `libimgengine.a` with the same ABI is not established by CMake's internal static target. | `imgengine/CMakeLists.txt` (internal STATIC vs public SHARED), [ABI.md](ABI.md) | PARTIAL | Record packaging design and third-party dependency/license closure, without promising static SDK support. |
| NDOC-09 / P1 | Root `LICENSE` is missing in audited tree despite historic MIT reference; upstream fork provenance and TurboJPEG/stb/dependency obligations require verification before SDK commercialization. | [PRINCIPAL_AUDIT_2026-10-31.md](PRINCIPAL_AUDIT_2026-10-31.md), root GitHub tree | OPEN / RISK | Owner/legal review; do not generate or assume a license. |
| NDOC-10 / P1 | Redis queue/job SRE and database recovery remain relevant only to legacy deployment, not synchronous Rust path. Their controls are not inherently fulfilled by Rust. | [DATABASE_RECOVERY.md](DATABASE_RECOVERY.md), [SRE.md](SRE.md), [OBSERVABILITY.md](OBSERVABILITY.md) | LEGACY | Add clear scope navigation, retain original evidence. |
| NDOC-11 / P2 | Design-patterns file includes sketches (Strategy, Factory, Observer, etc.) without proof of current code-level adoption. SOLID/DRY/DDD/hexagonal descriptions should be design heuristics, not compliance badges. | [design_patterns.md](design_patterns.md), [hld.md](hld.md), [lld.md](lld.md) | MIXED / STALE | Create source-backed pattern-to-symbol/evidence map, document trade-offs and hot-path cost. |
| NDOC-12 / P2 | P7.4 grayscale/borrowed input/scheduler/allocator closure was historically verified, and P7.5 corrected live plugin dispatch and per-request stderr in documented scope; stale P7.3 “blocked” rows must not override later closure. | [P7_4_EVIDENCE_REGISTER.md](P7_4_EVIDENCE_REGISTER.md), [P7_5_BRANCH_CONSOLIDATION.md](P7_5_BRANCH_CONSOLIDATION.md), [P7_5_GRAYSCALE_DISPATCH_EVIDENCE.md](P7_5_GRAYSCALE_DISPATCH_EVIDENCE.md) | VERIFIED — recorded | Link superseding closure, retain dated archaeology. Separate uncommitted local P7.5 worktree cannot be inspected via GitHub. |
| NDOC-13 / P2 | Native benchmark harness distinguishes render/decode/encode and optimized/scalar profiles, yet no universal nanosecond or throughput guarantee follows. | [NATIVE_BENCHMARKING.md](NATIVE_BENCHMARKING.md), [performance.md](performance.md) | IMPLEMENTED + OPEN | Require sample sizes, overhead calibration, pinned CPU, cache/memory, p50/p95/p99 and image fidelity; explicitly separate ns-scale primitive from ms-scale full image. |
| NDOC-14 / P2 | M0 strict C17 and C11 compatibility evidence are recorded, C23 isolated. Some old docs still say C11 is shipping or older PR not merged. | [M0_C17_PORTABILITY_AND_WASM_ADR.md](M0_C17_PORTABILITY_AND_WASM_ADR.md), [P12_2_ABI_2_2_CROP_DESIGN.md](P12_2_ABI_2_2_CROP_DESIGN.md) | VERIFIED — recorded + HISTORICAL | Prefer dated supersession notes to rewriting historical test results. |
| NDOC-15 / P2 | `docs/README.md` included literal `\\n` separators in table/navigation (introduced during prior docs update). This hurts rendering/navigation. | [README.md](README.md) | DOC DEFECT | Replace literal escape sequences with real Markdown newlines. |
| NDOC-16 / P2 | Change-scoped tests are written guidance; existing CI is unchanged. Native safety gates apply when C/FFI/security-boundary changes; documentation-only change needs no C compile manually. | [CHANGE_SCOPED_VERIFICATION.md](CHANGE_SCOPED_VERIFICATION.md) | POLICY DOCUMENTED, CI OPTIMIZATION NOT IMPLEMENTED | Do not claim CI path-filtering, disabled sanitizers, or waived release gates. |

## Verified or completed *within stated scopes*

- C17 portable shipping baseline, C11 whole-core compatibility regression, C23 containment — recorded M0 Ubuntu 24.04 evidence; not a C23 feature claim.
- Public C ABI v1 with additive ABI 2.0–2.3 exports; installed-header, export, ownership and ABI consumer tests recorded.
- Retained C scheduler; Rust supervisor, bounded admission, request lifecycle and narrow Axum adapters; synchronous JPEG/P8/P12 routes recorded.
- P7.4 borrowed-input ownership, scheduler shutdown, allocator/queue and grayscale correctness remediation in their tested usage; P7.5 dispatch and ABI request logging corrections.
- Native CTest, fuzz/sanitizer/real-image and sandbox test infrastructure; existing evidence is revision- and environment-specific.
- Rust `imgengine` FFI wrapper is `!Send/!Sync`, returns Rust-owned output and does not claim native mid-operation cancellation.
- Native diagnostics ABI-v1 no longer emit the former per-request stderr marker on the tested path.
- Current `libimgengine.so` plus internal `libimgengine_internal.a` build targets. Public static SDK remains separate.

## Partial, incomplete or unverified

- Fully bounded total native/Rust memory/disk/CPU budgets for hostile and concurrent images.
- Real-host P14 isolation, peak RSS, TLS/firewall, monitoring, rollback and public release evidence.
- Native metrics min/max atomicity and concurrent caller reachability; logging ring-buffer overflow and privacy across all call sites.
- Dedicated native/Rust observability contract with cardinality, sampled measurements and tested exporter behavior.
- Independently distributed public `.a`, portable binary CPU baseline and dependency/license provenance.
- General-purpose crop/compress/convert native APIs beyond existing sheet/encode capability.
- Per-request Rust FFI version-specific documentation for ABI 2.2/2.3: native C 2.3 exists, but P13 Rust/HTTP wiring remains unimplemented.
- Native hard-cancel/interruption, arbitrary multi-engine concurrency and universal zero-allocation.
- Older LLD/HLD source-to-contract synchronization, including benchmark targets and public header ownership.
- Rust tenant quota/paid API identity and billing remain future features, not C-core responsibilities.
- Separate dirty local P7.5 worktree, inaccessible through tracked GitHub.

## Native/Rust-specific evidence and design rules

- **HLD:** current stable interfaces and trust boundaries, process/thread topology, known limits, alternatives and dated ADRs. Keep old RFC intentions in historical sections only.
- **LLD:** file/function names, owned/borrowed pointers, structs and `struct_size` versioning, message/control-flow, locks and atomics, allocation/cleanup/error paths, performance budget and tests.
- **Security:** threat actor/input surface, mitigation status, current/native vs Rust ownership, residual risk, test/host evidence and open risk.
- **Observability:** C kernel measurement and return codes vs Rust request/admission telemetry; protect image contents, secrets, filenames and paths. Never add a log inside a per-pixel hot loop.
- **DSA:** SPSC/MPMC queues, ring capacity/backpressure, arena/slab lifetimes, SIMD dispatch selection, checked geometry, memory bandwidth and algorithm complexity. Add source-linked invariants instead of abstract praise.
- **Patterns:** use Strategy for validated CPU dispatch, Adapter/Facade for Rust FFI, ownership-safe Factory for native handle creation, and a bounded producer-consumer model only where current source proves them. Do not mandate every GoF pattern or pretend global registry = harmless Singleton.
- **Performance:** distinguish CPU cycles/ns for tiny dispatch from microseconds for prepared small kernels and milliseconds for full images. Preserve output equality and safety before optimizing. Benchmark on controlled hardware.
- **Business:** record SDK support cost, image/codec licensing, verification expense, hosting constraints and customer value before adding complexity. Unsubstantiated “nanosecond image pipeline” claims must not reach marketing.

## Document governance and next actions

1. Read [NATIVE_C_RUST_DOCS_MAP.md](NATIVE_C_RUST_DOCS_MAP.md) for canonical/historical document roles.
2. Correct presentation/status warnings in existing docs without erasing prior designs.
3. Add source-backed native observability and resource-cap contracts in a **separate approved engineering milestone**.
4. Keep `next.md` open items and `skills.md` review recipes current; root/nested `AGENTS.md` remain normative agent policy.
5. Do not merge documentation without human review. This audit itself changes no C, Rust, frontend, CMake, tests, workflows or build outputs.
