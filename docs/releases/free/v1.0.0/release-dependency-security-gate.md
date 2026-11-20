# F5-SUPPLY-04 — Free release-candidate dependency audit gate

**Status: MERGED in [PR #89](https://github.com/Rofikali/imgengine/pull/89), main commit `6c9e5ff7846d6d4e3b4ec63d62b19ac0d3af3008`; [post-merge CI SUCCESS](https://github.com/Rofikali/imgengine/actions/runs/44771824270). Source-locked dependency remediation remains OPEN; no public launch authorization.**

This is an *additional stricter pre-publication security condition*, not a
replacement for ordinary development CI, a dependency fix, a permanent release
attestation, or permission to publish.

## Which threshold applies?

| Context | Critical | High | Moderate | Low |
| --- | --- | --- | --- | --- |
| Ordinary pull-request/development CI | Fail | Fail | Report clearly; remediation open | Report |
| Free v1.0.0 release-candidate preflight | Fail | Fail | **Fail** | Report; still subject to reviewer disposition |

The normal CI continues to invoke `check-production-audit.mjs AUDIT_REPORT.json`,
so critical/high findings remain forbidden and the two current moderate findings
remain visible. The explicit `--release-candidate` mode is stricter: any
moderate, high or critical finding exits nonzero, **even when advisory-detail
records are missing**. Unknown arguments or old bypass flags exit with an error.
No release-waiver flag exists.

## Run in the pinned Ubuntu 24.04 Docker frontend environment

From `imgengine-saas/frontend` after `pnpm install --frozen-lockfile`:

```bash
pnpm run verify:free-release-dependencies
```

The command invokes the pinned package manager using
`pnpm audit --prod --json`, writes the report to a private temporary file,
and runs the existing sanitized checker in `--release-candidate` mode. Raw
registry output is not printed or committed, and the temporary report is
deleted. Exit codes:

- **0:** no moderate/high/critical advisories in the executed audit report;
  *not* release authorization (low findings and other launch gates need review)
- **1:** a prohibited advisory severity was reported
- **2:** invalid/missing audit, command/registry problem, checker execution
  failure, or unauthorized options; also blocks publication

The checker has negative unit tests for moderate findings with and without
metadata, malformed counts, high/critical findings and bypass options. The
runner has tests for registry/command failures, temporary-file permissions
and cleanup, low-only results and inconsistent exit status. These tests
run in development CI without misrepresenting the **current vulnerable
source lockfile** as ready for publication.

## Current actual risk (not patched)

The source-controlled lockfile still contains the two moderate transitive
findings documented in [the advisory review](dependency-advisory-review-2026-11-20.md):
`glob@13.0.7`, GHSA-3g3m-2fjr-6pw4, and `minimatch@10.2.6`,
GHSA-g3hv-75wg-m249. Prior independent experimental builds with
`glob@14.0.2` and `minimatch@11.0.1` used **only a temporary
regenerated trial lockfile** and do not repair the tracked frontend package
graph.

**Releasing Free v1.0.0 from that real source lockfile is still BLOCKED.**
The next dependency milestone requires an independently reviewed upstream-
compatible patch with a *package-manager-generated* production lockfile,
exact-head locked CI/Free static/browser checks, and source-frozen security
review. Do not set an ad hoc audit override or suppress results to force a
green release gate.

## Distinct launch requirements remain independent

Even a clean dependency audit is insufficient to publish: release artifact
provenance and immutable SHA-256, original source freeze, actual static host
CSP/HTTPS/rollback verification, final privacy/terms/support operator notices,
license/SBOM review, and a separate explicit owner go/no-go are required.
`pnpm run verify:free-publication-ready` intentionally remains
**fail-closed**, and this milestone does not change that behavior.

No hosting, domain, API, image upload, telemetry, production secrets,
billing, deployment, release tag, or owner approval is created by this gate.
