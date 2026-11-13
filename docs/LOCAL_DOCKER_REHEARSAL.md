# Local Docker Candidate Rehearsal

## Purpose and limits

This is a reproducible Windows 11 + Docker Desktop rehearsal of the current
P12 Rust/Nuxt candidate. It proves only the tested containers and local Docker
Desktop environment. It does **not** prove the actual 1 GiB/20 GB VPS, TLS,
DNS, firewall, registry-image provenance, canary behavior, or production
readiness.

The rehearsal uses a random ephemeral test key, an internal disposable Docker
network, a disposable named secret volume, non-root/read-only/capability-dropped
containers, and the existing P5/P8/P12 smoke scripts. It does not publish ports
or images and removes its containers, network, and secret volume on exit.

## Prerequisites

- Docker Desktop running in Linux-container mode;
- the repository checked out at the revision being rehearsed;
- sufficient Docker Desktop disk and memory to build the API and Nuxt images;
- the reusable smoke helper image. The script reports the exact build command
  if it is absent.

## Run

From the repository root in PowerShell:

```powershell
.\infra\scripts\run-p12-local-candidate.ps1 -Build
```

The first run builds the Rust API and Nuxt images from the current checkout.
Later runs can reuse images tagged with the current short Git revision:

```powershell
.\infra\scripts\run-p12-local-candidate.ps1
```

To write the redacted result somewhere explicit:

```powershell
.\infra\scripts\run-p12-local-candidate.ps1 -Build `
  -ResultsPath "$env:TEMP\imgengine-p12-local-candidate.json"
Get-Content "$env:TEMP\imgengine-p12-local-candidate.json"
```

Expected success includes:

- the Rust API P5/P8/P12 smoke suite;
- the same-origin Nuxt P12 proxy smoke suite;
- an empty Rust workspace root after the smokes; and
- no retained test containers, internal network, or named secret volume.

The result deliberately records configuration limits and image identities, not
an RSS peak. Docker Desktop resource allocation and local cgroups are not an
actual VPS operating-system budget.

## Related gates

- `scripts/verify_linux.sh` in Ubuntu 24.04 Docker remains the authoritative
  repository verification gate.
- `P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md` remains required before any VPS or
  public-release conclusion.
- The GitHub `P12 First MVP Release Candidate` workflow remains the retained
  CI candidate-evidence workflow; this local rehearsal is not a substitute for
  its revision-bound artifact.
