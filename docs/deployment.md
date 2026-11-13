# IMGENGINE Small-VPS Deployment Design

**Status:** Planned target deployment.

## Baseline

One Ubuntu VPS runs Caddy, Nuxt, and the Rust API via production Compose. Caddy alone publishes `80` and `443`; Nuxt and Rust are private services. Start with one render permit on a $5-class VPS and tune only from measured memory and latency evidence.

`infra/docker-compose.production.yml` is the additive production-profile
implementation of this topology. It uses immutable image digests, non-root
users, `cap_drop: [ALL]`, `security_opt: [no-new-privileges:true]`,
CPU/memory/pid limits, health checks, restart policies, bounded tmpfs request
workspaces, and file-backed Compose secrets. Development observability
profiles, MinIO, Redis, PostgreSQL, ELK, and Jaeger are not part of this
profile. Its exact contract, preflight command, and remaining gates are in
`P14_PRODUCTION_BASELINE.md`.

`docs/P8_VPS_RESOURCE_AUDIT.md` records a Docker Desktop constrained-container
experiment for the P8 candidate. It is useful service-level evidence, but it
does not validate a whole 1 GiB VPS: the measured API and frontend limits alone
sum to 896 MiB and exclude the OS, Docker, Caddy, logs, and page cache. Do not
derive deployment sizing or a production-ready claim from that experiment.

The P14 production-profile ceilings are starting safety guards rather than a
VPS-sizing result. They must be replaced or confirmed only after actual-host
peak-RSS, CPU, disk, cleanup, overload, restart, drain, and rollback evidence
has been recorded.

The future host-evidence procedure is
`P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md`. Its existence is not evidence that an
actual VPS has been tested.

## Delivery and Rollback

1. CI builds, tests, scans, generates an SBOM/provenance, and publishes immutable images.
2. A release manifest records image digests, native engine version, configuration schema, and migration compatibility.
3. Deploy by digest after health/readiness checks; preserve the prior manifest.
4. Roll back by redeploying the prior manifest. Do not roll back across destructive schema changes.

## Host Operations

- Patch the OS regularly; disable password SSH; restrict SSH by firewall/VPN; back up only configuration and release metadata unless a future persistent product feature requires data backup.
- Rotate application logs with bounded retention or ship stdout to a managed sink. Never place a full ELK stack on this VPS.
- Test restore of secrets/configuration and rollback at least once per release cycle.

