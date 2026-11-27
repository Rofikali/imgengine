# IMGENGINE — Commercial readiness, finance, pricing hypotheses and release gates

**Owner priority override — 27 November:** Execute original phases 1 → 2 → 3 → 5 → 8 first (live safety reconciliation, shared frontend, website/editor UX, hostname-independent performance/SEO/accessibility, and reusable Free/Pro/Studio UI foundation). Move original phases 4 → 6 → 7 to the bottom (custom domain, finance/marketing, monetization). Domain ownership is not confirmed; use the existing hostname for approved development/verification. Do not select DOM, commercial FIN/MKT or MON work on ordinary `go ahead` / `next` until the owner reopens it. Hostname-dependent canonical/sitemap/redirect/Search Console work waits for the permanent hostname. Existing F5 security, billing safety, approvals and cost controls still apply; no paid product implementation or speculative package extraction is implied by shared UI work.

**27 November extension:** [Shared UI/domain/monetization roadmap](IMGENGINE_F6_SHARED_UI_DOMAIN_MONETIZATION_ROADMAP.md) adds domain renewal and trademark checks, a provisional INR 1,000–2,000 annual domain budget hypothesis, Free acquisition and isolated monetization options, hypothetical Pro/Studio prices and RPM scenarios, cost accounting and owner approval gates. These are not actual quotes, earnings, approved prices, or authorization to spend.

**SDK/Desktop fifth-family commercial extension (proposed only):** See [ADR-PORTFOLIO-005](ADR_PORTFOLIO_FIVE_PRODUCT_RELEASE_STRATEGY_2026-11-19.md) and the [compatibility/release matrix](IMGENGINE_FIVE_PRODUCT_RELEASE_COMPATIBILITY_MATRIX.md). Keep the four-product financial model below unchanged until separately approved customer discovery. Native SDK redistribution/IP/codec licenses, desktop distribution signing/updates, offline licensing, customer support and possible shop SKU overlap need independent cost and legal analysis. No SDK/Desktop price, customer revenue, entitlement provider or release is approved by this add-on.

**Status:** FUTURE DECISION CHECKLIST, not launched software, an accepted price list, tax/legal advice or a budget authorization. Companion to the [four-product portfolio](IMGENGINE_FOUR_PRODUCT_PORTFOLIO.md) and [Developer API capacity/metering blueprint](IMGENGINE_DEVELOPER_API_USAGE_BILLING_CAPACITY_BLUEPRINT.md). [Live source of truth](../next.md) holds current milestone states. No account, ad account, payment provider, tax registration, domain or cloud service is created by this document.

## 1. One founder dashboard, four different economics

| Product | Customer | Activation metric | Payer's reason | Largest variable cost exposure |
| --- | --- | --- | --- | --- |
| IMGENGINE Free | Occasional visitors | Successful local output/download and repeat visit | None: tool is free | Static asset/Wasm CDN egress, ad/policy work, support, engineering |
| IMGENGINE Pro | Recurring individuals/creators | Repeated recipe/batch completion | Time saved and repeatable output | License/support, payment costs; device-local CPU usually belongs to customer |
| IMGENGINE Studio | Print shops/operators | Successful print sheet/order; minutes and reprints saved | Fewer mistakes, predictable physical prints, daily throughput | Server print compute where consented, support, print calibration, payment |
| IMGENGINE Developer API | Software companies | First successful authenticated API job, then retained calls | Outsourced reliable image processing | CPU/RAM/concurrency, egress, attacks/failures, metering, payment/support |

Ads/subscriptions/usage billing are **not interchangeable revenue metrics**. Free traffic != ad-eligible views; registered users != paying users; paid plan signups != successfully collected/recognized revenue; requests != billable successful work; gross receipts != profit. Report them separately.

## 2. Customer research and validation before price commitments

**Free:** Interview/observe at least 10 ordinary users on device, broken down by job (signature/photo/product), browser/phone and success/retry. Are five features sufficient? Does a no-upload privacy message improve trust? Measure local time to useful JPEG/PNG/WebP download, failed decoding, repeat use and support burden. Do not add 20 tools before measuring these.

**Pro:** Interview 10+ repeat creators/sellers about monthly jobs, process time, file naming, recipe import/export, batch limits and whether local execution solves their work. For each person, test a real recipe with stopwatch; ask willingness to pay for verified saved minutes, not only “Would you buy an app?” Pilot a limited nonpaid demonstration before enabling checkout.

**Studio:** Observe real service-center/photo-shop orders end-to-end. Record paper sizes, proof/print mistakes, reprints, staffing, printer calibration, existing costs and actual customer acceptance requirements from authoritative sources. Test with selected trusted operators and synthetic/consented photos. Count saved minutes per job and actual reprint/material savings; don't guarantee an authority will accept a photo.

**API:** Interview 5–10 developers with actual monthly operation counts, codec mix, file sizes, endpoint compatibility, expected error contracts and budget. A “developer interested” is not an integration; measure a successful sandbox integration and 30-day repeat usage before designing a large SDK/async platform.

