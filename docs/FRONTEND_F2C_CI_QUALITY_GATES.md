# F2c Browser-Local Compress — CI Quality and Acceptance Contract

**2026-11-19 F2c follow-up to historical 2026-11-18 status:** [PR #72](https://github.com/Rofikali/imgengine/pull/72) now adds an **eighth browser-quality job** on top of seven mandatory jobs (plus an independently merged P8 parser job). The 2026-11-18 “not implemented” baseline below is historical. Exact-head [GitHub browser run 44454426074](https://github.com/Rofikali/imgengine/actions/runs/44454426074) was **11/12 PASS; Firefox JPEG-with-EXIF preview FAIL** while Chromium desktop/mobile and WebKit passed. **F2C-PORTABILITY-01 remains IN_PROGRESS**: bounded Worker→local Canvas fallback, robust cancellation, independent JPEG APP segment checks with synthetic GPS and error-path tests are proposed in the same PR. These must earn their own current-SHA CI; edits and docs do not constitute browser PASS. Required F2c G01–G12 evidence remains independently gated, with no implicit release or risk waiver. Review perspectives: all **seven engineering levels**, plus **CISSP, CCIE, senior Linux/SRE, CA, MBA and management**, proportionate to the changed boundary. The next job after an owner-authorized merge is F4 Convert, not a premature F6 redesign.

**Status (2026-11-18): DOCUMENTED, NOT IMPLEMENTED AS A 12-GATE CI PIPELINE.** This document specifies mandatory acceptance evidence; it does **not** claim that F2c was committed, tested, merged, or deployed. It is the **single source of truth** for the F2c 12-category quality matrix. Agent rules live in [root AGENTS](../AGENTS.md) and [frontend AGENTS](../imgengine-saas/frontend/AGENTS.md); the current task/state belongs only in [next.md](../next.md). Existing scope/CI policy: [change-scoped verification](CHANGE_SCOPED_VERIFICATION.md). No existing required checks may be removed or bypassed as part of implementing this plan.

## 1. Verified baseline vs local report

- GitHub main examined: **8fd55235101463dc3eacbc56a110f83d14bb06fb**. The main workflow at [.github/workflows/ci.yml](../.github/workflows/ci.yml) defines **seven unconditional jobs**: native-build-test, native-sanitizers, native-fuzz, saas-integration, frontend-verification, rust-api-container, docker-linux-verification. Seven CI jobs are **not** twelve frontend-quality gates.
- The **frontend-verification** job currently installs locked dependencies and performs production dependency audit, Nuxt prepare, Vitest unit tests, Nuxt typecheck, and production build. It **does not execute Playwright**.
- Frontend [package.json](../imgengine-saas/frontend/package.json) specifies Node compatibility and **pnpm@10.34.9**; CI uses Node 24, Corepack, and frozen lockfile installation. Keep versions sourced from the repository, not an arbitrary globally installed pnpm version.
- The current [Playwright configuration](../imgengine-saas/frontend/playwright.config.ts) restricts tests to sheet-crop.spec.ts, with desktop/mobile Chromium projects; it is **not** F2c coverage, and Firefox/WebKit are not configured there.
- **User-provided local-agent report, not independently verified from GitHub:** F2c Compress exists as **uncommitted** work on local branch codex/f2c-local-compress; input JPEG/PNG, JPEG output quality 40–95 (default 80), 4 MiB and 4 MP bounds, white alpha compositing, metadata stripping, cancellation and Worker cleanup. Local git diff --check reportedly passed; frontend unit, typecheck, build, browser tests and PR CI **NOT RUN**. None of those features may be described as merged or remotely verified solely from that report.
- Existing [temporary picomatch risk decision](FRONTEND_PICOMATCH_TEMPORARY_NONPROD_RISK_ACCEPTANCE_2026-11-18.md) is **not a fix**. It is limited to one frontend non-production advisory and expires **2026-12-02 00:00 UTC**; it does not permit production release, other high/critical advisory waivers, or weaker Rust/native/infra gates.

## 2. The 12 quality gates

Status key: **PRESENT** means a related CI step exists on main (not F2c PASS); **PARTIAL** means supporting tooling exists but acceptance coverage is incomplete; **MISSING** means no verified automated check; **CONDITIONAL** applies only if the relevant boundary changes. No gate is certified PASS until its tests run on the exact candidate SHA and the run URL is recorded.

| Gate | Risk/acceptance evidence required for F2c | Main baseline | Required timing |
| --- | --- | --- | --- |
| G01 — Git/code hygiene | Diff whitespace, lint/format checks for touched TypeScript/Vue, no accidental source/generated artifacts | PARTIAL: diff check is local; no verified frontend lint/format CI step | PR |
| G02 — Static types | Nuxt/Vue typecheck, typed Worker messages, failure and cancellation contracts | PRESENT: Nuxt typecheck; new Worker typing must be verified | PR |
| G03 — Unit/property | MIME/signature and decode failures, 4 MiB/4 MP exact boundaries, JPEG quality range 40–95 and default 80, PNG alpha policy, error/result states | PARTIAL: Vitest runs, F2c cases unverified | PR |
| G04 — Production build | Frozen lockfile install, Nuxt prepare and build; check no browser-only API on SSR paths | PRESENT: install/prepare/build, F2c code absent from main | PR |
| G05 — Worker/integration | Blob/Worker message contract, cancellation races, repeated runs, terminate/restart, object URL and input/output buffer cleanup | MISSING: F2c-specific automation not verified | PR |
| G06 — Browser E2E | Actual JPEG/PNG pick → quality → Compress → valid JPEG download, corrupted input, error, refresh/navigation, mobile interaction | MISSING from main CI; existing Playwright testMatch excludes F2c | PR |
| G07 — Cross-browser | Desktop Chromium, Firefox, WebKit plus supported mobile viewport; record genuine browser limitations | MISSING for F2c | PR matrix, or pre-merge required compatibility workflow |
| G08 — Accessibility | Keyboard-only use, label/focus behavior, live error/status announcements, axe checks and responsive layout | MISSING for F2c in CI | PR |
| G09 — Privacy/metadata | Instrument network and fail on image-bearing outbound requests, including Nuxt/Rust API/third parties; decoded JPEG must exclude input EXIF/GPS/ancillary metadata; no file bytes in logs/storage | MISSING for F2c | PR; hard blocker |
| G10 — Security/resource abuse | Corrupt/truncated/polyglot images, real decoded dimensions vs metadata, decompression bomb/resource ceiling, Worker exceptions, denial-of-service loops; audit production dependencies | PARTIAL: dependency audit present; F2c robustness missing | PR; hard blocker |
| G11 — Performance/resource | Record fixture sizes, bytes in/out, browser/runner, duration distributions and repeat-run resource trends; apply only stable, evidence-based budgets | MISSING F2c benchmarks; no unmeasured speed promise | PR smoke measurement; extended nightly trends |
| G12 — Regression/boundaries | Previously merged Resize/Crop/Rotate and shared routing/UI behavior must continue to work; Rust API/C ABI/native checks when a shared boundary changes | PARTIAL: existing 7 CI jobs run; browser regression not integrated | PR plus applicable release lanes |

**Acceptance scope:** G01–G10 and G12 must have appropriate automated exact-SHA PR evidence; G11 requires a reproducible smoke record, with longer/flaky stress analysis on scheduled/manual CI. Gate categories may share CI jobs, but the PR must report **all twelve** separately. Where a gate is legitimately inapplicable, record **N/A + rationale and affected boundary** (never silently PASS). The existing seven unconditional jobs stay required; adding a gate does not replace them. Additional native sanitizer/fuzz, C11/C17/C23 compatibility, FFI or API contract matrices are required **when the changed boundary warrants them** and as part of relevant release verification; do not invent a blanket C23 qualification.

## 3. F2c contract and adversarial fixtures

1. Process selected JPEG/PNG entirely within the user's browser; no server upload or silent fallback. Validate **actual decoded** dimensions and MIME/content behavior, not only extension/file.type. Enforce maximum **4 MiB input** and **4 MP decoded pixels** (exact edge fixtures: under, equal, over). Explicitly decide whether zero-byte/animated PNG/malformed EXIF are rejected or handled safely and test the contract.
2. Output a genuinely decodable JPEG at requested quality **40–95** (default **80**). Compression quality is a codec hint, **not** a guaranteed output-byte reduction; test successful output and report cases where bytes grow. PNG transparency must composite over white. Alpha cases: transparent, semi-transparent, opaque, malformed.
3. Construct synthetic privacy fixtures with EXIF orientation/GPS and PNG ancillary metadata; independently parse the generated JPEG to prove prohibited metadata does not survive. Do not use real customer images in public CI or artifacts.
4. Trigger cancellation before decode, during processing, immediately after completion, and on route navigation/unmount; ensure only one completion wins, stale Worker responses cannot update state, and all Worker/object URLs/resources are released. Prefer deterministic mocks for races plus a real browser smoke test.
5. Monitor fetch/XHR, beacon, form submissions and other relevant outbound paths in real-browser tests; explicitly fail any image-byte transmission. Distinguish harmless application/asset requests from uploads using request bodies, endpoints and synthetic input canaries. No sensitive pixels/bytes in console, traces, analytics or persistent browser storage.
6. Use repeatable small synthetic fixtures, boundary-size fixtures and representative photo-like images licensed for repository use; record provenance and deterministic fixture generation. Tests should not depend on external URLs or real production services.

## 4. Executable CI implementation plan (next separate code PR)

- **Preserve** the seven existing jobs in [.github/workflows/ci.yml](../.github/workflows/ci.yml). Introduce a frontend-specific 12-gate reporting/check layer without falsely claiming a new job per gate. Maintain stable required job names, fail closed on unknown changes, and review GitHub branch protection before conditionalizing any job.
- Start from repository versions: **Node 24** and **packageManager pnpm@10.34.9**, **Corepack**, **pnpm install --frozen-lockfile**. In imgengine-saas/frontend use: **pnpm exec nuxi prepare**, **pnpm test**, **pnpm run typecheck**, **pnpm run build**. Existing browser script: **pnpm run test:browser**; currently this alone is NOT F2c verification because Playwright testMatch selects only sheet-crop.spec.ts.
- Add F2c-focused Vitest and Worker tests, then extend Playwright's test discovery to select the F2c spec; add Chromium/Firefox/WebKit projects deliberately, install compatible browsers in CI, and enable trace/screenshot **on failure only**. Avoid fake tests or marking skipped suites PASS. Run on a Linux runner, not dependent on the developer's Windows pnpm PATH.
- Add network privacy canary tests, metadata parsing, pixel-dimension/memory/abuse fixtures, keyboard/axe checks, failure-artifact retention, and a machine-readable 12-gate summary with exact SHA and run URL.
- Consider a **separate scheduled/manual extended lane** for stress/performance trends and relevant C/Rust/native safety matrices. An extended lane does not waive pre-merge browser/privacy/accessibility checks. No upload to external performance services or test collection of real customer content.
- Dependency/CI security: minimal permissions, pinned trustworthy action versions, locked transitive dependencies, no broad high/critical exceptions. Honor the existing dated picomatch risk decision; require timely patch or re-review, and no public release while it remains unresolved.
- If adding branch-protection required checks, ensure the corresponding job always reports a real result rather than silently disappearing due to path filters. Review workflow triggers, job names and PR/fork trust boundaries.
- In each PR: provide **branch, exact head SHA, workflow run URLs, 12-row PASS/FAIL/SKIPPED/NOT RUN/N/A matrix, test counts, relevant coverage, observed performance conditions, open risks, and reviewers**. Do not call green unit/build CI a successful 12-gate run.

## 5. Engineering ownership and release decision

| Accountable role | F2c testing responsibility |
| --- | --- |
| Junior engineer | Deterministic small fixtures, happy-path tests, formatting, reproducible failure report |
| Mid-level engineer | Vue state/Worker unit tests, boundary values, cleanup, typed errors |
| Senior engineer | Browser E2E, keyboard/mobile accessibility, race and negative cases |
| Staff engineer | Module/Worker ownership, test seams, CI architecture and cross-feature regression |
| Principal engineer | Browser-vs-Rust/C trust boundaries, failure modes, performance budgets, threat model |
| Distinguished engineer | Cross-browser portability, nondeterministic CI stabilization, scalability of test architecture |
| Fellow / organization-wide reviewer | Compatibility/release governance, system-level evidence standards and long-term technical risk |
| Security engineer | No-upload canaries, metadata/GPS removal, dependency advisories, abuse and resource ceilings |
| QA/SDET and SRE/DevOps | Coverage quality, browser matrix, trustworthy runners, artifacts, fail-closed required checks |
| Product/CA/MBA/management | User value, release cost, free-tier economics, acceptance evidence and explicit launch authorization |

These are **review lenses, not claims of seven staffed reviewers**. A single qualified engineer/agent may implement several roles, but evidence and independent review should scale with risk. Security decisions must name a decision owner and expiry; automated acceptance cannot replace a public-release authorization.

## 6. F2c state machine and Definition of Done

Use: **READY on remote main** → **LOCAL_WORK_REPORTED (not inspected)** → **PR_OPEN** → **CI_PENDING** → **READY_FOR_REVIEW (all applicable gates evidenced)** → **MERGED (real main SHA)** → **PRODUCT_RELEASE_REVIEW** → **RELEASED only after explicit authorized public launch**. A local agent can be IN_PROGRESS without changing the remote main status. Failed gates are **FAIL/BLOCKED**, not DONE. Do not advance a state from documentation edits, stale prior-commit runs or user-report alone.

**Next engineering action:** inspect the dirty local F2c branch without overwriting it; implement missing tests/CI on an appropriate feature branch, verify the exact PR revision, address findings, then request review. This documentation-only PR introduces **no F2c source code, test implementation, workflow change, CI result or production permission**.
