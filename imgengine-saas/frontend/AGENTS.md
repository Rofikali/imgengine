# AGENTS.md — IMGENGINE SaaS Frontend Engineering Constitution

## 1. Scope

These instructions apply to `imgengine-saas/frontend` and supplement the repository-level `AGENTS.md`.

The frontend is the presentation and client-experience layer of IMGENGINE. It must consume documented backend capabilities; it must not redefine backend architecture.

## 2. Current Backend Boundary

The currently implemented Rust HTTP capability is:

```
POST /api/v1/render
multipart/form-data
field: file
input: JPEG or PNG
output: completed JPEG
authentication: X-API-Key
mode: synchronous + ephemeral
```

The current P5/P6 backend is deliberately narrower than the legacy FastAPI API.

Do not assume the following exist in the Rust API unless a later contract explicitly implements them:

- job creation/polling
- durable job IDs
- `/api/jobs`
- `/api/presets`
- layout fields
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

## 15. P7 Principle

P7 is frontend foundation work, not an excuse to expand backend scope.

First establish a clean, typed, accessible, responsive frontend foundation against the verified Rust API. Add broader product capabilities only after the corresponding backend/ABI/native capability exists and is verified.
