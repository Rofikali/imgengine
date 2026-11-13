# P14 production deployment baseline

## Status

**Implemented configuration; not deployment evidence or public-release approval.**
`infra/docker-compose.production.yml` is an additive single-host deployment
profile for the existing Nuxt -> Rust/Axum -> C engine path. It does not
replace the legacy/development Compose stack, deploy an instance, or prove a
VPS capacity envelope.

The canonical first-MVP release decision record is
[`MVP_RELEASE_CHECKLIST.md`](MVP_RELEASE_CHECKLIST.md). This document supplies
deployment-profile evidence to that record; it does not close its actual-VPS,
canary, or release-authority gates.

## Security boundary

Only Caddy publishes host ports `80` and `443`. Nuxt and the Rust API are
private Compose services. Caddy terminates TLS and proxies same-origin browser
requests to Nuxt; Nuxt uses the private Compose network to reach Rust.

The three runtime services run as UID/GID `10001`, use read-only root
filesystems, drop Linux capabilities, set `no-new-privileges`, have bounded
tmpfs/PID/memory/swap settings, and restart only after an intentional failure.
Caddy has the sole additional `NET_BIND_SERVICE` capability needed for ports
80/443. A short-lived initializer owns only the permissions required to prepare
Caddy certificate/configuration volumes; it is not a network-facing service.
That initializer has a read-only root filesystem, no network, and only the
three capabilities required to establish named-volume ownership.

The Rust API continues to own authentication, request validation, bounded
admission, temporary workspace cleanup, and native invocation. This profile
does not add a second queue, scheduler, persistence layer, or browser-held
credential.

## Images and secrets

Set all image variables in a root-owned deployment environment file from
`infra/production.env.example`. Every image value must be a registry digest
(`image@sha256:...`), not a mutable tag. Builds occur in CI; production Compose
does not build from source.

Create two separate, root-owned mode-0600 files outside the repository:

- `IMGENGINE_API_KEYS_FILE`: the server-side key set accepted by Rust;
- `IMGENGINE_RENDER_API_KEY_FILE`: exactly one member of that key set, used
  only by the Nuxt server proxy.

The API and Nuxt entrypoints read these files at process start without printing
their values. The browser does not receive either secret. Rotate by replacing
the host secret files and recreating the relevant containers during an approved
maintenance window; record the affected image digests and time in the change
record.

## Local runtime evidence (2026-11-06)

The following evidence was produced locally in Docker Desktop from integrated
revision `093f1d69f47027ca5f0676cba7df42e1e5602866`. It is useful integration
evidence, but is **not** an actual-VPS test or authorization to expose public
traffic.

- The Rust API image built from `imgengine/rust/Dockerfile` and started as
  UID/GID `10001` with a read-only root, bounded `/tmp`, and an API key read
  from a mounted secret file. Its readiness endpoint returned success and an
  authenticated real-JPEG render returned HTTP 200.
- The Nuxt image built from `imgengine-saas/frontend/Dockerfile` and started
  under the same non-root/read-only posture with its server-side Rust credential
  read from a mounted secret file.
- The two images were connected only to a disposable Docker network and their
  test ports were bound to loopback. A browser-facing P12 proxy request returned
  a valid JPEG with the expected 2480 x 3508 canvas and exact-copy accounting;
  an over-capacity request returned the documented `422 does_not_fit` problem.
- All disposable containers, test network, and text-based secret artifacts were
  removed after the run. This evidence does not validate Caddy/TLS because the
  Caddy image pull did not complete in the local network environment.

## Local constrained P12 rerun (2026-11-13)

Revision `1780f65` was exercised in a disposable Docker Desktop network using
the current Rust API and Nuxt runtime images. The test deliberately used the
P14 application ceilings only: Rust API 256 MiB/0.75 CPU/64 PIDs with a 96 MiB
`/tmp` tmpfs, and Nuxt 192 MiB/0.50 CPU/96 PIDs with a 16 MiB `/tmp` tmpfs.
Both containers were non-root, read-only, capability-dropped, and
`no-new-privileges`; no host port was published and no test credential was
recorded.

- A P12 request through Nuxt's same-origin proxy produced a valid JPEG with
  canvas `2480 x 3508` and four planned/executed placements from the
  project-owned CC0 JPEG fixture.
- A genuine PNG derived from that fixture produced a valid JPEG response.
  Malformed declared-JPEG input returned `422 invalid_image`; an impossible
  copy layout returned `422 does_not_fit`.
- A four-request concurrent burst returned three `200` responses and one
  documented `429 overloaded` response. The live API workspace directory had
  zero entries after the workload.
- An idle `docker stop` completed with exit code zero and no OOM indication.
  After restart, `/readyz` succeeded and a subsequent P12 proxy request again
  returned a valid JPEG.
- `docker stats` recorded periodic samples rather than memory peaks: API
  91.13 MiB / 256 MiB and Nuxt 98.77 MiB / 192 MiB after workload activity;
  after restart, 74.62 MiB and 92.22 MiB respectively.
