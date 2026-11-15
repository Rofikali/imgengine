# IMGENGINE C17 core — principal engineering A–Z source audit
**Date:** 2026-11-15. **Source baseline:** main at ff5dd06755069d170d97c9ab323a2fa52915aa68. **Audit scope:** native C17 under imgengine/, its installed headers, compiled CMake targets, selected decoder/renderer/allocator/worker/dispatch/security/observability source paths, registered tests and existing HLD/LLD/pattern/benchmark documents. **Method:** source reading and design reasoning only. **No** build, runtime concurrency reproduction, sanitizer, fuzz, perf run, deployment, source edits or new benchmark for this audit. This is not a penetration-test or release certificate.

**Status vocabulary:** IMPLEMENTED means inspected source exists. RECORDED-VERIFIED means an older named revision/test record reports a pass. REVIEW-OPEN means safety/portability or evidence needs more work, not an exploit demonstration. PARTIAL means source exists but not a complete contract. HISTORICAL means earlier code/RFC instructions, not necessarily shipped. DO NOT mark REVIEW-OPEN findings resolved without exact-revision source/test evidence.

## 1. One-page engineering verdict

The native IMGENGINE is a substantial C17 system with TurboJPEG and stb-based decoding, slab/arena allocation, RGB layout/resizing, threaded scheduling, runtime CPU dispatch, PDF encoder for internal/CLI usage, CMake C17/C11 regression profiles and a Linux versioned shared-library ABI. Public ABI v1 plus additive layout/sheet releases 2.0–2.3 specify **12 exported symbols**, bounded sheet render, borrowed input and output-release ownership. [Public v1 header](../imgengine/include/imgengine/api/v1/imgengine.h), [layout](../imgengine/include/imgengine/api/v2/layout.h), [sheet](../imgengine/include/imgengine/api/v2/sheet.h), [ELF symbol map](../imgengine/cmake/libimgengine.map), [CMake](../imgengine/CMakeLists.txt).

**Three highest priority review items before substantial concurrency or external SDK exposure:**
1. **N-C01:** Work-stealing threads call pop on queues that have a single-consumer (SPSC) implementation. The owner can also pop concurrently. The observed queue protocol does not establish a multiple-consumer claim; a targeted reproducer/TSAN-or-equivalent plus correct algorithm design is needed.
2. **N-C02:** Native metrics mix atomic increments with ordinary max/min writes and ordinary concurrent reads; the implementation has no core_id bounds guard. Determine active concurrent caller/reachability, protect ownership/atomicity, add a focused test.
3. **N-C03:** C output-size budget is not a whole-request budget; decoded pixels, intermediate stb copy, slab pools, RGB canvas, resize scratch, encoded output, concurrent admissions and host OOM require an aggregate resource envelope.

Important additional review: sandbox is opt-in, not a separate decoder process; CPU arch build choices can compile x86-specific sources for other target labels; public installable static ABI SDK is not packaged; CLI/internal PDF encoder needs safety and output-error hardening before public exposure. The rest of this report gives exact paths and acceptance gates.

## 2. HLD: the implemented request path versus future services

~~~text
Browser / Nuxt BFF (outside native scope)
    -> Rust/Axum API + request supervisor (backend/)
    -> safe Rust FFI, versioned libimgengine.so API
    -> C opaque engine (one process-global active engine)
    -> synchronous public operation mutex
    -> decode borrowed encoded bytes using IO vtable
       -> JPEG TurboJPEG TLS decoder, PNG stb bridge,
          CMYK/YCCK fallback to stb
    -> private RGB layout request, copied placements and decoded slab
    -> bounded native scheduler task and worker
    -> RGB canvas + centered or selected crop + scalar/SIMD resize/blit
    -> TurboJPEG bounded JPEG encode
    -> caller owns and releases native output exactly once
~~~

