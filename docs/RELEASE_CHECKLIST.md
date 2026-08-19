# IMGENGINE Release Checklist

## Required Evidence

- PRD and SaaS contract reflect the behavior being released.
- Native CI, SaaS integration CI, migration checks, and Nuxt production build are green.
- `./imgengine-saas/scripts/verify.ps1` passes in the release-like local environment.
- JPEG, PNG, progressive JPEG, and CMYK JPEG fixtures complete with valid JPEG artifacts.
- Idempotency, owner isolation, audit timeline, persistent logs, and MIME-spoof rejection are verified.
- SLO dashboards and alert routes are reviewed by the on-call owner.
- A current verified PostgreSQL backup exists, and the latest recorded restore drill meets the recovery objective.
- The manual `imgengine-native-release-candidate` workflow is green and its archive SHA-256, deterministic archive check, manifest, ABI check, CTest, and native regressions are retained with the release record.

## Change Control

1. One user-visible outcome per loop.
2. Add the regression before or with the fix.
3. Record the rollout, rollback command, and feature-flag or image-tag reversal path.
4. Monitor job failure rate, queue publication failures, and processing latency after rollout.
5. Open a follow-up for every manual recovery or alert threshold breach.

## Native Artifact Approval

1. Dispatch `imgengine-native-release-candidate` with the exact CMake project version.
2. Download the retained archive and `.sha256` file; verify the checksum before distribution.
3. Confirm `manifest.json` contains the expected Git revision, public headers, CLI, shared library, and plugin hashes. Candidate archives use a relocatable `bin/`, `lib/`, and `include/` layout.
4. Publishing a GitHub Release or container tag is a separate approval action; this repository workflow creates a verified candidate only.
