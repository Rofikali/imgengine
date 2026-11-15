# IMGENGINE — performance engineering, C ABI, Rust, browser Wasm and desktop roadmap

**Status: PROPOSED architecture and measurement plan, NOT a benchmark result, not a code change and not a production claim (2026-11-15).** Reviewed `main` baseline: `acc29bbf002890471ba0a63f01f75a6fbcaa49f3`. Binding stack remains C17 native -> stable C ABI -> safe Rust FFI -> Rust/Tokio/Axum HTTP; Nuxt/Vue/TypeScript frontend. Browser Rust/Wasm and desktop are **proposals** requiring measured feasibility and explicit ADRs. No Python/FastAPI/Redis/PostgreSQL/new Python tooling in target architecture.

**Read before touching code:** [native AGENTS](../imgengine/AGENTS.md), [native source audit](C17_CORE_PRINCIPAL_SOURCE_LLD_DSA_SECURITY_AUDIT_2026-11-15.md), [native benchmarking contract](NATIVE_BENCHMARKING.md), [ABI](ABI.md), [Rust AGENTS](../backend/AGENTS.md), [Rust FFI](RUST_FFI.md), [Rust request lifecycle](RUST_REQUEST_LIFECYCLE.md), [Rust API](api.md), [frontend AGENTS](../imgengine-saas/frontend/AGENTS.md), [M0 Wasm ADR](M0_C17_PORTABILITY_AND_WASM_ADR.md), [P13 crop](P13_INTERACTIVE_CROP_CONTRACT.md), [current queue](../next.md), [related product/economics strategy](IMGENGINE_PLATFORM_PRODUCT_BUSINESS_STRATEGY_2026-11-15.md).

## 1. Performance terms and non-promises

- 1 **millisecond (ms)** = 1,000 **microseconds (µs)** = 1,000,000 **nanoseconds (ns)**.
- “**Micro milliseconds**” is not a standard target unit. Use **submillisecond** (<1 ms), **microsecond-scale** (µs) or **nanosecond-scale** (ns) with a **named operation and hardware**.
- A hot coordinate transformation, allocation fast-path or dispatch call may be appropriate for an **ns/µs** benchmark. A prepared small pixel kernel could be sub-ms on appropriate hardware. A full image decode+resize+JPEG encode can take **ms or longer** depending on size/codec/cold caches.
- A networked API includes TLS, network transfer, parsing, queues, decode/render/encode and serialization, so it is **not a 1-µs end-to-end product**. Even server-internal “processing time” must specify what it includes.
- **No universal latency number, 100K jobs/sec, lossless compression ratio, 1 GiB VPS capacity, energy saving, browser parity or “faster than libvips” is established here.** Require independent exact-input, exact-hardware evidence first.

### Workload taxonomy

| Metric ID | Stopwatch includes | Legitimate optimization and acceptance basis |
| --- | --- | --- |
| P-B0: tiny **geometry/dispatch** | Already initialized data -> one pure operation; account for timer overhead | Target ns/µs where feasible; correctness across representative and overflow inputs; report distribution |
| P-B1: **prepared resize/render** | Prepared decoded pixels and initialized engine -> transformed pixel buffer, no JPEG encode or I/O | Algorithm, cache, SIMD vs scalar parity; size-dependent p50/p95/p99 |
| P-B2: **decode** | Compressed buffer -> validated decoded pixels | Codec, format/orientation, memory peak, malicious fixtures |
| P-B3: **encode** | Pixel buffer -> bounded JPEG/PNG/WebP bytes when actually supported | Quality/file size vs time, max output, failure status |
| P-B4: **cold native full operation** | Native call input bytes -> encoded output bytes, allocation and cleanup | All relevant C costs, output parity and memory |
| P-B5: **warm local API service** | Rust HTTP handler accepted -> completed response body, excluding client network | Admission wait, FFI, decode/process/encode, serialization, request concurrency, timeouts |
| P-B6: **public client E2E** | Client upload start -> successful download completion | Client network/TLS, bandwidth, browser, reliability; never call a server-kernel benchmark |
| P-B7: **browser Wasm workflow** | Selected file -> local decode -> Worker start/compute -> local preview/export | Cold Wasm download/compile and warm operation reported separately; memory, mobile heat, no-upload assertion |
| P-B8: **desktop workflow** | Local file opened -> decode/process/save and UI ready | File I/O, startup, local permissions, installer/OS/architecture differences |

