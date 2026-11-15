# IMGENGINE active infrastructure

**Read first:** [infra AGENTS](AGENTS.md), [dated source/security/operations audit](../docs/INFRA_SECURITY_OPERATIONS_AUDIT_2026-11-15.md), [Astra handoff](../docs/ASTRA_ENGINEERING_START_HERE.md), [P14 release checklist](../docs/MVP_RELEASE_CHECKLIST.md), and [actual-VPS runbook](../docs/P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md). **Status: local/release-candidate infrastructure exists, but public deployment is not approved or verified.**

## Select the right environment

| Purpose | Files | Do not infer |
| --- | --- | --- |
| **P7 local developer integration** | `Dockerfile.p7-frontend-dev`, `docker-compose.p7-frontend-dev.yml`, `p7-frontend-dev.env.example`, `scripts/run-p7-*.sh` and `scripts/verify-p7-frontend-stack.*` | Not production image, public TLS or hardening evidence |
| **P8 Docker Desktop VPS simulation** | `Dockerfile.p8-vps-runtime`, `docker-compose.p8-vps-sim.yml`, `scripts/run-p8-vps-sim.ps1`, `scripts/run-p8-vps-workload.sh` | Not a real 1 GiB host, precise RSS peak, or clean-clone build guarantee |
| **P12 disposable candidate rehearsal** | `scripts/run-p12-local-candidate.ps1` | No Caddy/TLS, live host, public secrets or go/no-go |
| **P14 single-host production *configuration*** | `docker-compose.production.yml`, `Caddyfile.production`, `production.env.example`, existing `scripts/verify-production-compose.py` | No live TLS, public monitoring, actual VPS capacity or release approval |

The **active API** is Rust/Axum under `../backend/`, native C17 is `../imgengine/`, and frontend/Nuxt is `../imgengine-saas/frontend/`. The separate `../imgengine-saas/infra/` contains **retained legacy Python/worker/database and observability** configurations. Never combine the two Compose generations or add a Python/FastAPI/Celery/Redis/PostgreSQL service for the target product.

## Before approving any infrastructure change

1. Identify whether the work touches *development*, *simulation*, *disposable candidate*, or the *real P14 public deployment*. Use the right tests for that exact environment and no blanket system rebuilds.
2. Preserve sole Caddy exposure on host 80/443, private Rust network, non-root IDs, read-only/capabilities, bounded limits, immutable production image digests and private secret paths.
3. File-based Compose secrets use host-file bind mounts. **Actual host ownership/mode and in-container readability must be verified**; the Compose long-syntax `uid/gid/mode` fields are ignored for file-backed secrets (see [Docker services documentation](https://docs.docker.com/reference/compose-file/services/)).
4. Local scripts and static verifiers cannot approve public host DNS/TLS, cert renewal, firewall, capacity/oom, alerting, overload, backup, rollback, provenance or a profitable operating model.
5. Do not delete the existing Python-based Compose static verifier until a user-approved Rust/shell replacement is proven to have equivalent negative coverage and CI. **No new Python tooling** for target work.
6. For any real P14 run obtain explicit user authorization for candidate, host, secrets and irreversible external changes; retain exact digest/SHA, safe output, owner and rollback evidence. Never log or disclose keys or user images.

**First Astra infra task:** read [INFRA-01 and INFRA-06](../docs/INFRA_SECURITY_OPERATIONS_AUDIT_2026-11-15.md) and propose a metadata-only secret-mount/permissions verifier and improved static policy negative tests. That is an engineering design/review task, **not permission to make a production change**. Update the [next-action board](../next.md) only with source- and exact-revision evidence.
