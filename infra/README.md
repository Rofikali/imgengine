# IMGENGINE active infrastructure

> **Independent environment milestone states (2026-11-17):** [Live board](../next.md) tracks **INFRA-P7** (local Nuxt/Rust/C17 development; configured, current smoke not run), **INFRA-P8** (Docker Desktop VPS simulation; **BLOCKED** by malformed PowerShell OOM conditional and duplicate `finally` near lines 127–148, so do not run until parser-tested), **INFRA-P12** (disposable production-shaped local candidate; configured, current run not verified), and **INFRA-P14** (hardened production Compose/Caddy *configuration*, real-host TLS/secret/RSS/abuse/rollback unverified). Each track advances **only after its own actual host/commit/test evidence**. [Parent/child milestone and experiment protocol](../docs/IMGENGINE_PRODUCT_INFRA_MILESTONE_AND_CODEX_USAGE_PROTOCOL.md) defines `CONFIGURED → PARSER_PASS/TESTED → VERIFIED` as evidence-specific statuses, not general releases. Completing P7/P8/P12 cannot approve P14 or Free/Studio/API release.

> **2026-11-17 experiment and cost audit:** [Safe local P7/P8/P12/P14 experiment matrix and agent/CI-credit investigation](../docs/INFRA_EXPERIMENTS_AND_AI_CONTEXT_COST_AUDIT_2026-11-17.md). **P8-SIM HARNESS BLOCKED:** `scripts/run-p8-vps-sim.ps1` has a malformed OOM condition and duplicated `finally` at main SHA `1523d82f954` (lines 127–148). Do not execute P8 or claim runtime results until a separately reviewed INFRA-P8-01 script repair passes PowerShell parser and real local Docker tests. P7 dev and P12 disposable rehearsal remain candidates for local-only experimentation with synthetic credentials; P14 production configuration is static, not live-host evidence.

**Read first:** [infra AGENTS](AGENTS.md), [dated source/security/operations audit](../docs/INFRA_SECURITY_OPERATIONS_AUDIT_2026-11-15.md), [Astra handoff](../docs/ASTRA_ENGINEERING_START_HERE.md), [P14 release checklist](../docs/MVP_RELEASE_CHECKLIST.md), and [actual-VPS runbook](../docs/P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md). **Status: local/release-candidate infrastructure exists, but public deployment is not approved or verified.**

## Select the right environment

| Purpose | Files | Do not infer |
| --- | --- | --- |
| **P7 local developer integration** | `Dockerfile.p7-frontend-dev`, `docker-compose.p7-frontend-dev.yml`, `p7-frontend-dev.env.example`, `scripts/run-p7-*.sh` and `scripts/verify-p7-frontend-stack.*` | Not production image, public TLS or hardening evidence |
| **Free frontend-only local development** | `Dockerfile.free-frontend-dev`, `docker-compose.free-frontend-dev.yml` | One Ubuntu 24.04 Node/Corepack/pnpm service with Linux dependency/cache volumes; no Rust/C/database/browser engine, port, credential, or integrated-runtime evidence. Browser tests still require the pinned Playwright image. |
| **P8 Docker Desktop VPS simulation** | `docker-compose.p8-vps-sim.yml`, `backend/Dockerfile`, `imgengine-saas/frontend/Dockerfile`, `Dockerfile.p8-vps-workload`, `scripts/run-p8-vps-sim.ps1`, `scripts/run-p8-vps-workload.sh` | Not a real 1 GiB host, precise RSS peak, or public release; clean-checkout runtime builds need live verification |
| **P12 disposable candidate rehearsal** | `scripts/run-p12-local-candidate.ps1` | No Caddy/TLS, live host, public secrets or go/no-go |
| **P14 single-host production *configuration*** | `docker-compose.production.yml`, `Caddyfile.production`, `production.env.example`, existing `scripts/verify-production-compose.py` | No live TLS, public monitoring, actual VPS capacity or release approval |

The **active API** is Rust/Axum under `../backend/`, native C17 is `../imgengine/`, and frontend/Nuxt is `../imgengine-saas/frontend/`. The separate `../imgengine-saas/infra/` contains **retained legacy Python/worker/database and observability** configurations. Never combine the two Compose generations or add a Python/FastAPI/Celery/Redis/PostgreSQL service for the target product.

**P8-SIM-01 candidate:** [runtime/build reproducibility and acceptance gates](../docs/P8_SIM_01_RUNTIME_REPRODUCIBILITY.md) explain the new independent Rust/Nuxt/workload images and exact tests. The former `Dockerfile.p8-vps-runtime` remains **deprecated/historical**, not part of active P8 Compose. Runtime images no longer inherit the P7 development toolchain by design; fresh Docker builds and runtime behavior still require actual evidence.

