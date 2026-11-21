# IMGENGINE Pro — reuse existing infra/ for C17 + Rust native-first work

**State:** PRO-00 documentation under draft PR #95, NOT_MERGED. Product code and Docker runtime tests remain NOT_RUN for Pro. Owner selected C17/Rust verification and architecture first; **no Pro frontend implementation yet**.

## A. Reconcile the existing infrastructure before adding anything

Read the active infrastructure source of truth: [infra/README.md](../../infra/README.md), [infra/AGENTS.md](../../infra/AGENTS.md), the [Docker-only development policy](../../docs/IMGENGINE_DOCKER_ONLY_DEVELOPMENT_POLICY.md) and [technology policy](../../docs/TECHNOLOGY_AND_REPOSITORY_POLICY.md). This document **reuses** those configurations. It introduces no new Dockerfile, Compose file, workflow, image or new language/toolchain.

| Existing environment or file | Actual purpose | Pro native-first usage |
| --- | --- | --- |
| [infra/Dockerfile.p7-frontend-dev](../../infra/Dockerfile.p7-frontend-dev) | Ubuntu 24.04 image with CMake/Ninja, C compiler, Rust/Cargo and Node | **Use this image**, even though its name mentions frontend |
| [infra/docker-compose.p7-frontend-dev.yml](../../infra/docker-compose.p7-frontend-dev.yml) | Separate rust-api and frontend development services, shared source, Cargo and native build volumes | **Use only rust-api with run --no-deps and an overriding shell command** |
| [infra/scripts/run-p7-rust-api.sh](../../infra/scripts/run-p7-rust-api.sh) | Builds native C then starts Rust HTTP API | **Do not run it** for tests; override the service command so no API server starts |
| [infra/p7-frontend-dev.env.example](../../infra/p7-frontend-dev.env.example) | Local synthetic API key example | Make a private ignored infra/p7-frontend-dev.env when needed; never commit real credentials |
| [.devcontainer/devcontainer.json](../../.devcontainer/devcontainer.json) | P7 VS Code environment, defaults to frontend service | Not needed for this native-only invocation; no frontend started |
| [imgengine/Dockerfile.verify](../../imgengine/Dockerfile.verify) | Existing Ubuntu native verifier with additional sanitizer/fuzz dependencies | Preserve for comprehensive verification; no new Pro duplicate |
| [Makefile](../../Makefile) | Focused native and Rust command menu | Run these targets **inside** the P7 rust-api development container |
| [imgengine/scripts/verify_linux.sh](../../imgengine/scripts/verify_linux.sh) | Full C11/C17/sanitizer/fuzz/FFI integration matrix | Separate expensive verification; do not mislabel as a lightweight smoke |
| [infra/docker-compose.p8-vps-sim.yml](../../infra/docker-compose.p8-vps-sim.yml) | P8 resource-constrained Rust + frontend + workload simulation | **Not now**: depends on frontend and harness-specific gates, not a Pro feature smoke |
| [backend/Dockerfile](../../backend/Dockerfile) | Production-shaped Rust/C server build | Future server candidate only; not local Pro recipe test prerequisite |
| [infra/docker-compose.production.yml](../../infra/docker-compose.production.yml) and [infra/Caddyfile.production](../../infra/Caddyfile.production) | P14 production baseline with public TLS and secrets | **Do not use for Pro test**; no public deployment authorization |
| [.github/workflows/ci.yml](../../.github/workflows/ci.yml) | Current repository PR/main checks and trusted scope selection | **Remain authoritative**; do not add a parallel Pro workflow now |

Do not use imgengine-saas/infra (legacy Python stack), the historical infra/Dockerfile.p8-vps-runtime, or production secrets. P7 is **development** only, not proof of P8/P14 capacity, TLS, deployment, or commercial readiness. The current main/infra narrative may include dated historical test blockers; re-check the actual file and exact SHA before claiming a blocker remains fixed or unresolved.

## B. Windows 11 + Docker Desktop: test only C17/Rust

Execute the following **from repository root** in PowerShell. Windows runs Git/Docker orchestration only; all project build/test commands are invoked in Ubuntu 24.04 containers.

First inspect Git status/branch and check that Docker is in Linux mode:

~~~powershell
git status -sb
git branch --show-current
git rev-parse HEAD
docker info --format '{{.OSType}}'
~~~

Create the ignored local infra/p7-frontend-dev.env from [the example](../../infra/p7-frontend-dev.env.example) **only if absent**, with a non-production synthetic IMGENGINE_DEV_API_KEY. Do not overwrite an existing file or log a real value. Keep that env file untracked.

Render the **existing** P7 Compose and build only the Rust development service:

