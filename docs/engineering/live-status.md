# IMGENGINE milestone status

## GitHub-verified NOW / NEXT / WORKING / COMPLETED

Generated from the owner-reviewed milestone registry and GitHub PR/check facts. This is a **snapshot**, not a live GitHub view or release approval. Historical notes elsewhere in this repository may describe older PR states.

**Product:** IMGENGINE Free v1.0.0 — **release state: NOT_RELEASED** (owner approval required).

### NOW

**F5-SUPPLY-05** — Permanent upstream-compatible dependency and lockfile remediation (READY)

### NEXT

**F5-RC-01** — Immutable source-frozen static release artifact and independent verification (PLANNED) — waiting on: F5-SUPPLY-05

### WORKING / REVIEW

No tracked PR is currently in progress.

### COMPLETED (merged PRs)

- **F5-SUPPLY-03** — Disposable patched-major compatibility verification and Firefox isolation (MERGED) — [PR #88](https://github.com/Rofikali/imgengine/pull/88) — merge ec213cfc1814
- **F5-SUPPLY-04** — Strict no-moderate release dependency audit gate (MERGED) — [PR #89](https://github.com/Rofikali/imgengine/pull/89) — merge 6c9e5ff7846d
- **F5-DOC-RECON-01** — Post-merge release documentation reconciliation (MERGED) — [PR #90](https://github.com/Rofikali/imgengine/pull/90) — merge aaee4a259842
- **F5-DOCSYNC-01** — GitHub PR-driven milestone status generator (MERGED) — [PR #91](https://github.com/Rofikali/imgengine/pull/91) — merge 76d42f094e5d
- **F5-DOCSYNC-02** — Recover missing review PR on identical bot branch and preserve fail-closed permissions (MERGED) — [PR #93](https://github.com/Rofikali/imgengine/pull/93) — merge 8d25ac9235c1

### Other open PRs (not automatically assigned a milestone)

- [PR #94](https://github.com/Rofikali/imgengine/pull/94) — release(free): prepare gated v1.0.0 Cloudflare Pages Direct Upload
- [PR #69](https://github.com/Rofikali/imgengine/pull/69) — docs(infra): record P8 Docker smoke evidence
- [PR #63](https://github.com/Rofikali/imgengine/pull/63) — docs(codex): reusable milestone Agent Skill and lean AGENTS.md startup
- [PR #27](https://github.com/Rofikali/imgengine/pull/27) — Feat/p12 abi 22 native sheet

### Independent public-release blockers

- Production source-lockfile moderate advisories (glob and minimatch) remain unremediated
- Immutable Free-only release artifact, SBOM/provenance and independent release audit not approved
- Privacy, terms, support and operator/legal contact notices remain draft
- Live Cloudflare TLS/CSP/privacy, rollback and operating-cost evidence not verified
- Explicit owner release go/no-go and production deployment not authorized

**Safety:** PR merge is not product release. Exact-SHA CI, dependency audit, static privacy/security, legal/support, hosting/rollback, cost and owner go/no-go remain independent. No automatic merge, deployment, tag, audit waiver or change of authorization is performed.
