# AGENTS.md — IMGENGINE SaaS Frontend Engineering Constitution

> **LIVE FRONTEND DIRECTION (2026-11-16, overrides dated M1.2 task notes below):** F0 browser-first ADR [PR #55](https://github.com/Rofikali/imgengine/pull/55) is MERGED; M1.2 editor and M1.3 parity are already merged (PRs #47/#49). **Next task is F1 browser-local JPEG/PNG Resize** with strict privacy, resource, output and accessibility tests; no image upload, mandatory Rust/Wasm engine or server fallback. The owner-selected [four-product portfolio](../../docs/IMGENGINE_FOUR_PRODUCT_PORTFOLIO.md) names Free (**first five tools exactly Crop, Resize, Compress, Rotate, Convert**), later Pro creator workflows, Studio print workflows and independently metered Developer API; [live task states](../../next.md) outrank stale historical instructions below. Do not implement subscriptions, accounts, paid entitlements, ads, billing or more Free tools without separate decisions. [Skill 19](../../skills.md) and [commercial gates](../../docs/IMGENGINE_COMMERCIAL_READINESS_AND_UNIT_ECONOMICS.md) guide subsequent product PRs. Keep current seven GitHub CI checks and no automatic merge/deploy.

> **Current frontend status and authority (2026-11-15, post merged [PR #43](https://github.com/Rofikali/imgengine/pull/43)):** This directory contains Nuxt 4.6.4/Vue 3.5.44 TypeScript P5/P8/P12 workflows, the three same-origin proxies, and **M1.1 validated P13 crop transport in the sheet proxy**. The **browser crop UI, typed user-selection/serialization, M1.3 parity, browser-local Wasm, production accessibility/E2E, PDF, tenancy and billing remain unimplemented or unverified**. Recorded seven passing CI jobs: [PR #43 head](https://github.com/Rofikali/imgengine/actions/runs/43683324448), not a live release. Consult [source-based frontend/backend audit](../../docs/FRONTEND_LEGACY_BACKEND_AUDIT_2026-11-14.md), [Rust API contract](../../docs/api.md), [P12](../../docs/P12_PHYSICAL_SHEET_CONTRACT.md), [P13](../../docs/P13_INTERACTIVE_CROP_CONTRACT.md), [next.md](../../next.md), and [optional skills.md](../../skills.md). Current source and versioned contracts take precedence over historical P7.0 design descriptions.

> **Binding backend selection (2026-11-14):** The only approved **new IMGENGINE API/backend** is Rust/Axum under `backend/`, with safe FFI to C17. **Never** introduce or route a new frontend feature through Python/FastAPI, Django/DRF, Flask, Celery, Redis or PostgreSQL; the older backend is retained solely for authorized legacy maintenance/rollback. The existing Nuxt server-side proxy is a credential-protecting browser adapter, **not** permission to build a second independent backend. See [approved technology ADR](../../docs/TECHNOLOGY_AND_REPOSITORY_POLICY.md), [Rust-scoped AGENTS](../../backend/AGENTS.md), current [API contract](../../docs/api.md) and [next queue](../../next.md).

> **Astra frontend M1.2 start — READY:** Read the [source-backed frontend editor task](../../docs/ASTRA_P13_M1_2_FRONTEND_READY_HANDOFF_2026-11-15.md) and [Astra Track B](../../docs/ASTRA_ENGINEERING_START_HERE.md). **M1.1 is already merged**; do not reimplement Rust/Axum/proxy transport. The four existing `crop_*` fields are documented in [P13](../../docs/P13_INTERACTIVE_CROP_CONTRACT.md). **Browser `SheetRenderInput` and `createSheetRenderForm` currently do not serialize them**; add typed all-or-none optional fields plus an accessible crop UI in a focused M1.2 PR. Omitted crop preserves P12 centered fill. Test keyboard/touch/mobile, validation, result preview cleanup, and report exact SHA/CI. M1.3 image parity remains later.

## 1. Scope

These instructions apply to `imgengine-saas/frontend` and supplement the repository-level `AGENTS.md`.

The frontend is the presentation and client-experience layer of IMGENGINE. It must consume documented backend capabilities; it must not redefine backend architecture.

## 2. Current Backend Boundary

The implemented Rust HTTP capabilities are:

```
POST /api/v1/render
multipart/form-data: file
JPEG/PNG -> completed JPEG

POST /api/v1/layout-render
multipart/form-data: file, width, height, unit, dpi?, copies, columns?, gap, margin, output_format
FIT-only grid -> completed JPEG + native plan accounting

POST /api/v2/sheet-render
multipart/form-data: file, sheet_width_mm, sheet_height_mm,
  photo_width_mm, photo_height_mm, dpi?, copies, margin_mm,
  gap_x_mm, gap_y_mm, output_format,
  crop_x?, crop_y?, crop_width?, crop_height?  # ALL FOUR or NONE
physical sheet: omitted crop = P12 centered-fill; complete crop = P13 selected crop (ABI 2.3)
completed JPEG + native plan accounting
```

All three routes require server-side `X-API-Key` authentication and run synchronously through the Rust request lifecycle and bounded admission path. They are ephemeral; the API does not create durable jobs or output URLs.

The P12 sheet contract defaults to 300 DPI, accepts DPI 1..=1200, accepts millimetre decimals up to three fractional digits, and rejects a request when the exact requested copy count cannot fit. JPEG is the only output format in this slice.

Do not assume the following exist in the Rust API unless a later contract explicitly implements them:

- job creation/polling
- durable job IDs
- `/api/jobs`
- `/api/presets`
- PDF output
- durable output URLs
- job logs
- Idempotency-Key semantics
- client-supplied API credentials
- backend cancellation
- persistent image storage

Legacy FastAPI functionality may remain during migration, but frontend migration must be contract-driven and reversible.

## 3. Frontend Owns

The frontend owns:

- Nuxt/Vue application structure
- pages and routing
- UI/UX
- upload interaction
- client-side validation and user-friendly error presentation
- HTTP API client/adapters
- loading, success, failure, overload, timeout, and disconnect UX
- accessibility
- responsive behavior
- browser-compatible state management
- frontend tests
- frontend build/dependency hygiene
- presentation telemetry that contains no secrets or image contents

## 4. Frontend Must Not Own

Never move these responsibilities into browser code:

- C scheduler
- native engine lifecycle
- Rust request lifecycle
- Rust admission/concurrency control
- FFI
- workspace creation/cleanup
- server resource limits
- native security policy
- server-side authentication enforcement
- API-key validation
- secrets
- persistence
- image retention policy

The browser is untrusted.

## 5. Secrets

Never expose `IMGENGINE_API_KEYS` or any server credential to client-side JavaScript.

Nuxt runtime configuration must distinguish public browser configuration from private server configuration. A value needed only by the Rust backend must never be placed in `runtimeConfig.public`, serialized into HTML, embedded in client bundles, or logged.

The preferred production browser model is a same-origin or controlled server-side API boundary where credentials remain server-side.

## 6. API Client Rules

Do not scatter raw `$fetch` calls across components.

Create a typed API boundary with:

- explicit request/response types
- stable error classification
- timeout/abort handling where supported by the contract
- no secret handling in browser code
- no assumptions about legacy job persistence
- tests for HTTP status and problem-type mapping

Do not silently translate a synchronous response into a fake job model.

## 7. File Handling

Uploaded images are untrusted.

The frontend may perform convenience checks such as file selection, accepted media type, and user-facing size validation, but server validation is authoritative.

Do not read entire images into long-lived application state unnecessarily.

Do not persist uploaded image bytes in localStorage, IndexedDB, analytics payloads, URLs, or logs unless a separately approved product requirement explicitly requires it.

## 8. UX State Model

The first Rust render flow should have explicit states such as:

```
idle
-> validating
-> uploading
-> processing
-> success
-> error
```

Overload, authentication failure, invalid image, unsupported media, request too large, deadline exceeded, disconnect, and unavailable/shutdown responses must remain distinguishable when the backend contract exposes them.

Do not invent progress percentages when the backend has no progress telemetry.

## 9. Accessibility and Quality

New UI must be:

- keyboard usable
- semantically structured
- label-associated
- understandable without color alone
- responsive
- usable on common desktop/mobile widths
- explicit about asynchronous state and errors

Avoid unnecessary UI libraries unless their cost and value are justified.

## 10. Dependencies

Prefer the smallest dependency set.

Before adding a package, document:

1. problem solved,
2. why native Nuxt/Vue APIs are insufficient,
3. bundle/build impact,
4. maintenance/security implications.

Do not add a state-management library, UI framework, upload library, or HTTP abstraction merely for convenience.

### Current frontend security and performance boundaries

- Three Nuxt server routes perform bounded multipart parsing, private credential forwarding, and safe error mapping; they are *similar but not interchangeable*. Preserve each P5/P8/P12 contract and test before considering any DRY refactor.
- Client-side AbortController and temporary object-URL release are **not native C cancellation**. The Rust request lifecycle may finish native work after a browser disconnect.
- Never interpret a server-to-server shared API key as end-user authentication, tenant isolation, or paid quota enforcement.
- **P13 M1.1 is implemented/merged:** `server/api/sheet-render.post.ts` accepts and forwards the exact four `crop_x`, `crop_y`, `crop_width`, `crop_height` fields; `types/api.ts` and `utils/render-form.ts` still need M1.2 browser selection + serialization. Do not invent additional HTTP fields or imply M1.2/M1.3 complete.
- Browser-local image processing, when later approved, must not upload original image bytes or silently switch to server processing. Worker/Wasm remains **planned**.
- Frontend-only implementation should run focused Vitest/typecheck/build and relevant browser/a11y tests; do **not** manually rebuild the native C core or run its full sanitizer/fuzz suite unless a native/shared boundary changed. Required GitHub CI and release checks remain mandatory.
- Track dependency-audit findings and review Nuxt devtools behavior in production. Previous P14 dependency results are dated evidence, not a fresh vulnerability scan.

## 11. Testing

**CI-first output requirement:** Use [minimal-AI-context test-log policy](../../docs/CI_FIRST_MINIMAL_AI_CONTEXT_POLICY.md) and [change-scoped verification](../../docs/CHANGE_SCOPED_VERIFICATION.md). For Nuxt UI, run relevant local component/browser/a11y tests and avoid manual native re-verification without a shared C/FFI change; when a PR is published, let existing GitHub Actions run all seven jobs and read only their exact-SHA statuses or focused failed-step excerpts. A local unpushed test report is not GitHub CI evidence.

Frontend changes should use layered verification appropriate to the change:

- type checking
- build
- component/unit tests where behavior warrants them
- API contract tests for the client boundary
- accessibility checks for important user flows
- browser/E2E tests for critical upload/render/download paths when the tooling is introduced

The authoritative Linux integration environment remains Ubuntu 24.04 in Docker.

## 12. Documentation

Clearly distinguish:

- Planned
- Implemented
- Validated

Frontend documentation must reference the authoritative backend contract instead of copying stale legacy API assumptions.

## 12.1 Work-tracking and evidence handoff

**This file defines scoped frontend engineering rules; it is not the project backlog.** Before a frontend milestone, read [root next.md](../../next.md) for the current F/H/O/P status ID and next acceptance gate, the [source-backed frontend/backend audit](../../docs/FRONTEND_LEGACY_BACKEND_AUDIT_2026-11-14.md), [change-scoped verification](../../docs/CHANGE_SCOPED_VERIFICATION.md), the authoritative versioned API contract, and optional [skills.md](../../skills.md). Do not duplicate whole roadmaps or speculative routes here.

- A feature can be **planned**, **implemented in Nuxt**, **verified at a specific SHA**, or **released**; state precisely which. A merged docs-only PR is not frontend functionality or public release.
- For a change to P5/P8/P12, preserve the exact multipart fields, server-only credential, server-confirmed counts and safe error semantics. P13 M1.1 Rust/Axum/typed **server proxy** crop transport is already implemented; M1.2 must wire the **browser type/form/workflow/editor** and keep omitted crop as P12 centered-fill. M1.3 separately verifies full browser/native pixel parity.
- For browser-local work proposed after M1, the frontend may own ephemeral in-browser processing and state, but **must not** claim the work stays private until no-image-upload, telemetry, memory cleanup and optional server-fallback consent have been verified. The prohibition on server persistence in section 4 does not prohibit explicitly approved local, ephemeral user editing.
- When a milestone lands, update the relevant **next.md** ID with exact status, PR, test-head SHA, concrete verification and unresolved issues; update the relevant API/product document, not this AGENTS rulebook with an entire test report.
- Product/CA-MBA check: document customer task success and accessibility, device and network cost, bundle size, support/maintenance cost and any new server processing cost before adding broad image-editor dependencies or monetization features.
- **Testing scope:** for Nuxt-only changes, run focused frontend tests/typecheck/build and browser accessibility tests when relevant. Do not manually rerun native CMake/ASan/UBSan/fuzz without a shared native/FFI change. Existing required CI and release checks are never waived.

## 13. Git

Keep frontend changes scoped.

Before commit:

- inspect status and diff
- run relevant checks
- check for secrets
- check generated files
- avoid unrelated backend/native changes

Never push without explicit user authorization.

## 14. Architecture Rule

The frontend adapts to the backend contract:

```
Nuxt/Vue/TypeScript
        |
        v
Typed frontend API boundary
        |
        v
Rust Axum API
        |
        v
Request lifecycle
        |
        v
Bounded admission
        |
        v
C scheduler + native engine
```

The frontend must not recreate any lower-layer state machine.

## 15. P12/P13 Authority Principle

The P12 physical-sheet UI adapts to the verified ABI 2.2/Rust contract and the **additive P13 ABI 2.3 optional user crop**. It may collect physical dimensions, copy count and a **user-selected normalized source-relative crop rectangle** as preview/interactions, but it must not calculate **authoritative decoded-image crop pixel coordinates**, placement, resource budgets, or success counts; the server/native engine remains authoritative. Omitting all four P13 fields preserves the centered-fill P12 path. Display server-confirmed canvas dimensions and execution counts. Keep P8 FIT distinct from P12/P13 sheet workflows. A local crop-preview operation does **not** make the eventual Render request browser-only: source photo bytes are submitted to the existing server on user action.
