# IMGENGINE — Four-product portfolio, product requirements and phased business plan

**Decision:** On 2026-11-16 the owner selected four product names and business directions: **IMGENGINE Free, IMGENGINE Pro, IMGENGINE Studio and IMGENGINE Developer API**. The Free **first release includes exactly five tools**: **Crop, Resize, Compress, Rotate, Convert**. This document is the canonical *portfolio and execution proposal*, not acceptance of prices, launches, taxes, billing providers, storage engines, performance numbers, or automatic deployment. **Status: PORT-00 owner-directed documentation PR; paid implementations PLANNED/DEFERRED; no new product publicly released.**

**Source baseline:** main at `e26a9194a3c9f58059fcd81c8bd78cffd2199c33`, after F0 [PR #55](https://github.com/Rofikali/imgengine/pull/55) merged. F0 establishes browser-local resize **before** optional measured Rust/Wasm. [Live milestones](../next.md) are authoritative for current states; old roadmap milestones and snapshots are historical, not additional launch requirements. Existing [binding stack ADR](TECHNOLOGY_AND_REPOSITORY_POLICY.md), [F0 ADR](F0_BROWSER_FIRST_SEQUENCING_ADR.md), [P12 physical sheets](P12_PHYSICAL_SHEET_CONTRACT.md), [P13 crop](P13_INTERACTIVE_CROP_CONTRACT.md), [security/actual-host release checklist](MVP_RELEASE_CHECKLIST.md), [technical performance plan](IMGENGINE_PERFORMANCE_AND_MULTICHANNEL_ARCHITECTURE_2026-11-15.md), and [milestone continuation runbook](AGENT_MILESTONE_CONTINUATION_RUNBOOK.md) take precedence on their respective technical and release boundaries.

**Companion guides:** [Developer API, usage ledger and capacity blueprint](IMGENGINE_DEVELOPER_API_USAGE_BILLING_CAPACITY_BLUEPRINT.md) and [commercial release/finance/customer validation gates](IMGENGINE_COMMERCIAL_READINESS_AND_UNIT_ECONOMICS.md). No code or deployment changes are authorized by this plan.

## 1. One platform, four distinct customer jobs

| Product | Who pays / uses it | Customer job | Commercial mechanism | Launch state |
| --- | --- | --- | --- | --- |
| **IMGENGINE Free** | Students, applicants, individuals | One image, one task, no account or server upload | Free browser tools; ads **only after** approval/policy/privacy checks | **Five tools planned, not released** |
| **IMGENGINE Pro** | Creators, small sellers, frequent individual users | Repeat or automate a recipe for many images | Subscription or prepaid workflow access **hypothesis**; ad-free optional | **Planned; no billing or paid entitlements** |
| **IMGENGINE Studio** | Computer shops, photo studios, print/application agents | Reliable operator flow and print sheets for paying customers | Studio seat/shop subscription or job-pack **hypothesis** | **Planned; existing P12/P13 server foundation is NOT a paid Studio product** |
| **IMGENGINE Developer API** | Software developers, SaaS/e-commerce integrators | Automate image jobs in their applications via stable HTTP APIs | Metered successful work with quotas, prepaid or subscription + overage **hypothesis** | **Planned; existing private Rust API is NOT a metered multi-tenant public API** |

Keep offers distinct. Pro pays for personal/professional repeatability; Studio pays for *print/shop* accuracy and operator productivity; API pays for *integrated server compute*. Studio does **not** silently include unrestricted API usage. Free does **not** become an email/phone collection funnel. All product prices, entitlements, credit weights, tax treatments and launch dates require separately recorded owner decisions and customer evidence.

### Differentiation and product positioning

- **Free promise:** five useful, accessible, privacy-first local operations. Avoid deceptive artificial CPU throttling or mandatory uploads merely to upsell.
- **Pro promise:** save time on repeat work (recipes, bounded batch, consistent export and naming), not sell access to a basic crop algorithm.
- **Studio promise:** faster, auditable, print-accurate operator workflow (presets, margins, DPI, actual placement count, proof/print output), not an unverified government-acceptance guarantee.
- **API promise:** predictable, documented, capacity-bounded processing and fair billing with trustworthy errors, usage reporting and support; not unsupported nanosecond or infinite-throughput claims.
- **Not now:** Canva-scale rich editor, audio/video, generative AI background removal, large collaborative storage, Kubernetes/microservices, native SDK packaging, mobile app, desktop installer or mass SEO doorway pages without independent need.

## 2. IMGENGINE Free — exactly five tools in first launch

| ID | Tool | Minimum useful first-release behavior | Must test / disclose |
| --- | --- | --- | --- |
| F1 | **Resize** | Local JPEG/PNG import; integer target width/height, aspect lock, preview and PNG download. Preserve [F0 contract](F0_BROWSER_FIRST_SEQUENCING_ADR.md). | Exact output dimensions, overflow/pixel caps, orientation, stale-result cleanup, low-end mobile memory, no-upload proof |
| F2a | **Crop** | Manual selectable region with keyboard/touch controls, accurate export of selected area, explicit aspect and reset. | Rectangle bounds, 1-pixel/odd edges, coordinate rounding, focus, cancellation, privacy |
| F2b | **Rotate** | Lossless-in-intent right-angle 90°, 180°, 270° transforms of a local source; explicitly describe re-encoding quality when exported as JPEG/PNG. | Width/height swap, orientation/EXIF behavior, transparency, repeated-operation state, metadata disclosure |
| F2c | **Compress** | Local JPEG/WebP quality-oriented export where supported; show actual resulting size and failure honestly. | File-size and visual quality, impossibility of guaranteed target KB, codec support, decoded/output RAM caps, no silent image upload |
| F4 | **Convert** | Browser-supported conversion among **JPEG, PNG, WebP**, with explicit supported format list and output download. | MIME signature and decode validation, alpha-to-JPEG background policy, browser support differences, EXIF/color metadata loss, failures |

**Exactly five tools** in the initial Free public product. **Flip, PDF, print sheets, passport presets, watermark, batch, background removal, target-20KB guarantees** and other tools may be later additions only through separate milestones. A rotate UI may reuse transform primitives but is not a sixth advertised free tool. Existing **P12/P13 server-backed print/crop** is a separate implementation path and does **not** make F2a's **zero-upload** browser crop complete.

**Common free-tool contract:**
1. Client selects a file; strict type/signature/dimension/byte limits, memory budget and defensive decoding apply before allocation and processing.
2. Render using Nuxt/Vue/TypeScript and browser APIs **on device**. Evaluate Web Workers and Rust/Wasm in a later measured F3 decision; do not make Wasm a prerequisite to shipping F1/F2/F4.
3. Absolutely no source bytes, result bytes, sensitive filenames or EXIF in server HTTP, logs, analytics, cache keys, persistent storage or third-party ad SDKs. Serving page assets is allowed and not image upload. Do not auto-fallback to Rust/Axum; any future server path requires informed opt-in and a separate contract.
4. State orientation, color, lossy re-encoding, transparency, unsupported-codec and metadata behavior. In-browser image outputs need explicit dimensions and download controls.
5. Accessible keyboard/mouse/touch operation, actionable errors, disabled states while busy, honest progress, mobile layout and localization-ready copy.
6. Revoke object URLs, release ImageBitmaps/Workers/canvases, cancel or discard stale work after replace/reset/unmount, cap job time/dimensions/batch and verify repeated operations. Browser/Wasm is **not** a security boundary.
7. Perform synthetic fixture checks, privacy network capture including failure paths, unit/browser/e2e negative tests, dependency/license review and existing required GitHub CI.

**Free site structure, not an implementation promise:** a useful landing page plus routes such as `/resize-image`, `/crop-image`, `/compress-image`, `/rotate-image` and `/convert-image`. Every page must offer the real accessible tool, honest limitations and meaningful help; do not publish duplicate thin SEO pages. No accounts necessary at Free launch. If advertising is later enabled, obtain network/publisher approval, policy review, consent where applicable and suitable placement away from crop/preview/Download controls. Ad revenue, traffic, approval and profitability are **unknown**.

## 3. IMGENGINE Pro — paid creator workflow, not paid basic pixels

**Target customers:** frequent creators, listing sellers, freelancers and people repeating the same job. Discovery hypothesis: they pay to reduce manual steps, not because a single local resize is exclusive.

**Initial candidate paid differentiators** (one feature per separately reviewed PR):
- **PRO-01 — reusable named recipes:** dimension, aspect, export format, quality, naming rule and version; local export/import of schema-versioned recipe JSON can be first, without an account or cloud database. Validate every imported field; avoid executing user-provided scripts.
- **PRO-02 — bounded batch processing:** apply a recipe to a selected group with defined file-count, pixel, memory and concurrency limits; per-file errors and cancellation; partial results list; downloadable bounded ZIP where safe. Browser-local to reduce variable hosting cost.
- **PRO-03 — multi-step workflow:** deterministic resize → crop → rotate → compress/convert ordering with preview; explain repeated JPEG quality loss and preserve originals.
- **PRO-04 — productivity polish:** reusable profiles, job outcome report, naming collision handling, accessibility, ad-free experience where ads exist, and explicitly stated offline behavior.

**Not included automatically:** cloud sync, unlimited batch processing, background removal AI, team seats, unlimited remote processing, API credits, service-level agreements or storage. Browser-only DRM/entitlement checks are not tamper-proof; selling Pro requires a commercially honest value model and a **separate approved entitlement/account/payment design** where enforcement is needed. No subscription screen or payment integration until this decision exists.

**Acceptance:** demonstrate actual saved minutes per repeat job versus Free, repeat-user engagement and willingness to pay; verify no-upload, device limits, retry/cancel, ZIP safety, saved-recipe schema migration and clear refund/support terms. **Price experiment examples only:** ₹199/month or prepaid job packs; not approved rates or revenue forecasts.

## 4. IMGENGINE Studio — professional print/service-center product

**Target customers:** photo studios, computer shops, print agents and other businesses that handle multiple customers and print layouts daily.

**Candidate milestones:**
- **STU-00 customer discovery:** observe 10–20 representative operators; measure current minutes/order, crop/print mistakes, price sensitivity, computer/printer setup and connectivity; choose one repeatable job, not a Canva-style suite.
- **STU-01 validated print presets:** paper sizes, DPI, physical output dimensions, gap/margin/bleed/crop marks as actually supported; supported output format and versioned preset library. Verify regional/authority-specific specs from authoritative dated sources with review/expiry; never guarantee photo acceptance.
- **STU-02 operator workflow:** select file → manual crop/placement → preset → copies/sheet → print-ready proof → download/print. Reuse existing P12/P13 contracts as *foundations*, preserving centered-fill omission and ABI 2.3 compatibility; do not misrepresent the currently server-backed workflow as browser-local.
- **STU-03 multiple-customer bounded batches:** explicitly chosen files and safe temporary workspace, individual job accounting, restart/cleanup, bulk export with no cross-customer leakage. Avoid persistent sensitive customer images by default.
- **STU-04 optional advanced shop capabilities** only after evidence: print calibration, operator accounts/seats, controlled reprint tokens, locally saved named recipes, consented remote processing, priority *server* admission backed by measured capacity.

**Paid value:** consistent print quality, reduced operator rework, fewer wasted sheets and shorter service time. Studio has precise privacy/customer consent obligations; retention of ID/passport photos is **opt-in and separately reviewed**, not a default account feature. Printing, image acceptance and physical calibration require on-device validation.

**Operational gate:** browser-local Studio features may be prototyped without exposing Rust. Any public server-backed Studio feature requires native C-A0 safety disposition, aggregate resource and admission evidence, real-host P14 controls, TLS, abuse prevention, rollback and separate human release approval. A $10 VPS or Docker memory ceilings do **not** prove Studio concurrency.

**Price experiment examples only:** ₹399/month per shop or bounded job packs; benchmark customer savings and support costs before publication. Do not advertise multi-user SaaS/SLAs that do not exist.

## 5. IMGENGINE Developer API — B2B, metered server compute

**Customer:** application developers embedding image processing in their own e-commerce, SaaS, photo-service or print workflows. Their benefit is not building a renderer, keeping versions patched, handling input attacks or managing capacity themselves.

**Candidate order (not shipped):**
1. **API-00 discovery + measurable baseline:** interview developer integrators; collect exact output formats, throughput, payload sizes, failure rates and fairness expectations. Benchmark native C17/Rust response bytes, peak RSS, queue/admission, p50/p95/p99 and cost per successful request on chosen hardware.
2. **API-01 versioned contract + sandbox:** design stable service URLs, request validation, binary output, structured problem JSON, capability discovery, documentation, TypeScript/Rust example clients and a bounded test key with explicit finite quota. Never publish existing shared Nuxt→Rust key as a customer credential.
3. **API-02 tenancy and enforcement:** per-customer identity, scoped revocable/rotatable keys, server-side auth, quotas, resource-based admission, 429/backpressure, idempotency, safe audit logs, signed versioned entitlements, hard budget caps and customer-visible usage reporting.
4. **API-03 durable metering + commercial controls:** choose an approved metadata storage/provider architecture; immutable billable ledger, price-version snapshots, dedupe, reconciliation, payment/refund/failed-renewal handling and manual dispute procedure. Image bytes remain ephemeral.
5. **API-04 controlled private beta:** real authenticated clients with conservative usage caps, tested support paths, invoice/export accuracy, measured cost/abuse/capacity and revocable access.
6. **API-05 public paid launch** only after separate owner go/no-go, security/privacy/payments/tax signoff, P14 actual-host evidence, recovery and monitoring. No guarantee of 99.9% SLA until operational evidence supports it.

**Billing:** start with a small, auditable product contract such as prepaid units or a base allowance with measured overages; **one billing unit is not necessarily one HTTP request**. Determine image complexity weights using measured decoded pixels, codec and CPU/memory/egress costs. Invalid/overloaded/failed requests should not be billed as successful completed transformations; document network loss and retry/idempotency semantics. Implement usage accounting in Rust/application policy or an approved managed provider, **never inside C**. See [API metering/capacity blueprint](IMGENGINE_DEVELOPER_API_USAGE_BILLING_CAPACITY_BLUEPRINT.md).

**Existing reality:** Rust/Axum provides protected server-side rendering and binary JPEG / problem-details JSON responses through a private Nuxt proxy. It is **not yet** a productized public multi-tenant, metered, customer-self-service API; current formats/endpoints cannot be advertised as future capabilities without proof. Do not promise WebP API, async queues, PDF, streaming, webhook delivery, SDKs or bulk processing until designed and tested.

## 6. Shared HLD and clear Hexagonal/DDD boundaries

~~~text
Public internet
  |
  +-- IMGENGINE Free (Nuxt/Vue/TS) -- browser codecs / Canvas
  |                                    [optional measured Worker/Rust-Wasm]
  |                                    image bytes stay on device
  |
  +-- IMGENGINE Pro (Nuxt) ----------- local saved recipe / bounded batch
  |                                    [future account/entitlement adapter]
  |
  +-- IMGENGINE Studio (Nuxt) -------- browser print tools OR consented proxy
  |                                    -> Rust/Axum request validation
  |
  +-- IMGENGINE Developer API ------- edge auth/quotas/admission (future)
                                       -> Rust/Axum application services
                                         -> safe FFI -> C17 .so engine
                                       -> JPEG binary response
                                       -> durable usage ledger (future ADR)
                                       -> billing provider (future ADR)
~~~

**Responsibilities:** C17 owns pixel/geometry/codec/scheduler and versioned ABI, **not** customer billing or passwords. Rust owns server authentication, tenant policy, request limits, error mapping, metering and transaction boundaries *after approved implementation*. Nuxt owns UI, browser-local pipelines and safe same-origin proxy; browser-provided entitlement/usage headers are **untrusted**. External identity/billing/storage providers are replaceable adapters, not a mandate for microservices or a database.

**Preferred patterns only when justified:** functional core/imperative shell for pure transforms; strategy/capability registry for codecs; adapter interfaces for metering, authentication and payment providers; explicit finite-state machines for jobs and billing; dependency inversion to test error handling. Keep clean interfaces, short modules, SOLID/DRY/security hygiene; **do not** force event buses, CQRS, factories or DDD aggregates into a one-file browser crop tool. Shared deterministic test fixtures are more important than shared code when runtime environments differ.

**Portability:** C17 and Rust .so native on a Linux server require the right target architecture and runtime libraries. Browser Rust/Wasm requires its own build/ABI threat model and benchmark and is *not* the same binary as Linux `libimgengine.so`. No Python/FastAPI/Django/Flask, Celery, Redis, PostgreSQL or new Python tooling; existing legacy components and verifiers are retained until separately approved, tested retirement. A durable billing-data decision must be its own storage ADR, not implicit permission to introduce a forbidden service.

## 7. User journeys and entitlement boundaries

| Journey | No login? | Upload image to server? | Entitlement and storage |
| --- | --- | --- | --- |
| Free one-file tool | **Yes**, by design | **No**, default and required | No user account, no photo persistence |
| Pro saved local recipe / local batch | Possible, depending on commercially honest license mechanism | No for local-only workloads | Schema-versioned local recipe; license/access model later ADR; no default cloud sync |
| Studio local print preset | Possible for proof-of-value | No if truly local | Validated preset, source/expiry, no customer photo retention |
| Studio Rust/C17 print | Identity/consent requirements to decide | **Yes, only after user chooses** | Server-side control, ephemeral image, verified cleanup; no unapproved public exposure |
| Developer API paid job | **No**, server credential required | **Yes**, caller knowingly sends data | Per-tenant identity and durable usage **metadata**, no persistent uploaded images by default |

Pro/Studio/API names are **not** equivalent to separate always-on deployments. Avoid adding database/queue/extra processes just because there are four brands. Shared infrastructure must not allow one customer's overload to starve all customers. Paid operations require separately tested quotas and isolation.

## 8. Prioritized dependencies, not an endless infrastructure backlog

| Phase | ID | What the agent may work on | Hard gate |
| --- | --- | --- | --- |
| **NOW** | **PORT-00** | This portfolio/catalogue, linked agent process and coherent live queue; docs PR only | Owner direction recorded, source facts checked, exact PR CI, human merge |
| **NOW / NEXT** | **F1** | First browser-local resize vertical slice | F0 merged (PR #55), browser privacy/correctness/accessibility, one bounded PR |
| **NEXT** | **F2a → F2b → F2c** | Crop, rotate, compress separately | No-upload, device resource caps, negative/preview tests for each |
| **NEXT** | **F4 → F5** | Convert, then free-site release readiness | Five functioning tools; user privacy/hosting, ad/legal/accessibility, separate launch decision |
| **PARALLEL SPIKE** | **F3** | Compare Worker/browser native to Rust/Wasm on measured workloads | Does **not** delay five-feature release without verified blocker |
| **AFTER first user evidence** | **PRO-00/01/02** | Interviews, saved recipes, bounded batch, export | Explicit entitlement/price/privacy decisions before paid checkout |
| **AFTER studio interviews** | **STU-00/01/02** | Real print workflows/presets; avoid speculative full editor | Versioned preset, test proofs, server P14 gate where applicable |
| **AFTER backend safety + developer evidence** | **API-00 → API-05** | Capacity baseline, sandbox, tenancy, ledger, beta, paid API | C-A0/relevant native fixes; P14 actual-host; security/billing/storage ADR; explicit launch |
| **LATER** | SDK/desktop/enterprise/AI | Only if demand and economics support it | License, provenance, target-platform verification and business owner decision |

**Do not treat all listed phases as simultaneous coding authorization.** Select the highest-priority **READY** bounded next milestone, starting with F1. All four product *directions* were requested, but paid implementation, billing provider procurement, payment collection, secrets, domains, tax registration, launch and merge still require explicit later approval. Update [`next.md`](../next.md) after each actual milestone and reconcile merged PRs before repeating.

## 9. Release-quality matrix and measurable KPIs

| Category | Free launch | Pro / Studio | Developer API |
| --- | --- | --- | --- |
| Correctness | Saved fixtures, exact resize/crop/rotation, image formats, no-upload | Batch isolation, profile/print DPI, repeatability | Versioned API, response accounting, retries/idempotency |
| Security | CSP/privacy, memory and decompression-bomb limits, dependency provenance | Accounts/entitlements if present, customer-photo consent, ZIP/preset import safety | Tenant-scoped keys, abuse/rate/quotas, C-A0, per-tenant ledger, audit |
| UX | Accessibility/mobile/keyboard, error recovery and support docs | Operator time-to-done, cancellation, print proof, ad-free UX | Copyable SDK samples, error codes, status/capability docs |
| SRE | CDN/build uptime, page asset bandwidth, errors | If hosted: actual-server limits, backup/rollback and support | Real P14 host, TLS/monitoring/alerting, 429 fairness, overload and recovery |
| Finance | Real ad RPM (if approved) vs traffic/domain/hosting/support costs | Conversion, churn, refunds, contribution per paying customer | Cost per successful unit, egress/CPU/RAM, bad-debt, revenue reconciliation, margin |
| Privacy/legal | Notice, cookie/ad consent review, license/terms/accessibility | Payment/renewal/refund and sensitive-document policies | B2B terms/DPA as needed, processor agreements, tax, incident response |

**Founder's evidence loop:** interview 10–20 actual users/professionals, manually observe a repeated job, record minutes saved and tolerated errors, ask willingness-to-pay *before* fixing SKU/prices; avoid treating website visits as customers or gross revenue as profit. Track activation (successful download), repeated weekly use, successful batch/print/API jobs, conversion, retention, support/refunds, cost per operation, CDN/Wasm egress and advertising policy risk.

**Financial formulas:** gross billings != recognized revenue != profit; contribution = net collected/recognized revenue less directly attributable processing, bandwidth, payment, refunds, support and tax/cost allocations (state convention). Break-even customers = fixed monthly cost / positive contribution per active paying customer. For API, use cost per billable success **including failed/abusive traffic** in numerator and compare across p50/p95 workloads, not a fabricated minimum $5 VPS cost. Prices/FX/provider tariffs require dated provider check and CA/tax review in India and other markets. See [finance checklist](IMGENGINE_COMMERCIAL_READINESS_AND_UNIT_ECONOMICS.md).

## 10. Essential missing decisions and safeguards (explicitly deferred)

1. **Legal/IP** — select actual product license and third-party code/codec/license obligations; no inferred MIT license. Ensure useful terms, privacy policy, refund/consumer/payment, governing jurisdiction, advertiser disclosures and appropriate India/other-market professional compliance review.
2. **Privacy/security** — ID/passport images need extra care; local-first by default, no analytics image/filenames, consent/retention and incident-response contracts for any server feature.
3. **Monetization** — no AdSense approval or page RPM guaranteed; no published Pro/Studio/API prices, customer accounts, quotas, credit balances or paid checkout presently implemented.
4. **Data architecture** — choose payment identity, durable metadata and migration/reconciliation design explicitly; no default PostgreSQL/Redis. Verify backups, restoration, deletion, retention, key rotation and audit evidence before shipping.
5. **Capacity/quality** — real CPU/peak RSS/admission/egress measurements, test mobile devices, malformed uploads, concurrent loads, cancellation/timeout/rollback and independent image-quality tolerances.
6. **Operations** — support email/workflow, documented refund/dispute escalation, status page if justified, reliability objectives after observed evidence and actual operator ownership; don't sell support/SLA promises before capacity.
7. **Growth** — customer research, one differentiated tool page per need, helpful tutorials, SEO/accessibility, content accuracy, referral/affiliate disclosure as needed; no fake traffic or forced ad clicks.
8. **Governance** — one narrow PR per milestone, seven existing CI checks remain required, owner approval before merge/deploy/spending, never mark docs as implemented features, and never automatically start a paid service.

## 11. Ownership and decision record

| Decision | Owner status | Implementation status | Requires later decision |
| --- | --- | --- | --- |
| Browser-first five-feature Free | **OWNER-SELECTED** | **PLANNED / F1 next** | Feature PRs; then hosting/privacy/launch approval |
| Four product names Free/Pro/Studio/Developer API | **OWNER-SELECTED** | **PORTFOLIO DOCUMENTATION ONLY** | Separate commercial SKUs, prices, account/payment and release decisions |
| No new Python; C17/Rust/Nuxt stack | **BINDING EXISTING ADR** | P12/P13 server foundation implemented; Free pipeline not shipped | Any architectural exception must have its own owner-approved ADR |
| Paid Pro or Studio entitlement | **NOT APPROVED** | **NOT IMPLEMENTED** | Value validation, provider/entitlement/account ADR, taxes/refunds and pricing |
| Public metered Developer API | **NOT APPROVED TO LAUNCH** | **NOT IMPLEMENTED** | Capacity, quota, ledger, billing provider, storage ADR, P14 and security signoff |
| Server/hosting/public launch | **NO DEPLOYMENT AUTHORIZATION** | P14 config, not proven host | Domain/TLS/firewall/secret/monitoring/rollback/egress evidence and owner go/no-go |

**Review rule:** Any other document that labels a planned Pro/Studio/API feature as completed should be corrected from exact code/PR evidence. When this **documentation PR** merges, mark **PORT-00 MERGED** with the real merge SHA in the next authorized status update; do not mistakenly mark F1/Pro/Studio/API as merged or released.