**Decision gates:** each paid SKU needs a named customer segment, measured customer job, tested value improvement, acceptable willingness-to-pay range, CAC channel hypothesis, support/refund estimate and a reversible pricing experiment. Keep reasons and negative findings; do not promise demand.

## 3. Price/entitlement experiments — examples, not adopted prices

| Offer | Example *hypothesis* | Included value hypothesis | Exclusions to state |
| --- | --- | --- | --- |
| Free | ₹0; eligible ads only after publisher approval | Five local image tools with usable output | No account/remote storage; extras not silently required |
| Pro | ₹199/month **or** prepaid workflow pack | Named recipes, bounded repeat/batch and ad-free view | No unlimited jobs, cloud sync, API credits or guaranteed offline DRM |
| Studio | ₹399/month/shop **or** print-job pack | Validated print presets, efficient operator flow, multi-photo print sheets | No guarantee of visa compliance, unlimited server rendering or 24/7 SLA |
| Developer API | Finite sandbox then prepaid credits **or** included units + capped overage | Versioned B2B integration, safe compute, usage record | No unlimited public API, no automatic overage, no ambiguous free retries |

These are **illustrative** INR figures from early discussion, not prices to implement, advertise or invoice. API pricing has deliberately **no amount** until real cost/benchmark evidence. Prices by geography, GST/tax applicability, transaction fees, FX, subscription renewals, consumer refund rules and invoicing must be independently reviewed with an appropriate qualified Indian accountant/adviser and jurisdictional counsel before charges are activated. **Never assert a specific law or tax exemption without dated official verification.**

**Payments/entitlements architecture decision still needed:** subscription lifecycle (trial, active, unpaid, cancellation, grace, renewal), prepaid packs, seat vs account scope, invoice, proration/refund, offline entitlement and misuse, payment failure, recoverability, privacy, cryptographic key handling and durable records. Use independently reviewed ADR and approval, **not** hidden additions to the native engine or a Redis/PostgreSQL assumption. Honest client-local offline features may be copied by a motivated user; design commercial value around continued convenience/support/service, not a fictional unbreakable browser paywall.

## 4. Financial model: income statement, cash flow and economics

Use one month for each metric and track units/currency; never mix paid clients with operations or dollars with rupees without dated FX.

~~~text
Free advertising revenue (illustration) =
    (monetized pageviews / 1,000) × observed page RPM

Paid recognized revenue =
    accepted billed units or subscription service value in period
    minus recognized discounts/refunds (consistent accounting method)

Monthly variable costs =
    host/container CPU and memory attributable to jobs
    + CDN/Wasm/static and output bandwidth
    + payment/provider fees, metering, fraud/refund exposure
    + directly attributable support and support infrastructure

Contribution = paid/advertising net revenue - variable costs

Net operating result =
    contribution - fixed host/domain/tools/support/marketing/engineering costs
    (plus applicable accounting adjustments and taxes as advised)

Break-even active Pro/Studio accounts =
    fixed attributable monthly costs / average positive
    per-customer monthly contribution
~~~

**Example only:** 20 paying Pro users at **hypothetical ₹199** gives gross **₹3,980** before any taxes, payment fees, hosting, refunds, salaries or support. It is **not ₹3,980 profit**. If a business spends $10 monthly on a VPS, a ₹399 subscription cannot be compared against that expense without FX, costs, payment collection and shared service allocation. No guaranteed RPM, margin, conversion, free-tier permanence or customer count is assumed.

**Accounting ledger categories:** gross processed payments; taxes collected; recognized revenue; deferred prepaid credit balances/liability until consumed under adopted policy; payment fees; hosting/infrastructure; bandwidth/egress; refunds/chargebacks; marketing; support; depreciation/engineering labor if included; receivables/payables; exchange gains/losses; reconciliation discrepancies. The API credit ledger's technical “credits” must reconcile to financial obligations and invoice/refund policy. Cash received is **not automatically revenue**.

**Runway:** budget at least fixed recurring costs + unexpected egress + support/refund contingencies + domain/renewal charges. Require explicit max spend/alerts. A “free tier” may have account eligibility, time, egress, idle reclamation or card authorization risks; verify official prices and quota before deployment. Keep current launch at $0/few-dollars where technically feasible and avoid promising profitability.

## 5. Service operations, privacy and risk review by tier

**All products:** document the owner/legal entity, support contact, accessible UI, privacy policy, acceptable use, content/IP rights, third-party libraries/codecs and licensing, data processing/deletion, material change notices, abuse/security reporting, error handling, release rollback, recovery ownership and documented claim limits. Do not publish a generic “government accepted” guarantee or unmeasured “nano-millisecond” advertisement.

**Free with optional ads:** publisher approval isn't automatic. Review current publisher content and invalid-click policies, ad placement separate from Drop/Convert/Download actions, cookies/consent rules, no sensitive-photo context disclosure to advertiser scripts, ads not injected into the editor or binary file, performance and accessibility. Separate analytics events from file content. Ad income is optional; Free remains useful without it.

**Pro:** prove license/account compatibility with browser-local operation, imported recipe safety, ZIP resource limits, cancellation, restore/support, refund expectations, high-contrast/mobile usability and graceful plan downgrade that doesn't destroy users' local files.