**Golden rule:** report **unit + operation + input + CPU + concurrency + baseline revision**, not “IMGENGINE processes images in 1 µs.”

## 2. Current C/Rust facts that materially limit design

- Native `imgengine/` uses strict C17 shipping, C11 consumer and whole-core compatibility regression, decoder/resize/print pipeline, SIMD dispatch, slab/arena and threaded C scheduler. Versioned public ELF shared `.so` ABI is implemented; a public independently installable `.a` SDK is **not** established. Internal static targets are not proof of customer SDK exportability.
- Current installed public C API operations involve an engine-global active instance and a synchronous **operation lock**, so multiple worker threads inside one task do **not** demonstrate multiple concurrent independent public ABI jobs. Before buying CPU cores or building a throughput business model, measure serialization/queuing explicitly.
- Native high-priority review: `N-C01` SPSC ring owner + thieves consumer-reachability hypothesis; `N-C02` atomic/plain metrics access question; `N-C03` whole-request peak decoded+scratch+canvas+output+Rust/concurrent RSS exposure. They are **not proven exploits**, and the agent must design a deterministic reproducer/trace before touching concurrency or removing locks.
- Rust `backend/` already supplies safe FFI facade, bounded supervisor and synchronous Axum routes. Browser abort does not imply native mid-operation cancellation; keep ownership, deadline and cleanup semantics explicit.
- `docs/NATIVE_BENCHMARKING.md` already defines `bench_lat`, pinned Linux runner, comparable optimized/portable profiles, five independent runs, median run p95/worst p99, a prepared-render-only historical 4K <2ms aspiration. Extend **without replacing or weakening** that contract.
- `imgengine-saas/frontend/` currently runs Nuxt 4, Vue/TS, custom CSS and server-proxy-backed P5/P8/P12 features. Browser Rust/Wasm does not exist just because Rust runs in backend. Tailwind adoption is a separate frontend dependency decision.

## 3. A shared platform does not mean identical binaries everywhere

~~~text
                      PRODUCT / DOMAIN CONTRACTS
            Image geometry, DPI, rounding, metadata, export
                           + golden fixtures
                                  |
         +------------------------+-------------------------+
         |                        |                         |
  Browser (free local)        API / server               Desktop (later)
  Nuxt/Vue/TypeScript         Rust/Axum                 Vue UI + Rust shell
  Dedicated Web Worker        Admission/HTTP             Local filesystem auth
  Browser decode/encode       Safe Rust FFI              Safe Rust FFI
  Rust/Wasm pixel kernels     C17 .so / C ABI           C17 .so or packaged static
         |                        |                         |
 Local browser download     HTTP result to client      Local file output
~~~

- **Portable product contracts and fixtures** must be shared across channels for geometry, physical DPI, rounding, crop position, alpha policy, EXIF orientation and deterministic errors.
- Do **not** assume C17 shared-library machine code is the same as Wasm code. A Rust/Wasm port or a narrowly compiled portable C-to-Wasm geometry library is an **independent target and security/build decision**.
- **Browser-first feasibility recommendation:** native browser decode/encode + Worker-hosted limited Rust/Wasm for useful pixel/geometry operations, compared against browser-only processing and M0's C17-Wasm proposal. Adopt only after ADR, bundle/latency/quality/mobile-memory evidence. Do not duplicate entire native TurboJPEG + scheduler into Wasm.
- **Web Worker is not a security boundary.** Validate untrusted files/decoded dimensions, time and total allocated memory; keep images local, keep WASM free of credentials and prevent implicit network fallback. Browser SIMD/thread features require capability gating; thread configuration must account for cross-origin isolation implications, third-party resources and CSP.
- **Desktop later:** candidate Tauri v2 Rust shell + Vue components with native C17 shared/static packaging after supply-chain/licensing/security and customer-demand validation. Compare platform-specific WebView behavior, optional sidecar, signed updates, code signing, OS permissions and installer maintenance. No new desktop code in this plan.

## 4. C17 engine: source-aligned optimization sequence

**Never optimize a suspected race. Sequence is safety -> baseline -> profiling -> measured optimization -> proof.**

