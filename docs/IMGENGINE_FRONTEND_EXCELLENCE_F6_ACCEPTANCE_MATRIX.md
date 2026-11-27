# IMGENGINE F6 — Frontend acceptance, tests, benchmark and release evidence matrix

**F6-00 — CI evidence wiring candidate (27 November):** Add measurement fixture execution to the existing Nuxt verification job and retain a detached JSON asset inventory from the already-built, audited Free static distribution in the existing browser job. No new build, dependency, browser matrix or job; no timing or measured bundle-size claim yet. PR #128 exact head 95b85f0 remains untouched during its [integration run](https://github.com/Rofikali/imgengine/actions/runs/45969914525). New dependent branch changes are NOT RUN, NOT MERGED. See [baseline and measurement procedure](IMGENGINE_FRONTEND_F6_BASELINE_AND_TOKENS.md). F5 release reconciliation, existing gates and owner approvals remain binding.

**F6-00 measurement preparation — 27 November:** PR #127 merged as cfb7886; PR #128 head 95b85f0 is under integration CI ([run 45969914525](https://github.com/Rofikali/imgengine/actions/runs/45969914525)). Prepare a dependent, non-PR development branch with read-only compiled asset sizing and fixtures; do not disturb active CI. No dependencies or workflow changes. Tests and actual artifact measurements NOT RUN. See [baseline and tokens](IMGENGINE_FRONTEND_F6_BASELINE_AND_TOKENS.md). F5 live host retrieval returned DisabledError; release/hosting verification and owner approvals remain open. No further shell/layout migration until F5 disposition and required baseline.

**F6-00 / F6-01A — 27 November, development candidate:** PR #127 head 9937aee passed all 12 jobs ([run 45967764812](https://github.com/Rofikali/imgengine/actions/runs/45967764812)); owner merge pending. Source-only baseline and first seven shared palette/font tokens prepared on separate batched development branch; static CSS value round-trip checked, browser/build tests NOT RUN for this candidate. See [baseline and tokens](IMGENGINE_FRONTEND_F6_BASELINE_AND_TOKENS.md). F5 live reconciliation remains open (live URL retrieval unavailable); do not deploy or claim release. Preserve all routes, HTML, privacy, CSP, runtime and processing behavior. Full candidate CI and baseline runtime metrics outstanding; no new dependencies or spending.

**Owner priority override — 27 November:** Execute original phases 1 → 2 → 3 → 5 → 8 first (live safety reconciliation, shared frontend, website/editor UX, hostname-independent performance/SEO/accessibility, and reusable Free/Pro/Studio UI foundation). Move original phases 4 → 6 → 7 to the bottom (custom domain, finance/marketing, monetization). Domain ownership is not confirmed; use the existing hostname for approved development/verification. Do not select DOM, commercial FIN/MKT or MON work on ordinary `go ahead` / `next` until the owner reopens it. Hostname-dependent canonical/sitemap/redirect/Search Console work waits for the permanent hostname. Existing F5 security, billing safety, approvals and cost controls still apply; no paid product implementation or speculative package extraction is implied by shared UI work.

**27 November extension:** [Shared UI/domain/monetization roadmap](IMGENGINE_F6_SHARED_UI_DOMAIN_MONETIZATION_ROADMAP.md) supplies additional work-package exit evidence: shared-shell regression, native disclosure accessibility, explicit static route additions, compiled-output/CSP/no-upload checks, baseline-relative compressed CSS/JS, hostname/TLS/redirect/rollback, and isolated monetization review. No new test pass claimed. Existing F5/F6 gates remain mandatory.

**Status:** PLANNED TEST CONTRACT / NO F6 CODE OR TEST RUN CLAIMED (2026-11-19). Governing strategy: [Frontend Excellence Program](IMGENGINE_FRONTEND_EXCELLENCE_PROGRAM_F6.md). Binding instructions: [root AGENTS](../AGENTS.md), [frontend AGENTS](../imgengine-saas/frontend/AGENTS.md), [live next.md](../next.md). Existing [F2c 12-category requirements](FRONTEND_F2C_CI_QUALITY_GATES.md) and [Free v1 F5 release checklist](MVP_RELEASE_CHECKLIST.md) are independent; this matrix **does not postpone, replace or weaken either**.

## 1. Evidence record (one row per feature/PR)

Use this compact format; no invented green checkmarks.

| Required field | Record |
| --- | --- |
| F6 ID and exact scope | F6-XX, customer job, path(s), before/after behavior |
| Evidence | PR URL, base SHA, exact head SHA, CI workflow/run URL and timestamp |
| Environment | Browser/version, OS, device class, CPU/memory when meaningful; Node/pnpm/toolchain from lockfile |
| Fixtures | Synthetic filenames/description, dimensions, codec, relevant privacy/EXIF/alpha/bomb/corruption cases |
| Focused checks | Unit, Nuxt typecheck/build, E2E browsers, accessibility, privacy, applicable security |
| Performance | Baseline/candidate SHAs; cold/warm; p50/p95/p99 where significant; transfer, memory and output parity |
| Reporting states | PASS / FAIL / NOT_RUN / N/A + reason for each applicable gate |
| Product/business | Activation/saved time/cost impact or UNKNOWN; data retention/consent review |
| Owner decision | Reviewers, risks, rollback, next change, release decision independent from PR merge |

