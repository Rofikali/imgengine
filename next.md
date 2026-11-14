# IMGENGINE — NEXT: Active execution queue

**Last reconciled:** 2026-11-14. **Base audited:** `main` at `14d1a8683f492ec2c1128a2014442cc85cd832e9`. This file is the *next-action dashboard*, not the historical source of truth.

- Full inventory and decisions: [docs/PROJECT_STATUS_AND_DECISION_REGISTER.md](docs/PROJECT_STATUS_AND_DECISION_REGISTER.md)
- Long-term image-only product strategy: [docs/PRODUCT_ROADMAP_BROWSER_FIRST.md](docs/PRODUCT_ROADMAP_BROWSER_FIRST.md)
- Test/time/token budget policy: [docs/CHANGE_SCOPED_VERIFICATION.md](docs/CHANGE_SCOPED_VERIFICATION.md)
- Release blocker evidence: [docs/P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md](docs/P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md)

## Current priority — do one bounded milestone at a time

| Order | Next action | State | Completion evidence |
| --- | --- | --- | --- |
| 0 | Review documentation PR #30 and reconcile status against repository and local worktrees | Draft review | Accurate inventory, no accidental changes to local dirty worktrees |
| 1 | **M1.1** P13 normalized crop transport: ABI 2.3 -> safe Rust FFI -> Axum -> typed Nuxt proxy | Not started in this queue; native ABI already exists | Focused FFI/API/proxy tests, P12 ABI 2.2 behavior unchanged |
| 2 | **M1.2** accessible interactive crop UI and preview | Planned | Keyboard/drag UX, object-URL cleanup, component/browser tests |
| 3 | **M1.3** end-to-end crop parity and release candidate | Planned | Exact revision evidence; no unreviewed passport-compliance claims |
| 4 | **M2** Astra-led Worker/Rust-Wasm architecture and feasibility slice | Planned, **after M1** | ADR, browser memory/latency/bundle/fidelity/security measurements; explicit go/no-go |
| 5 | **M3** browser-only crop, resize, rotate and image conversion | Planned | No-upload tests, device compatibility, bounded resource behavior |
| 6 | **M4–M5** bounded target-KB compression, basic browser print sheets and versioned presets | Planned | Impossible-target errors, DPI/physical-size and export validation |
| 7 | **M6–M9** launch, local batch/presets, professional tools and paid API when justified | Planned | User demand, security, business and release gates |

**Parallel blocker:** P14 actual-VPS verification is still required for any public server-backed launch. Do not confuse browser-Wasm readiness with server operational readiness.

## Execution contract for Astra, Terra and other agents

1. Before any milestone, inspect the current branch, merge base, diff, contracts and this queue. Never assume a historical note is still true.
2. Write a narrow contract and acceptance tests; implement **only one milestone** in a feature branch. Update status docs in the same PR.
3. **Frontend-only or isolated Rust/Wasm milestone:** run focused frontend/Worker/Wasm tests, typecheck and relevant browser checks. **Do not manually rebuild C, run CMake, ASan/UBSan, native fuzz, or the full native verifier** if C/ABI/shared code was not changed. Existing required GitHub CI still applies; do not bypass it.
4. **Native C, public headers, FFI, codec, scheduler or shared-boundary changes:** run impacted native/ABI/FFI and security gates, escalating to full matrix as required.
5. **Release or security-sensitive milestone:** run exact-revision required release gates; never claim a pass based only on old CI evidence.
6. Capture full logs outside agent context; report only command, revision, result, and relevant failure lines. Prefer caches and incremental builds. Avoid repeatedly printing successful logs.
7. Preserve local dirty worktrees and legacy code. Never delete or migrate without parity/rollback decision.
8. After verification, prepare a PR for independent review. Do not merge, deploy, expose public endpoints or retire legacy without an explicit go/no-go.

## Carry-forward from the original `next.md` (historical backlog, not verified current tasks)

The original text below is retained verbatim to avoid losing intent. Its Redis/Postgres/worker recommendations apply to the **legacy** stack unless explicitly adopted by a new Rust/Axum contract. The existence of an item here does not prove it is still open or that its proposed solution remains appropriate. Reconcile each item against current code and evidence before scheduling.

---

replace removed prints or prints Debug with observability logger
add sanitizer ci job for native-engine

Next Principal-Level Moves
Stabilize local platform
Add a make verify / PowerShell script: build, migrations, health checks, JPEG/PNG/CMYK smoke jobs, output validation.
Make this the required gate before every change.

Production observability
Keep JSON stdout as primary; use Fluent Bit/OpenTelemetry → Loki or ELK.
Alert on API 5xx, worker failures, queue backlog, job latency, and storage errors.

Reliability contract
Define SLOs: e.g. 99.9% API availability, p95 submit latency, p95 completion time.
Add dashboards and runbooks for Redis, Postgres, worker, storage, and native-engine failures.

Native-engine quality
Add regression fixtures for JPEG baseline/progressive/CMYK/YCCK and PNG.
Run them in Docker CI with sanitizers and fuzzing for decoder boundaries.

Security and tenancy
Keep key ownership/idempotency; add API-key rotation, per-tenant quotas, upload content sniffing, malware scanning policy, and audit retention.

Delivery discipline
Use PRD → contract → tests → implementation → deployment evidence for every loop.
Avoid new features until the verification pipeline and incident runbooks are repeatable.