1. **C-A0 read-only exact caller topology:** inspect SPSC pop from owner/work-stealers and metrics producer/consumer concurrency/bounds at exact SHA; derive scheduler ownership invariants, precisely explain which interleavings are possible, and design focused deterministic tests. No speculative rewrite.
2. **Total resource budget:** for each workload calculate compressed input, decoded pixels (format/bands), any decoder copies, crop view vs copy, canvas, per-worker scratch, encoder output, Rust request buffers, tmpfs and container overhead. Measure **peak RSS** across admitted concurrency and cleanup. An output byte cap alone is insufficient.
3. **Correctness oracles:** stable golden images across JPEG/PNG, grayscale, progressive JPEG, EXIF orientation, alpha handling, crops, mm/DPI, borders and overflow/malformed inputs. Compare actual pixels with documented tolerance rather than requiring JPEG-byte-identical output across encoders.
4. **Profiler:** run pinned CPU, controlled governor, identical fixtures/compiler/flags and `perf` or compatible profile. Determine % time decode, kernel, encode, allocation, locks/queue and I/O. Track output size and quality simultaneously.
5. **Low-risk wins:** avoid needless full-frame copies, reuse decoded buffers when safe, choose strides/tiles deliberately, batch tiny dispatches, avoid redundant allocations, measure layout precomputation; no off-the-books SIMD changes.
6. **SIMD/CPU specialization:** preserve scalar oracle and runtime feature detection; AVX2/AVX512 only when CPU supports; benchmark warmed and cold workloads, memory bandwidth and energy/thermal effects. No `-march=native` public portability promises. ARM64/NEON requires real compilation and test proof.
7. **Concurrency redesign only if justified:** document global ABI lock semantics, exactly-once scheduler dispatch and queue invariants first. If throughput needs >1 independent request, evaluate isolation/process pools vs concurrent handles vs internal scheduling with evidence, memory and crash containment; avoid violating existing ABI or launching unreviewed native worker pools.
8. **Public SDK release only after safeguards:** C11-compatible installed headers, semantic symbol/version policy, `.so` and (if genuinely requested) `.a` packaging, independent external consumer CI, ownership/error docs, license/SBOM, sanitizers/fuzzers, reproducible build, portability and crash isolation.

