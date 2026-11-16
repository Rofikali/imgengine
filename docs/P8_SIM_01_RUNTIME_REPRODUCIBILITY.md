# P8-SIM-01 — Clean runtime images and truthful Docker VPS simulation

**Status:** IMPLEMENTATION PROPOSED / runtime tests not yet run (2026-11-16). Scope: **P8 local simulation only**, not the separate P14 `infra/docker-compose.production.yml`. Baseline `main`: `7448731a35e5c606a6c52681de06a08a8797eb52`. Root [AGENTS](../AGENTS.md), [infra AGENTS](../infra/AGENTS.md), [INFRA source audit](INFRA_SECURITY_OPERATIONS_AUDIT_2026-11-15.md) and [P14 runbook](P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md) remain authoritative. The real Docker runtime/host has **not been inspected** by this GitHub-only change.

## Engineering decision

The prior `infra/Dockerfile.p8-vps-runtime` relied on an unpinned local P7 development image and an untracked `.docker/.cargo/registry` folder. Worse, its `runtime` stage inherited the build/compiler toolchain and `frontend-runtime` inherited the Rust server runtime. It could not honestly demonstrate a clean-source build or minimal runtime dependency closure.

**Active P8 Compose now uses three distinct images:**

| Service | Build context and Dockerfile | Runtime contents | Boundary |
| --- | --- | --- | --- |
| `rust-api` | repository root, `backend/Dockerfile`, `runtime` stage | C17 shared lib, Rust/Axum executable, required native runtime libs, curl health probe | UID 10001; readonly FS; bounded cgroup; no host-published port |
| `frontend` | `imgengine-saas/frontend/`, `Dockerfile` final stage | Node 24, built Nuxt server assets and entrypoint | UID 10001 in Compose, readonly FS, host **127.0.0.1:3000 only** |
| `workload` | repository root, `infra/Dockerfile.p8-vps-workload` | Ubuntu 24.04, Bash, curl, ImageMagick, test fixture and existing workload shell script | UID 10001; one-shot profile; readonly FS; bounded CPU/memory; no host port |

The historical `Dockerfile.p8-vps-runtime` remains in tree with a clear deprecation warning, not as an active Compose reference. No new services, brokers, databases, Python scripts, production secrets, host ports or paid cloud dependencies are introduced. The existing native-C/Rust and Nuxt Dockerfiles were **reused**, not forked, to avoid policy divergence.

**Readiness:** Rust now probes `GET /readyz` (the actual authenticated-free readiness endpoint); an HTTP 405 on POST-only `/api/v1/render` is no longer misrepresented as readiness. The Nuxt probe uses built-in Node `fetch` because the standalone Node Alpine runtime does not install curl. Compose service dependencies, the original localhost binding and all existing cgroup ceilings are preserved. Workload executes as non-root with no-new-privileges.

**Reproducibility scope:** A fresh clone no longer needs the locally named P7 image or a prepopulated untracked Cargo registry. It still requires network-accessible Ubuntu/Node package sources and Cargo/npm dependencies, build platform and compatible package metadata. Current **base image tags are mutable**, so image provenance/digest pinning and full SBOM attestations remain separate supply-chain work. This PR does not claim bit-for-bit reproducible images.

## How to verify (actual Docker Desktop Linux engine)

Run only from a clean, isolated, authorized checkout of this PR, never a production host; use the existing PowerShell 7+ or Windows PowerShell simulation harness. The harness generates a random **simulation-only** key, creates local containers, captures results and cleans up the P8 project. Do not use real secrets.

~~~powershell
git status -sb
git rev-parse HEAD
docker compose version
./infra/scripts/run-p8-vps-sim.ps1 -Build
~~~

Record the actual commit SHA, Compose/Docker versions and the report file. For a separate **no-secret-value** config check, generate a throwaway key and never paste it into the agent response or GitHub logs:

