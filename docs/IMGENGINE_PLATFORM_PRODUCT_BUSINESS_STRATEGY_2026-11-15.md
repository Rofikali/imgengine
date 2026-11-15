# IMGENGINE platform, product and business strategy — decision proposal (2026-11-15)

**Status: PROPOSED; not adopted pricing, not a launch authorization, and not evidence of customer demand.** Repo baseline reviewed: `main` at `acc29bbf002890471ba0a63f01f75a6fbcaa49f3`. This document is a business/product decision aid with a staged execution plan. It changes no code, price, contract, deployment, CI or legacy component. The user has expressed interest in browser-first free tools, a high-performance C17/Rust engine, an API, a desktop product and a sustainable large business; a single initial product/customer wedge must still be validated.

**Authority and navigation:** [root engineering rules](../AGENTS.md) · [Seven-level operating model](SEVEN_LEVEL_ENGINEERING_OPERATING_MODEL.md) · [Astra handoff](ASTRA_ENGINEERING_START_HERE.md) · [technology ADR](TECHNOLOGY_AND_REPOSITORY_POLICY.md) · [living queue](../next.md) · [browser-first roadmap](PRODUCT_ROADMAP_BROWSER_FIRST.md) · [native performance and cross-channel architecture](IMGENGINE_PERFORMANCE_AND_MULTICHANNEL_ARCHITECTURE_2026-11-15.md) · [agent execution brief](IMGENGINE_STRATEGY_AGENT_START_2026-11-15.md). Source and versioned contracts outrank aspirational marketing.

## 1. Executive decision

**Build one platform, not four competing codebases:**
1. **Acquire trust and organic reach:** useful, accessible, privacy-first **free local browser image tools** built on Nuxt/Vue/TS, browser codecs and a small Worker-hosted Rust/Wasm engine where performance and deterministic semantics justify it. Default image bytes remain on-device.
2. **Prove a paying niche:** **Studio/Print Pro** workflow value (repeatable passport/application photo sizes, printable A4/A5/A6 sheets, saved recipes, output-quality validation, bounded batch, naming/export and efficient offline work). Evaluate photo studios/print shops/agents first because native sheet geometry and the founder's practical context are advantages; validate demand rather than assuming.
3. **Sell developer/business automation later:** authenticated metered **Rust/Axum -> safe C ABI -> C17** processing API with deterministic results, usage limits and support. Public SDK / licensed offline engine is a separate, packaging/security/legal milestone.
4. **Desktop only on demand:** a professional offline desktop app (candidate: Tauri 2 + Rust + native C, Nuxt-derived Vue UI) after reproducible offline installer, signed updates, commercial demand and support economics justify an additional delivery channel.

**Distinct job-to-be-done** is more powerful than a generic “billion-dollar image tool” slogan. Initial niche: “A studio or service-center operator converts one customer photo to exact, repeatable application/print assets, quickly, privately and with few mistakes.” This is a **hypothesis** to test with customers; broader reach to API/platform is conditional.

**What not to do now:** no Canva-sized layer/video/audio editor, no full native C decoder port to Wasm, no cross-platform desktop installer, no new database/broker, no new full-time multi-service platform, no mass SEO doorway pages, no unaudited AI image generation, no speculative GPU/assembly rewrite, no grand valuation claim.

## 2. Current source-based starting position

| Slice | Status from current repository/docs | Open decision or proof |
| --- | --- | --- |
| Strict C17 native renderer, SIMD, scheduler, public versioned ABI v1 and 2.0–2.3, shared `.so` | **IMPLEMENTED / recorded tests**, not global security or performance certification | C-A0 queue/metrics concurrency hypothesis, aggregate RSS and source-aware benchmarks |
| Public external installable static `.a` SDK | **NOT ESTABLISHED** (internal static build != exported SDK) | public install/export, independent C11 consumer/link/license/reproducibility |
| Safe Rust wrapper, supervisor/admission, Axum API under `backend/` | **IMPLEMENTED/recorded**; currently server/synchronous work | native request lifecycle, cancellation/resource, public auth/metering/tenancy not established |
| Nuxt/Vue/TS and same-origin proxy under `imgengine-saas/frontend/` | **IMPLEMENTED/recorded**; Tailwind not in inspected package.json | accessible UI, live browser tests, consumer journey, production dependency assessment |
| Local browser image processing via Rust/Wasm | **PLANNED**; M0 ADR deferred implementation | compare browser-native/Wasm/C17-Wasm; prove Workers, parity, privacy, memory |
| P13 selected crop end-to-end | **PARTIAL:** native ABI 2.3 geometry exists; Rust transport/UI incomplete | protect P12 centered fill, freeze contract, test parity |
| Single-host production | **CONFIGURATION/LOCAL EVIDENCE ONLY** | real-host TLS/secrets/abuse/peak RSS/observability/rollback, human go/no-go |
| Paying users, prices, revenue, subscriber churn, retention, billing | **UNKNOWN / NOT VALIDATED** | customer interviews and measured monetization experiment |

