# F5-RC-ARCHIVE-01 — Deterministic Free v1.0.0 static candidate archive

**State: IMPLEMENTATION IN REVIEW; NOT A PRODUCTION APPROVAL OR PUBLIC RELEASE.**
**Date:** 2026-11-21.
**Fixed source checkpoint before this change:** `6969b75032c04f0c2f531d23f657686d1610feb9` on `release/free/v1.0.0`; [post-merge 12/12 CI #44891016668](https://github.com/Rofikali/imgengine/actions/runs/44891016668) passed. Release branch does **not** acquire this PR's new evidence code until independently reviewed and merged into a new approved release-source revision.

## Scope and motivation

A static-site digest alone does not establish that the *serialized artifact* is exactly the verified output. The existing detached [F5-RC-EVID-01](F5_RC_EVID_01.md) inventories the source-bound files, validates nine routes and produces `candidate-manifest.json`. This milestone adds a **deterministic normalized GNU tar** for a Free-only review candidate plus a SHA-256 checksum of the actual archive bytes. There is no signing, release tagging, Cloudflare deployment, credential/billing action, legal approval, or production authorization.

The manual [Free preflight](../../../../.github/workflows/free-v1-release-preflight.yml) remains `workflow_dispatch`, `contents: read` and fail-closed in its independent owner/legal/publication job.

## Exact contract

The script `imgengine-saas/frontend/scripts/write-free-static-archive.mjs` takes exactly:

```bash
# Inside approved Ubuntu 24.04 / Node 24 / pnpm 10.34.9 environment.
# Run only after pnpm run build:free-static, verify:free-static, and four-browser QA.
node scripts/write-free-static-archive.mjs \
  .output/public \
  "$RUNNER_TEMP/imgengine-free-rc-evidence" \
  "$GITHUB_SHA"
```

1. Require a 40-hex exact source SHA, audited static root and detached output folder. Reject existing evidence files, symlinked output directory aliases, output within the site root and unsafe static content.
2. Call the existing `auditFreeStatic` to enforce the nine route allowlist, `404.html`, `_headers`, no source maps, no symlink files, no APIs, no dynamic Cloudflare Function/Worker entrypoints, CSP and size constraints.
3. Serialize **only explicitly inventoried vetted files** into `candidate-static.tar` using GNU tar with sorted input, normalized owner/group, constant mtime and removed PAX atime/ctime. There are no empty/unapproved directory records. The archive never includes backend/C/Rust services, source directories, `.env`, Git metadata or customer images.
4. Extract the candidate into a private temporary directory and **run the full static audit again** on the serialized bytes; compare the extracted digest/count/bytes against both the original pre-archive and post-archive audits. Reject any discrepancy. Calculate SHA-256 of the resulting tar file.
5. Save `candidate-archive.json` beside the tar, containing the real source SHA, source tree digest, file count, content bytes, tar SHA-256 and explicit **null** owner approval, SBOM/license sign-off, cloud deployment and final release tag. Manifest state remains `CANDIDATE_NOT_APPROVED`.
6. CI uploads **only the two JSON files** — `candidate-manifest.json` and `candidate-archive.json` — as short-lived **review-only metadata**. The tar remains in ephemeral runner temporary storage; no unapproved distributable website is uploaded to GitHub Artifacts, GitHub Releases or Cloudflare. Reproducibility of an archive digest is not the same as having a protected immutable archive outside the runner.

## Verification, performance and security boundaries

- `node --test scripts/write-free-static-archive.node-test.mjs` verifies deterministic hashes, real archive bytes, nine routes, no accidental final approval, rejection of invalid SHA/embedded output/symlink parent/forbidden API files, overwrite refusal and manual workflow non-deployment.
- Ordinary frontend CI runs this unit test; manual preflight reruns it after the full static/browser security boundary.
- GNU tar requires an **Ubuntu/Linux** runner; no Windows-host-only build, Docker service, Python, new npm package, remote callback or added paid resource.
- The process is linear in approved static file bytes plus extraction; no unsupported nanosecond speed claim. The duplicate audit and round-trip extraction consume temporary disk/CPU as an intentional release-safety cost. Recheck GitHub Actions minutes and disk quota before authorizing many runs.
- Archive bytes may be reproducible but do **not** prove build reproducibility from two independent clean builders, authenticity of the toolchain, legal correctness, live host behavior or owner authorization. Source SHA is a source identity, not an attestation signature. Do not label this output `RELEASED`.

## Acceptance and deployment follow-up

- Exact-head full CI must pass **all** required CI jobs, including native/Rust/Docker, zero-exemption privacy/CSP browser quality and the unit archive contract. The individual run result must be reviewed before merge.
- After merge, update `release/free/v1.0.0` to a reviewed source commit, repeat post-merge full CI and manually trigger the **read-only** Free release preflight on that exact branch/SHA.
- Still missing: independently retained immutable archive and SBOM/license evidence, authoritative Cloudflare Free account/project and verified cost, approved actual privacy/terms/support/operator contact, verifiable detached post-freeze legal/security/owner GO, live HTTPS/CSP/404/rollback/no-upload tests. The publication guard must not be disabled, bypassed or `|| true`-ed.
- **Final `free-v1.0.0` annotated tag only after all approvals**, on exact approved commit; create no generic `v1.0.0` tag or potentially misleading tag before that decision. Archive reviewer and hosting operator must record actual IDs and UTC times. See [owner A-to-Z guide](HOST_ON_CLOUDFLARE_A_TO_Z.md) and [go/no-go worksheet](CLOUDFLARE_LAUNCH_GATE_AND_SMOKE_TEST.md).

**Rollback:** Revert this bounded script/test/preflight feature as a unit if checks fail. No product API/ABI, hosted Cloudflare configuration, dependency versions or existing security thresholds change.