**Relevant context:** [libvips architecture](https://www.libvips.org/API/current/how-it-works.html) explains demand-driven regions and low-memory pipelines; use as a comparator and architectural study, not a reason to replace a working C engine without measured need.

## 5. Rust native integration / HTTP API / future commercial readiness

**Rust remains the safe application/control plane**, not a second pixel-kernel implementation by default:
- Safe opaque wrapper maps exported C ABI and response buffers into scoped Rust lifetimes; all `unsafe` is narrow and documented; one owner must free native results exactly once even on error/panic/disconnect.
- API validates allowed formats, declared and decoded dimensions, request bytes, DPI/geometry, CPU budget, runtime deadline, native encoded output size, and expected memory. Errors are consistent and secrets/images are redacted.
- Admission control must bound **active requests AND whole-host memory**, including failed/retrying attempts; overload returns an explicit safe error rather than indefinite queueing.
- Public API design is a **later ADR**: user/organization identity, revocable keys, least privilege, per-tenant quotas and burst limits, pricing/metering by successful billable operation, abuse controls, invoice reconciliation, refunds/credits, service terms and storage/privacy handling. Existing server-only Nuxt proxy shared key is **not** external customer authentication.
- Keep outputs ephemeral; introduce persistent account or billing **metadata** only after explicit approved storage/privacy ADR, never by smuggling in Redis/PostgreSQL/new Python. Do not promise no database for commercial accounting forever.
- Provide versioned `/v1` contracts, size/latency limits, integration samples and reliable status errors; no arbitrary remote URL fetches or SSRF-prone input path without a threat model.
- Before public exposure, follow [P14 actual host evidence](P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md): secret host-file semantics, TLS/DNS, cgroup/total RSS, abuse, observability, canary/rollback and owner approval.
- Security reference: [OWASP API4 unrestricted resource consumption](https://api-security.owasp.org/editions/2023/en/0xa4-unrestricted-resource-consumption/) and [OWASP File Upload](https://cheatsheetseries.owasp.org/cheatsheets/File_Upload_Cheat_Sheet.html).

## 6. Benchmark and release matrix for future agents

| Environment | Workloads | Correctness / security evidence | Measurement / guardrail |
| --- | --- | --- | --- |
| Linux C17 portable | geometry, prepared render, decode/encode, full native | ABI and C11 consumers, C17/C11, ASan/UBSan, fuzz, scalar oracle | CPU model, flags, p50/p95/p99, throughput, RSS/allocations |
| Linux optimized x86_64 | scalar/AVX2/AVX512 gated | feature detection, fallback, bit/pixel tolerance | Same CPU input and profile, **no mixed baseline** |
| Rust FFI/Axum | full ephemeral requests, overload, disconnect, invalid/malicious | handle lifetime, error mapping, panics, rate/admission, no secrets/log leaks | queue wait + service time, concurrent peak RSS, CPU, network |
| Browser Chrome/Firefox/Safari desktop | Worker local resize, crop, encode | image never leaves device, local privacy, orientation/color, UI a11y | cold Wasm startup, warm p95, bundle bytes, memory |
| Browser representative low-memory mobile | large and small images, repeated jobs, context loss, cancel | worker cleanup, limits, controlled fail without tab crash | peak memory estimates, task/UX success and heat/battery where measurable |
| Desktop (later) Windows/Linux/macOS | local file-to-file, batch, offline, print | file permissions, signed installer/update, path traversal, untrusted images | cold start, per-image p95, RAM/CPU, crash and recovery |
| Real candidate VPS (later) | realistic concurrency and adversarial input | host secret perms, TLS, firewall, resource caps, restart/rollback | successful jobs per month, total cost per success, peak host RSS |

**Evidence format for every measured result**: `{git_sha, artifact_digest, input_hash, codec, width, height, dpi, output_contract, machine, cpu_flags, compiler, build_profile, warmed, runs, concurrent_jobs, p50_ms, p95_ms, p99_ms, throughput_success_per_s, peak_rss_bytes, result_validation, failures}`. Fields lacking measurements should explicitly be `NOT MEASURED`.

**Performance gate policy:** begin by measuring current baseline; nominate a numerical target only after a representative workload and business value are selected. Use regression budgets per stage and preserve correctness. A docs-only PR has **zero new benchmark evidence**.

## 7. Next three engineering tasks (proposed, small and independently reviewable)

### PER-00 — FIRST: C/Rust performance truth + resource model (read-only)
- Current baseline SHA, source/ABI locks, queue and metrics reachability, current `bench_lat` and Rust service test invocation.
- Produce exact benchmark scenarios and expected output oracles; avoid rewriting benchmark tools until audited.
- Map total memory and predicted per-concurrency footprints; mark unknowns; don't claim a VPS fits.
- Produce test/reproducer proposal for C-A0 first and discuss a measurement-only follow-up PR.

### BROWSER-00 — PARALLEL DISCOVERY: first local Rust/Wasm MVP (read-only)
- Compare limited Worker Rust/Wasm vs browser APIs vs C17/Wasm and reconcile M0 ADR, P13 sequencing and front-end source.
- Define one validated local **resize** workflow, MIME/dimension caps, CSP and no-upload assertion; no Tailwind dependency choice before audit.
- Present bundle/mobile memory/quality/test plan; build the first implementation only after ADR approval.

### COMMERCIAL-00 — INDEPENDENT RESEARCH: customer and price validation
- Talk to 10 studio/service-center operators; record workflow, exact pain, measured time/rework and willingness to pay; choose a single pilot.
- Do not implement billing or buy infrastructure from interview answers alone. Propose Studio features by observed repeated work; API and desktop remain later.

**Link to:** [business and product strategy](IMGENGINE_PLATFORM_PRODUCT_BUSINESS_STRATEGY_2026-11-15.md) and [agent task brief](IMGENGINE_STRATEGY_AGENT_START_2026-11-15.md).

## 8. Decision rights

Astra/Codex/human engineers may read, test and draft approved bounded changes. They may not claim that code is production-safe based on docs, alter a public ABI incompatible with consumers, delete old Python verification tools, swap an unsafe queue without a reproducer, publicly deploy, merge, charge customers, rotate secrets or spend money without separately authorized scope. Apply the seven-level review model proportionally; an independent senior review is desirable for native unsafe, new API auth, desktop OS privileges and release.

**External technical references (re-check when implementing):**
- [MDN OffscreenCanvas](https://developer.mozilla.org/en-US/docs/Web/API/OffscreenCanvas)
- [MDN Worker createImageBitmap](https://developer.mozilla.org/en-US/docs/Web/API/WorkerGlobalScope/createImageBitmap)
- [Tauri v2 architecture](https://v2.tauri.app/concept/architecture/)
- [libvips](https://www.libvips.org/)
- [OWASP file upload](https://cheatsheetseries.owasp.org/cheatsheets/File_Upload_Cheat_Sheet.html)
- [OWASP API resource use](https://api-security.owasp.org/editions/2023/en/0xa4-unrestricted-resource-consumption/)