For documentation-only PRs, link consistency, scope and status truthfulness are in scope. Do **not** run or claim C/Rust sanitizers as a docs-only test. Required GitHub CI still applies to the PR. Shared native/FFI changes require their own ABI/sanitizer/fuzz matrix; API/server changes require Rust/host evidence.

## 2. Milestone acceptance matrix

| Milestone | Mandatory tangible output | Minimum acceptance/evidence | Blocking failure |
| --- | --- | --- | --- |
| **F6-00 Baseline** | Route/component/dependency inventory, source-backed risk register and baseline report | Exact SHA and supported-device/browser matrix, observed tool flow, Lighthouse/lab conditions vs real field data, tests present/missing, privacy/security status, prioritization | Invented bundle/CWV or claiming present tests cover missing E2E |
| **F6-01 Design system** | Token spec and reusable components using existing stack first | Contrast, visible focus, zoom/reflow, reduced motion, component-state tests, documentation, reviewed screenshots and rollback | Inaccessible buttons/dialogs or global regressions |
| **F6-02 Navbar/layout** | Responsive accessible route discovery, footer, mobile menu | Keyboard open/escape/focus return, touch, route navigation, active state, no broken URLs, CLS checks | Navigation inaccessible or dead links disguised as paid products |
| **F6-03 Tool UX** | Uniform file choose/drag-drop, preview, errors, reset/export/cancel | All five tools retain output correctness, limits, object URL/Worker cleanup, mobile/keyboard tests, observed download success and no-upload network capture | Uploaded image bytes, stale result after cancel, incorrect export |
| **F6-04 SEO** | Crawlable five real pages, unique truthful titles, canonical, sitemap/robots and links | Inspect built/rendered HTML, status codes/canonical, structured-data validity where used, content reviewed and tool accessible | Thin or misleading pages, canonical/index mismatch, dead routes |
| **F6-05 Speed/memory** | Reproducible browser process and transfer benchmark | Device/network/fixture definitions; warm and cold; p50/p95/p99 when sample size supports; peak memory where measurable; output/fidelity parity, error and cancel paths | Faster but lower-fidelity or memory/privacy regression |
| **F6-06 Worker/Wasm ADR** | A/B/C (optional D) prototype evidence + adopt/reject decision | Compare native browser, Worker, targeted Rust/Wasm and optional C→Wasm, cold/warm total latency, copies, bundle, memory, mobile, security, parity, ongoing cost | Unmeasured Wasm commitment or silent server fallback |
| **F6-07 Dev tooling** | Reviewed CI/additive test workflow and stable fixtures | Existing seven jobs preserved; Playwright cross-browser, privacy/negative and axe; visual baseline reviewed; flake response and test runtime recorded | Bypassed required jobs, false PASS or unbounded flaky checks |
| **F6-08 Metrics** | Privacy/consent ADR, event dictionary, retention plan and minimal implementation proposal | No image bytes/metadata/filename/customer IDs in events; consent/legal checks where needed, no third-party leakage, opt-out; aggregation accuracy | Unconsented sensitive telemetry or unapproved provider |
| **F6-09 i18n/SEO growth** | Demand-supported locale/content experiment | Native reviewer/translations, locale redirects/canonical/hreflang when applicable, useful content, no mass doorway pages | Inaccurate regulatory photo claims or copied/keyword spam |
| **F6-10 Free v1.x release** | Specific release candidate and owner go/no-go | All selected mandatory F6 child gates and core Free privacy/security checks pass at exact SHA; documented rollback/live verification; no known production-blocking exception | Treating merged PR, local browser or docs as proof of public release |

**Note:** F6-07 is an extension, not a reason to delay minimum F5 browser/privacy/accessibility tests. F6-06 is **conditional**: an evidence-backed decision to remain on browser APIs is a legitimate PASS for the ADR, not a missing feature.

## 3. Page and navigation contract