**Studio:** consent for any upload, default ephemeral deletion, operator/customer isolation, physical dimensions/print proof, dated sourced presets, no storage of customer ID images by default, correct paper/print configuration and tested reprint costs. Offline print workflows deserve explicit local-device and printer testing.

**Developer API:** no unbounded anonymous production render; per-tenant key, auth/scope, revocation, dynamic quotas/concurrency, rate limits, signed plan terms, idempotent charge effects, immutable usage ledger, monthly reconciliation, usage and spend caps, billing-provider fail closed, fraud/abuse, incident/support and rollback. Stronger P14 native/host requirements than the five browser-local tools.

**Data retention and backup decision:** don't assume all products need a DB. Image bytes ephemeral by default; durable *commercial metadata* may be essential for paid invoices, licenses, credits, disputes and security audit. Approve a storage ADR separately (and its cost, migrations, backup/restore, retention and deletion). C17 native must not store or manipulate customer identities or payments.

## 6. Checklists and explicit release owners

### Free launch — F5 (five-tool site)
- [ ] Resize, Crop, Compress, Rotate, Convert all independently implemented; successful export and negative tests on supported browsers/devices.
- [ ] Each tool has documented type/format limits, memory bounds, orientation/metadata/alpha policy, clear errors and no-upload network evidence including failure paths.
- [ ] Accessibility review for keyboard/focus/screen readers and real-device mobile smoke.
- [ ] Privacy notice, third-party dependency provenance, minimal analytics (no image/filename) and ad policy/consent reviewed if ads enabled.
- [ ] Real chosen hosting build/artifact, CDN/egress cost estimate, operational rollback/help contact and explicit owner deployment approval.
- [ ] Required exact-revision CI and release-candidate checks complete. No server-backed paid feature required for Free launch.

### Pro launch
- [ ] At least one validated repeated-work feature (recipes/batch), privacy, cancellation and price-value research.
- [ ] License/account model, durable metadata need, safe recipe/ZIP formats and entitlement bypass risk explicitly decided.
- [ ] Payment provider, terms, renewal/cancellation/refund and tax/invoicing review approved; live provider test and support process.
- [ ] Pricing/plan limits and entitlement server enforcement (where applicable) verified; test free/pro downgrade and offline failure.
- [ ] Owner-approved go/no-go and actual margin from early real customers before scaling.

### Studio launch
- [ ] Real operator pilot, named accurate print preset source/review/expiry and physical print validation.
- [ ] Customer-image consent/privacy/cleanup, printable output, operator isolation, device/printer and error recovery proven.
- [ ] Paid plan/seat/job economics, invoices/refunds/support and entitlement decision accepted.
- [ ] Server-side work, if any, passes native C-A0 safety disposition, exact host P14 resources/security/recovery tests and owner authorization.
- [ ] No unproven official-photo acceptance claims.

### Developer API launch
- [ ] Signed versioned API contract, customer-scope keys, auth/quotas/idempotency, errors, examples and bounded sandbox.
- [ ] Atomic durable usage/credits and duplicate/crash recovery, provider reconciliation, monthly records, overage cap/refunds and finance audit.
- [ ] Security review for abuse, cross-tenant leakage, uploads and native codec safety; benchmark workload p50/p95/p99, peak RSS, actual cost per billable success.
- [ ] Capacity tested on actual host with TLS/DNS/firewall/secrets/monitoring/rollbacks; proven resource reserve; rate-limit and fairness tests.
- [ ] Pricing, tax/legal/privacy and support terms approved; private beta quality and operational evidence accepted.
- [ ] Separate explicit owner paid-public-API launch and spending approval. No SLA claims without real operational proof.

## 7. Prioritized owner decisions remaining open

| Decision ID | Scope | Must decide before |
| --- | --- | --- |
| COMM-01 | Free hosting, domains and ads/ad privacy policy | Public Free F5 and monetization |
| COMM-02 | Pro/Studio feature entitlement, price/plan, refunds and license | Any live Pro/Studio checkout or enforcement |
| COMM-03 | Identity, durable billing-metadata storage/provider ADR, encryption/backup | Production accounts, recurring billing or API ledger |
| COMM-04 | API credit/usage weights, idempotency and billing settlement semantics | Paid API beta |
| COMM-05 | Tax, invoices, GST/foreign-currency and applicable laws reviewed by qualified adviser | Collection of money |
| COMM-06 | Native C-A0 + P14 host capacity/abuse/rollback and security go/no-go | Public Rust/C17 image processing |
| COMM-07 | Third-party license/product ownership, rights/privacy policies | Public commercial distribution |
| COMM-08 | Customer interviews, support commitments, real variable-cost evidence | Final rate card/paid scaling |

**Guardrails:** For every milestone write current state in [`next.md`](../next.md): planned, approved, in progress, PR open, CI pending, ready for review, merged or released. Do not count a merged *documentation PR* as an implemented or deployed product. Preserve history and exact-SHA CI, and keep the seven required jobs unchanged unless a separately reviewed policy is approved. No automatic merge, payment setup, deployment or spending.