~~~powershell
$env:IMGENGINE_VPS_SIM_API_KEY = ([guid]::NewGuid().ToString('N') + [guid]::NewGuid().ToString('N'))
try {
  docker compose -f infra/docker-compose.p8-vps-sim.yml --profile workload config --quiet
} finally {
  Remove-Item Env:IMGENGINE_VPS_SIM_API_KEY
}
~~~

**Acceptance checklist** (do not report PASS until run at the PR SHA):

- [ ] `docker compose config --quiet` succeeds, and `--profile workload config --services` lists only `rust-api`, `frontend`, `workload`; no legacy FastAPI/Redis/DB or public API port.
- [ ] Build succeeds from an isolated clean checkout **without** `imgengine-p7-dev-rust-api:latest` and `.docker/.cargo/registry` prerequisite.
- [ ] Image inspection shows Rust runtime and frontend runtime do not inherit the historical P7 development image; record actual image IDs/tags and runtime dependencies. Verify the Rust library can be linked/loaded.
- [ ] Rust `/readyz` and Nuxt `/` are healthy under the existing P8 cgroup constraints, with correct internal Nuxt→Rust request and authentication handling.
- [ ] Existing P8 workload returns its expected valid/invalid status classes, bounded burst behavior, JSON latency summary and empty Rust workspace after execution. Save report and logs in private temp storage, no secrets/image bytes in chat.
- [ ] Non-root UIDs, readonly root, cap drop, no-new-privileges, CPU/PID/memory/tmpfs ceilings, and **127.0.0.1:3000 only** are verified via rendered config and `docker inspect`.
- [ ] P8 simulation does **not** launch Caddy, expose Rust 8000 on host, manage real certificates, use production keys, deploy or change `docker-compose.production.yml`.
- [ ] Existing seven-job GitHub CI passes for the actual PR head. **Old/other SHA PASS is not new evidence.**
- [ ] If Docker build/health/probe fails, report the exact failing stage and a bounded log excerpt; do not weaken runtime isolation to force green.

## Source-backed caveats and next milestone

The simulation's limits sum to **640 MiB Rust + 256 MiB Nuxt + 128 MiB workload = 1024 MiB of configured container ceilings** when workload profile is active, before host/Docker overhead. Cgroup ceilings are **not** measured peak RSS or proof of safe operation on an actual 1 GiB VPS. The PowerShell harness samples `docker stats` every 500 ms and now reports the **actual Docker cgroup version**, not a fixed author's configuration. The workload fixture and its response comparisons remain the existing P8 FIT checks.

**Deferred P8-SIM-02:** Expand existing workload to P12 physical-sheet and P13 crop parity/negative/EXIF cases; measured cgroup memory events and realistic image/codec concurrency; stronger clean-clone/SBOM/digest provenance. Do not mix these features into this bounded cleanup PR. Real P14 host, DNS, TLS, abuse control, keys, certificate backup, rollback, alerting and financial cost per successful image remain **separate blocked release gates**.

## Multi-level review and business value

- **Junior/Mid:** Existing scripts/build commands unchanged from users' perspective; clear outputs and health probes.
- **Senior/Staff:** One canonical Rust Dockerfile and one canonical Nuxt Dockerfile, avoiding duplicate divergent production-like build logic.
- **Senior Staff/Principal:** Correct runtime isolation, no hidden source/cache dependencies, scope-limited backward compatibility and explicit P8→P14 difference.
- **Security/SRE:** Smaller image exposure, non-root/workload hardening, real readiness, measured—not assumed—resource and process evidence.
- **CA/MBA/management:** Avoid duplicate full Docker builds, account for CPU/RAM and CI/container minutes, make no unverified 1 GiB capacity or low-dollar hosting claim; spend decisions remain the owner's.

**Human gate:** Draft PR only; no merge/deploy/secret/CI-policy changes. The original workload should remain passing before any decision to merge.