Target information architecture for planning (confirm actual existing routes before change): \`/\`, \`/resize-image\`, \`/crop-image\`, \`/compress-image\`, \`/rotate-image\`, \`/convert-image\`. Each must have a functional tool or accurately labeled future state, a single useful H1, unique metadata, meaningful instructions, supported formats and limitations, linkable help and a simple privacy statement. No forced account for Free. No indexable fabricated Pro/API/Studio functionality.

Navbar and dropdown must preserve keyboard focus, Escape behavior, meaningful labels, actual internal links, screen-reader reading order, sufficient contrast and responsive reflow. Hero visuals must not displace core tool interaction or create excessive layout shifts. Footer needs real contact/privacy/terms/support information where applicable, not invented business details.

## 4. Latency and memory measurements

**Published UX target candidates** (industry guidance, not measured IMGENGINE): LCP ≤2.5 s, INP ≤200 ms, CLS ≤0.1 at field p75 for sufficiently represented sessions. Explicitly label lab vs field, sample size and representativeness. Avoid setting mandatory source-agnostic byte or millisecond numbers before a baseline.

| Measure | Start → stop | Include in report |
| --- | --- | --- |
| Route usability | Navigation → interactive upload controls | Cold/warm cache, JS/CSS transfer, network/device |
| Decode | User selects a fixture → validated decoded pixels | Format, image dimensions, peak allocation, invalid path |
| Preview | Decode complete → visible updated preview | UI blocking/long tasks, memory |
| Export E2E | File selection → verified downloadable result | Decode/process/encode/preview/export, p50/p95/p99 |
| Worker | Worker create → ready; message send → result | Startup, serialization/transfer, abort/restart cleanup |
| Wasm | Module download → compiled/initialized; warm processing | Bundle + compile cost, runtime, boundary copies, memory |
| Repeated runs | N selected/cancelled/replaced fixtures | Object URLs, Workers, peak memory trend, leaks/OOM |

Use synthetic input under/at/over limits and representative quality/crop/format cases. Detect impossible memory measurement rather than calling unavailable browser heap telemetry zero. Verify actual output bytes, dimensions and color/alpha/EXIF behavior; no unsafe performance speed shortcuts.

## 5. Security/privacy and browser matrix

- **File controls:** byte/pixel/axis/output ceilings, extension ≠ trusted content, source validation and clear rejection, decompression-bomb and corrupted-source negative tests.
- **Privacy controls:** capture fetch/XHR/beacon/forms and potential third-party calls using synthetic canaries; ensure **no photo bytes, filenames or EXIF** travel to application API, analytics, logs, storage, caches or network on success/failure/cancellation.
- **Lifecycle controls:** ImageBitmap/Worker/object URL revocation, stale-result suppression, abort/route-change/replace/rapid-click races, memory cleanup and explicit unsupported-browser behavior.
- **Supply chain:** pinned dependency manifests, production audit, license/provenance, CSP and trusted third-party script review; the time-limited picomatch exception is **non-production only** and needs fresh status review at launch.
- **Accessibility/browser controls:** Chromium, Firefox, WebKit where supported by tested pipeline, representative low-end Android/mobile viewport, keyboard-only with focus/error announcements, at least manual assistive-technology review, reduced motion and high zoom.
- **Server boundary:** Nuxt/Rust proxy for separately approved server-backed product features is **not** a Free local processing fallback. Public Rust/C17 demands C-A0/P14 and relevant real-host security/network tests.

Every test gate is scoped: pure CSS change requires focused UI/browser/a11y checks plus mandatory CI, not automatic full native fuzz locally. Shared C/Rust/FFI changes require independent native ABI safety evidence.

## 6. Proposed F6 engineering PR template

~~~markdown
## F6-XX — customer outcome
Current next.md state; exact base/head SHA; affected routes/components/contracts.

### Evidence and baseline
Expected outcome; before/after UX; synthetic fixtures and browser/OS/device.

### Verification
- Unit: PASS/FAIL/NOT_RUN
- Typecheck/build: PASS/FAIL/NOT_RUN
- Browser E2E and privacy: PASS/FAIL/NOT_RUN
- Accessibility/manual keyboard: PASS/FAIL/NOT_RUN
- Security/dependency: PASS/FAIL/NOT_RUN
- SEO/performance (if relevant): PASS/FAIL/NOT_RUN
- Required GitHub CI seven jobs: PASS/FAIL/NOT_RUN
- Native/Rust checks when shared contracts changed: PASS/FAIL/NOT_RUN/N/A + why

### Economics, risk, rollback
User task success, engineering/CDN/support cost if measured; data-handling risk;
known defects, rollback trigger and steps.

### State / human decision
Feature PR proposed/merged? Product still NOT RELEASED unless separately signed off.
Update next.md and affected canonical docs; next READY task must be rechecked.
~~~

## 7. Recommended independent F6 discovery order

After F5 actual release, **F6-00** audit first. Then pick the highest measured customer pain: typically F6-01/F6-02/F6-03 for interaction, F6-04 for discoverability, F6-05 for measurable latency, and F6-07 for durable tests. Evaluate F6-06 **only after a measured processing bottleneck**. Analytics/i18n follow privacy/legal/demand review. Preserve distinct release and commercial approval for Pro/Studio/API/SDK/Desktop; do not assume a prettier Free UI implies entitlement/payment readiness.