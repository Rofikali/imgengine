# P14 Actual-VPS Evidence Runbook

## Status and purpose

**Planned; do not execute without an approved candidate, real Ubuntu host, and
deployment change approval.** This runbook collects the evidence needed to
evaluate the first-MVP P12 path on its intended low-resource VPS. It does not
authorize public traffic, change DNS, publish images, or establish production
readiness.

The target estimate is approximately 1 GiB RAM and 20 GB storage. Record the
provider, plan, CPU architecture/count, storage type, swap, Ubuntu release,
and actual available disk before drawing any capacity conclusion. Do not infer
those facts from Docker Desktop or from configured Compose limits.

The deployment scope is limited to the existing single-host topology:

```text
Internet -> Caddy -> Nuxt -> Rust/Axum -> lifecycle/admission -> C engine
```

The P12 operation remains synchronous and ephemeral. Native work is not
cancelled mid-operation by client disconnect, deadline, or graceful drain.

## Preconditions

Before starting, retain the following outside the repository:

1. A passing `P12 First MVP Release Candidate` workflow artifact for the exact
   Git revision.
2. Immutable registry digests for Caddy, Rust API, and Nuxt images. Local image
   IDs are not deployment identities.
3. A prior known-good deployment manifest for rollback, or an explicit record
   that this is the first host installation and has no prior manifest.
4. Root-owned, mode-0600 secret files containing the server API key set and
   the single Nuxt proxy key. Do not place secret values in shell history,
   Compose files, logs, artifacts, or this repository.
5. A maintenance window, named operator, alert recipient, and explicit
   permission to bring up the host-local candidate.

Do not run the workload against a public or uncontrolled service. Use
project-owned/licensed fixtures and an isolated candidate domain or a temporary
host firewall rule until the controlled-canary decision.

## 1. Record host baseline

Run as an authorized host operator and retain the command output with secrets
redacted:

```bash
date --utc --iso-8601=seconds
uname -a
cat /etc/os-release
lscpu
free -h
swapon --show
lsblk -o NAME,SIZE,TYPE,FSTYPE,MOUNTPOINTS
df -hT /
docker version
docker compose version
docker info --format '{{json .}}'
```

Record:

- total/available RAM and swap before deployment;
- total/available root-disk space before image pull;
- Docker data-root filesystem and free space;
- idle OS process memory, if the provider exposes it; and
- the exact candidate Git revision and all three registry digests.

Do not report the nominal 1 GiB or 20 GB plan as usable IMGENGINE capacity.
The operating system, Docker daemon, Caddy certificate storage, logs, page
cache, update headroom, and rollback images consume part of that budget.

## 2. Render preflight without deployment

Create `/etc/imgengine/production.env` from
`infra/production.env.example`, substituting only immutable image digests and
host-local paths. Do not source a file that is world-readable.

```bash
sudo install -d -m 0700 /etc/imgengine /etc/imgengine/secrets
sudo stat -c '%a %U:%G %n' /etc/imgengine /etc/imgengine/secrets \
  /etc/imgengine/secrets/api-keys /etc/imgengine/secrets/render-api-key

set -a
. /etc/imgengine/production.env
set +a
docker compose -f infra/docker-compose.production.yml config --format json \
  > /tmp/imgengine-production-compose.json
python3 infra/scripts/verify-production-compose.py /tmp/imgengine-production-compose.json
sha256sum /tmp/imgengine-production-compose.json
```

The expected secret-file mode is `0600`; directories should not be
group/world-readable. The Compose verifier proves configuration structure, not
DNS, certificate issuance, image provenance, or host firewall behavior.

## 3. Controlled candidate start and idle evidence

Only after preflight approval, start the exact digest-pinned candidate using
the approved deployment procedure. Before running workloads, record:

```bash
docker compose -f infra/docker-compose.production.yml ps
docker stats --no-stream
df -hT / /var/lib/docker
docker compose -f infra/docker-compose.production.yml exec -T rust-api \
  curl --fail --silent --show-error http://127.0.0.1:8000/readyz
```

