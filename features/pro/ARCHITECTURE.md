# IMGENGINE Pro — native-first HLD, LLD, threat model and interface contracts

**PRO-00 / DESIGN PROPOSAL ONLY (2026-11-21).** The owner selected IMGENGINE Pro but explicitly deferred new Pro frontend implementation. This defines a prospective native C17 + Rust implementation, not already implemented methods, public ABI additions, working CLI, API or server execution. Read [feature PRD](README.md), [status/evidence](STATUS.md), [native test plan](NATIVE_TESTING.md) and [technology ADR](../../docs/TECHNOLOGY_AND_REPOSITORY_POLICY.md) together.

## 1. HLD: separation of concerns

```text
future local CLI or approved user interface (NOT IMPLEMENTED)
       |
       v
Rust recipe parser and schema-version validator
       |
       v
Rust application: bounded batch planner, input admission, lifecycle
       |                          |
       v                          +--> Rust export writer/manifest
safe Rust FFI façade                  (paths, collision handling, atomic output)
       |
       v
Existing C17 public ABI -> native scheduler -> codec/geometry/encode
       |
       v
Encoded result -> output verification -> privacy-safe manifest
```

Do not bypass native admission/scheduler or invent that current Rust API implements Pro. Preserve the explicit distinction between a native CLI/local runner and browser-local Pro: a Linux `.so` cannot be invoked directly by Nuxt in the browser. Future frontend decision compares native browser APIs, Worker/Wasm and desktop/hosted alternatives based on measured requirements.

### Bounded contexts / Hexagonal ports

- **Recipe contract** (pure Rust domain): schema version, stable validated operation grammar, independent output-profile definitions. Must not import HTTP, file APIs or billing.
- **Batch execution** (Rust application): admission, lifecycle, bounded fan-out and scheduling. Ports for image executor, input source, output writer and clock/metrics allow fault injection; no Kafka or event bus assumption.
- **Native engine adapter**: small `unsafe` FFI in existing Rust wrapper, owns native allocation/free and status translation. C owns codecs, pixel geometry, layout and existing scheduler—not recipes/accounts.
- **Export adapter**: safe directory resolution, filename collision policy, temp-file/write/commit, manifests; no user-supplied absolute output paths sent to C.
- **Future identity/entitlements** (separate context, **NOT NOW**): payments, accounting and licensing only after an explicit commercial/storage ADR.

Principles: DRY validated operation semantics, SOLID where adapter boundaries are real, clean/secure modular code, testable pure policy plus imperative I/O. DDD is a modeling aid, not a reason to add CQRS, microservices, distributed events or a separate database.

## 2. Proposed LLD data contract (Rust sketches, not committed types)

- `Recipe { schema_version, recipe_id, recipe_version, operations, outputs }`
- `Operation { type, parameters }` with a strict allowlisted tagged union; unknown types fail, no arbitrary shell commands, executable scripts or dynamic native options.
- `OutputProfile { name, codec, dimensions, fit_mode, quality, filename_template, metadata_policy }`
- `ExecutionBudget { max_input_bytes, max_decoded_pixels, max_total_pixels, max_outputs, max_peak_memory_bytes, max_parallel_jobs, deadline }`
- `JobResult { job_id, input_digest, profile_id, status, error_code, output_digest?, output_dimensions?, elapsed_ms? }`
- `BatchManifest { schema_version, recipe_digest, engine_abi, successful, failed, cancelled, results }`

These are *semantic shapes*, not automatically permitted public HTTP endpoints or concrete existing symbols. Fail closed on malformed/unknown versions; stable serialized codes with no raw pointers/internal filesystem locations. Units explicit; pixels and byte budgets must use checked multiplication/addition and account for **decoded + intermediate + output + concurrent** allocations.

## 3. Execution state machine

`RECEIVED -> VALIDATED -> ADMITTED -> RUNNING -> OUTPUT_READY -> VERIFIED -> COMMITTED`; failure exits to `REJECTED`, `FAILED` or `CANCELLED/ABANDONED` with cleanup evidence.

- Validate signature/format, per-input bytes, decoded dimensions and input count before costly allocation.
- Reserve capacity before execution (including all intended profiles); reject overload rather than unbounded queuing.
- Once in C ABI execution, Rust may abandon response, but must **not claim guaranteed mid-call native cancellation**.
- Verify encoded output format and declared geometry; write safely to a temporary path then publish per platform's documented rename/flush guarantees. Never silently overwrite original or successful peer output.
- Handle partial success as explicit per-file/per-profile outcomes; aggregate batch status cannot silently mask failures.
- Release C allocations exactly once, bound Rust buffers, clean scratch on errors/abandonment within documented crash limits. No image bytes or sensitive names in logs.