Source: [ABI implementation](../imgengine/src/api/imgengine_v1.c), [layout adapter](../imgengine/src/api/imgengine_v2_layout.c), [sheet adapter](../imgengine/src/api/imgengine_v2_sheet.c), [private renderer](../imgengine/src/pipeline/layout_render_private.c), [worker loop](../imgengine/src/runtime/worker_loop.c), [decoder dispatch](../imgengine/src/io/decoder/decoder_dispatch.c), [encoder](../imgengine/src/io/encoder/encoder_entry.c). External billing, tenants, rate enforcement and HTTP are Rust/business concerns, **not** C feature requirements.

**Architecture truth:** public operations are serialized by engine->operation_lock and engine creation uses a process-global g_engine_active lock. Worker_count > 1 does not imply two public ABI operations render in parallel. Measure admission queue latency separately from internal worker parallelism before pursuing 100K ops/s targets. [Engine API](../imgengine/src/api/imgengine_v1.c).

## 3. LLD: interfaces, data ownership, failure boundaries

| Boundary or object | Current implementation | Contract / important limitation |
| --- | --- | --- |
| Engine lifecycle | [v1 source](../imgengine/src/api/imgengine_v1.c): calloc, operation mutex, one global active engine; worker count 1–64 | Opaque C ABI. Shutdown vs other unsynchronized users of the same engine is not shown thread-safe; Rust must control handle lifetime. |
| C ABI requests | [v1](../imgengine/include/imgengine/api/v1/imgengine.h), [layout](../imgengine/include/imgengine/api/v2/layout.h), [sheet](../imgengine/include/imgengine/api/v2/sheet.h) | struct_size/version/reserved validation; input borrowed for synchronous call; placement metadata copied before queue submission. |
| Compressed input | [non-owning decode adapter](../imgengine/src/api/api_decode_secure.c), [private render](../imgengine/src/pipeline/layout_render_private.c) | Do not free/unmap user buffer; legacy mmap file path [is separate](../imgengine/src/api/api_file_mmap.c). |
| Decoded image | [TurboJPEG](../imgengine/src/io/decoder/decoder_entry.c), [stb](../imgengine/src/io/decoder/stb_bridge.c), [slab](../imgengine/src/memory/slab_hot.c) | Native slab owns RGB buffer; stb can temporarily hold a second decoded buffer before copying. |
| Canvas and temporary resize | [canvas allocator](../imgengine/src/pipeline/canvas_alloc_buffer.c), [layout resize](../imgengine/src/pipeline/layout_resize_validate_alloc.c), [arena](../imgengine/src/memory/arena_alloc_aligned.c) | Slab has mutex and allocated bitmap. Arena is bump allocator, external synchronization and reset lifetime restriction. |
| Task handoff | [private renderer](../imgengine/src/pipeline/layout_render_private.c), [scheduler submit](../imgengine/src/runtime/scheduler_submit.c), [worker loop](../imgengine/src/runtime/worker_loop.c) | Caller-owned stack task waits for terminal state before state free; shutdown/drain tested for defined limited contract. |
| Public output | [layout release](../imgengine/src/api/imgengine_v2_layout.c), [v1 release](../imgengine/src/api/imgengine_v1.c) | On failure, outputs zeroed. On success C allocates, Rust copies/releases through documented matching function. |
| C symbol/API compatibility | [version map](../imgengine/cmake/libimgengine.map), [exports JSON](../imgengine/abi/exported_symbols.json) | ABI 1.0–2.3 additive 12 public symbols; only include/imgengine installed. Public distributable static .a not present despite internal static target. |
| Internal PDF | [PDF implementation](../imgengine/src/io/encoder/pdf_encoder.c) | PDF 1.4 with embedded JPEG; no public bounded PDF sheet ABI or production compatibility claim. |

**Design recommendation:** document each component's owner, borrower, allocate/release function, thread-safety, invalidation point, error behavior and test in a dedicated contract; do not rely on comments that say “zero-copy” without specifying temporary copies or lifetime.

## 4. DSA / algorithms: what really exists

