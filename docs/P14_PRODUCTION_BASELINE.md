# P14 production deployment baseline

## Status

**Implemented configuration; not deployment evidence or public-release approval.**
`infra/docker-compose.production.yml` is an additive single-host deployment
profile for the existing Nuxt -> Rust/Axum -> C engine path. It does not
replace the legacy/development Compose stack, deploy an instance, or prove a
VPS capacity envelope.

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

## Dependency-audit posture (2026-11-06)

`npm audit --omit=dev` reported zero critical and zero high findings for the
runtime dependency graph, with ten moderate and two low findings. The remaining
runtime Nuxt/Nitro findings require a separately scoped Nuxt 4 migration or an
explicit, time-bounded risk decision before any canary.

The earlier critical development-only Vitest/Tinypool findings were remediated
on 2026-11-06 by upgrading Vitest to 5.1.0 and `vue-tsc` to 3.3.14. Frontend
tests (36/36), typecheck, and production build passed after the upgrade. Both
the runtime-only and full build/test audit commands then reported zero critical
and zero high findings; the full graph has the same twelve moderate/low
Nuxt-chain findings. Development tooling is not copied into the final Nuxt
runtime image, but it remains part of the trusted CI/build boundary and must be
audited with the runtime graph.

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

## Remaining release blockers

1. Complete the planned Nuxt 4 migration or separately risk-accept its
   remaining moderate/low runtime and build-chain advisories for a time-bounded
   canary.
2. Produce actual-VPS capacity and operational evidence.
3. Complete TLS/DNS/firewall validation using the real domain and host.
4. Run retained release-candidate artifacts, monitoring/alert checks, rollback
   exercise, and a controlled canary while the legacy route remains available.
5. Obtain an explicit public-release decision.
