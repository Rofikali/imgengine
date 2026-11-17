# AGENTS.md — IMGENGINE active infrastructure

> **INFRA ENVIRONMENT PARENTS (2026-11-17):** Use [milestone protocol](../docs/IMGENGINE_PRODUCT_INFRA_MILESTONE_AND_CODEX_USAGE_PROTOCOL.md) and [live queue](../next.md) to track `INFRA-P7` development, `INFRA-P8` VPS simulation, `INFRA-P12` private candidate, `INFRA-P14` production readiness **independently**. After each actual environment test record tested SHA/host, command results, cgroup/cleanup as applicable and `PASS/FAIL/NOT_RUN`. Do not mark `VERIFIED` on configuration alone, infer a real 1 GiB VPS from Docker cgroup caps, or equate a development smoke PASS to public production. P8 PowerShell harness is still source-broken at current main; dedicated syntax repair and real Docker test are prerequisites. Continue to avoid public services, credentials, spending or deployment without explicit approval.

> **INFRA-P8-01 verified-source blocker (2026-11-17):** The committed `scripts/run-p8-vps-sim.ps1` at `main` `1523d82f954` has an unterminated OOM `-match` expression and duplicated `finally` near lines 127–148. This is a **static source finding; no PowerShell/Docker execution occurred**. Do not tell the user to run `-Build` before a focused repaired-script PR and parser test; preserve all P8/P12/P13 and OOM checks. [Experiment/AI-cost audit](../docs/INFRA_EXPERIMENTS_AND_AI_CONTEXT_COST_AUDIT_2026-11-17.md) has the least-expensive safe verification order. No production or CI edits from this note.

> **P8-SIM-02:** [Sheet/crop and cgroup evidence runbook](../docs/P8_SIM_02_SHEET_CROP_RESOURCE_EVIDENCE.md) adds a bounded Nuxt→Rust→native P12/P13 fixture suite to the P8 simulation, preserves original P8 workload, and records cgroup peak/events when available. Full Docker execution at exact SHA is required before claiming success. M1.3 pixel parity, native verification and P14 actual host release are separate gates. No automatic deployment, production-secret use or merge.

**Scope:** `infra/` and all child files. Follow [root AGENTS](../AGENTS.md), the [approved Rust/C17/Nuxt stack](../docs/TECHNOLOGY_AND_REPOSITORY_POLICY.md), [source-backed infra audit](../docs/INFRA_SECURITY_OPERATIONS_AUDIT_2026-11-15.md), [P14 release checklist](../docs/MVP_RELEASE_CHECKLIST.md), [actual-VPS runbook](../docs/P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md), [Astra start guide](../docs/ASTRA_ENGINEERING_START_HERE.md) and [next.md](../next.md). These are task rules, **not** permission to deploy.

## 1. Scope, ownership and environments

- **Current product:** C17 native engine in `imgengine/`; active Rust/Axum API + safe FFI in `backend/`; Nuxt/Vue/TypeScript BFF in `imgengine-saas/frontend/`.
- **This `infra/` directory is the active P7 dev, P8 simulation, and P14 single-host production candidate.** The separate `imgengine-saas/infra/` directory belongs to the **retained legacy** Python/FastAPI/Celery/Redis/PostgreSQL stack. Never accidentally enable its DB/queue/ELK/Jaeger profiles in the Rust MVP.
- Production `docker-compose.production.yml` and `Caddyfile.production` are **configurations**, not evidence of a deployed, secure or profitable production service. P8 simulation is not actual 1 GiB VPS proof. Development-only secrets and debug/build tools must never become production settings.
- **No new Python scripts/tooling, FastAPI, Celery, Redis or PostgreSQL** for new product work. The existing `infra/scripts/verify-production-compose.py` is a retained verification tool; do not delete, weaken or bypass it before a Rust or narrowly constrained shell replacement passes parity and required CI.

## 2. Security boundaries — required review