| Actual data structure or algorithm | Source | Complexity in this implementation | Invariant / status |
| --- | --- | --- | --- |
| Centered FIT / centered FILL crop | [crop geometry](../imgengine/src/pipeline/layout_crop_geometry.c) | O(1) geometric arithmetic | 64-bit cross products prevent float aspect comparison; normalized crop at fixed 1,000,000 scale and rounding envelope, source-backed. |
| Rectangle placement validation | [private renderer](../imgengine/src/pipeline/layout_render_private.c) | O(P²) pairwise overlap detection; P ≤ 64 | Bounded and reasonable for today; no spatial tree needed at this max. |
| RGB bilinear resize | [resampler](../imgengine/src/pipeline/layout_resize_resample_bilinear.c) | O(Wout×Hout×3) pixels; axis precompute O(Wout+Hout) | 16.16 fixed-point weights and scalar fallback; compare all optimized kernels against an independent oracle. |
| SIMD strategy / opcode dispatch | [selector](../imgengine/src/pipeline/jump_table_select_resize.c), [jump table](../imgengine/src/pipeline/jump_table_register.c) | O(1) dispatch after cold registration | Scalar/AVX2/AVX512 choices; compiled target/CPU feature and plugin overrides require validation. |
| Power-of-two SPSC ring | [create](../imgengine/src/runtime/queue_create.c), [push](../imgengine/src/runtime/queue_push.c), [pop](../imgengine/src/runtime/queue_pop.c) | O(1) push/pop, bounded capacity | Producer/consumer count invariant is **not consistent** with [steal](../imgengine/src/runtime/scheduler.c), see N-C01. |
| Sequence-number MPMC ring | [init](../imgengine/src/runtime/queue_mpmc_init.c), [push](../imgengine/src/runtime/queue_mpmc_push.c), [pop](../imgengine/src/runtime/queue_mpmc_pop.c) | Amortized O(1) with contention-dependent retries; bounded storage | C11 acquire/release sequence coordination, capacity rounding bounds. Permitted MPMC use has regression tests. |
| Native work stealing | [scheduler](../imgengine/src/runtime/scheduler.c) | O(number of workers) victim scan per unsuccessful steal | Multiple consumers of SPSC victim queue unresolved; aggressive busy-spin can cost CPU. |
| Slab free-list + allocation bitmap | [create](../imgengine/src/memory/slab_create.c), [alloc/free](../imgengine/src/memory/slab_hot.c) | O(1) allocation/free under mutex | Address/range/alignment validation, double-free bitmap and poison; destroy with live borrowers unsupported. |
| Arena bump allocator | [arena](../imgengine/src/memory/arena_alloc.c), [aligned](../imgengine/src/memory/arena_alloc_aligned.c) | O(1) bump; O(1) reset | Not concurrency-safe or a general free-list; do not hold borrows after reset. |
| Output JPEG bounds | [encoder](../imgengine/src/io/encoder/encoder_entry.c) | Encoding O(pixels), buffer bound from tjBufSize | Bounded destination allocation; not whole-process resource budget. |

Do **not** introduce heaps, linked lists, maps, bloom filters, event buses, distributed queues, GPU or another allocator simply to demonstrate knowledge of DSA. First prove a constraint and profile the current code.

## 5. Design patterns, SOLID, DRY and clean-code audit