- The rendered `infra/docker-compose.production.yml` passed
  `infra/scripts/verify-production-compose.py` with placeholder immutable
  digests and non-secret paths. The check confirmed Caddy-only public ports,
  digest pinning, runtime hardening, internal API networking, and file-backed
  application secrets without starting a production service.

This is **local Docker constrained-runtime evidence only**. Docker Desktop's
Linux VM, host filesystem, and Docker daemon were outside the service cgroups.
The run did not include Caddy/TLS, a true 20 GB disk envelope, disk-pressure
or OOM experiments, exact peak-RSS measurement, active-request graceful drain,
or actual-VPS operating-system/service overhead. It does not close any
actual-VPS or public-release gate.

## Dependency-audit posture (2026-11-07)

The frontend now uses Nuxt 4.6.4 and a Corepack-pinned pnpm 10.34.9 workflow.
A clean Linux container passed frozen installation, Nuxt type generation, all
36 Vitest tests, typecheck, production build, and the frontend runtime-image
build. The migration made direct Vue usage explicit and records `esbuild` as
the sole approved install-time build script.

`pnpm audit --prod` reported zero critical and zero high findings, with two
moderate and one low transitive findings. The moderate findings are `glob`
13.0.7 and `minimatch` 10.2.6 beneath Nitro's `@vercel/nft` build chain. A
blind major-version override was deliberately not applied because it would
change that upstream tool's dependency contract without evidence. The low
`deepmerge` 4.3.1 finding below Rollup's node-resolve plugin has no patched
version reported by the advisory. These findings are not suppressed and require
an upstream-compatible Nuxt/Nitro update or an explicit, time-bounded risk
decision before any canary.

The earlier critical development-only Vitest/Tinypool findings were remediated
on 2026-11-06 by upgrading Vitest to 5.1.0 and `vue-tsc` to 3.3.14. Development
tooling is not copied into the final Nuxt runtime image, but it remains part of
the trusted CI/build boundary and must be audited with the runtime graph.

On 2026-11-13, the compatible `node-forge` patch was updated from 1.4.2 to
1.4.3 through pnpm's supported `pnpm.overrides` field. `listhen` declares
`node-forge: ^1.4.0`, so this preserves its upstream patch-level contract. A
fresh production audit then reported zero critical, zero high, two moderate,
and one low finding; it no longer reported the patched `node-forge` TLS,
signature, or header-processing advisories. Frozen offline install, all 36
frontend tests, typecheck, and production build passed. The remaining
`glob`/`minimatch` and `deepmerge` findings remain release-risk decisions and
are not suppressed by this patch.

The frontend CI job now runs `pnpm audit --prod --json` and fails closed on
critical or high findings. It prints the moderate/low counts rather than
suppressing them, so any release candidate still requires the documented risk
decision for unresolved findings.

`P12 First MVP Release Candidate` is a manual GitHub workflow that retains a
revision-bound production-Compose policy render, local image identities, logs,
and authenticated Rust/Nuxt P12 smoke evidence from hardened disposable
containers. It does not publish images, deploy a host, create registry
digests, validate Caddy/TLS, or close actual-VPS/canary gates.

## Preflight and rollback

Before any controlled canary, render and verify the exact deployment profile:

```bash
set -a
. /etc/imgengine/production.env
set +a
docker compose -f infra/docker-compose.production.yml config --format json > /tmp/imgengine-production.json
python3 infra/scripts/verify-production-compose.py /tmp/imgengine-production.json
```

Bring-up and rollback require separate change approval. A deployment record
must contain the exact image digests, rendered Compose SHA-256, secret-version
identifiers (never values), domain, operator, health results, and the prior
known-good manifest. Roll back by restoring the prior manifest/digests and
recreating the services; do not overwrite image tags in place.

## Resource posture and limitations

The configured application ceilings are intentionally conservative starting
guards: Caddy 96 MiB, Nuxt 192 MiB, and Rust API 256 MiB, with one existing
admission slot. Their 544 MiB sum is **not** a claim that a 1 GiB VPS is safe:
Ubuntu, Docker, Caddy certificate work, logs, page cache, image pulls, and
temporary operational work need measured headroom. The existing 16 MiB/64 MiB
encoded-output budgets are not full RSS limits.

Before public exposure, measure peak RSS, CPU, disk, and temporary workspace
usage on the intended VPS under valid, boundary, malformed, overload, restart,
drain, and disk-pressure workloads. Then set final resource limits and retain
the measurements with the release candidate.

`P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md` defines the required host inventory,
redacted evidence, bounded workload matrix, rollback exercise, and closure
conditions for that future gate. It is a procedure, not a claim that the host
evidence exists today.

## Remaining release blockers

1. Obtain an upstream-compatible remediation for the remaining moderate/low
   Nuxt/Nitro build-chain advisories, or make an explicit, time-bounded risk
   decision before any canary.
2. Produce actual-VPS capacity and operational evidence.
3. Complete TLS/DNS/firewall validation using the real domain and host.
4. Run retained release-candidate artifacts, monitoring/alert checks, rollback
   exercise, and a controlled canary while the legacy route remains available.
5. Obtain an explicit public-release decision.
