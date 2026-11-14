# IMGENGINE native C17 / Rust: contract-to-evidence and engineering review matrix

**Status:** Documentation-only reconciliation of the current source and existing documentation. **Baseline:** main at 2a92166006e0160df0503319176a6b29b28e183d, 2026-11-14. **No builds/tests performed for this report.** This matrix supplements, and does not replace, the already-merged [81-file documentation map](NATIVE_C_RUST_DOCS_MAP.md) and [dated findings register](NATIVE_C_RUST_DOCUMENTATION_AUDIT_2026-11-14.md).

**Scope:** C17 core; shared/static library packaging; Rust safe FFI, Rust admission/lifecycle and Axum; native security, architecture, observability, data structures, algorithms, and performance. No frontend, browser Wasm, migration execution or implementation change.

## Status rules

- **IMPLEMENTED**: source and interfaces are present; not blanket proof of safety, release or performance.
- **RECORDED VERIFIED**: a dated report records test success for a specific narrow usage and revision, not rerun in this PR.
- **PARTIAL**: lower-level implementation exists, but an advertised SDK/public operation or validation boundary is incomplete.
- **OPEN / REQUIRES EVIDENCE**: an explicit prerequisite is not demonstrated.
- **DESIGN / HISTORICAL / LEGACY**: intended behavior, older system, or prior evidence; do not treat as shipping current behavior.

## Native and Rust high-level design (source-aligned)

| Boundary | Owner | Existing entry/source | Contract / limitation |
| --- | --- | --- | --- |
| Rust HTTP | Rust API | imgengine/rust/imgengine-api/src/lib.rs | API ingress, authentication, multipart validation, safe errors; bounded synchronous operation |
| Rust lifecycle | Rust | imgengine/rust/imgengine-supervisor/src/request_lifecycle.rs | Request ownership, graceful drain and response abandonment; no native mid-operation interruption |
| Rust admission | Rust | imgengine/rust/imgengine-supervisor/src/admission.rs | Bounded owned-input channel and overload; not a second native scheduler |
| Safe FFI | Rust | imgengine/rust/imgengine/src/lib.rs and sys.rs | Unsafe declarations isolated; !Send/!Sync engine, guarded native output, Rust-owned output copy |
| Public native ABI | C | imgengine/include/imgengine/api/v1/imgengine.h, v2/layout.h, v2/sheet.h | Versioned 1.0 to 2.3 exported symbols; exact struct_size and ownership contract |
| Engine execution | C | imgengine/src/api/imgengine_v1.c, imgengine_v2_layout.c, imgengine_v2_sheet.c | Single active process-scoped engine; operation mutex; borrowed encoded input; explicit output release |
| Scheduling | C | imgengine/src/runtime/scheduler*.c, queue*.c | Existing worker/queue ownership retained; no unproved concurrent destroy |
| Image operations | C | imgengine/src/io, src/pipeline, src/arch | JPEG/PNG ingress, layout, crop, resize, JPEG output; some PDF/print helpers remain internal |
| Resources/security | C and Rust | imgengine/src/memory, src/security; Rust admission/lifecycle | Encoded output cap is not total decoded/intermediate/aggregate RSS cap |

The public ABI, not a private C header or historical RFC, defines an SDK consumer contract. Operation-specific P8 FIT, P12 centered-fill and P13 native user-crop contracts must remain distinct.

## Low-level design and owned resource matrix