| Concept | Evidence from source | Classification / guidance |
| --- | --- | --- |
| Strategy | CPU capability → selected resize implementation, decode BULK/STREAM strategy | **IMPLEMENTED**, bounded internal strategy/dispatch, not an installed SDK plugin feature. |
| Adapter / ports-and-adapters | Public C ABI → private renderer; IO vtable registration; Rust safe facade | **IMPLEMENTED at selected boundaries**; Rust product/business policy should remain outside C. |
| Object pool | Slab allocation/free and NUMA placement | **IMPLEMENTED**. |
| Builder / pipeline stages | Native layout/pipeline build and compilation and stage execution | **IMPLEMENTED INTERNAL**; exact purity/zero-allocation slogans in historic HLD are not proven for all paths. |
| Registry and plugin descriptor | Static linker-section descriptors and opcode registration | **IMPLEMENTED INTERNAL**, not a verified general-purpose dynamic plugin marketplace. [loader](../imgengine/src/runtime/plugin_loader.c) ignores its directory parameter and traverses linker-section descriptors. |
| Factory/Observer/Decorator/Proxy as GoF names | [design_patterns.md](design_patterns.md) contains sketches and suggested additions | **DOCUMENTATION / PROPOSAL**, not automatically implemented. Do not add unused patterns. |
| Single-responsibility / separation | Security/input, memory, queue, codec, layout, ABI split into many source files | **PARTIAL GOOD DESIGN**; some global registries, duplicate status converters and many tiny wrappers require maintainability review, not mass refactor. |
| Open/closed and interface segregation | Versioned public headers/additive symbols and narrow private helpers | **GOOD ABI DIRECTION**. Keep plugin and internal header types out of public installed API. |
| Dependency inversion | C IO vtable and opaque ABI enable testing/portability; Rust depends on public C ABI | **IMPLEMENTED at boundaries**; globals limit isolation in some modules. |
| Clean code and comments | [worker loop](../imgengine/src/runtime/worker_loop.c), [scheduler init](../imgengine/src/runtime/scheduler_init.c), [jump table](../imgengine/src/pipeline/jump_table.c), [resampler](../imgengine/src/pipeline/layout_resize_resample_bilinear.c) contain long commented-out historical implementations, “(REPLACE)” directions and unsupported 30x speed claims | **DOCUMENTATION/MAINTENANCE DEBT**. Preserve provenance in Git; remove only through a focused source refactor with tests, never during a security fix. |
| Build/source of truth | [CMake](../imgengine/CMakeLists.txt) lists many split translation units and optional codegen, internal/static and shared library targets | **PARTIAL maintainability concern**. Measure compile-time and maintenance cost; review whether the DSL codegen custom command has a reproducible generator before relying on it. |

**LLD rule:** a design pattern must identify exact source functions, scope, ownership/thread guarantees, benefit, failure mode and tests. “Use SOLID/DDD/hexagonal/event driven everywhere” is **not** an acceptance criterion.

## 6. Security/correctness register — severity is triage priority, not exploit certification