Do **not** revise [next.md](../next.md) statuses to DONE from this proposal. A proposed shift toward frontend-first must reconcile the current M1→M2 ordering through a reviewed ADR, and must **not** waive P0 native security work or break existing P12/ABI.

## 3. Product audiences and the reason they might pay

| Customer | Repeated problem | Free acquisition offer | Paid value worth interviewing for | Acquisition channel hypothesis |
| --- | --- | --- | --- | --- |
| Student/application user | Resize signature/photo, crop, reduce size under size rules | Local one-off crop, resize, rotate, export/compress | Usually not the first subscription segment; optional one-off convenience only if independently validated | Useful accurate guides and local-language discoverability |
| Photo studio / print shop | Repeat sizes/copies/sheets, rework, batches, print fidelity | Trial of basic local tool and one basic sheet | Saved shop presets, custom exact sheet recipes, repeat batch, naming/ZIP, offline desktop, training/support | Direct demonstrations and referral from operators |
| Government-form/online-service agent | Many daily passport/signature/ID jobs, time pressure/privacy | Free manual single-job tools | Reusable validated form presets, guided workflow, bounded batch and operator productivity | Interviews, demonstrations, local agent networks |
| Developer building an app | Deterministic image transforms without operating codecs | Documentation, open examples, tiny sandbox quota after rate control | Per-successful-transform API, quotas, webhooks only if needed, stability/versioning/support | Developer docs, SDK and integration examples |
| Enterprise publisher/printer | Repeatability, high-volume reliability, private data | Documentation and qualified demo | Contractual support, private/offline distribution, support/SLA, audit and bespoke integration | Direct sales after reliability evidence |

**Start narrow, validate deeply, then expand:** a useful free browser utility reaches individuals; a studio workflow is an initial possible payer; a secure developer API creates a second long-term market. Do not assume studio revenue alone supports venture-scale valuation.

## 4. Free vs paid — proposed entitlement boundary

**Principle:** free local compute is not artificially crippled to force payment. Charge for **saved workflow, repeatable professional productivity, integrations, managed server compute, support and dependable business outcomes**, not for the mathematical operation “crop one photo” or a fabricated performance advantage.

| Capability | Proposed free browser tier | Potential Studio/Print Pro tier | API/enterprise tier | Status today |
| --- | --- | --- | --- | --- |
| One-image crop/resize/rotate/flip | **Free**, local-only | Included, ergonomic precision controls | Metered when executed remotely | Browser-local planned |
| JPEG/PNG/WebP conversion and ordinary compression | **Free** if browser supports, no upload; quality/size warnings | Preset-based export combinations | Server format/quality contract when supported | Local planned |
| Custom target KB | **Free bounded local attempt**, show unattainable result honestly | Reusable target profiles and batch queue | Metered remote transforms by validated workload | Planned |
| Basic single-user printable sheet / passport preview | **Free** limited interaction complexity, correct print geometry | Complex/custom layouts, saved shop templates, repeated workflow | Server/prepress after native support proven | P12 server path exists, local planned |
| Batch operations | Small, device-safe batch may remain free | Repeated production batch, saved recipes, naming/ZIP and recovery | Usage-metered server/bulk API | Planned |
| User accounts and synced presets | Not needed for local free features | Optional paid sync **only after approved durable metadata/privacy ADR** | Organization entitlements after approved billing/storage ADR | Undecided |
| Desktop offline app | No desktop product yet; evaluate limited free trial vs paid license | Paid value: audited workflow automation/offline convenience, not access to basic pixels | Managed/private fleet licensing after validation | Not implemented |
| Public API calls | No unbounded anonymous free server access | May be add-on, not included implicitly | Trial quota with abuse controls, then usage-based tiers/contracts | Current API is not a public metered SaaS |
| Public `.so`/`.a` SDK | Docs/sample client may be free | Studio not relevant | Commercial integration/support, private distribution, license compliance after packaging proof | Shared `.so` exists, public `.a` open |
| Priority support, team roles, SLOs | Community/FAQ | Paid assistance if staffed | Contractual support/SLAs only after proven operations | Not established |

**Avoid untrustworthy paywalls:** do not degrade output quality, put watermarks on private personal documents, deliberately slow local users, hide essential export, or imply government-issued compliance without verified specifications. Premium differentiation must save labor, reduce mistakes, enable trusted integrations or include actual service costs.

## 5. Pricing, customer discovery and evidence gates

**No prices are adopted.** Price discovery is a test, not a coding instruction.