1. Preserve **only Caddy on host 80/443**; never host-publish Nuxt or Rust API. Keep the Rust API on the internal Compose `app` network; verify actual host firewall, public DNS, TLS renewal, and Caddy upstream behavior.
2. Keep immutable production image digests; non-root UID/GID 10001; read-only filesystem; minimal required capabilities; no-new-privileges; bounded PIDs, memory/swap, CPU and tmpfs; private secret mounts; sanitized logging. Do not treat a static config check as runtime enforcement.
3. **Compose file-backed secret caveat:** Docker Compose ignores long-syntax `uid`, `gid` and `mode` for **file-sourced secrets** because it uses bind mounts. Treat host file ownership/mode and live in-container readability as **release-critical**; see [official Docker Compose services reference](https://docs.docker.com/reference/compose-file/services/). Use documented root:GID-10001, 0440 files and restrictive parent directories, then verify with metadata-only `stat` and non-secret access tests; never print credential contents.
4. The API and Nuxt entrypoint scripts load file-backed keys and export them to their respective **server process environments**. They are not exposed to browser config by design, but “file-backed” does **not** mean no runtime environment variable. Review process/log/debug exposure and rotations at the release boundary.
5. Separate health checks: Caddy's `/_internal/healthz` serves a static response and **does not check Nuxt/Rust readiness**. Rust `/readyz`, frontend readiness, authenticated smoke, public TLS and rollback all require separate verification and appropriate alerting.
6. Rate/abuse protection, client quotas, upload/output limits, memory envelope, aggregate concurrency and public request economics must be threat-modelled **before** public traffic. Rust's shared server-to-server key is **not** end-user identity/tenancy.
7. Never embed production secrets into `.env` files, logs, build args, Docker layers, PR comments, outputs or model context. Keep production certs and Caddy named volumes out of throwaway cleanup scripts.

## 3. Reproducibility and source of truth

- P7 dev: `docker-compose.p7-frontend-dev.yml`, `Dockerfile.p7-frontend-dev`; bind-mounted source and mutable developer tools are **not deployable**.
- P8 simulation: `docker-compose.p8-vps-sim.yml`, maintained `../backend/Dockerfile` and `../imgengine-saas/frontend/Dockerfile`, standalone `Dockerfile.p8-vps-workload`, and `run-p8-vps-sim.ps1`. P8-SIM-01 proposes replacing previous P7-development-image inheritance with independent runtime stages. The old `Dockerfile.p8-vps-runtime` is deprecated and historical; consult [P8-SIM-01 runbook](../docs/P8_SIM_01_RUNTIME_REPRODUCIBILITY.md). **Do not assert clean-checkout reproducibility, minimal deployed dependencies, or 1 GiB host fitness until fresh Docker build, inspect and workload evidence exists.**
- P14 production: `docker-compose.production.yml`, `production.env.example`, `Caddyfile.production`, `verify-production-compose.py`; **no build** or database/queue services in the deployment Compose. Verify rendered config plus actual image manifests, secret permissions, network and runtime evidence before deployment.
- Keep versioned `infra/` docs close to source changes; [infra README](README.md) and [audit](../docs/INFRA_SECURITY_OPERATIONS_AUDIT_2026-11-15.md) distinguish implemented configuration, historical recorded checks, partial, planned and blocked.

## 4. Verification by change scope

| Scope | Focused developer-loop verification | Required integration/release evidence |
| --- | --- | --- |
| Documentation only | Source-linked accuracy, scope and relative Markdown links; **no native build** | Existing required CI remains unchanged |
| P7 dev scripts/Compose | Rendered config, localhost-only exposure, toolchain and P5/P8/P12 smoke on the affected environment | Affected CI/tests; do not turn P7 into production |
| P8 simulation | Fresh isolated Docker build prerequisites, CLI/workload, actual cgroup memory/CPU/output, cleanup | Original P8 evidence **does not** qualify as real-host proof |
| P14 Compose/Caddy/secret policy | Compare rendered config with versioned policy; focused negative tests, secrets metadata, image/runtime compatibility | Exact candidate digest/SBOM, real TLS/DNS/firewall, actual host capacity, alerting, recovery, rollback and independent go/no-go |
| Rust/C ABI/runtime image boundary | Rust API/FFI and relevant native gate, secure Docker build and runtime smoke | Required CI and release checklists; no skipped security gates |

**Keep a command/evidence ledger:** SHA and image digest; host/OS/CPU/memory; command/exit status; observation; what was not tested; threats/risk owner; next decision. Do not paste secrets, raw user image data, huge logs, or uncertain benchmarks.

## 5. Principal engineering, SRE and CA/MBA rules

- Architecture: preserve Caddy (edge) → Nuxt BFF → private Rust API → C17 ABI; prefer minimal single-host deployment. No unnecessary DDD framework, microservice, queue, load balancer, metrics cluster or Kubernetes without measured need.
- Reliability: establish objective readiness, backup of required **Caddy certificate data and configuration**, certificate renewal, alert destination, on-call ownership, downtime expectations, recovery time/objectives and reversible rollout.
- Capacity: cgroup limits are **ceilings**, not guaranteed workload capacity. Measure Caddy/Nuxt/Rust aggregate RSS including decoded images and intermediate buffers, tmpfs, host OS, Docker daemon, network, swap and concurrent overload; do not promise a 1 GiB VPS supports 10–100 active users.
- Business: report fixed cost/host, idle resource overhead, CPU/RAM per successful image, bandwidth, cloud free-tier conditions, failure/retry/abuse cost, expected support and payment/compliance overhead. Use real tariffs and evidence, not invented profitability.
- Git/governance: read the current branch, preserve dirty worktrees, make one small task-scoped PR, link current `next.md` IDs; **no merge, deploy, DNS change, production-secret rotation or legacy retirement without explicit approval**. Raise blockers rather than silently weakening security controls.
