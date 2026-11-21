# IMGENGINE Free v1.0.0 — Cloudflare Pages A-to-Z owner guide

**Date checked:** 2026-11-21. **Decision:** Cloudflare Pages Free is technically suitable for the Free static build; **PUBLIC RELEASE HOLD** until the independent launch gates below pass. This file is a practical deployment procedure, **not** approval to upload or a claim that any Cloudflare account/project/URL exists.

For the security/engineering policy see [the canonical release operator playbook](CLOUDFLARE_PAGES_RELEASE_PLAYBOOK.md), [release plan](release-manifest.json), [static-only contract](static-only-contract.md), and [deployment and rollback runbook](deployment-runbook.md). For an auditable sign-off use [the release decision worksheet](CLOUDFLARE_LAUNCH_GATE_AND_SMOKE_TEST.md). Do not copy outdated status paragraphs from historical records as current facts.

## 1. What exactly will be hosted?

**Only** `imgengine-saas/frontend/.output/public/` from the **approved, frozen Git source SHA**. The Free product supplies five **browser-local** image tools: resize, crop, compress, rotate and convert, plus public homepage, privacy, terms and support routes. The browser downloads HTML/CSS/JavaScript over HTTPS; image data is supposed to remain local to the user's device. Normal CDN request metadata is a separate data category and must be disclosed accurately.

No GitHub source tree, `.output/server`, Nuxt/Nitro runtime, C/Rust backend, Docker services, image-upload endpoints, database, Pages Functions, Workers, R2, API token or paid service is part of the Free deployment. **Do not choose a generic Nuxt SSR/Workers deployment or Git-connected automatic builds** as a shortcut; that would violate the source-controlled Free release plan.

Required distribution root: `index.html`, `404.html`, `_headers`, all nine pre-rendered routes and hashed static assets. `_headers` is consumed by Pages for static asset responses and must remain at the upload root. `404.html` prevents fallback-to-index success for unknown paths. `_worker.js`, `_routes.json`, source maps, runtime folders, images uploaded by users or secrets are forbidden.

## 2. CURRENT GO/NO-GO: do not mistake a merge for a release

