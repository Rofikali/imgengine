# IMGENGINE Developer API — proposed contract, tenant safety, metering and capacity blueprint

**Status:** DESIGN ONLY / NO IMPLEMENTATION AUTHORIZED. This is the detailed future **Developer API** plan accompanying the [four-product portfolio](IMGENGINE_FOUR_PRODUCT_PORTFOLIO.md). It does **not** make the existing Rust/Axum server a public paid API. Keep [Rust API documented reality](api.md), [technology policy](TECHNOLOGY_AND_REPOSITORY_POLICY.md), [native/Rust performance taxonomy](IMGENGINE_PERFORMANCE_AND_MULTICHANNEL_ARCHITECTURE_2026-11-15.md), [C-A0 audit](C17_CORE_PRINCIPAL_SOURCE_LLD_DSA_SECURITY_AUDIT_2026-11-15.md) and [P14 actual-VPS runbook](P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md) authoritative. The route examples below are illustrative future contracts, **not live endpoints**.

## A. Customer and commercial contract before implementation

- **Developer personas:** indie app owner with occasional uploads; e-commerce platform with repeat product photography; print-service SaaS requiring physical sheets; enterprise integrator with strict quota, uptime and data-protection needs.
- **Job:** securely send an image plus a stable versioned transform recipe; receive a deterministic-size encoded result with machine-readable metadata and actionable errors, paying only for clearly defined work.
- **API isn't IMGENGINE Pro/Studio:** Pro and Studio are user-facing productivity plans. API is an **independently metered B2B** SKU with separate keys, quotas, invoices, accepted terms and abuse protection. No unlimited included API calls for other tiers.
- **Pricing hypotheses:** fixed finite sandbox quota, prepaid credits or monthly included capacity + capped overage. No public fee, initial free-credit amount, credit weight, minimum balance or SLA has been authorized. Partner interview + cost ledger first.
- **Product promise:** version stability, bounded processing, fair metering, transparent error/status definitions, API key revocation, privacy-first ephemeral payload handling, reliable documentation and support. Do **not** market guaranteed 1ms processing or 100,000 requests/sec.

## B. Technical interface proposal (requires explicit API ADR)

Illustrative future request, **not an existing endpoint**:

~~~http
POST /v1/images/resize HTTP/1.1
Host: api.example.invalid
Authorization: Bearer <client-api-key>
Idempotency-Key: example-client-job-0001
Content-Type: multipart/form-data; boundary=...

file: JPEG/PNG payload
width: 800
height: 600
output_format: jpeg
~~~

Illustrative response:

~~~http
HTTP/1.1 200 OK
Content-Type: image/jpeg
X-Request-Id: <safe-server-request-id>
X-Usage-Id: <stable-completed-operation-id>
Content-Length: <known-byte-length>

[actual JPEG binary bytes]
~~~

Failures return a safe, versioned JSON problem envelope (`application/problem+json`), never partial image bytes or native diagnostics. Proposed status cases: 400 invalid input, 401 unauthorized, 403 forbidden/disabled, 413 resource envelope exceeded, 415 unsupported media, 422 invalid encoded image, 429 per-client/global overload, 503 unavailable, 504 deadline (only if safe to make that determination); source-backed contracts may select different codes and must be tested. Define whether output was produced/consumed on disconnect. No customer-provided native flags, file paths, output budgets or dangerous processing options.

**Contract to freeze before code:** versioned routes and parameter grammar; unit/rounding/EXIF/color policy; input file and decoded-pixel caps; supported codec list; stable error codes; sync vs asynchronous decision; binary response vs stream semantics; request IDs and idempotency; image-byte retention/cleanup; response headers and cancellation semantics; sunset/deprecation policy. Existing native C ABI and P12/P13 contracts remain unchanged unless a separately authorized compatible change is required. New formats are capability-gated, not advertised speculatively.

**SDK strategy:** first supply a tiny audited curl/TypeScript/Rust example with configurable endpoint/key, timeouts, capped retries and typed errors. Do **not** publish an installable native C SDK, Wasm SDK or Python library by default. Version headers/capability discovery matter more than a large client generator.