For the published Caddy endpoint, validate the intended domain only after DNS
and firewall approval. Record certificate issuer/expiry, HTTPS response, and
security headers; never bypass certificate verification merely to make the
test pass.

## 4. Bounded workload matrix

Use fixed, documented request counts and project-owned fixtures. Save only
redacted summaries: status class, duration, output dimensions/counts, sampled
container usage, workspace-entry count, restart count, and free disk. Never
retain uploaded image bytes or API keys in the evidence bundle.

| Case | Required observation |
| --- | --- |
| P5 JPEG and PNG | `200` completed JPEG; cleanup completes. |
| P8 FIT JPEG and PNG | `200` with server-confirmed canvas/count headers. |
| P12 A4/300-DPI JPEG and PNG | `200`, `2480 x 3508`, exact requested count. |
| P12 capacity rejection | `422 does_not_fit`; no partial output or workspace leak. |
| Malformed image | Safe `422 invalid_image`; service remains healthy. |
| Oversized body/resource case | Safe `413 resource_limit`; service remains healthy. |
| Bounded concurrent burst | Completion and documented `429 overloaded` classes only; record distribution. |
| Restart recovery | Intentional idle restart; readiness and a subsequent valid request recover. |
| Graceful drain | Run only with explicit test coordination; prove accepted work drains and new live-handler work returns `503` while draining. |
| Disk/tmpfs pressure | Only on an isolated candidate. Prove safe failure, cleanup, recovery, and retained free-space headroom. |

Sample resource collection during each bounded case:

```bash
docker stats --no-stream --format '{{.Container}},{{.CPUPerc}},{{.MemUsage}},{{.PIDs}}'
df -hT / /var/lib/docker
docker compose -f infra/docker-compose.production.yml exec -T rust-api \
  sh -c 'find /tmp/imgengine-workspaces -mindepth 1 -maxdepth 1 -print | wc -l'
```

`docker stats` is a periodic observation, not a peak-RSS measurement. Use a
host-approved sampler or cgroup accounting supported by the actual host to
record peak memory, and label its sampling interval and blind spots.

## 5. Limits and decision record

Do not change production limits during an uncontrolled run. After the matrix,
derive or reject each candidate value from measurements:

- Caddy, Nuxt, and Rust API memory/swap/PID/CPU limits;
- Rust admission capacity and queue behavior;
- request body and encoded-output budgets;
- temporary workspace/tmpfs and Docker-disk headroom;
- allowed workload envelope, latency, overload, and cleanup thresholds.

The current 16 MiB/64 MiB JPEG destination budgets do not bound total request
memory. The configured Caddy 96 MiB, Nuxt 192 MiB, and Rust API 256 MiB
ceilings sum to 544 MiB, before OS/Docker/proxy/page-cache/operational
headroom. A successful run does not prove that the VPS is adequate until the
measured full-host budget is reviewed.

## 6. Rollback and evidence retention

Before a canary, exercise the approved rollback procedure with the prior
manifest/digests and record:

- operator and UTC timestamps;
- source and target digest manifests;
- health/readiness before and after rollback;
- smoke-request result classes;
- rollback duration; and
- any workspace or disk-cleanup result.

Retain the revision-bound release-candidate artifact, rendered Compose SHA-256,
registry digest manifest, redacted host/workload summaries, and rollback
record according to the release process. Do not retain private fixture data or
secret material.

## Completion and non-claims

This gate may be marked **actual-VPS evidence complete** only when the real
host inventory, bounded workload results, cleanup, overload, restart/drain,
disk-pressure, TLS/firewall, and rollback evidence are retained for one exact
candidate revision and reviewed by the release owner.

Completion of this runbook still does not itself authorize a public canary,
claim profitability, or retire the legacy route. Those require the separate
canary and explicit go/no-go decisions in `MVP_RELEASE_CHECKLIST.md`.