## 4. Versioning / compatibility

Version separately: Pro product release, recipe schema, native C ABI, Rust crate, output manifest and optional future HTTP/API contract. C17 production baseline, C11 consumer ABI checks and C23 isolated experiments follow existing native rules. Any new ABI symbol or struct requires ABI symbol/layout, allocation/free, threading and external consumer tests across the supported matrix. Document metadata/ICC/alpha and JPEG quality-loss implications; do not assume deterministic binary equality across encoders, CPU dispatch or library versions without proving it.

For output quality, verify exact geometry and specified codec policies, plus perceptual/visual golden fixtures where needed. SIMD and scalar paths must retain prescribed correctness. Keep an immutable provenance of image fixture digests and toolchain/image versions in test evidence.

## 5. Threat model and secure code gates

| Threat | Boundary/mitigation | Gate |
| --- | --- | --- |
| Decompression bomb / huge dimensions | byte and decoded-pixel checks, aggregate RSS/admission and deadlines | malicious-size fixtures + measured RSS |
| Decoder memory corruption | narrow codec dependency set, sanitizer/fuzz, sandbox review | ASan/UBSan/fuzz evidence when relevant |
| Recipe injection / unsafe flags | strict tagged schema, no scripts/paths/CLI interpolation | malformed/untrusted recipe tests |
| Zip-slip / filename collision | normalize confined output paths, reject traversal, no overwrite by default | ZIP/path fixture tests |
| Sensitive data leakage | ephemeral input/output by default, zero image content in logs/metrics; metadata policy | privacy review and failure-path tests |
| Cross-job contamination | per-job ownership, private temporary paths, explicit cleanup | concurrent/aborted jobs and leak tests |
| Native FFI lifetime/threading | RAII, isolated unsafe, documented scheduler serialization | ABI and Rust Miri/sanitizer where applicable |
| Supply chain / package licensing | SBOM, dependency advisories, codec and root project license reconciliation | release signoff, not assumed now |
| Commercial abuse | future accounts/entitlements, quotas, budget and refund design | blocked until approved future ADR |

No assertion that ordinary conversion guarantees malware removal, marketplace photo acceptance or visual fidelity. CISSP privacy and least-privilege, CCIE timeouts/backpressure/private network, and Linux/SRE resource/rollback requirements are explicit **when a server deployment is actually proposed**, not mandates to host Pro now.

## 6. CI/CD, performance and finance acceptance

- **Native milestone CI:** C17 debug/release/CTest; C11/public ABI compatibility; Rust safe FFI & external consumer; fixture correctness; change-scoped native sanitizers/fuzz, then all required exact-head GitHub CI.
- **Docker:** Windows host uses Docker Desktop Linux engine; Ubuntu 24.04 existing `infra/docker-compose.p7-frontend-dev.yml` `rust-api` service is invoked with `run --no-deps` and an overridden shell command for C/Rust-only tests; existing `infra/Dockerfile.p7-frontend-dev` reused. Actual results are **NOT_RUN** until executed.
- **Pro frontend:** no frontend code/config or Pro Playwright/UI tests authorized at this stage. Existing required repo-wide CI still applies.
- **CD:** **NOT AUTHORIZED**. No Pro hosted endpoint, container push, secrets, tag, payment, deploy, cloud account or SLA.
- **Measured performance:** p50/p95/p99 throughput and memory under standardized image size/codec, warm/cold repeated runs and all errors counted. Compare serial vs bounded concurrent variants and scalar vs SIMD only with correctness parity.
- **Commercial decision:** test repeated customer jobs and time saved; collect price sensitivity, support/refund cost and conversion data. Track net revenue versus cash, contribution, acquisition, churn, runway and billing reconciliation only when legally/commercially approved.
- **Responsibility:** junior/mid write clean testable slices; senior ensures errors/budgets; staff owns ABI/contract evolution; senior-staff handles rollout/compatibility; principal owns threat model and architecture; distinguished weighs product portfolios and simplicity. Product/finance/security/operations reviews complement rather than replace exact tests.

## 7. Implementation order

`PRO-00-NATIVE-01` **ABI/capability audit + Docker evidence** -> `PRO-01` recipe schema/validation -> `PRO-02` bounded batch executor -> `PRO-03` ordered transforms -> `PRO-04` multiple profiles -> `PRO-05` manifest/export -> later UI and commercial milestones **only after separate owner review**.

One short-lived `feat/pro/<milestone>` PR per bounded implementation, with source, tests, impacted docs, status transitions and explicit NOT_RUN/FAIL residual risk. **No unreviewed merge or public release.**
