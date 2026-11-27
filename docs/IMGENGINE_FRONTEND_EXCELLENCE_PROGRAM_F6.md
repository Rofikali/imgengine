# IMGENGINE Frontend Excellence Program (F6) — Post-Free v1 strategy

**CI-EFF-01 — development batching and trusted scope refinement (27 November, IMPLEMENTED on development branch; NOT VERIFIED):** Owner requested faster iterations. Batch bounded commits on a non-main branch with no open PR, then open/update one integration PR at a meaningful checkpoint; do not use `[skip ci]`, force-push main, disable required checks, or treat untested work as complete. Proposed exact-path selector classifies ordinary agent/status and named planning Markdown as docs-only, and four known frontend shell/tool-registry paths as frontend-only. Always-on policy/doc-sync and aggregate checks remain; native, unknown, CI, release/security, dependency, infrastructure, deletions/renames and malformed diffs retain fail-closed full scope. Main push and merge queue remain full; exact release candidate full verification and owner approvals remain mandatory. Trusted-base selector means this policy change itself runs full CI and becomes effective only after reviewed merge. Measure real job elapsed time and runner cost, not assumed speedup. See [CI efficiency work package](engineering/CI_EFFICIENCY_DEVELOPMENT_BATCHING.md).

**Owner priority override — 27 November:** Execute original phases 1 → 2 → 3 → 5 → 8 first (live safety reconciliation, shared frontend, website/editor UX, hostname-independent performance/SEO/accessibility, and reusable Free/Pro/Studio UI foundation). Move original phases 4 → 6 → 7 to the bottom (custom domain, finance/marketing, monetization). Domain ownership is not confirmed; use the existing hostname for approved development/verification. Do not select DOM, commercial FIN/MKT or MON work on ordinary `go ahead` / `next` until the owner reopens it. Hostname-dependent canonical/sitemap/redirect/Search Console work waits for the permanent hostname. Existing F5 security, billing safety, approvals and cost controls still apply; no paid product implementation or speculative package extraction is implied by shared UI work.

**27 November extension:** [Shared UI, domain and monetization roadmap](IMGENGINE_F6_SHARED_UI_DOMAIN_MONETIZATION_ROADMAP.md) refines existing F6 IDs with shell/header/footer/home/About/Contact/editor work, domain, SEO, customer discovery and commercial gates. Reconcile owner-reported live Workers URL with F5 first; no second deployment or release claim. Use `go ahead` / `next` and the live board, not historical task selection.

Free navigation and homepage implemented for review. All nine Free routes share a labelled header with Home, desktop tool links and a native mobile Tools disclosure. Keyboard checks cover Enter, Tab, Escape and focus restoration. The homepage presents five descriptive cards. Canonical URLs and the sitemap await the permanent public hostname decision.

**Status (2026-11-19): OWNER-REQUESTED DOCUMENTATION PROGRAM / PROPOSED EXECUTION, NOT IMPLEMENTED OR RELEASED.** This records direction and reviewable work packages, not approval for new services, dependencies, analytics, ads, cloud spend, public launch, merges or a Wasm rewrite. The actual next READY task must always come from [live next.md](../next.md) and current GitHub evidence; a dated status assertion is not fresh CI proof. [F6 acceptance & verification matrix](IMGENGINE_FRONTEND_EXCELLENCE_F6_ACCEPTANCE_MATRIX.md) provides testable exit conditions.

## 1. Mission, scope and source of truth

Deliver a genuinely useful, attractive, fast, accessible, privacy-first **browser-local** IMGENGINE frontend that can be evolved coherently across Free, Pro, Studio and eventually Desktop — **without prematurely rebuilding the existing C17 engine in WebAssembly or deploying speculative infrastructure**.