| ID | Priority | Observed source / reasoning | Confidence and smallest safe next action |
| --- | --- | --- | --- |
| **N-C01** | **P0 review** | [queue_pop.c](../imgengine/src/runtime/queue_pop.c) reads head then consumes data and stores head for SPSC; [worker_loop.c](../imgengine/src/runtime/worker_loop.c) pops own queue, and [scheduler.c](../imgengine/src/runtime/scheduler.c) permits other workers to pop that victim's same queue, possibly concurrently. This violates the normal single-consumer ownership assumption and could duplicate/loss tasks on adversarial scheduling. | **Direct source topology, failure not yet reproduced**. Build 2–8-worker stress harness where owner+multiple thieves contend; independent per-task exactly-once observer, ASAN/UBSAN and TSAN or schedule-controlled oracle. If reproduced, design proper MPMC/stealable deque ownership or disable theft as a narrow fix; do not rewrite the entire scheduler. |
| **N-C02** | **P0 review** | [metrics.c](../imgengine/src/observability/metrics.c) increments total_cycles/requests atomically but directly reads them in export; min/max latency are ordinary non-atomic read-modify-writes. No guard for core_id < IMG_MAX_CORES. | **Direct mixed-access source, concurrent call graph unverified**. Find actual callers and sampling/writer threads; add bounds, consistent atomics/sharded snapshot or a protected measurement path, verify with concurrent tests. No claim of user-data exposure. |
| **N-C03** | **P0 release evidence** | [input validator](../imgengine/src/security/input_validator.c) allows dimensions up to 16384² but real slab may reject large buffers; [stb bridge](../imgengine/src/io/decoder/stb_bridge.c) allocates decoded stb pixels, then copies into slab; canvas/resize/encoder have separate allocations. Bounded JPEG [encoder](../imgengine/src/io/encoder/encoder_entry.c) caps destination only. | **Architecture evidence gap**: establish worst-case per-request and aggregate cgroup memory equation including concurrent jobs and C/Rust copies; OOM and malformed inputs. Do not weaken existing dimension/output limits. |
| **N-C04** | **P1 security** | [CMake](../imgengine/CMakeLists.txt) defaults IMGENGINE_SANDBOX=OFF; [sandbox](../imgengine/src/security/sandbox.c) returns success as a no-op when disabled. When enabled [filter](../imgengine/src/security/sandbox_build.c) still allows openat, clone, mmap etc; [apply](../imgengine/src/security/sandbox_apply.c) uses SECCOMP_FILTER_FLAG_TSYNC (thread/process-wide), not a separate decoder process. | **Direct implementation**. Confirm production build flags/container constraints; test reachable decoder under filter and portability/fail-closed behavior. Do not call seccomp “decoder isolation” or add a sandbox without threat/compatibility review. |
| **N-C05** | **P1 portability** | [CMake](../imgengine/CMakeLists.txt) defaults to -march=native except explicit x86 portable baseline; architecture detection advertises aarch64/scalar but SOURCES_ARCH_X86 is included unconditionally and [scheduler](../imgengine/src/runtime/scheduler_submit.c) uses _mm_pause while MPMC uses x86 pause builtin. | **Direct build-path concern, cross-target build untested**. Verify clean x86-64 portable baseline and separate ARM64 build before claiming Oracle Ampere/ARM compatibility. Isolate arch-specific files/intrinsics and runtime dispatch; do not change C17 shipping standard. |
| **N-C06** | **P1 API product scope** | [CMake](../imgengine/CMakeLists.txt) builds imgengine_internal STATIC from common sources but public ABI v1/v2 adapters are only included in SHARED imgengine and only shared binary/CLI are installed. | **Confirmed packaging gap**. Define public static ABI target, installation/headers/dependency closure, name/version, PIC/visibility/license and external C/Rust consumer tests; do not claim libimgengine.a public SDK exists. |
| **N-C07** | **P1 PDF hardening before exposure** | Internal [pdf_encoder.c](../imgengine/src/io/encoder/pdf_encoder.c) computes floats with division by dpi but checks no nonzero DPI, does not robustly verify all fprintf/fwrite/fclose return codes, and lacks a public bounded PDF output ABI. | **Direct code gap in internal path**. Validate dpi/size, check write failures/partial file cleanup, deterministic PDF parser/print validation and explicit limits before public Rust transport. No evidence that Rust P12 offers PDF. |
| **N-C08** | **P1 queue/latency/availability** | [runtime_wait_task](../imgengine/src/runtime/scheduler_submit.c) spins up to 100,000 polls, then sched_yield; [worker_loop](../imgengine/src/runtime/worker_loop.c) uses repeated _mm_pause; public engine calls serialize through operation_lock. | **Design/performance concern**. Measure CPU wasted under idle/load and P95/P99 tail; consider condition variable/futex after proven need; no unverified claim that another algorithm is faster. |
| **N-C09** | **P1 product correctness** | [input_validator.c](../imgengine/src/security/input_validator.c) rejects pixels-to-compressed-byte ratio over 400; extremely compressible valid images can be rejected, even if they otherwise fit a strict budget. | **Policy tradeoff**. Create small/highly-compressible clean-image fixtures to distinguish intended product policy from codec failure; do not silently relax bomb protection. |
| **N-C10** | **P1 plugin claims** | [plugin_loader.c](../imgengine/src/runtime/plugin_loader.c) ignores supplied directory and uses start/stop linker section; [plugin descriptor validation](../imgengine/src/plugins/plugin_registry_validate.c) only compares abi_version; [descriptor load](../imgengine/src/runtime/plugin_loader_descriptor.c) prints registration/mismatch and uses opcode tables. | **Internal mechanism exists, third-party dynamic loading unverified**. Define trusted plugins only, no untrusted .so execution, bounds/caps and registration authority tests. Do not advertise a supported plugin SDK. |
| **N-C11** | **P2 defensive implementation** | [canvas_alloc_buffer.c](../imgengine/src/pipeline/canvas_alloc_buffer.c) computes (size_t)pw * ph * 3 and pw * 3 without local checked helpers; public private-render path bounds geometry earlier, but legacy/CLI callers need separate reachability proof. | **Context-dependent**. Verify callers/limits and add narrow checked-size test if reachable with untrusted CLI values. |
| **N-C12** | **P2 fuzz correctness** | [fuzz_hooks.c](../imgengine/src/security/fuzz_hooks.c) casts arbitrary byte addresses to uint32_t pointers for reads. Byte buffers have no guaranteed uint32_t alignment; strict alignment/aliasing sanitizers may flag this if harness executed. | **Direct potentially undefined representation, active harness reachability unverified**. Prefer memcpy into scalars in a scoped test utility fix and verify whether this hook is registered/reached. |
| **N-C13** | **P2 ABI and lifetime** | [imgengine_v1.c](../imgengine/src/api/imgengine_v1.c) destroys engine with its mutex but doesn't provide a general concurrent-client handle reference lifetime contract; only one global active engine permitted. | **Design restriction**. Rust must own stop/drain and prevent concurrent use-after-destroy; any multiengine/concurrent API is a new compatibility/security design task, not a free optimization. |
| **N-C14** | **P2 historical source hygiene** | Old commented code and “FILE n (REPLACE)” in scheduler/worker/layout, old targets in [hld](hld.md), sample Factory/Observer/Decorator not backed by active implementation in [design_patterns](design_patterns.md). | **Confirmed documentation debt**. Keep historic audits but create one modern owner/status index; restrict cleanup to separately tested maintenance PRs. |

