# IMGENGINE infrastructure security, operations and economics audit — 2026-11-15

**Repository basis:** `main` at `6838ed73403235450da310f0e7fa6f8a8d6f8bf0`, after merged PR #38. **Scope:** all 18 tracked files under **active** `infra/`, their referenced Rust/Nuxt image entrypoints and GitHub CI, P14 deployment/release contracts, and a scope comparison with `imgengine-saas/infra/` legacy. **Method:** source/static documentation review only. **No** real server, Docker build, container, security scanner, production secret, penetration test, latency benchmark, Compose render or CI run was executed for this audit. Previous P14 recorded evidence has its original SHA and limits.

**Decision:** infra is **sufficient as a hardened local/production candidate** and as a starting point for an Astra infrastructure *review*. It is **not** ready to be declared a secure, economical public release until the specific blockers below are verified/closed. This audit does not authorize software/configuration changes or deployment.

Read [infra-scoped AGENTS](../infra/AGENTS.md), [infra README](../infra/README.md), [P14 baseline](P14_PRODUCTION_BASELINE.md), [P14 real-host runbook](P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md), [MVP release checklist](MVP_RELEASE_CHECKLIST.md), [technology ADR](TECHNOLOGY_AND_REPOSITORY_POLICY.md), [Astra handoff](ASTRA_ENGINEERING_START_HERE.md), [next.md](../next.md) and [change-scoped verification](CHANGE_SCOPED_VERIFICATION.md).

## 1. Current HLD and environment boundary

```text
PUBLIC INTERNET
   | DNS -> TCP 80/443, HTTPS/ACME
   v
Caddy (only published host ports, edge network)
   |
   v
Nuxt 4 server/BFF (edge + internal app network)
   | private shared Rust credential, server-only
   v
Rust/Axum API (internal app network only, /healthz /readyz)
   |
   v
bounded Rust admission -> safe FFI -> native C17 scheduler/decoder
   | private ephemeral per-request workspace
   v
JPEG result returned to browser; no job database
```

The actual image code lives in `backend/` and `imgengine/`; Nuxt is `imgengine-saas/frontend/`. **`imgengine-saas/infra/` is a separate retained legacy Python/Celery/Redis/PostgreSQL/nginx/observability topology**, not an alternate production template to copy for Rust. Do not move/delete it without data and rollback approval. The active root `infra/` contains no new Python service or database.

## 2. Complete tracked `infra/` file inventory (18 reviewed)