| Resource / algorithm | Invariant | Existing status | Verification / open question |
| --- | --- | --- | --- |
| Encoded request bytes | C borrows synchronous bytes; never retains/releases them; queued tasks hold decoded native-owned data | RECORDED VERIFIED P7.4-B | Preserve borrow regression on changes to decoder dispatch |
| Engine handle | C creates/destroys opaque handle; safe Rust wrapper is not Send or Sync | IMPLEMENTED | Keep single-engine and serialized operation rule; document lifetime in new ABI |
| Native JPEG output | A successful operation transfers native allocation to one designated C release; Rust guard releases after copying to Vec | RECORDED VERIFIED in ABI/FFI tests | New static SDK must preserve same release discipline |
| P8 placements | C validates counts, dimensions, and copies caller metadata before scheduler handoff | IMPLEMENTED | Resource limits, exact outcome and compatibility tests |
| P12 physical sheet | Centered-fill and complete planned/executed-count contract | IMPLEMENTED / RELEASE OPEN | Native JPEG only; verify actual-host RSS/concurrency |
| P13 normalized crop | Native ABI 2.3 uses fixed-point rectangle and rejects invalid geometry | IMPLEMENTED NATIVE, PARTIAL PRODUCT | Rust FFI/API transport not proven complete |
| Slab and arena | Defined ownership, bounded supported reuse, arena thread confinement | RECORDED VERIFIED P7.4-C2 | Destruction with live borrows and concurrent arena use unsupported |
| SPSC / MPMC queues | Bounded capacity, no lost terminal accepted task, known lifetime | RECORDED VERIFIED P7.4-C/C2 | No claim of arbitrary scheduler destruction concurrency |
| SIMD dispatch | Scalar output is reference; optimized paths require exact semantic parity | RECORDED VERIFIED P7.4/P7.5 narrow grayscale cases | Do not infer every AVX-512 kernel was independently benchmarked |
| PDF encoder | Internal PDF function exists | PARTIAL / INTERNAL | Public PDF ABI, validated physical output, safe error handling absent |
| Public static libimgengine.a | CMake builds imgengine_internal STATIC, but public ABI archive/install is not established | OPEN | Define installed static target, dependency closure, C consumer and license policy |
| Portable CPU binary | -march=native appears in default native build flags | OPEN for redistribution | Build baseline conservative and dispatch optional optimized code |
| Native cancellation | ABI v1 and native execution are non-interruptible mid-call | KNOWN LIMIT | Rust deadline may discard post-completion result, not terminate native work |

## Security threat and evidence priorities

| ID | Priority | Boundary and finding | Required next proof |
| --- | --- | --- | --- |
| S01 | P0 | Untrusted JPEG/PNG decoding | Check per-entry decode path, data validation order, file/byte lifetimes and fuzz regression |
| S02 | P0 | Total memory and disk risk: input/encoded limits do not bound decoded pixels, canvas, copies, temp and aggregate process usage | Define allocation graph and peak-RSS/resource accounting, then test actual host |
| S03 | P0 | Isolation: seccomp helper exists, but no proof every decoder runs in networkless, independently killable child | Audit activated process topology and seccomp/no-new-privs policy; record unsupported cases |
| S04 | P0 | Native metrics.c updates totals atomically but min/max fields without the same synchronization | Trace all readers/writers and call paths; only mark as demonstrated race after concurrency reproduction or rigorous proof |
| S05 | P1 | Native plugin callback and legacy mmap routes have distinct ownership and trust assumptions | Do not expose arbitrary plugins or reuse mmap-release path for borrowed Rust bytes |
| S06 | P1 | P7.5 eliminated per-request raw-ABI stderr diagnostics in tested path | Preserve negative regression and avoid image/filename/secret text in logs |
| S07 | P1 | Public binary licensing and repository provenance remain unresolved | Owner verifies root LICENSE, upstream, vendored code and TurboJPEG/stb notices before redistribution |
| S08 | P1 | Rust HTTP boundaries have bounded admission, but no paid multi-tenant quota/accounting contract | Treat commercial tenant, quota and metering as future Rust policy, never native core responsibility |

Potential vulnerabilities listed here are **audit hypotheses**, not proof of an exploit. Separate recorded mitigations from pending runtime evidence.

## Native C vs Rust observability contract proposal