Suggested willingness-to-pay interview **stimuli, not commitments**:
- Studio productivity plans: test several locally relevant price points, e.g. **₹199, ₹499, ₹999 per month** *as hypothetical survey or pilot offers*, and consider a perpetual license with paid upgrades/support only if installers and support make sense.
- API: compare **metered charge per successful validated transform**, minimum monthly commitment for reserved capacity, or prepaid credits **after** a resource/abuse cost model. Do not copy competitors' prices without current verification.
- High-volume customer: negotiated minimums, rate/concurrency limits, SLA exclusions, overage and support terms; don't promise unlimited.

**Interview experiment (before implementing paid auth/billing):**
1. Interview at least **10 real prospective operators** in one niche; observe current workflow, tools, volumes, mistakes, time lost, customer privacy needs, equipment/mobile constraints, willingness to switch and payment method.
2. Prototype a *single* end-to-end job (photo -> valid crop -> physical sheet or signature/file-size rule -> downloadable artifact) and measure completion, manual steps and errors in a supervised trial.
3. Run a small pricing test with **actual commitments** (written pilot intent, deposits only through properly configured/approved accounting and payment process), not yes/no compliments.
4. Record acquisition source, repeated weekly use, task success, support burden, willingness-to-pay and retained customers. Distinguish free users, prospects, activated users and paid accounts.
5. Set explicit human-selected thresholds for continue/pivot/stop after collecting baseline data. Do not invent conversion or retention targets as if market facts.

### Finance/CA/MBA framework

For a measured calendar period, distinguish **revenue** (earned), **cash received**, **direct cost of service**, **operating expenses**, **taxes**, **capital investment** and **liabilities**.

- **Variable cost per successful billable server operation** = all variable compute, bandwidth, transaction fees and failed/abusive attempts attributable to the period / successful billable operations in that period.
- **Contribution per paid operation** = realized net revenue per paid operation minus corresponding variable cost (including costs of failures, retries, refunds and payment fees).
- **Gross margin** = (recognized net revenue - service cost) / recognized net revenue when nonzero, using a consistently defined accounting basis.
- **Simple break-even transaction count** = fixed period expenses / positive contribution per transaction; if contribution <= 0, additional sales do not reach break-even under that model.
- **Cash runway** = unrestricted cash / net cash burn per period when burn > 0. Do not equate profit with bank balance.
- **Price ceiling** is driven by credible customer value and competition; **price floor** depends on variable cost, acquisition/support and required margin. Free users still incur website asset traffic/support costs.
- Include **GST/invoicing/local compliance, payment fees, refunds and privacy obligations** in an accountant/lawyer-reviewed implementation decision; this document does not supply jurisdiction-specific tax advice.

**Hypothetical calculation example (not forecast):** A studio paying ₹499/month that costs ₹80/month in allocated variable support/hosting would contribute ₹419 before allocated fixed costs, taxes and acquisition. That one account proves nothing about market size; validate actual collection, churn and support burden. Do not count free tool requests as paid conversions.

### Business metrics

Instrument aggregated, privacy-preserving metrics **without image bytes, local filenames or precise personal document data**. With suitable consent and a lawful design, monitor:
- Local task completion/error by tool, client class and browser capability (coarse, non-identifying);
- Estimated bytes **not uploaded** (derived locally and optionally aggregate), site/Wasm asset transfer and CDN costs;
- First successful workflow, repeat usage and user-reported minutes saved (avoid tracking identifiable image content);
- Studio trials, activated businesses, repeat paid use, cancellations, net revenue, gross margin, support minutes;
- API valid/failed/abused requests, p50/p95/p99, throughput, memory, bandwidth, COGS per success, on-call load and operating cash burn.

## 6. Why competitors matter — not a copying exercise

