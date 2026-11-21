# F5-RC-EVID-01 — Detached, source-bound Free static candidate evidence

**State (2026-11-21): PR #98 REVIEW CANDIDATE, NOT MERGED, NOT A RELEASE.**

## Verified prerequisite

[F5-SUPPLY-05 PR #97](https://github.com/Rofikali/imgengine/pull/97) merged to `main` as `c1141f13c4b0b2742013a39493244c8ab9b4a1e9`. The patch uses a pinned pnpm-generated frontend lockfile, `glob@14.0.2`, `minimatch@11.0.1`. [Source-locked post-merge security CI #44877676779](https://github.com/Rofikali/imgengine/actions/runs/44877676779) PASSED the zero-moderate/high/critical gate; [PR #97 exact-head full CI #44876698219](https://github.com/Rofikali/imgengine/actions/runs/44876698219) PASSED. Cross-major transitive overrides still require independent risk review at final freeze. This is *not* proof that final notices, artifacts, Cloudflare or release approval are ready.

## Why this exists

The manually triggered Free v1.0.0 preflight already checks the pinned source lockfile, browser-only tools, compiled security headers and four-browser static output. Previously it printed a one-off shell directory digest, but did not produce a stable, machine-verifiable evidence JSON bound to a specific source commit. It also used `git diff --exit-code` inside a container where Git metadata may not be usable. This bounded milestone addresses **candidate provenance only**, not publication approval.

## Contract

`imgengine-saas/frontend/scripts/write-free-candidate-evidence.mjs` calls the existing `auditFreeStatic` source verifier rather than duplicating its privacy, secret, server-route, `_headers`, 404, size, symlink, CSP or static allowlist policy.

It creates an **out-of-tree JSON file** with:

- exact 40-character Git source SHA, `schema_version`, product version `1.0.0` and static output path;
- deterministic SHA-256 tree digest from existing verified static content records; file count, byte total, and all nine required Free routes;
- fixed build environment identifier and `CANDIDATE_NOT_APPROVED` state;
- explicit nulls for final immutable archive hash, owner/legal/security/hosting approval, Cloudflare deployment, and final Git tag.

The writer rejects an invalid source SHA, incomplete audit evidence, route mismatch, output under compiled public root, symlinked evidence-parent path and existing output. CI unit tests enforce fail-closed behavior. Candidate metadata is **not** a digital signature, supply-chain SBOM, legal approval, immutable artifact archive or proof of Cloudflare response headers.

## How to execute after PR merge

GitHub Actions → **IMGENGINE Free v1.0.0 manual release preflight** → Run workflow for the reviewed source SHA. The source-audit job and static/browser QA remain independent. The publication/legal guard intentionally still fails on the draft notices and missing independent owner approval; that is expected and must **not** be waived.

Only when the source-locked audit, full static build, exact existing browser tests, static verifier and Node safety checks pass, the workflow writes `candidate-manifest.json` under the runner's temporary directory and attaches a **review-only metadata artifact** (7-day retention). This uploads no unapproved site or source images. Retrieve the evidence by its exact run ID and inspect its source SHA against the checked-out revision and the static-tree digest. The generated static site remains ephemeral on the CI runner; no Pages upload, release tag or GitHub Release is created.

**Independently required before release:** frozen-source archive and digest/provenance/SBOM/license review, final approved notices and operator contact, detached legal/security/owner attestation, live Cloudflare HTTPS/CSP/cache/404/privacy/rollback checks, Free-plan financial assessment and explicit final go/no-go. The prior intentionally blocked publication guard remains unchanged.

## Rollback, security and architecture

Revert this PR's script/tests/manual-preflight changes as a unit. No C ABI, Rust FFI/API, native output, image bytes, network request behavior, dependencies, SaaS pricing or security-check thresholds change. Continuous integration tests are read-only; the release preflight remains **manual-only, `contents: read`**, and cannot write `main`, Git tags, Cloudflare credentials or paid services. Keep durable root/scoped AGENTS, HLD/LLD and policy docs human-reviewed; the GitHub milestone bot should only own the generated status region.