~~~powershell
docker compose --env-file infra/p7-frontend-dev.env -f infra/docker-compose.p7-frontend-dev.yml config --quiet
docker compose --env-file infra/p7-frontend-dev.env -f infra/docker-compose.p7-frontend-dev.yml build rust-api
~~~

Run C17 Debug + CTest, safe Rust façade and external FFI smoke and the Rust supervisor **without launching frontend, browser, or HTTP server**:

~~~powershell
docker compose --env-file infra/p7-frontend-dev.env -f infra/docker-compose.p7-frontend-dev.yml run --rm --no-deps --entrypoint bash rust-api -lc 'set -euo pipefail; make native-test JOBS=2; make rust-test JOBS=2 RUST_CRATE=imgengine; make rust-test JOBS=2 RUST_CRATE=imgengine-ffi-smoke; make rust-test JOBS=2 RUST_CRATE=imgengine-supervisor'
~~~

**Why this works in design:** P7 rust-api already mounts the repository at /workspace/imgengine and provides C/Rust tools and persistent Cargo caches. The run command overrides infra/scripts/run-p7-rust-api.sh rather than starting the server; --no-deps avoids frontend dependencies. No service ports are published by docker compose run unless specifically requested. This is an **untested proposed command**; actual build/exit outcomes, installed Rust toolchain compatibility, missing dependencies and host limits must be recorded during a local run.

For ABI/C11/sanitizer/decoder/fuzz modifications, use the existing release-candidate verification method from [imgengine/Dockerfile.verify](../../imgengine/Dockerfile.verify) and root [Makefile](../../Makefile), including the full verifier when warranted. The P7 dev image does not automatically include every specialized fuzz/sanitizer dependency. Do not run heavyweight full verification for docs-only edits.

### Worktree safety

Another agent may be editing Free. P7 mounts the repository and uses persistent named volumes; run Pro in a separate clean Git worktree and avoid concurrent access to identical generated build/output paths. Prefer isolated compose project names only if confirmed to avoid volume/cache interference. Never wipe another agent's volumes, build directories or environments without authorization.

## C. Change-scoped CI and acceptance

| Scope | Required evidence before completed |
| --- | --- |
| PRO-00 docs/P7 instructions | Review exact repository paths and Markdown links; new Docker runtime **NOT_RUN** until executed |
| PRO-00-NATIVE-01 | C17 debug CTest, Rust FFI + consumer + supervisor inside P7, toolchain versions, exact source SHA, environment |
| PRO-01 recipe schema | Valid/malformed/overflow/unknown-operation and schema-version tests, without UI |
| PRO-02 batch orchestration | Resource reservations, partial failures, safe cleanup, cancellation boundary, bounded RSS |
| PRO-03 transform semantics | Golden images, orientation/alpha/metadata policy, ABI compatibility, native sanitizers when changed |
| PRO-04 multi-profile outputs | Budget/fan-out, output correctness, collision and failure isolation |
| PRO-05 manifests and export | Stable status/error schema, hash/format/geometry, safe output paths |
| Existing repository PR CI | All selected required jobs at **exact PR head**; no exemption for Pro planning |
| Pro frontend/browser and public release | **DEFERRED / NOT IMPLEMENTED / NOT RUN** |

Track PASS/FAIL/NOT_RUN separately for **configuration**, **Docker build**, **C tests**, **Rust tests**, **ABI/fuzz/sanitizers** and **GitHub CI**. A pass on one existing shared component does not mark PRO-01..PRO-18 completed. Measure repeated p50/p95/p99, peak memory and test fixtures before making speed claims.

## D. Security, operations and finance

C17 owns image operations and native scheduler; Rust application logic owns recipe contract, admission and safe FFI; never invent immediate native-call cancellation or unsafe Send/Sync guarantees. No new Python backends/scripts, Redis, PostgreSQL, queuing service, microservice, public API, registry image push or project frontend code at this stage.

P7 uses a developer image, development credentials and writable source volumes: **not production-hardening evidence**. Caddy/TLS/host limits under P14 remain separate. No Pro CD pipeline, release tag, user billing, hosting charge, secrets purchase or deploy is authorized. Before commercial launch, require real user workflow/time-saved evidence, license and tax/accounting review, refunds/entitlement policy, privacy, C/FFI security, resource/host evidence if deployed, release artifact/rollback and owner go/no-go.

## E. Next bounded PR

After draft PRO-00 PR #95 is reviewed and merged, a focused PRO-00-NATIVE-01 implementation/test-evidence PR should inspect the existing ABI and run the **existing infra P7 rust-api service** inside Ubuntu 24.04, then document exact test outcomes. Do not add another Dockerfile/Compose until a measured, reviewable gap in infra is demonstrated.

See [Pro requirements](README.md), [Pro architecture](ARCHITECTURE.md), [status](STATUS.md), [infra rules](../../infra/AGENTS.md), and [live next](../../next.md).
