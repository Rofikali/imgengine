# IMGENGINE Small-VPS Deployment Design

**Status:** Planned target deployment.

## Baseline

One Ubuntu VPS runs Caddy, Nuxt, and the Rust API via production Compose. Caddy alone publishes `80` and `443`; Nuxt and Rust are private services. Start with one render permit on a $5-class VPS and tune only from measured memory and latency evidence.

The production Compose file uses immutable image digests, named internal networks, non-root users, `cap_drop: [ALL]`, `security_opt: [no-new-privileges:true]`, CPU/memory/pid limits, health checks, restart policies, a bounded tmpfs request workspace, and Docker secrets or root-owned host-mounted secret files. Development observability profiles, MinIO, Redis, PostgreSQL, ELK, and Jaeger are not part of this baseline.

## Delivery and Rollback

1. CI builds, tests, scans, generates an SBOM/provenance, and publishes immutable images.
2. A release manifest records image digests, native engine version, configuration schema, and migration compatibility.
3. Deploy by digest after health/readiness checks; preserve the prior manifest.
4. Roll back by redeploying the prior manifest. Do not roll back across destructive schema changes.

## Host Operations

- Patch the OS regularly; disable password SSH; restrict SSH by firewall/VPN; back up only configuration and release metadata unless a future persistent product feature requires data backup.
- Rotate application logs with bounded retention or ship stdout to a managed sink. Never place a full ELK stack on this VPS.
- Test restore of secrets/configuration and rollback at least once per release cycle.