| Signal | Responsible layer | Unit / capture boundary | Do not log |
| --- | --- | --- | --- |
| Decoder, resampler, composition and JPEG encoder durations | C | ns or CPU cycles for small kernels; us/ms for complete stages; calibrated at boundaries | pixels, filenames, image metadata, decoded content |
| Allocator capacity/exhaustion, scheduler queue depth | C | bounded counters and terminal error classes | pointers or private paths |
| Rust admission, queue wait, overload, accepted/completed and deadline counts | Rust | monotonic duration and low-cardinality counters | request body or API secrets |
| FFI duration and output copies | Rust | per-operation duration, bytes only in aggregate | raw native diagnostics |
| API request failures and lifecycle | Rust | status category and generated correlation identifier | customer names, image bytes, upload filenames |
| Operational alerts and capacity | Operator layer | host RSS, restart/drain, disk, rate of errors | personally identifying request contents |

**Implementation caveat:** this is a proposed authoritative telemetry shape, not an assertion that all signals currently exist or are race-free. Prefer low-overhead sampling or coarse-grained counters off hot pixel loops. Define units, overflow, read synchronization, buffer truncation, cardinality and tests before exposing native metrics to Rust.

## DSA, SOLID, design patterns and performance evidence

| Concept | Existing code evidence | Correct claim |
| --- | --- | --- |
| Strategy / CPU dispatch | src/pipeline/hardware_registry.c, jump tables, scalar/AVX2 | Implemented technique; require parity for each operation |
| Adapter / Facade | Rust imgengine crate over versioned C ABI | Implemented safe-facing boundary; no unsafe escape |
| Producer/consumer | C runtime bounded SPSC/MPMC queues and Rust bounded admission | Implemented with narrow lifecycle invariants |
| Object pool / arena | src/memory/slab*, arena* | Internal memory reuse, limited thread/borrow contract |
| SOLID / DRY / hexagonal | Separation of Rust policy from C execution and ABI | Directionally supported; not a mechanical compliance certificate |
| Factory / Observer / Decorator | docs/design_patterns.md gives guidance and sketches | Not an implemented feature until a source path/test is traced |
| Pixel transforms | proportional to touched pixels plus resampling filter taps | Nanosecond full-image claims invalid without measurement |
| Public ABI dispatch | bounded public entry/symbol lookup | Possible nanosecond-scale primitive only with calibrated benchmark |
| Complete decode→render→encode | size-, codec-, quality- and CPU-dependent | Usually microseconds-to-milliseconds or more; only measured fixture-specific claims are valid |

**Measurement gate:** stage definitions; input/output hashes; exact Git SHA; CPU model, frequency/governor, OS, compiler/flags; warm/cold, memory pressure, p50/p95/p99, peak RSS, throughput; reference image parity. Do not compare sanitizer timings with optimized release timings. Do not measure no-op dead-code-eliminated loops and call them engine performance. Avoid unproven <2 ms 4K or 100K+ operations/sec marketing.

## Documentation and approval procedure

1. Read root and imgengine AGENTS, then the 81-file documentation map, historical/current audit, this matrix, next.md, skills.md and the specific operation contract.
2. For each finding record: status, exact C/Rust files and symbols, prior test evidence, unresolved assumption, severity, reviewer and next verification.
3. A documentation-only PR updates the cited index/register and checks Markdown links/diff; it **does not** invoke a full C build/sanitizer/fuzz matrix.
4. Any later C/FFI/security change goes through focused tests and appropriate mandatory ABI/sanitizer/fuzz integration and release gates; never weaken CI to save tokens.
5. Historic docs remain readable and dated; mark superseded sections rather than altering the meaning of previous test evidence.
6. No changes to source or actual production flags are authorized by this design matrix.

**Business lens:** optimize the work needed to make stable shared/static SDK releases supportable, not the volume of features. Capture maintenance, security and license support burden before new image formats or complex plugin APIs. No profitability claim is made.