## C. Reference architecture — isolate customer concerns from image execution

~~~text
Client application
  -> HTTPS edge: TLS, WAF/request size, IP abuse, operational limits
  -> Rust/Axum HTTP: parse + validate + authenticate tenant + scope
  -> Quota gate: hard usage/budget + per-tenant admission + overload policy
  -> Rust lifecycle and C17 safe FFI: decode/process/encode/cleanup
  -> binary output or structured problem response
  -> durable metering adapter / idempotency record (separate approved ADR)
  -> billing adapter / provider / reconciliation
  -> customer usage dashboard and support/audit access (redacted)
~~~

**Identity boundary:** service-to-service key currently used between Nuxt and Rust is *not* a public tenant credential. Customer API keys must have separate identifier + random secret, scope, ownership, creation/expiry, secure hash storage, rotation/revocation and server-side validation. Never embed secrets in browser JS, PR/docs, telemetry, URLs or return them after creation. Prefer a short prefix for identification and constant-time verification where applicable; rotate without unexpectedly breaking active integrations. Decide token signing vs lookup and provider integration in a separate security ADR.

**Architectural contracts:**
- **Rust application services** own authentication, quotas, usage lifecycle, idempotency, payments adapter and typed errors, *not* the native engine or the Nuxt browser.
- **C17 ABI/worker** owns validated image computation and releaseable output memory. Customer IDs and billing metadata must not cross the ABI unless demonstrated necessary and safely versioned.
- **Nuxt** can show docs/account/usage pages; it is not the authoritative quota/billing gate. A B2B client might invoke Rust directly via an edge gateway rather than the existing trusted Nuxt BFF, but that topology needs a separately approved threat model/network/host capacity test.
- **Durable metadata** is different from uploaded images: keys, successful-usage events, credits, payment identities, audit and invoices must survive restarts; raw images remain ephemeral. **Do not use an in-memory counter for money.**
- **Storage ADR required:** compare explicit owner-approved options (e.g. Rust-managed embedded SQLite under secure backup/locking controls versus provider-hosted billing/identity records), host failure modes, migration, encryption, availability, cost, reconciliation and owner control. The existing technology ADR expressly forbids *automatic* adoption of PostgreSQL/Redis/Python; no hidden exception.
- **No premature jobs/queues/microservices:** start synchronous only if measured size/latency allows it. Bulk/async notifications and job persistence need separate contract/design/release approvals.

## D. Fair metering and exact-once **effect**, not magical exactly-once transport

**Potential billable unit:** one successfully completed requested image transformation *after* authorization and within agreed dimensions, with price-version snapshot. Input size is not sufficient to estimate expense—decoded pixels, selected codec, output bytes, CPU duration and memory concurrency matter. Define charges for each operation from actual benchmarks and customer interviews; no numbers invented.

**Draft request accounting state machine** (not implemented):

1. `RECEIVED`: validate size, version, image type and request shape. Invalid requests **not billable**.
2. `AUTHENTICATED`: resolve tenant/key, signed scopes and plan; obtain tenant policy. Missing/invalid or suspended account rejects without billing.
3. `QUOTA_RESERVED`: atomically reserve a bounded concurrency slot and at-most-cost allowance. Rate-limit or budget-limit rejects without billing. Charge units cannot be unlimited just because the caller has a valid key.
4. `RUNNING`: dispatch only validated work to bounded Rust/C17 lifecycle. Respect CPU/RAM/decoded size/deadline and already documented cancellation limits.
5. `OUTPUT_READY`: validate successfully encoded result and sizes, release native resources, calculate actual billable units using the pinned tariff/version and operation record. Avoid exposing success if the ledger cannot persist a required charge.
6. `SETTLED`: durably commit **one** usage event per operation; mark reserved balance consumed, handle response delivery and retry state. Document the case where work succeeded but network failed before the client received it: idempotent retries must not generate duplicate charges; decide response retention/expiry vs safe replay (ephemeral privacy may forbid replay).
7. `FAILED_OR_ABORTED`: reconcile and release unused reservation, record safe failure class, never charge failed work as success. Crash recovery must find orphan reservations.
8. `RECONCILED`: daily comparisons of completed usage events, payment provider statements, customer credit balances/refunds and invoices; discrepancy alerts and support correction journal, with immutable correction provenance.