**Authority and constraints, in order:** [root AGENTS](../AGENTS.md) and [frontend AGENTS](../imgengine-saas/frontend/AGENTS.md) > [live next.md](../next.md) > [approved browser-first F0 ADR](F0_BROWSER_FIRST_SEQUENCING_ADR.md), [technology policy](TECHNOLOGY_AND_REPOSITORY_POLICY.md), [five-product portfolio ADR](ADR_PORTFOLIO_FIVE_PRODUCT_RELEASE_STRATEGY_2026-11-19.md), [four-product customer promise](IMGENGINE_FOUR_PRODUCT_PORTFOLIO.md), [performance/multichannel plan](IMGENGINE_PERFORMANCE_AND_MULTICHANNEL_ARCHITECTURE_2026-11-15.md), [F2c 12-category gates](FRONTEND_F2C_CI_QUALITY_GATES.md), [commercial readiness](IMGENGINE_COMMERCIAL_READINESS_AND_UNIT_ECONOMICS.md), [seven-level operating model](SEVEN_LEVEL_ENGINEERING_OPERATING_MODEL.md), [MVP release checklist](MVP_RELEASE_CHECKLIST.md). Historical status paragraphs in older documents are not current implementation facts.

**Platform target remains:** Nuxt 4/Vue/TypeScript frontend; strictly C17 native core; Rust/Axum only for approved server-backed workflows; **no new Python/FastAPI, Redis, PostgreSQL or Python tooling**. Existing legacy services/tools are retained until separately and safely retired. No account is required for Free; no Free image bytes, filenames or EXIF are uploaded, persisted, included in telemetry, or automatically sent to Rust. Serving ordinary site assets is not an image upload.

## 2. Sequencing decision: ship a credible Free, then deepen it

**Prelaunch F5 has non-deferrable minimums.** Free v1 must ship the existing **five and only five** tools: Crop, Resize, Compress, Rotate, Convert. Each requires accurate output, error and resource bounds, usable mobile/desktop layout, keyboard/touch accessibility, no-upload/privacy verification, dependency security, trustworthy basic SEO and support/disclosure text, and owner-approved hosting/legal/release readiness. F5 is **not** complete merely because five feature PRs merge. The canonical [F2c matrix](FRONTEND_F2C_CI_QUALITY_GATES.md) must be met for Compress; do not postpone critical tests to F6.

**Postlaunch F6 adds systematic excellence**, deep UI/design improvements, broader SEO content, enhanced visual regression and performance gates, and experiments that must earn their complexity. Optional F6 investigation/documentation can begin earlier if it does not displace the current READY Free milestone, but implementation priority remains **F2c Compress → F4 Convert → F5 release review**, subject to actual latest GitHub state. F6 is not a prerequisite for F5 unless a **real, evidenced launch-blocking defect** is found.

**Status vocabulary:** PROPOSED, READY, IN_PROGRESS, PR_OPEN, REVIEWED, MERGED, VERIFIED, READY_FOR_RELEASE_REVIEW, RELEASED, BLOCKED/DEFERRED. An F6 documentation PR can only establish *planning*. Green feature CI is not actual public product release. Do not mark F6 as READY merely because this document merges.

## 3. HLD — UX and execution boundaries

~~~text
Public content & SEO shell
  ├─ Nuxt page routes, semantic HTML, metadata, meaningful help content
  ├─ Shared accessible design tokens, navigation & tool UI components
  └─ Client file interaction (only after user intentionally selects file)
       ├─ Input limits, MIME/decode validation & decoded pixel/memory budgets
       ├─ Browser-native decode/Canvas/encode (default)
       ├─ Optional Worker (evidence-led; cancellation + cleanup)
       ├─ Optional targeted Rust/Wasm or portable C17→Wasm (separate ADR)
       └─ Local preview -> verified export -> revoke/release all resources

Separate opt-in server-supported workflows, if/when approved:
Nuxt same-origin proxy -> Rust/Axum policy/admission -> safe FFI -> C17
Never a silent fallback from Free browser-local tools.
~~~

Prefer **one design system and per-tool workflow modules**, with typed input/output contracts, composables for file lifecycle, shared validation and status UX, and deterministic fixtures for geometry/metadata behavior. Keep domain processing out of decorative components. No premature global store, heavy animation framework, second backend, microservice or forced UI-library switch. Prefer performance-aware progressive enhancement and capability detection.

## 4. F6 roadmap — independent bounded PRs