- [PR #98](https://github.com/Rofikali/imgengine/pull/98) is **MERGED** in main at `f00a7ac522d0f90e3e834022c400c30973e8f4e3`. Its original desktop-Firefox full-load timeout and a subsequent main timeout are historical failures; neither established a privacy leak, and neither merge overrode the browser gate.
- [PR #103](https://github.com/Rofikali/imgengine/pull/103) isolated the Firefox/WebKit privacy-route test lifecycle. Current main `889be673220d4489325ff63a7a1ad6642b17fa2b` passed [full CI #44888710794](https://github.com/Rofikali/imgengine/actions/runs/44888710794), including the four-browser static-boundary matrix and all selected native/Docker/Rust/frontend jobs. This clears that CI regression only; the final release gate remains **NO GO** until frozen-source, legal, attestation, artifact and live-host evidence exists.
- The production `glob/minimatch` lockfile fix from [PR #97](https://github.com/Rofikali/imgengine/pull/97) passed [strict audit #44877676779](https://github.com/Rofikali/imgengine/actions/runs/44877676779) on that historic source. **Recheck the exact frozen source's audit**, compatibility and licenses; earlier green results are not release attestation.
- `/privacy`, `/terms`, `/support` are **PRE-LAUNCH DRAFTS**, not operator/legal-approved notices; the publication script `scripts/check-free-publication-ready.mjs` intentionally **always fails** until a separate, verifiable post-freeze sign-off mechanism is implemented. Do **not** patch or ignore its exit code to upload.
- No approved immutable production archive and SHA-256 provenance/SBOM/licenses bundle, signed owner/security/legal/hosting approval, verified live Cloudflare response, rollback proof or `free-v1.0.0` tag exists in the verified evidence for this document.
- The milestone bot's inability to create GitHub review PRs is **separate**: repository **Settings > Actions > General > Workflow permissions > Allow GitHub Actions to create and approve pull requests** is an owner-controlled setting. It does not authorize a Free product release.

**Allowed now:** review the Free plan, set up account security, prepare source/build tooling, run local offline/static tests, review the checklists. **Not approved:** a public `pages.dev` deploy, release tag, automatic deployment or paid resource. A Pages URL becomes reachable on the Internet as soon as deployed; `noindex` alone is not access control.

## 3. Cloudflare account setup (owner only; no deployment)

1. Open <https://dash.cloudflare.com/>. Sign up/sign in and confirm you control the account email. Select **Free** and verify that the account has not been upgraded or enabled additional paid features. Enable two-factor authentication and store recovery methods securely.
2. In the dashboard, find **Workers & Pages**. Review the Pages Free limits, plan, memberships and any billing/usage settings. A **`*.pages.dev` address does not require buying a domain or a VPS**; a custom domain is optional and domain registration may cost money.
3. **Choose Direct Upload when an approved deployment is ready**. In current Cloudflare documentation the dashboard path is **Workers & Pages → Create application → Get started → Drag and drop your files** (labels can change). Do **not** select Git integration: its branch auto-publish is not part of this release plan, and Direct Upload projects cannot later be converted in place to Git integration.
4. **Stop before pressing Deploy site** while launch gates are blocked. Project creation during the drag-and-drop flow may immediately publish an asset; do not assume a draft project is private. For an owner-authorized internal rehearsal, use access controls configured and tested **before** sharing preview URLs. Preview URLs are Internet-reachable unless protected. The current owner request does not independently authorize a public rehearsal upload.

Official references: [Cloudflare Direct Upload](https://developers.cloudflare.com/pages/get-started/direct-upload/), [Pages Free limits](https://developers.cloudflare.com/pages/platform/limits/), [Preview protection](https://developers.cloudflare.com/pages/configuration/preview-deployments/).

## 4. Prepare the exact static build in the existing Linux/Docker workflow

1. Use the repository's Ubuntu **24.04** / Playwright **1.58.2** verified container build environment, Node **24**, and `pnpm@10.34.9`. Do not substitute Windows local builds for authoritative CI. Source must be a **reviewed, immutable commit**, not whichever `main` happens to be checked out.
2. In that Ubuntu environment, at the repository root, inspect the source: `git rev-parse HEAD`, `git status --porcelain`, `git tag -l free-v1.0.0`. Freeze the approved source when authorized. From `imgengine-saas/frontend` run these **preparation** commands:

```bash
# Execute inside an approved Ubuntu 24.04 container with Node 24 and pnpm 10.34.9
cd imgengine-saas/frontend
node --version
corepack enable
pnpm --version
pnpm install --frozen-lockfile

# These are mandatory source and static boundaries, not optional warnings
pnpm run verify:free-release-dependencies
pnpm run typecheck
pnpm test
pnpm run build:free-static
pnpm run verify:free-static
pnpm run test:browser:free-static

# Observe the generated upload ROOT, not a Nitro/server output
test -f .output/public/index.html
test -f .output/public/404.html
test -f .output/public/_headers
test ! -e .output/public/_worker.js
test ! -e .output/public/_routes.json
```

The four-browser runner needs the correctly installed Playwright browser engines and sufficient shared memory; using the same pinned CI container avoids false discrepancies. The exact-head **full native/Docker/browser GitHub CI** must also be green. The separate browser-local test suite remains a release check as documented in the existing playbook.

3. The **manual, read-only** workflow **GitHub → Actions → IMGENGINE Free v1.0.0 manual release preflight → Run workflow** can create candidate evidence after static verification. It intentionally keeps the publication/legal job failing while authorizations are missing. The source-bound candidate JSON is **not** an immutable signed archive and must not be mistaken for permission to deploy.
4. After closing the release blockers, an independent release operator must freeze the exact source SHA, hash/store the complete static artifact in a protected immutable archive, collect SBOM/license/security evidence, compare manifests and verify the approved **post-freeze** publication attestation. Capture the source SHA, artifact SHA-256, CI run IDs, reviewer and UTC approval time outside any self-referential tracked Git manifest.

**Never upload the source root, `.output/` as a whole, `.output/server`, `node_modules`, `backend`, `infra`, `.env` or a generic `nuxt build` output.**

## 5. Deploy after an explicit, documented GO decision

Only after all gates in [the launch-gate worksheet](CLOUDFLARE_LAUNCH_GATE_AND_SMOKE_TEST.md) are complete and owner approves the exact artifact and target account:

**Dashboard method (recommended for the first release):**

1. Log into the correct Cloudflare account, open **Workers & Pages**, and select **Create application → Get started → Drag and drop your files**.
2. Enter an owner-approved, available project name. The eventual production URL follows `https://<actual-project>.pages.dev` (Cloudflare may append characters if the name is taken); **never guess or record this as a real deployed site before Cloudflare reports it**.
3. Drag the **contents of the approved static output folder** (or ZIP with the output files at the ZIP root) into Direct Upload. The archive root must have `index.html`, `404.html` and `_headers`, **not** an extra `public/` parent folder. Cloudflare dashboard supports folders and ZIPs.
4. Review project, Free plan and uploaded files, then select **Deploy site** / **Save and Deploy** as the dashboard directs. Record the **actual** URL and deployment ID. This operation makes the site reachable; no independent confirmation step should be assumed.

**Authenticated CLI alternative (only on the owner's trusted terminal after GO):**

```bash
# From imgengine-saas/frontend after verified output exists:
npx wrangler login
npx wrangler pages project create     # first time: choose the approved project/branch
npx wrangler pages deploy .output/public --project-name=<ACTUAL_PROJECT_NAME>
```

Do not paste API tokens into chat, PRs, shell screenshots or files. The CLI consumes a **folder**, not ZIP. For deliberate preview vs production deployment, inspect the official `--branch` behavior and the project's actual production branch before invoking; Direct Upload does not automatically publish each `main` push.

Official method: <https://developers.cloudflare.com/pages/get-started/direct-upload/>.

## 6. Validate the REAL Cloudflare deployment before announcing v1.0.0

Use the **actual** `https://...` URL from the dashboard and fill out [the deployment evidence worksheet](CLOUDFLARE_LAUNCH_GATE_AND_SMOKE_TEST.md). Required observations:

- HTTPS/certificate, root and five tool routes plus `/privacy`, `/terms`, `/support` (approved final notices); verify all nine static routes and correct navigation on phone and desktop.
- Root and tool responses include **actual** live CSP with `default-src 'none'`, `script-src 'self'`, `connect-src 'self' blob:`, `frame-ancestors 'none'`, `X-Content-Type-Options: nosniff`, `X-Frame-Options: DENY`, `Referrer-Policy: no-referrer`; reconcile actual header scope and redirect behavior against the approved `_headers`.
- `/does-not-exist`, `/api/render` and other forbidden APIs return a **real HTTP 404**, not the homepage, a successful SPA fallback or a function response.
- All five features work in Chromium, Firefox and WebKit/Safari; keyboard/accessibility, PNG/JPEG/EXIF behavior and resource cleanup are acceptable; inspect Developer Tools to ensure no image bytes, filenames or metadata are sent to an application server. Do not claim Cloudflare receives **no** normal request metadata.
- Confirm there is no Pages Function/Worker deploy, no external runtime dependency, unexpected analytics, paid resource, custom cache rule or changed CDN asset. Verify actual usage, budget and rollback path.
- Record the exact approved source SHA, archived digest, checked CDN URL/deployment ID, real response headers, pass/fail matrix, operator/owner approvals and UTC timestamp. If any required check is red or unknown: **NO GO**, do not announce or tag a public release.

## 7. Version identity, rollback and finance

- Canonical **proposed** final annotated Git tag is `free-v1.0.0`, **not** an unreviewed `v1.0.0`. Create it only after the release's independent source, archive, security/legal, owner and live-host decision have been recorded. Never rewrite a published tag to conceal a bad deployment.
- Cloudflare Pages supports reverting to an earlier production deployment, but **the first deployment may have no known-good rollback target**. Owner must approve an emergency exposure restriction/withdrawal plan before launch. Preserve the immutable source archive outside Cloudflare: provider deployment retention is not an indefinite artifact archive.
- **Cost policy:** Free static-only Pages requests are listed as free and unlimited; current Pages Free plan includes **20,000 site files**, **25 MiB max per asset** and applicable deployment limits. Optional domains, Workers/Functions, R2 or upgrades have separate policies/costs. Confirm billing/usage settings yourself; avoid creating any chargeable product just to host IMGENGINE Free.

Official references: [Pages pricing (static vs Functions)](https://developers.cloudflare.com/pages/functions/pricing/), [Limits](https://developers.cloudflare.com/pages/platform/limits/), [Rollbacks](https://developers.cloudflare.com/pages/configuration/rollbacks/), [HTTP headers](https://developers.cloudflare.com/pages/configuration/headers/), [Serving Pages/404](https://developers.cloudflare.com/pages/configuration/serving-pages/).

## 8. Release decision summary

**YES, suitable hosting architecture. NO, not production-authorized today.** First repair/re-verify the Firefox privacy CI regression on the **latest release source**, then approve real legal/support/privacy information, implement/verify independent publication attestation and immutable artifact archive, review Cloudflare free-plan exposure, and obtain explicit owner GO. Only then use the above deployment process. The GitHub source merge, this guide and any ChatGPT response do **not** create a Cloudflare account, public site, final release tag or approved artifact.