**Nuance:** There is no exactly-once network delivery guarantee. A stable idempotency key plus digest of canonical request fields and uploaded content *may* detect true retries, but hashes of sensitive image bytes need threat/privacy/retention review; don't log/hash filenames or retain images by default. If a key is reused for different inputs, reject safely. Define key TTL, concurrency locking, retry/timeout policy and replay limitations before implementation. Client retry must not charge a second time for a previously settled operation. If the worker fails after reservation and before charge, an atomic ledger strategy must release/refund safely. Decide what is billable if output is produced but transmission fails; do not hide the ambiguity in product terms.

**Canonical proposed metadata only** (illustrative field names):
- `request_id`, `tenant_id`, `api_key_id` (never secret), `idempotency_key_hash`, `request_contract_version`, `operation`, `price_version`, `reserved_units`, `settled_units`, `settled_at`, `status` and failure class.
- Optional measured fields: original byte count, validated decoded pixels, encoded byte count, CPU time, wall time, limit class, request size tier, capacity and instance ID.
- Immutable `ledger_event_id` / monotonic audit ordering, provider reconciliation reference, correction event and actor. No photo bytes, EXIF contents, names, credentials or original download URLs.

**Ledger invariants / test cases:** unique event per settled operation; retries with same key+payload produce at most one charge; different payload on reused key rejects; simultaneous requests cannot exceed quota or credits; crash before/after settle reconciles correctly; provider webhook duplicates/out-of-order deliveries are safe; refund/chargeback adds auditable correction rather than deleting historical charge; negative balances blocked unless explicitly authorized; invoices match signed-off ledger; client cannot change tariff headers; account/key revoke works immediately or within documented bound.

## E. Pricing and tenant-cost safety

**Quota dimensions:** requests/minute, in-flight operations, decoded-pixel capacity, compressed input MB, output bytes, per-tenant daily/monthly credit cap, API key scopes and overall service global budget. HTTP 429 is a policy event; if hardware exhausted without quota violation, 503/429 requires an explicit API contract. Admission must avoid starvation; weighted fairness only after capacity testing.

**Cost model per operation:**

~~~text
True service cost per successful output =
  (CPU + memory/host share + egress + failed attempts + storage/ledger
   + payment/provider fees + refunds/chargebacks + support + operations)
  / successful billable outputs in the measured period

Net contribution =
  recognized revenue after discounts/refunds/tax accounting policy
  - attributable variable operating costs

Break-even paid developer customers =
  fixed recurring costs / positive average contribution per active customer
~~~

**Example only, not a price commitment:** if an operation occupies 0.2 single-core CPU-seconds and there is one continuously available core, the naive CPU-only ceiling is 5 operations/s; practical throughput is **lower and workload dependent** due to encoding, memory, I/O, locks, queues and safety margin. The existing engine uses synchronization that may serialize some native jobs. A 1 GiB VPS with fixed Compose memory limits has **no demonstrated paid-API capacity**. CPU utilization target, workload mix, billing units, max burst and pricing must come from actual host tests and fail-safe defaults.

**Business rules before any launch:** published rate card with transparent unit definition and version, effective-date policy, hard-capped customer budgets by default, alert thresholds, invoice downloadable history, credits/refund terms, unauthorized-use dispute process, taxes/currency/FX check, fair-use policy and a way to disable API keys quickly. Never silently auto-raise limits or overage bills. Consider prepaid credits first if it makes spending safer for both customer and founder; provider selection is separate.

## F. Load-testing and capacity characterization

**Per exact environment** record: Git revision/image digest, CPU model/architecture, memory/available RAM, provider plan and tariff, Linux/container limits, native library versions, codec and input distribution, request concurrency, p50/p95/p99, total RPS with error counts, kernel memory high-water mark + OOM events, Rust/Node/process memory, egress, output bytes, CPU-seconds/success, response-abandonment and queue delay. Compare cold and warm separately. Do not infer performance from C microbenchmarks alone.