| File(s) | Role | Source-aligned status |
| --- | --- | --- |
| [`Caddyfile.production`](../infra/Caddyfile.production) | Public TLS/ACME domain, static health response, security headers, web upstream | **CONFIGURED; real DNS/TLS/headers/renewal not verified** |
| [`docker-compose.production.yml`](../infra/docker-compose.production.yml) | Caddy + init, Nuxt and internal Rust; image digests, secrets, non-root, read-only, cgroups | **CONFIGURED; local static policy recorded; actual host not verified** |
| [`production.env.example`](../infra/production.env.example) | Placeholder digest, host-file paths, ACME values and JPEG output ceilings | **EXAMPLE ONLY; no actual digest/secret or deployment evidence** |
| [`scripts/verify-production-compose.py`](../infra/scripts/verify-production-compose.py) | Existing Python static rendered-JSON policy checker; CI integration | **IMPLEMENTED TOOL; limited to checked static fields, not live security** |
| [`Dockerfile.p7-frontend-dev`](../infra/Dockerfile.p7-frontend-dev), [`docker-compose.p7-frontend-dev.yml`](../infra/docker-compose.p7-frontend-dev.yml), [`p7-frontend-dev.env.example`](../infra/p7-frontend-dev.env.example) | Ubuntu 24.04 dev toolchain, bind mounts and localhost-only web | **DEVELOPMENT ONLY; not hardened production image** |
| [`scripts/run-p7-nuxt.sh`](../infra/scripts/run-p7-nuxt.sh), [`scripts/run-p7-rust-api.sh`](../infra/scripts/run-p7-rust-api.sh) | Build and run typed Nuxt→Rust→native dev stack | **IMPLEMENTED DEV SCRIPTS**, Rust Cargo path updated to `backend/` |
| [`scripts/verify-p7-frontend-stack.sh`](../infra/scripts/verify-p7-frontend-stack.sh), [`scripts/verify-p7-frontend-stack.ps1`](../infra/scripts/verify-p7-frontend-stack.ps1) | Local smoke, oversized request, backend outage, client-secret check | **IMPLEMENTED DEV CHECKS; local evidence only** |
| [`scripts/verify-p7-render-contract.sh`](../infra/scripts/verify-p7-render-contract.sh), [`scripts/verify-p8-layout-contract.sh`](../infra/scripts/verify-p8-layout-contract.sh) | P5/P8 response/image/invalid-input contract smoke | **IMPLEMENTED NARROW CONTRACT CHECKS; not production capacity** |
| [`Dockerfile.p8-vps-runtime`](../infra/Dockerfile.p8-vps-runtime), [`docker-compose.p8-vps-sim.yml`](../infra/docker-compose.p8-vps-sim.yml) | Resource-constrained Docker Desktop simulation, separate image stages | **HISTORICAL SIMULATION CANDIDATE; reproducibility/provenance gaps** |
| [`scripts/run-p8-vps-sim.ps1`](../infra/scripts/run-p8-vps-sim.ps1), [`scripts/run-p8-vps-workload.sh`](../infra/scripts/run-p8-vps-workload.sh) | Health/workload/error classes, latency distribution, periodic Docker stats, workspace cleanup | **IMPLEMENTED MEASUREMENT HARNESS; not exact peak RSS or real host** |
| [`scripts/run-p12-local-candidate.ps1`](../infra/scripts/run-p12-local-candidate.ps1) | Disposable private Rust+Nuxt images, generated keys, non-root runtime, P12 smoke | **PRIOR LOCAL EVIDENCE; does not include Caddy/TLS or public-VPS approval** |

The [P14 baseline](P14_PRODUCTION_BASELINE.md) records local smoke and dependency audit at its earlier SHA; it distinguishes constrained Docker Desktop behavior from actual low-RAM VPS proof.

## 3. Observed strengths — do not confuse with release evidence

- **Layer isolation:** production Compose exposes only Caddy `80/443`; Rust is attached solely to `app: internal:true`; Nuxt connects edge and app. No PostgreSQL, Redis, Celery, legacy Python container or DB service is part of the P14 Compose.
- **Runtime policy:** Caddy, Rust and Nuxt declare UID/GID `10001`, read-only root, `cap_drop: ALL`, `no-new-privileges`, PID and memory/swap ceilings, tmpfs and healthcheck; Caddy adds `NET_BIND_SERVICE`. Root-only Caddy volume initializer has no network and a narrowed capability list. These are **configuration checks**, not proof of kernel sandbox effectiveness.
- **Secrets and product privacy:** two separate Compose `file:` secrets; backend and Nuxt entrypoints check nonempty/readable files, then export only to their server process. The app's render credential is intended to remain out of public browser runtime config. No durable image-job database is configured.
- **Supply chain intent:** production Compose uses `image` variables intended to contain `@sha256:` digests and performs no source build at deployment; examples contain placeholder values. CI calls the static production-Compose verifier.
- **Resource/cleanup evidence:** P8 scripts sample `docker stats` and verify cleanup; P12 local candidate exercises real handlers and constrained containers. These do not measure true peak memory for arbitrary codec/workload or certify live TLS.

## 4. Prioritized evidence and design-risk register

These are **source-based findings and release verification gaps**. Unless marked otherwise, they are not confirmed exploits or evidence that any public instance exists.