| ID / order | Deliverable | Acceptance anchor | Default priority |
| --- | --- | --- | --- |
| **F6-00** | Baseline inventory of existing routes/components/CSS/dependencies, accessibility, mobile, SEO, bundle metrics, field/synthetic latency, tests and security gaps | Exact SHA, reproducible evidence, risks and ranked low-cost fixes; no invented numbers | FIRST after F5 |
| **F6-01** | Design-system foundation: typography, color/tokens, spacing, light/dark treatment, button/form/file-dropzone, dialogs, status, focus and editor components | Accessible documented tokens/components, snapshot/interaction tests, no user-flow regression | HIGH |
| **F6-02** | Semantic responsive navbar, tool discovery, footer, breadcrumbs and navigation for touch/keyboard | Correct route active state, mobile menu escape/focus, no layout shifts, meaningful non-deceptive product labels | HIGH |
| **F6-03** | Five-tool editor UX consistency: choose/drag-drop, preview, settings, progress, cancel/reset, export, retry/errors | No-upload network tests; mobile/desktop/keyboard success, cleanup and output correctness | HIGH |
| **F6-04** | Technical SEO and content quality: route metadata, canonical, sitemap/robots, useful help, internal links, crawlable pages, appropriate prerender/SSR decisions | Audit rendered output and indexability, not just client DOM; avoid thin keyword pages | HIGH |
| **F6-05** | Performance/latency/memory and delivery budgets: bundle split, image UI, main thread, processing cold/warm paths | Repeated controlled benchmark with p50/p95/p99 where practical, peak memory and quality/behavior parity; actual improvements | HIGH |
| **F6-06** | F3-aligned browser Worker/Rust-Wasm/C17-Wasm feasibility ADR and bounded prototype | Browser-native baseline wins by default unless Wasm improves agreed measure without privacy, quality, memory or bundle regression | CONDITIONAL |
| **F6-07** | Expanded quality automation: visual regression, E2E matrix, a11y, SEO checks, profiling and dependency hygiene | New checks additive to seven required CI jobs; flake/review ownership; exact-commit evidence | HIGH, can run alongside |
| **F6-08** | Consent-appropriate privacy-safe event definitions and product funnel dashboard | Only aggregate tool activation/success/failure/duration where genuinely necessary, no file data/IDs, consent/privacy ADR and opt-out | LATER |
| **F6-09** | Internationalization and content-growth experiments | Demand proof and accurate translations, language/canonical/hreflang correctness where applicable, no bulk doorway pages | LATER |
| **F6-10** | Free v1.x optimization release review after selected child work | Independent owner release decision, tested tagged artifact, security/compatibility/rollback and confirmed live outcome | RELEASE GATE |

The numbers are **program IDs, not promises about Git history or chronological PR sequence**. Do not automatically upgrade a minor/major product version because a style refactor merges. Later Pro/Studio/Desktop adaptations require separate entitlement/privacy/release reviews. SDK/Desktop and public API implementation are not created by this plan.

## 5. UX and design quality contract