**P8-SIM-02 candidate:** [P12/P13 sheet/crop smoke and measured cgroup evidence](../docs/P8_SIM_02_SHEET_CROP_RESOURCE_EVIDENCE.md) runs through the same P8 Docker workload image. It does **not** replace M1.3 browser/native pixel parity or P14 real-host validation. Runtime outcome is **NOT RUN** until the full `run-p8-vps-sim.ps1 -Build` harness completes at the exact PR SHA.

## I01-I12 infrastructure verification matrix

This is a source-to-evidence map, not a record of passed environments. Record each exact SHA, Docker host and command in `next.md`; P7/P8/P12/P14 do not advance one another.

| Gate | Existing executable evidence | Current gap / required evidence |
| --- | --- | --- |
| **I01 Source and parser integrity** | `scripts/test-run-p8-vps-sim-parser.ps1` parses the P8 harness and probes its OOM predicate; Compose `config --quiet` is available. | Run parser on the exact candidate; no Docker workload implied. |
| **I02 Image/reproducibility** | P7/P8 Compose names Dockerfiles; P8 workload has a standalone Dockerfile. | Fresh clean-context image builds/digests remain unverified. |
| **I03 Compose/edge policy** | `scripts/verify-production-compose.py` checks rendered services, images, ports, networks and hardening declarations. | Static only; preserve this retained Python verifier until a proven replacement exists. |
| **I04 Secrets and trust boundaries** | P7 verifier checks the test key is absent from Nuxt client output; P12 creates a disposable file-backed secret volume. | Actual P14 host ownership/mode and runtime exposure remain unverified. |
| **I05 Isolation and resource ceilings** | Compose declares non-root, read-only roots, dropped caps, PIDs, CPU/memory/swap and tmpfs limits. | Docker/host enforcement and aggregate memory capacity need measured runtime evidence. |
| **I06 P7 functional integration** | `verify-p7-frontend-stack.*`, render and layout contracts cover JPEG/PNG, malformed/oversized input, 413/503 and private Rust auth. | Run on an isolated Docker host with a synthetic key at the candidate SHA. |
| **I07 P8 workload/cleanup** | P8 workload covers FIT, P12/P13, negative cases, bounded burst, workspace cleanup and periodic stats. | Run only after I01; capture workload JSON, `memory.peak`, `memory.events`, OOM state and cleanup. |
| **I08 P12 candidate rehearsal** | `run-p12-local-candidate.ps1` creates disposable images/network/secret volume and runs API/Nuxt smoke. | No current exact-SHA execution; it is not Caddy/TLS evidence. |
| **I09 P14 edge/TLS/network** | Caddyfile and rendered Compose describe sole 80/443 exposure and private Rust network. | Real DNS, TLS, renewal, firewall and upstream readiness require an approved host. |
| **I10 Observability and recovery** | Health checks, P8 report fields and cleanup paths exist. | Alerts, backup, restart/drain, rollback and retention are not real-host validated. |
| **I11 Security/adversarial behavior** | P7/P8 negative inputs, output limits, sandbox declarations and production static policy checks exist. | Abuse/rate controls, host secret metadata and actual kernel/network enforcement remain open. |
| **I12 Exact-SHA CI and release governance** | Existing seven CI jobs remain required; P8 parser CI is additive. | Exact candidate CI, review and any real-host approval are required; no merge/deploy follows from local checks. |

## Before approving any infrastructure change

1. Identify whether the work touches *development*, *simulation*, *disposable candidate*, or the *real P14 public deployment*. Use the right tests for that exact environment and no blanket system rebuilds.
2. Preserve sole Caddy exposure on host 80/443, private Rust network, non-root IDs, read-only/capabilities, bounded limits, immutable production image digests and private secret paths.
3. File-based Compose secrets use host-file bind mounts. **Actual host ownership/mode and in-container readability must be verified**; the Compose long-syntax `uid/gid/mode` fields are ignored for file-backed secrets (see [Docker services documentation](https://docs.docker.com/reference/compose-file/services/)).
4. Local scripts and static verifiers cannot approve public host DNS/TLS, cert renewal, firewall, capacity/oom, alerting, overload, backup, rollback, provenance or a profitable operating model.
5. Do not delete the existing Python-based Compose static verifier until a user-approved Rust/shell replacement is proven to have equivalent negative coverage and CI. **No new Python tooling** for target work.
6. For any real P14 run obtain explicit user authorization for candidate, host, secrets and irreversible external changes; retain exact digest/SHA, safe output, owner and rollback evidence. Never log or disclose keys or user images.

**First Astra infra task:** read [INFRA-01 and INFRA-06](../docs/INFRA_SECURITY_OPERATIONS_AUDIT_2026-11-15.md) and propose a metadata-only secret-mount/permissions verifier and improved static policy negative tests. That is an engineering design/review task, **not permission to make a production change**. Update the [next-action board](../next.md) only with source- and exact-revision evidence.
