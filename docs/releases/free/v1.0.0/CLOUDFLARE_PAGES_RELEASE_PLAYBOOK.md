# IMGENGINE Free v1.0.0 — Cloudflare Pages release operator playbook

**Release state (2026-11-21): HOLD — no final tag, GitHub Release or public deployment authorized or performed by this documentation work.** The owner requested the `v1.0.0` public launch and Cloudflare hosting. This is authorization to **prepare the release procedure**, not evidence that security, legal or provider acceptance tests have passed.

**Owner action guide:** [Cloudflare Pages Free A-to-Z](HOST_ON_CLOUDFLARE_A_TO_Z.md) and [external release decision + live smoke record](CLOUDFLARE_LAUNCH_GATE_AND_SMOKE_TEST.md). **Important:** [merged PR #98](https://github.com/Rofikali/imgengine/pull/98) was followed by a [FAILED exact-head browser CI #44885261064](https://github.com/Rofikali/imgengine/actions/runs/44885261064) (desktop Firefox full-load timeout). That merge does not clear release gates. The publication guard still fails intentionally, and Cloudflare real-host verification does not exist in this record. Do not create a tag or publish a Pages site without separately verified GO.

## 0. Exact release identity and scope

- Product: **IMGENGINE Free**. The source release plan fixes product version `1.0.0` and **namespaced final Git tag `free-v1.0.0`**. Do **not** create a competing generic `v1.0.0` tag or create the final tag from an unverified development head.
- Five static, browser-local image tools: Resize, Crop, Compress, Rotate, Convert. No account, server API, Rust/C runtime, image upload, external image-data transfer, Pages Functions, Workers or paid feature.
- Host: **Cloudflare Pages Free using Direct Upload** of the reviewed `imgengine-saas/frontend/.output/public/` directory. Automatic Git branch deployments are prohibited by the existing [release plan](release-manifest.json) to prevent a raw build or draft notices from going live.
- [Official Pages Direct Upload](https://developers.cloudflare.com/pages/get-started/direct-upload/), [Wrangler deployment](https://developers.cloudflare.com/pages/how-to/use-direct-upload-with-continuous-integration/), [Pages limits](https://developers.cloudflare.com/pages/platform/limits/), and [Pages rollback](https://developers.cloudflare.com/pages/).

## 1. Hard blockers — cannot be waived through release tagging

| Gate | Required evidence | Current status |
| --- | --- | --- |
| Source dependency audit | Pinned `pnpm install --frozen-lockfile` and `pnpm run verify:free-release-dependencies` return success with **zero critical/high/moderate** advisories | **Historical PASS**: PR #97 merged pinned remediation; [source-locked audit #44877676779](https://github.com/Rofikali/imgengine/actions/runs/44877676779) passed. **Final frozen SHA re-audit and cross-major compatibility approval still required** |
| Public notices | Actual operator-approved privacy, terms and support pages with reviewed privacy/retention/contact details | **BLOCKED**: intentionally PRE-LAUNCH DRAFT |
| Publication guard | `node scripts/check-free-publication-ready.mjs` succeeds only with independently verifiable, source-frozen owner/legal/security/host attestations, not an ad hoc flag or bypass | **BLOCKED**: implementation deliberately fails closed |
| Exact-source evidence | All required GitHub CI plus local tools, Free static browser matrix, privacy/CSP allowlist and static output inventory on frozen source SHA | **NOT VERIFIED** for final release |
| Legal/financial and live provider | Owner contact, no unapproved analytics/telemetry, selected Free plan, provider security headers/TLS, real 404, rollback, charge exposure | **NOT VERIFIED** |
| Owner go/no-go | Owner reviews exact source/artifact/approvals, then explicitly authorizes final tag **and** public production upload | **Requested in principle; independent launch evidence still missing** |

**Do not bypass the production audit, edit security test timeouts/assertions, remove draft notices without actual legal approval, or drop the publication guard to force a release.** Historical passing PR checks do not attest a new frozen release artifact.

## 2. Run the read-only release preflight

After this preparation PR is merged (not before), open GitHub **Actions → IMGENGINE Free v1.0.0 manual release preflight → Run workflow** on the intended review commit. The [manual workflow](../../../../.github/workflows/free-v1-release-preflight.yml) uses Ubuntu 24.04 / Playwright v1.58.2, pinned Node 24 and pnpm 10.34.9. It has **read-only** repository permissions and separate jobs for source-lockfile zero-moderate audit, fail-closed publication/owner/legal guard, and Free static generation/verification with full browser regression.

The **expected current result remains FAILED/BLOCKED** for publication/legal/independent-attestation gates; production dependency findings must be rechecked on the exact selected SHA, rather than assumed blocked from an obsolete lockfile. Green artifact generation does not override those failures. The workflow **does not** create a Git tag, publish to Cloudflare, store secrets or create a GitHub Release. GitHub Actions usage has plan-specific limits; avoid unnecessary reruns.

After successful source-locked re-audit, compatibility review and approved notices, freeze a specific source SHA and rerun the complete set of gates. Record exact SHA, workflow URLs, native/Rust/Docker CI, static inventory tree hash, source+artifact contents, privacy/legal owner signoff, license/SBOM review and intended Cloudflare project in a detached attestation. A Git commit of this document does not provide such attestation.

## 3. Prepare Cloudflare Pages, without deploying yet

The **owner** must create/sign into their own Cloudflare account and select the intended **Free** plan. Never paste account API tokens or private contact information into an issue/PR or this chat. No connected Cloudflare account is available to this engineering workflow.

In <https://dash.cloudflare.com/> choose **Workers & Pages → Create application → Pages → Direct Upload**. Create a clearly identified static Pages project, verify plan/budget, and keep production unpublished until the launch gates are complete. Check the project's actual account/plan, domain, project name and upload policy. **Do not select Pages Git integration with automatic `main` production deployments.** Cloudflare's generic Nuxt preset often targets a server build; IMGENGINE's reviewed Free output is instead `imgengine-saas/frontend/.output/public`, not `dist` or `.output/server`.

If you cannot keep the project from deploying during creation, stop before project creation until the release is approved. Pages project creation, DNS changes, billing and production deployment are distinct owner-controlled operations.

## 4. Produce and upload the approved artifact (only *after* all gates pass)

Use the **exact frozen release source**, not a moving `main`, in the approved Ubuntu 24.04 Docker environment with pinned Node/pnpm. From `imgengine-saas/frontend`, after package manager setup and install, the essential commands are:

```bash
pnpm install --frozen-lockfile
pnpm run verify:free-release-dependencies
pnpm run verify:free-publication-ready
pnpm run typecheck
pnpm test
pnpm run test:browser:local-tools
pnpm run build:free-static
pnpm run verify:free-static
pnpm run test:browser:free-static
```

Check the generated root `_headers`, `404.html`, and nine permitted routes; confirm **absence** of `_worker.js`, `_routes.json`, Pages Functions, server endpoints, secrets, backend source and static image/upload egress. Build and copy an **immutable** archive and independent SHA-256 manifest outside the tracked release plan. Retain CI, provenance and rollback evidence; never publish build directories other than the verified static allowlist.

Only after a passing independent publication attestation and owner approval, use **one** of the officially supported methods:

- **Cloudflare dashboard:** Direct Upload the approved `.output/public/` directory, not the source repository, and record the actual deployment ID and URL from Cloudflare.
- **Wrangler CLI (authenticated operator environment):** `npx wrangler pages deploy .output/public --project-name=<ACTUAL_PAGES_PROJECT>`. Provide account ID and Cloudflare API token via operator-managed environment/secret store if needed; do not commit them. Confirm target project and production branch/deployment semantics before invoking.

**Never invent** a Pages project name, `*.pages.dev` URL, deployment ID, token, domain, or successful deploy.

## 5. Verify live Pages before announcing

On the **actual Cloudflare URL**, independently verify HTTPS/TLS, live `Content-Security-Policy` and security headers, expected redirects/cache rules, 404 on nonexistent `/api/` endpoints, nine static routes including three approved notices, correct browser-local transformations on desktop/mobile, accessibility, no customer-image upload/telemetry, and Cloudflare analytics/retention settings. Inspect CDN request metadata and applicable privacy notices; local HTTP simulation is not proof of provider behavior.

Record the actual **deployment ID, public URL, approved source SHA, artifact SHA-256, test timestamps/results, operator review and rollback target**. Confirm any paid limits or spend alerts before broader publicity; Free does not guarantee no charge under every optional service or account setting.

## 6. Create the final Git release identity (after frozen evidence)

When **all** required gates are satisfied and owner gives final go/no-go, create the annotated tag on the **reviewed immutable source commit**. The source release plan's tag identity is `free-v1.0.0`:

```bash
git fetch origin --tags
git switch --detach <VERIFIED_FROZEN_SOURCE_SHA>
git tag -a free-v1.0.0 -m "IMGENGINE Free v1.0.0 — verified release"
git push origin refs/tags/free-v1.0.0
```

Never overwrite/repoint a published tag. Optionally publish a GitHub Release referencing this exact tag with verified release notes and artifact digests, **without including backend binaries or unapproved builds**. A release tag does not upload to Cloudflare; record both tag and provider deployment separately. If the chosen operational sequence is deploy → verify → publish tag, retain immutable source and approval evidence across all steps.

## 7. Rollback and incident procedure

Use Cloudflare Pages dashboard's documented rollback to a **previous verified deployment**, validate restored TLS/CSP/privacy/no-upload controls and route behavior, update the incident/status record, and pause publicity. A failing release gate or unavailable rollback evidence means **NO GO**. Never replace the final Git tag to cover a bad deployment; create an independently reviewed patched release version instead.

### Owner approvals still required

Actual operator/legal contact content, final notices, source+artifact approval, selected Cloudflare account/Pages project and plan, any billing exposure, live-host risk acceptance and the release go/no-go. This document, the manual workflow, and any GitHub merge do not substitute for those decisions.