| ID | Priority | Finding and exact source | Required next proof/action |
| --- | --- | --- | --- |
| INFRA-01 | **P0 before public release** | `docker-compose.production.yml` uses **file-backed secrets** with long-form `uid/gid/mode` attributes. Per the [official Docker Compose services reference](https://docs.docker.com/reference/compose-file/services/), for `file:` sources Docker Compose uses bind mounts and **silently ignores** these settings. `production.env.example` and P14 runbook prescribe root:GID-10001/0440 host permissions, but the Python verifier only inspects rendered policy strings. | Verify actual **host file owner/group/mode**, restrictive parent dirs, read-only mount and live UID 10001 access via metadata-only `stat`/safe checks. Never print values or claim 0440 comes from Compose flags. Add focused fail-closed automated metadata checks in a separately approved tooling PR. |
| INFRA-02 | **P0 before public release** | `Caddyfile.production` routes public browser traffic to Nuxt, whose server sends a shared Rust API key. This is **not** client identity, per-user authorization or cost/abuse control. No edge rate-limit/auth policy or per-client quota is defined in the audited root infra files. | Threat-model abusive uploads, anonymous traffic, request bursts, service exhaustion, credentials/rotation, safe 429/503 responses and either measured edge limits or independent justified controls. Test at exact candidate with safe bounded load; avoid inventing pricing or external WAF. |
| INFRA-03 | **P0 release blocker** | Production memory ceilings: Rust 256 MiB, Nuxt 192 MiB and Caddy 96 MiB (**544 MiB sum of container ceilings**), plus host OS/Docker/cert storage requirements; these are **ceilings**, not actual peaks. Rust temporary workspace uses 96 MiB tmpfs, Nuxt and Caddy add 16 MiB each; tmpfs is not necessarily independent of container memory accounting. Current P14 docs say actual 1 GiB VPS capacity unverified. | On user-approved actual host measure peak cgroup RSS/memory events, idle overhead, encoded+decoded+intermediate allocations, CPU, cgroup OOM, tmpfs/disk/cert usage, concurrent overload, startup, cleanup and rollback. Do not promise 10–100 concurrent users. |
| INFRA-04 | **P1, P8 simulation reproducibility** | `Dockerfile.p8-vps-runtime` uses mutable local `FROM imgengine-p7-dev-rust-api:latest` and `COPY .docker/.cargo/registry` despite that path being **absent from the audited tracked Git tree**. Clean-clone build depends on separately prepared local image/cache; not a proven deterministic build. | Inventory build cache/bootstrap prerequisites and confirm a fresh isolated Docker checkout build with recorded versions/digests; if P8 simulation is retained, design explicit reproducible dependencies in a separate PR. |
| INFRA-05 | **P1, P8 image attack surface** | In `Dockerfile.p8-vps-runtime`, the runtime stage starts `FROM imgengine-p7-dev-rust-api:latest` and `frontend-runtime FROM runtime`. Thus the P8 simulation runtime **inherits dev toolchains**, contrary to its leading comment claiming no compilers/source/build caches; the Nuxt image also inherits Rust API executable/native libraries. This is **not** the separate P14 production deployment Dockerfile. | Correct misleading docs and, if still used, build minimal runtime images independently. Prove actual SBOM/image size/runtime paths; don't promote P8 simulation images to production. |
| INFRA-06 | **P1 security verification gap** | `scripts/verify-production-compose.py` is an existing static rendered JSON checker. It verifies several guardrails, but does **not** inspect actual file permissions, Caddyfile TLS config, image provenance, runtime mounts, or effective capabilities. `caddy` `cap_add` check tests set inclusion, not **exact** allowed capabilities; extra service privileges such as `privileged`, `pid: host`, `devices` are not forbidden by this checker. | Add focused negative-policy tests and exact allowed capability/privilege/network/mount checks in a separate Rust or tightly constrained shell replacement PR; preserve current CI gate, and supplement with live `docker inspect` and actual-host evidence. |
| INFRA-07 | **P1 secrets lifecycle** | The checked-in `backend/docker-entrypoint.sh` and `imgengine-saas/frontend/docker-entrypoint.sh` read secret files then export `IMGENGINE_API_KEYS` and `NUXT_IMGENGINE_RENDER_API_KEY` to **server process environment**, respectively. The client is not intended to receive these, but file-backed launch does not mean credentials never appear in process environment. | Audit env/process/debug/log exposure and server-only Nuxt runtime config; verify redaction, secret rotation and container restart on actual deployment. Consider direct secret-file reading only with backward-compatible Rust/Nuxt tests. |
| INFRA-08 | **P1 availability/observability** | `Caddyfile.production` `/_internal/healthz` returns static `ok`; it proves Caddy routing, **not** Nuxt or Rust readiness. Production Compose separately checks Nuxt and Rust `/readyz`, but no active production metrics collector, alert destination or on-call evidence is in the audited root `infra/`. | Test end-to-end HTTPS → Nuxt proxy → authenticated Rust operation, Caddy/ACME renewal, 503/429 recovery, logs/trace privacy, low-overhead metrics/alerts and real owner. Avoid copying legacy ELK/Jaeger into the new stack by default. |
| INFRA-09 | **P1 supply chain** | Production Compose expects immutable image digests, but checked-in `production.env.example` uses placeholders. P7 dev Dockerfile fetches NodeSource script via `curl | bash`; base images use mutable Ubuntu/Node tags in current build definitions. No complete digest/signature/SBOM/reproducible build provenance is proved by the file list. | Publish reviewed release-specific digests, SBOM/dependency license provenance, vulnerability disposition and reproducible build evidence. Do not use dev or P8 images as production candidates just because they build. |
| INFRA-10 | **P1 operational completion** | P14 real-host runbook exists; no recorded actual-VPS TLS, firewall, disk-pressure, disaster recovery, shutdown/canary, Caddy volume restore or alerting closure in audited infra sources. | Follow `P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md` on an approved host with deployment approval. Record renewal/backup/recovery/roll-back exercises, host baseline and exact SHA/digests; require human go/no-go. |
| INFRA-11 | **P2 policy and UX** | Caddy HSTS includes `includeSubDomains`; no actual domain/subdomain inventory is in the repository. There are security headers, but no proof for real production responses, CSP requirements, proxy error leakage or mobile/browser reachability. | Confirm intended domain/subdomains all use HTTPS before enabling HSTS for the real zone; test headers/TLS/redirects/cookie/auth behavior on the actual domain. |
| INFRA-12 | **P2 architecture/tooling consistency** | `verify-production-compose.py` is a **retained existing Python verifier**, while the approved *new-work* policy prohibits new Python tooling. The production deployment itself has no Python service; legacy `imgengine-saas/infra/` is separate. | Do not delete existing verifier or required CI. If standardizing all checks on Rust/shell, maintain parity (including negative tests) in a separate explicit task. |
| INFRA-13 | **P2 business/capacity economics** | There is no measured fixed/variable hosting cost, bandwidth/egress, per-successful-image CPU/RSS, abuse/retry cost, actual free-tier restrictions or customer-count capacity evidence in `infra/`. | Build CA/MBA unit-cost and stop-loss decision record after host measurements: cost/request, idle hosting bill, cost/GB and monthly ceiling, support burden, concurrency and conversion thresholds. Distinguish revenue hypotheses from facts. |

**Important interpretation:** The confirmed Docker Compose *semantics* for file-backed secrets and P8 runtime Dockerfile inheritance are evidence-backed configuration facts; whether they are exploitable on an actual host requires independent testing. Absence of a control **within audited root infra** does not prove an external provider/CDN has no such control.

## 5. Verification, performance and security boundaries

| Evidence type | Known | Unknown/not yet demonstrated |
| --- | --- | --- |
| **Static policy** | `ci.yml` renders production Compose and runs existing `verify-production-compose.py`; some fields checked | Actual host file permissions, Caddy TLS, container runtime effective privileges, adversarial mutations |
| **P7 dev** | localhost-only 3000, local key, P5/P8 smoke, backend-outage mapping | Prod server hardening, public identity, external network, real capacity |
| **P8 simulation** | explicit cgroup limits/workload/latency distributions and 500 ms periodic `docker stats` | Clean-checkout image build from tracked files, actual memory peaks, physical 1 GiB host |
| **P12 local candidate** | generated disposable key, private containers, Rust+Nuxt smoke, bounded resource test | Caddy/TLS, DNS, host firewall, real public secret lifecycle, alerting and rollback |
| **P14 production** | hardened proposed Compose with non-root, separate networks and digest placeholders | Real host/DNS/TLS/resources/canary/release; no claim of 10 or 100 users |

For an **infrastructure-only** implementation PR, run exact Compose/entrypoint/Caddy/static-policy tests and affected Docker smoke; for changed Rust/native linking or launch behavior include relevant Rust/C ABI gates. Required GitHub checks remain mandatory. A **docs-only** PR needs Markdown links/status/diff checks and no manual C rebuild/ASan/UBSan/fuzz.

## 6. Principal-engineer and CA/MBA decision framework

- **HLD:** single-host edge vs application trust boundary, no premature DB/queue/microservices, separate legacy infra and approved Rust/C17 product.
- **LLD:** explicit ports/networks/capabilities, secret ownership/readability, entrypoint file→env process handoff, read-only filesystem, tmpfs/cleanup/kill signals, resource/cgroup enforcement, readiness and failure handling.
- **SOLID/DRY/design patterns:** Compose files describe configuration and dependencies, not a formal proof of SOLID/DDD/hexagonal correctness. Prefer one documented security policy source with targeted tests; avoid duplicating configuration logic that silently drifts. Use dependency injection/interfaces where they solve actual Rust/Nuxt test seams; avoid event-bus/observability platform without a measurable need.
- **Security review:** map exposed services, hostile uploads, capacity exhaustion, dependency provenance, log/secret disclosure, persistent Caddy storage, in-container effective UID/caps and backup/rollback operations. Rate-limit decisions need measured public abuse profiles.
- **Performance:** full image processing commonly includes decode, allocation, placement and encode; nanosecond/microsecond optimization of small C operations is a distinct benchmark from HTTP and VPS latency. Track P50/P95/P99, CPU-ms/success, bytes/image, RSS peak, OOM and failed request cost.
- **CA/MBA economics:** establish fixed cost, variable bandwidth/CPU/disk costs, server capacity at actual workload, free-tier constraints, sensitivity to abusive/failed traffic and owner labor before paid feature forecasts. Document assumptions and profit margin separately.

## 7. Astra infrastructure handoff (review-only first)

**Track D — Infra/P14 evidence review:** Read `infra/AGENTS.md`, this audit, `infra/docker-compose.production.yml`, `infra/Caddyfile.production`, `infra/scripts/verify-production-compose.py`, the relevant Dockerfiles and [P14 runbook](P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md). Start with **INFRA-01 / INFRA-06**: propose a non-secret host-permission check and stronger policy negative cases; separately review P8 simulator provenance and the production abuse/resource model. Deliver a source-backed **HLD/LLD threat table + acceptance tests + small PR plan**. Do not edit live configurations, secrets, DNS, CI or production instances without authorization. All follow-up risk fixes need their own scoped implementation PR and required CI; document what remains unverified.

### Definition of done for an infra finding

An item is not **CLOSED** until the actual changed source/configuration, test command, SHA, expected/observed outcomes, reviewer and residual risk are documented. **P14 public readiness** remains BLOCKED until actual approved-host evidence and human decision are complete.
