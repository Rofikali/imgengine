# P7.0 Frontend Architecture Audit

**Status:** AUDIT COMPLETE — implementation intentionally not started.

**Base:** `main` after merged P6 request-lifecycle work.

## Executive finding

The existing frontend is a legacy-oriented prototype and is **not aligned with the verified P5/P6 Rust API contract**.

It currently models asynchronous durable jobs, presets, layout parameters, polling, job logs, and output URLs. The verified Rust slice instead exposes synchronous ephemeral JPEG rendering through:

```
POST /api/v1/render
multipart/form-data
file=<JPEG|PNG>
X-API-Key=<server-managed credential>
        ->
completed JPEG response
```

Therefore the frontend should be modernized behind a typed API boundary before product UI expansion.

## Evidence reviewed

- `imgengine-saas/frontend/package.json`
- `imgengine-saas/frontend/nuxt.config.ts`
- `imgengine-saas/frontend/app.vue`
- `docs/api.md`
- `docs/API_CONTRACT_PARITY_GATE.md`
- repository and native/backend AGENTS guidance

## Findings

### F1 — Backend contract mismatch: HIGH

Current `app.vue` calls:

- `/api/jobs`
- `/api/jobs/:id`
- `/api/jobs/:id/logs`
- `/api/jobs/:id/output`
- `/api/presets`

These are not part of the verified Rust P5/P6 contract.

**Action:** replace the legacy job model during P7 implementation with a typed synchronous render client.

### F2 — Legacy layout model: HIGH

The UI submits width, height, DPI, rows, columns, gap, padding, border, bleed, and crop fields.

The current Rust endpoint intentionally accepts only the `file` multipart field.

**Action:** do not send unsupported fields. Preserve layout UI only as a future capability-backed feature, or remove it from the first Rust path.

### F3 — Authentication architecture: HIGH

`nuxt.config.ts` defines `NUXT_API_KEY`. The browser-side `app.vue` does not actually use it, and exposing a backend API key to browser code would be unsafe for a shared SaaS service.

**Action:** establish a server-side/same-origin credential boundary. Never put `IMGENGINE_API_KEYS` or equivalent backend secrets in public runtime config or client bundles.

### F4 — Nuxt baseline: MEDIUM

The current package declares Nuxt `3.17.0`, while repository direction targets modern Nuxt 4/5.

**Action:** audit the Nuxt upgrade path separately. Do not combine framework migration with uncontrolled UI redesign.

### F5 — Dependency baseline: MEDIUM

The frontend currently has a very small dependency set, which is good. There is no testing framework or typed API-client abstraction visible in the reviewed files.

**Action:** introduce only the tooling justified by P7 acceptance criteria.

### F6 — Component structure: MEDIUM

The complete application is concentrated in `app.vue`, including API calls, state, domain types, form behavior, polling, log formatting, and styling.

**Action:** split responsibilities into a small page/component/composable/API-client structure rather than creating a large monolithic page.

### F7 — Error/state model: MEDIUM

Current UI treats most failures as generic JavaScript errors and assumes job polling.

**Action:** model backend problem classes explicitly: authentication failure, invalid image, unsupported media, request too large, overload, deadline, unavailable/shutdown, network/abandonment, and unexpected server failure where applicable.

### F8 — Progress semantics: LOW

The verified backend is synchronous and does not expose processing progress.

**Action:** show real upload/processing state, but do not invent percentage progress.

### F9 — Data retention: MEDIUM

The current frontend assumes downloadable job artifacts and logs. The Rust target is ephemeral and does not expose durable output URLs or job logs.

**Action:** make the first Rust UI consume the completed response directly or through an explicitly documented transient browser flow.

## Target P7 frontend boundary

```
Nuxt/Vue/TypeScript
  |
  +-- pages / route
  +-- presentation components
  +-- upload state
  +-- typed render API client
  +-- error/problem mapping
  +-- accessibility/responsive UI
  |
  v
Rust Axum POST /api/v1/render
  |
  v
Rust lifecycle + bounded admission
  |
  v
C scheduler + ABI v1 + C engine
```

## Explicit non-goals for P7

Do not implement as part of this frontend phase:

- C scheduler migration
- ABI changes
- native cancellation
- backend output quota
- workspace quota
- decoded-pixel product policy
- distributed rate limiting
- PostgreSQL/Redis/Celery
- durable image storage
- legacy API retirement
- production capacity claims

## P7 implementation sequence

1. Create the frontend-specific AGENTS contract.
2. Establish a typed API client for `POST /api/v1/render`.
3. Define frontend problem/error mapping.
4. Establish secure server-side authentication boundary.
5. Replace legacy job/polling assumptions.
6. Build a minimal accessible upload/render result flow.
7. Add responsive visual system/components.
8. Add frontend tests and build/type verification.
9. Verify against the Ubuntu 24.04-backed backend contract.
10. Perform adversarial review for secret leakage, stale endpoints, unsupported fields, oversized client state, and error disclosure.
11. Commit as a focused P7 slice.
12. Ask before push.

## Acceptance criteria for the first P7 implementation

- No client code calls legacy `/api/jobs`, `/api/presets`, or job-log/output endpoints for the Rust path.
- Browser code never contains `IMGENGINE_API_KEYS` or a backend API key.
- Render requests send only the currently supported multipart `file` field.
- Successful JPEG/PNG rendering displays the returned JPEG correctly.
- Backend error classes are user-visible without exposing internal diagnostics.
- Loading and failure states are deterministic.
- No fake progress.
- No image bytes are written to browser persistence or telemetry.
- Type checking/build pass.
- Relevant automated tests pass.
- No backend/native scope expansion.
- Documentation says exactly what is implemented versus planned.

## Decision

P7 should proceed as a **frontend foundation and contract-alignment phase**, not as a visual redesign of the legacy asynchronous job application.

The existing backend contract is the source of truth for the first Rust-powered UI.