**Representative matrix:**
- Small/medium/large JPEG and PNG; portrait/landscape/odd sizes; alpha; progressive/EXIF where supported.
- Invalid header/MIME, too many pixels, decompression bombs, malformed data and truncated payload.
- Exact boundary input/output/decoded budget, timeout, abort/disconnect, duplicate retry, oversize upload, unsupported format.
- Concurrency 1/2/4/8 only as allowed by RAM; multiple tenants with one noisy client; quota/credit exhaustion.
- Idle/warm/cold startup, restart and cleanup; cgroup OOM and no leaked sensitive files.
- P12/P13 sheet vs simple transform *if* those are included in the future API contract; **different unit cost classes**.

**Go/no-go**: demonstrate no persistent disk leaks, sane response-code classifications, fair rejection, stable p95 under the agreed offered load, memory+host overhead below physical budget with reserve, cost-per-success margin that survives failure/retry/malicious traffic, and recoverable restart/rollback. No universal throughput or availability guarantee.

## G. Security, operations and business launch gates

1. **Threat model:** malicious uploaded images, high-compression bombs, oversize decoded buffers, path traversal, cross-tenant leakage, credential theft, API abuse, replay, request floods, SSRF if future URL fetch exists, billing manipulation and accidental PII retention.
2. **API docs and compatibility:** OpenAPI contract where useful, versioned examples, code sandbox, stable error catalog, deprecation windows, a real status/help contact and support boundaries.
3. **Supply chain:** maintain C17/C11/FFI/ASan/UBSan/fuzz/Ubuntu verification when relevant, image codec patch policy, pinned provenance, signed release candidates and artifact identity; browser Free CI is not an API release substitute.
4. **Runtime:** real P14 network isolation, public TLS/DNS, file-backed secret permission checks, host firewall, bounded memory/CPU/swap/PIDs/tmpfs, private render credentials, audit/log redaction, monitoring/alerts, backup of account/ledger/certificate data and rollback drills.
5. **Payments/privacy/legal:** obtain jurisdiction-appropriate counsel/accountant review of digital services taxes, invoices, refunds/consumer contracts, B2B privacy terms/data processor responsibility and breach incident response. Never assert India GST eligibility/exemption without a real dated review.
6. **Operations:** define support hours and escalation, billing correction owner, key reset/rotation, incident and abuse response, data retention/deletion and operational recovery; no fabricated SLAs.
7. **Release approval:** separate explicit owner signoff of billing provider, database/storage ADR, cost risk, public API activation, DNS/secrets, any payment obligations and ongoing vendor spend.

## H. Delivery sequence and evidence

| Milestone | Deliverable | Done only when |
| --- | --- | --- |
| API-00 | Customer discovery, measured benchmark, cost envelope | Customer requirements + recorded exact-hardware cost metrics |
| API-01 | Reviewed API/tenant/contract design and private sandbox | Tests for validation/auth/quotas, versioned docs, synthetic test keys and private access; reviewed PR |
| API-02 | Durable metering/idempotency and approved storage/payment ADR | Crash/race/webhook reconciliation tests, schema migration/backup, real invoice and credit ledger audit |
| API-03 | Private partner beta | Revocable keys, caps, consent/privacy, real load/security/operational measurements and correction support |
| API-04 | Pricing/business release signoff | Unit economics, taxes/refunds/terms, independent security review, actual host release proof |
| API-05 | Public paid API (separate owner-go/no-go) | All exact-SHA CI/P14 + rate-limit, cost/abuse, payment and rollback approval |

**Next code milestone remains F1 Free resize, not API-01.** Drafting this Developer API plan is not permission to open a payment account, introduce a database, purchase hosting, run production traffic, merge or deploy. Preserve the owner-selected four-product portfolio, keep one small PR per implementation milestone, and update [the live queue](../next.md) with only verified state.