**Information architecture:** home + five working tool pages (\`/crop-image\`, \`/resize-image\`, \`/compress-image\`, \`/rotate-image\`, \`/convert-image\`). A route shown in this plan is a proposed/verify-first path, not proof it exists. Each page must deliver a useful tool before asking for account registration or pricing. Unreleased Pro/Studio/API/Desktop must not be represented as available.

**Design tokens:** spacing, typography scale, color/contrast, radii, elevation, motion, density and semantic status — no dependency introduction without ADR and cost/benefit. Prefer intentional use of custom CSS and existing Vue components; evaluate Tailwind or other libraries only as a bounded choice. Design for narrow mobile viewport first, reduced-motion preferences, zoom at 200%/400%, dark/light contrast and keyboard-only use. Demonstrate hover/focus/disabled/loading/error/success states.

**Accessibility:** target WCAG 2.2 AA where applicable, manual keyboard/screen reader review, semantic labels, focus order/trap/return, large touch targets, responsive reflow, announced errors/status, drag-drop alternative and visible progress/cancellation. Automated axe assertions **augment**, not replace, manual testing.

**Privacy UX:** a clear accurate no-upload claim for *local* tools, no confusing server-render routes, explicit supported formats/limits, honest lossy-reencode/transparency/orientation behavior, downloadable output and understandable failure recovery. Prefer no dark-pattern upsell, forced registration or unnecessary animation.

## 6. SEO architecture (not an SEO-page factory)

Evaluate prerendered/SSR explanatory pages and client-only local processing separately. Search-visible copy should be crawlable without triggering file work; verify actual rendered HTML and canonicalization. Use Nuxt typed metadata facilities such as \`useSeoMeta\`, content-aware titles/descriptions, canonical URLs, a valid sitemap and robots policy, Open Graph previews, internal linking and structured data **only where actually eligible**. Audit broken links, redirects, duplicate titles and structured-data claims.

Each tool page needs a real functioning tool, original examples, formats, FAQ/help only if genuinely helpful, privacy disclosures and human review for accuracy. Never generate large quantities of near-identical dimension/keyword pages, fake reviews or unwarranted "government-approved" photo claims. Search Console and structured-data results require *actual site verification*; documentation cannot claim indexing or ranking.

## 7. Performance engineering and representative budgets

**Web UX targets (planning thresholds, not measured IMGENGINE):** Core Web Vitals at field p75: LCP ≤2.5 s, INP ≤200 ms and CLS ≤0.1. Field data must be identified as field; local Lighthouse/lab runs cannot be substituted for real-user p75. If user traffic is too small for a field percentile, report INSUFFICIENT_DATA and run a reproducible controlled lab baseline instead. Document network throttling, device/browser, CPU pressure, commit/build and cache state.

**Image operation stopwatch taxonomy:** \`BROWSER-0\` tool route usable; \`BROWSER-1\` user selects file → validated decode; \`BROWSER-2\` decode → preview; \`BROWSER-3\` selected file → downloadable output incl encode; \`BROWSER-4\` Worker init and communication; \`BROWSER-5\` Wasm cold download/instantiate and warm execution; \`BROWSER-6\` repeated operations/cancel/peak memory, CPU/battery where measured. Use synthetic safe JPEG/PNG/WebP fixtures covering portrait, landscape, alpha, EXIF/GPS, oversized/corrupt and low-end mobile. Avoid misleading nanosecond full-image claims.

**Budgets to determine empirically in F6-00**, not invent as current results: initial JS/CSS transfer, Wasm cold size, long tasks, interaction blocking, main-thread processing, memory allocations/high-water mark, p50/p95/p99 for representative image sizes and correct export fidelity. Any reported improvement must include baseline and candidate SHA, runner, fixtures, multiple samples and no correctness/privacy regression. Route-level lazy loading and reduced third-party scripts may outperform a new engine.

## 8. F6-06 Rust/Wasm/Worker decision record

Test **A. browser-native only**, **B. browser APIs in a Worker**, **C. narrow Rust/Wasm processing kernels with JS orchestration**, and optionally **D. safely portable C17→Wasm**. A spike is not a commitment to ship. Compare output parity/fidelity and codec/browser support; bundle/download/compile cold costs; warm compute; total copies across JS↔Wasm; Worker lifecycle; WASM linear memory peak; mobile responsiveness, battery heat (where observable), threat/supply-chain surface, a11y and total maintenance cost.

**Default decision:** retain the simpler browser implementation if the evidence does not prove a material gain. Web Workers/Wasm are not security sandboxes. Never bundle an entire native scheduler, codec library or server C ABI merely because a C17 core exists; the Linux \`.so\` cannot simply run in a browser. SIMD, threading and cross-origin isolation require browser/platform checks; preserve no-upload behavior and no automatic server fallback.

## 9. Security, CISSP/CCIE and release checks

**Browser threat model:** hostile compressed images/decompression bombs, misleading MIME, oversized decoded buffers, untrusted recipe/metadata, unsafe file names, corrupt/animated inputs, stale Worker completions, cross-route object URLs, unexpected fetch/beacon/form/analytics transmission, XSS/CSP, dependency vulnerabilities and unsafe third-party scripts. Hard-cap file bytes, decoded pixels, output buffers, job time and concurrency; cancel/discard stale work and explicitly release ImageBitmap/Worker/object URLs/canvases. Use only synthetic privacy fixtures with no real customer IDs/photos in CI or telemetry.

**Network/CCIE boundary:** verify browser no-upload using network instrumentation even on error; scope asset CDN/cache/CSP and HTTPS/headers without exporting user photos. Server features (Studio/API) have separate consent, Rust admission/C17 native safety, TLS, private network, quotas, host memory/CPU/egress, C-A0/P14, rollback and real-host evidence. An image-bearing outgoing request from Free is a **hard failure** regardless of performance benefits.

**Known dated risk:** root [temporary frontend picomatch non-production waiver](FRONTEND_PICOMATCH_TEMPORARY_NONPROD_RISK_ACCEPTANCE_2026-11-18.md) expires **2026-12-02 UTC** and is not a production approval; confirm its current fix/status before launch. Preserve all seven existing GitHub required jobs and the F2c 12-category acceptance matrix. Never mark NOT_RUN or older SHA checks PASS.

## 10. Engineering review, dev tooling and governance

- **Seven perspectives:** L1 clear semantic UI/tests; L2 complete functional flows; L3 security/correctness/performance; L4 shared design system/contract integration; L5 rollout/CI/compatibility; L6 Worker/Wasm/platform decisions and total cost; L7 long-horizon platform leverage/simplicity. Apply proportionally, not seven uncontrolled agents. [Seven-level review guide](SEVEN_LEVEL_ENGINEERING_OPERATING_MODEL.md).
- **Dev tooling proposals:** existing pnpm lockfile/Node version, Vitest/Nuxt typecheck/build, Playwright Chromium/Firefox/WebKit, privacy network guard, axe + manual a11y, screenshot/visual diff with reviewed baselines, Lighthouse profiling, dependency auditing and a small trace/benchmark harness. CI changes need separate tests and approval; avoid new Python/build wrappers and unsupported dependency upgrades.
- **HLD/LLD/ADR policy:** new architecture decisions only when a change affects execution engine, compatibility, global design tokens, data flow or trust boundary. Small UI tasks need a short PR contract and focused tests, not mandatory large diagrams. Prefer a single source of truth, precise module responsibilities, DRY/SOLID when appropriate, and measurable failure/rollback cases.
- **PR handoff:** base SHA; narrow user problem; changed paths; claimed scope; unit/E2E/privacy/a11y/SEO/perf results or NOT_RUN; browser/device/fixture; screenshot or artifact where safe; security/demand/cost impact; rollback; links to changed canonical docs and next.md. No unreviewed merge, deployment, DNS, ads, analytics provider, payment or spend.

## 11. CA/MBA/product/management evidence gates

| Domain | Decision question | Metric and caution |
| --- | --- | --- |
| Activation | Can a user finish a task? | Successful local downloads / attempted valid tasks; no filenames or image content |
| Quality | Is the output useful and correct? | Failure/redo rate; validated dimensions, format and fidelity |
| Retention | Does a user come back? | Consent-appropriate aggregate repeat use; avoid pervasive tracking |
| SEO/growth | Does helpful content bring users? | Qualified organic landing traffic and successful tool use, not keyword page count |
| Finance | What does each improvement cost? | Engineering effort, CDN/Wasm bandwidth, QA and support cost versus measured value |
| Future Pro/Studio | Is there a paid problem? | Actual time/rework saved and willing-to-pay interviews, not inflated signup counts |
| Operations | Can changes be supported? | Crash/cancel rates, accessibility/support cases, test flake, dependency patch latency |

Gross traffic is not revenue. Browser-local compute can reduce server variable cost but still incurs frontend delivery, QA, acquisition, legal and support cost. Do not introduce subscriptions or complex analytics without approved commercial/privacy architecture. No ROI or profitability number is implied.

## 12. Launch/next-action rules

**Immediate NOW:** finish the verified next Free task, **F2c Compress**, followed by **F4 Convert** and **F5 release gates** unless actual GitHub evidence updates sequencing. Do not use F6 as permission to stop or repeat already-merged work.

**F6 activation condition:** F5 owner-approved Free v1 release is confirmed, or the owner explicitly authorizes an independent bounded F6 discovery task that does not obstruct F5. Begin with **F6-00 baseline audit and priority register**, not a wholesale redesign. Only then select one highest-leverage F6-01…F6-09 PR. F6-10 is a new release gate, independent of individual feature merges.

**Status owner:** update [next.md](../next.md) from actual PR/CI/release evidence. Keep detailed F6 acceptance here and in the [companion matrix](IMGENGINE_FRONTEND_EXCELLENCE_F6_ACCEPTANCE_MATRIX.md); do not bloat the live board or rewrite historical F0.