# IMGENGINE First-MVP Release Checklist

## Purpose and scope

This is the canonical release-decision checklist for the first public IMGENGINE
MVP: the synchronous, ephemeral P12 physical-sheet workflow:

```text
Browser -> Nuxt -> Rust/Axum -> RequestLifecycleService
        -> bounded Rust admission -> C scheduler -> libimgengine -> JPEG
```

It applies to `POST /api/v2/sheet-render` and its same-origin Nuxt proxy. It
does not silently retire the legacy `/api/generate` path or reinterpret the
historical durable-job release checklist. See
[`RELEASE_CHECKLIST.md`](RELEASE_CHECKLIST.md) for that legacy-path checklist.

Status labels are deliberately evidence-specific:

- **Implemented**: source/configuration exists; this is not production proof.
- **Validated in CI/Docker**: the stated repository verification evidence ran.
- **Open**: required before public exposure; no claim is implied.
- **Decision required**: an accountable release owner must explicitly accept or
  reject the candidate.

## Current baseline

| Area | Current evidence | Status |
| --- | --- | --- |
| P12 API/ABI and centered crop-to-fill | Additive ABI 2.2, Rust FFI, authenticated Axum route, and Nuxt proxy are recorded in `P12_PHYSICAL_SHEET_CONTRACT.md` and `P12_DEPLOYMENT.md`. | Implemented; validated by the documented Ubuntu/Docker gates. |
| Input, admission, deadline, and output controls | P5/P6 lifecycle limits, P9 bounded output, and P12 request validation are implemented. Their configured values are conservative integration controls, not capacity promises. | Implemented; not a complete RSS/disk bound. |
| Deployment profile | P14 Compose profile has private services, Caddy-only public ports, non-root containers, read-only roots, dropped capabilities, secrets files, bounded tmpfs/PID/memory settings, readiness, and digest-only image policy. | Implemented configuration; not deployed evidence. |
| Supply-chain posture | The pnpm migration recorded no critical/high production audit findings. Moderate/low transitive Nuxt/Nitro build-chain advisories remain. | Open risk decision. |
| Target VPS capacity | Local constrained Docker evidence exists; the actual approximately 1 GiB/20 GB VPS has not been measured. | Open. |

## Release evidence gates

### 1. Candidate integrity

- [x] The intended code revision is identified and the repository's documented
  Ubuntu 24.04 verifier has passed for the candidate evidence cited in P12/P14.
- [x] Native ABI compatibility, Rust FFI, supervisor/API, real-image,
  sanitizer, sandbox, fuzz, frontend test, typecheck, and build evidence are
  recorded for the implemented slices.
- [ ] Produce and retain release-candidate image digests, SBOM/provenance,
  rendered production Compose configuration, and exact verification logs for
  the release revision.
- [ ] Review remaining moderate/low dependency advisories and record either an
  upstream-compatible remediation or a time-bounded, accountable risk decision.

### 2. Product and security boundary

- [x] The P12 contract keeps P8 FIT behavior unchanged and defines the new
  centered crop-to-fill physical-sheet operation separately.
- [x] The browser uses a same-origin proxy; backend API keys are private Nuxt
  runtime configuration and are not sent to the browser.
- [x] The API rejects undeclared/duplicate multipart fields and does not accept
  client-controlled paths, output budgets, or native resource limits.
- [ ] Execute a browser workflow smoke test against the exact release-candidate
  images, including JPEG/PNG, malformed input, capacity rejection, and download
  validation.
- [ ] Confirm deployment-secret file ownership/modes and real host firewall,
  TLS, DNS, and certificate-renewal behavior.

### 3. Resource and resilience evidence

- [x] Per-operation dimensions, copy count, input/admission, deadline, and
  bounded-output guards exist. Native work remains non-cancellable once
  admitted, as documented by ABI v1 semantics.
- [ ] Measure peak RSS, CPU, latency, tmpfs/disk use, cleanup, overload,
  restart, graceful drain, and recovery on the actual target VPS using valid,
  boundary, malformed, and concurrent workloads.
- [ ] Derive final API/Nuxt/Caddy memory, CPU, PID, tmpfs, concurrency, and
  admission settings from those measurements. Do not treat configured Compose
  limits or encoded-output budgets as a whole-system memory proof.
- [ ] Demonstrate that resource exhaustion rejects safely without corrupt output,
  service wedging, or leftover workspaces.

### 4. Operational readiness and rollback

- [ ] Configure redacted metrics/log retention, alert delivery, on-call owner,
  and a documented incident path appropriate to the first-MVP operating model.
- [ ] Deploy the candidate by immutable digest to the target VPS, verify
  `/healthz` and `/readyz`, then exercise a rollback to the retained prior
  manifest without relying on mutable image tags.
- [ ] Keep the legacy route available through the approved rollback window;
  retirement remains a separate, explicit decision.
- [ ] Run a limited controlled canary with defined failure, cost, cleanup, and
  support thresholds.

### 5. Public release decision

- [ ] A named release owner reviews the completed evidence, remaining risks,
  target-VPS measurements, canary result, support plan, and economics.
- [ ] That owner records an explicit go/no-go decision for public traffic.

## Non-claims and remaining limitations

- This checklist does **not** claim production readiness, profitability, or
  sufficient capacity for a 1 GiB VPS.
- The current bounded-output limit is not a complete bound on decoded source,
  canvas, intermediate buffers, Rust response copies, aggregate admission work,
  or temporary workspace use.
- Client disconnect, deadline, and shutdown evidence do not provide native
  mid-operation cancellation.
- No database, Redis, durable queue, object store, permanent image storage, or
  job-polling model is introduced by this first-MVP path.

## Closure rule

Public exposure is permitted only after every open checkbox required for the
chosen release scope has recorded, revision-specific evidence and a named
release owner makes the explicit public-release decision. A passing local or
Docker verification gate alone does not close this checklist.
