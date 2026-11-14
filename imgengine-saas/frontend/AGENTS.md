# AGENTS.md — IMGENGINE SaaS Frontend Engineering Constitution

> **Current frontend status and authority (2026-11-14):** This directory contains the implemented Nuxt 4.6.4/Vue 3.5.44 TypeScript frontend, typed P5/P8/P12 render workflows, and three same-origin server proxies. This is **not** a claim of public release, complete accessibility/browser-E2E coverage, user-selected P13 crop, browser-local Wasm, PDF export, tenants, or billing. Consult [source-based frontend/backend audit](../../docs/FRONTEND_LEGACY_BACKEND_AUDIT_2026-11-14.md), [Rust API contract](../../docs/api.md), [P12](../../docs/P12_PHYSICAL_SHEET_CONTRACT.md), [P13](../../docs/P13_INTERACTIVE_CROP_CONTRACT.md), [next.md](../../next.md), and [optional skills.md](../../skills.md). Current source and versioned contracts take precedence over historical P7.0 design descriptions.

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
  gap_x_mm, gap_y_mm, output_format
physical sheet + centered crop-to-fill -> completed JPEG + native plan accounting
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
- User-controlled crop remains a separate M1 feature; do not invent a P13 browser-to-Rust crop field before the backend contract is implemented and validated.
- Browser-local image processing, when later approved, must not upload original image bytes or silently switch to server processing. Worker/Wasm remains **planned**.
- Frontend-only implementation should run focused Vitest/typecheck/build and relevant browser/a11y tests; do **not** manually rebuild the native C core or run its full sanitizer/fuzz suite unless a native/shared boundary changed. Required GitHub CI and release checks remain mandatory.
- Track dependency-audit findings and review Nuxt devtools behavior in production. Previous P14 dependency results are dated evidence, not a fresh vulnerability scan.

## 11. Testing

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
- For a change to P5/P8/P12, preserve the exact multipart fields, server-only credential, server-confirmed counts and safe error semantics. The Rust endpoint remains authoritative; the P13 user-selected crop contract must first be implemented through Rust/Axum and the typed proxy.
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

## 15. P12 Principle

The P12 physical-sheet UI is an adapter to the verified ABI 2.2/Rust contract. It may collect physical dimensions and copy count, but it must never calculate authoritative placement, crop coordinates, resource budgets, or success counts in the browser. Display server-confirmed canvas dimensions and execution counts. Keep the P8 FIT mode distinct from the P12 centered-fill mode.