**Not resolved by this documentation:** which N-C01 schedules actually manifest; N-C02 concurrent reachability, real deployment sandbox mode, ARM64 compilation, cloud economics or any exploitability. A full path-by-path static analyzer, TSan build and complete fuzz-corpus review would require an explicit executable test milestone.

## 7. Quality, tests and CI evidence: strengths and gaps

**Present/recorded:** [CMake test registration](../imgengine/CMakeLists.txt) contains public ABI consumer/export/compatibility, sheet and ownership tests, private layout/crop geometry, decoder/bounds/sandbox, allocator/queue, scheduler shutdown, independent SIMD grayscale-oracle and resize tests. [Linux verification script](../imgengine/scripts/verify_linux.sh) runs C17 normal tests, C11 regression, ASAN/UBSAN, Rust FFI/API/real-image integration, sandbox lifecycle, and two bounded fuzz targets. Older [P7.4 evidence](P7_4_EVIDENCE_REGISTER.md) records scoped pass results; [native benchmark rules](NATIVE_BENCHMARKING.md) use controlled self-hosted hardware and five runs.

**Still needed for the new findings:**
- N-C01 concurrent *owner plus multiple thieves* consumer test (existing [allocator/queue test](../imgengine/src/tests/test_allocator_queue_ownership.c) checks SPSC reuse and MPMC concurrency, not this topology).
- N-C02 metrics writer/exporter concurrent reachability, race test and core-index bounds.
- N-C03 end-to-end cgroup peak-memory/total allocations under malicious inputs and admitted requests.
- N-C04 sandbox enforcement checks under the actual production binary/profile, not only [lifecycle test](../imgengine/src/tests/test_sandbox_lifecycle.c).
- ARM64 portability, real JPEG/PDF and P95/P99 performance under fixed reproducible CPU/image fixture/correctness contracts.

**Developer loop:** docs-only changes: check statements and links, **do not compile/fuzz**. C/FFI/integer/memory/concurrency changes: focused deterministic tests first, ASAN/UBSAN/TSan where relevant, ABI consumer and Rust FFI regression, then required Ubuntu/CI/release gates. Always distinguish newly executed tests from older recorded successful CI runs.

## 8. Performance engineering: nanoseconds vs real image workloads

The target is not “every image completes in nanoseconds.” Distinguish:
- **ns-scale microbenchmark:** jump-table dispatch, checked integer operation, arena bump/queue primitive in isolation, after overhead calibration.
- **microsecond scale:** small prepared hot kernels/resizes on pinned compatible hardware, only if measured.
- **milliseconds or longer:** decode+RGB buffer creation+layout+crop+resize+JPEG encoding+Rust FFI+HTTP+network, strongly dependent on size, quality, hardware and concurrency.