Public benchmarks/examples consulted as of 2026-11-15:
- [Squoosh](https://github.com/GoogleChromeLabs/squoosh) demonstrates local-only browser compression and explicitly describes its privacy behavior. That is a **viability reference**, not proof a generic compressor is commercially differentiated.
- [TinyPNG API](https://tinypng.com/pricing/api) advertises 500 free compressions per month and metered charges above; check at pricing launch, because competitor rates change.
- [Cloudinary pricing](https://cloudinary.com/pricing) uses a credit system for transformations, storage and bandwidth. Its wider workflow/platform functions illustrate why B2B buyers may pay for integrated service rather than pixel speed alone.
- [libvips](https://www.libvips.org/) is an established low-memory, high-performance competitor/technical reference; compare representative performance honestly on identical operations/hardware, do not advertise a speed win without controlled measurements.

The defensible advantage to **test** is deterministic physical print layouts, superior local privacy, studio workflows, an approachable integration API, resource efficiency and audited native code—not a claim of universal maximum speed.

## 7. Staged business and engineering roadmap

These IDs are **strategy proposals**; they are NOT changes to the binding [next.md](../next.md) queue until a priority ADR is reviewed. Security and release blockers remain in parallel.

| Sequence | Stage / owner lens | Customer outcome and milestone | Gate to continue |
| --- | --- | --- | --- |
| **S0 — FIRST** | Principal + product + security + finance | Review F0 browser-first priority ADR; native C-A0 concurrency/resource audit; define one studio job and 10 customer interviews; catalog performance baselines | Short source-backed ADR, exact risk/ownership, test plan and customer evidence plan; no product rewrite |
| S1 | Senior/Staff frontend + systems | Accessible Nuxt navigation and single browser-local **resize** vertical slice, using Worker + actual Rust/Wasm only when justified; source image stays local | Mobile/browser compatibility, no-upload network assertion, memory and visual fidelity tests |
| S2 | C/FFI senior + principal | Investigate native C queue/metrics reachability, aggregate RSS, C11/C17 ABI/ASan/UBSan/fuzz evidence; produce safe bounded fixes only after reproducer | No unverified race/fix; exact candidate tests and benchmark evidence |
| S3 | Product + frontend | Free crop, rotate, flip, compression, conversion, basic sheets in narrow separate PRs; preserve P12/P13 contracts | Task success, accessibility, input stress and local-only privacy |
| S4 | PM + studio design + CA/MBA | Studio repeat workflow: saved local presets, physical sheet precision, **bounded** batch/ZIP naming; pilot with willing shops | Measured repeat business use and credible willingness to pay |
| S5 | Rust/SRE/security | Controlled API beta: stable ABI and error contracts, sandbox/resource limits, per-client auth/quotas/metering, pricing and invoicing ADR | Real-host P14, abuse tests, cost-per-success, legal/privacy approval and recovery |
| S6 | Senior Staff + product | Desktop feasibility only after paid workflow demand: Tauri/Rust/C integration, offline packaging, signed update and local filesystem authorization | Installer/security/support costs vs actual studio benefit |
| S7 | Principal/Distinguished | Public C/.so/.a SDK, enterprise licensing, native portability, advanced pipeline/streaming and partnerships | External consumer tests, license compliance, channel-specific return on investment |

**Quality gate:** DO NOT ship S5 API or public SDK merely because S1 browser UI passes. Core native safety and live release gates are independent.

## 8. Desktop roadmap and monetization choices

**Candidate**: [Tauri v2 architecture](https://v2.tauri.app/concept/architecture/) supports Rust application logic and OS webviews; separately review how to bundle the native C shared library, supported Linux/Windows/macOS toolchains, code signing, updates, file permissions and licensing. This is a design candidate **not an approved dependency or an implemented desktop app**.

Desktop-specific features that may justify paying: offline batch import/export, validated print templates, folder workflows with explicit permissions, precise paper/print settings, local saved recipes and small-shop support. Browser free remains strong. Avoid adding persistent cloud accounts as a condition of offline use.

## 9. Financial discipline for a potentially large company

**Founder priorities in order:** reliable working product -> meaningful users -> repeated paid problem -> healthy unit economics -> repeatable distribution -> scalable engineering. A “billion-dollar business” is a possible distant business ambition, not a technical acceptance criterion.

- Keep fixed costs and infrastructure simple while discovering a buyer; never claim free-tier capacity is guaranteed or cost-free forever.
- Do not hire, build enterprise compliance, buy GPU/cloud services, adopt a database, or deploy multiple regions before a measured need.
- Maintain a monthly ledger of cloud/hosting, domain, tooling, subscriptions, refunds, payment fees, support and tax exposure; record cash and profit separately.
- Review build-vs-buy using customer value, time-to-market, security, license and total cost—not language prestige or a claimed nanosecond improvement.
- Treat business model changes, price adoption, legal commitments, payment/identity storage and spending as **human-approved decisions**.

## 10. Immediate handoff to coding agents and staff

**First next work item = STRAT-00, read-only architecture/product/performance discovery and proposed priority ADR** as defined in [Agent Start Brief](IMGENGINE_STRATEGY_AGENT_START_2026-11-15.md). The same intake template should guide Astra/Codex and human engineers; role labels are proportional review lenses, not seven separate privileged agents.

**STRAT-00 outcomes:** verify actual HEAD and open P0 issues; inspect current C/Rust/frontend/bench source; identify one customer journey and actual constraints; propose browser-first sequencing ADR without silently changing accepted M1→M2 dependencies; produce performance workload spec (tiny geometry vs render vs API), staged free/paid entitlements with hypotheses, risk register and a small implementation PR proposal. Stop before implementation, merge, public deployment, accounts/billing and spend.

**External references are research context, not binding contracts** and require fresh verification on commercial decisions. Current source, scoped AGENTS and versioned ABI/API contracts always take precedence.