For each accepted speed claim record: exact SHA; CPU model/instructions/frequency/governor; compiler/version/build portable vs optimized; real fixture/hash and sizes; warmups/repetitions; p50/p95/p99; allocations and peak RSS; bytewise/pixel quality oracle; and hot vs cold vs end-to-end boundary. Avoid optimizing architecture before queue correctness and resource safety. [Benchmark contract](NATIVE_BENCHMARKING.md).

## 9. Principal/CA/MBA/management decision gates

- **Customer value:** what image operation (passport sheet, studio batch, export) will be delivered, its fidelity/legal limitations, and its measured time saved.
- **Cost:** worst-case C allocation, CPU milliseconds per successful request, bytes transferred, over-cap rejection/abuse overhead, license/SBOM and incident/support load; assign actual host price rather than inventing revenue.
- **Risk-adjusted sequencing:** queue/metrics/resource correctness before public throughput claims; portable binary and public .a only if external SDK consumers need it; PDF only with tested real customer/printing need.
- **Ownership:** C owner handles codec, memory/scheduler, ABI and CPU kernels; Rust owner handles request/admission/auth/quotas and safe FFI; infra owner handles cgroups, secrets, monitoring, real host; product owner chooses pricing/presets. No database, Redis/Celery, Python or new microservices by default.
- **Governance:** one narrow change per PR; source test commands and observed failures; rollback and documented remaining risk. No Git merge or public deploy without explicit owner approval.

## 10. Astra C17 worklist and definition of done

| Milestone | Task | Approval and exit evidence |
| --- | --- | --- |
| **C-A0, first** | **Read-only concurrency review** of owner/thief SPSC and metrics write/export; construct a minimal race/exactly-once test plan with caller graph and CPU/build details. | **DOC REVIEW ONLY** first; no scheduler rewrite. Findings classified confirmed vs unverified. |
| **C-A1** | Implement a **focused concurrency reproducer** and/or behavior fix only if explicitly approved. | Queue ownership semantics, exactly-once tasks across 1/2/4/8 workers under stress, no deadlock/loss, Rust ABI behavior, sanitizers and full required CI. |
| **C-A2** | Model aggregate image request memory/CPU and actual sandbox/decoder reachability, including stb intermediate copy and output. | Tested resource envelope with malformed/valid inputs and actual cgroup capacity; no arbitrary claim of sandbox isolation. |
| **C-A3** | External static .a SDK/provenance/portable CPU plan after actual user need. | Public ABI/installation/consumer parity, dependencies, license and reproducible image/CPU baseline; no feature added without approval. |
| **C-A4** | PDF/legacy CLI/clean-code narrow hardening and docs HLD/LLD modernization. | Proved reachable need, safe I/O failures and output correctness, no public ABI break, bounded tests. |
| **C-A5** | Performance baseline after correctness/security, then measured algorithmic changes only if ROI clear. | Controlled pinned-host reproducibility, p95/p99 + memory and fidelity parity, economic justification. |

**Astra kickoff suggestion:** Read [root AGENTS](../AGENTS.md), [native AGENTS](../imgengine/AGENTS.md), [next](../next.md), [Astra main handoff](ASTRA_ENGINEERING_START_HERE.md), this source audit, [CMake](../imgengine/CMakeLists.txt), [public ABI](ABI.md), queue submit/steal/pop, metrics, decoder, allocator and tests. **Start C-A0 only**: source-backed concurrency/ownership findings and deterministic test design, with no application source edit until authorized. Preserve the existing ABI, C17 target, and unmodified Rust/Nuxt/infra boundaries. Once a specific implementation PR is approved, use scoped tests and required CI, report exact SHA and remaining risk.

**Do not interpret this report as proof that all 477 tracked files under imgengine/ were individually line-reviewed**. The tree/build inventory was considered, and the named critical C source/header/test files were read directly. Exhaustive static analysis, generated-code coverage, user-provided private deployment artifacts and runtime verification remain outside this documentation-only review.
