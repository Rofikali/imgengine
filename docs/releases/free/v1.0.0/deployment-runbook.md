# IMGENGINE Free v1.0.0 — Cloudflare Pages static deployment and rollback

**Status:** FUTURE PROCEDURE ONLY — NOT DEPLOYED / NO PRODUCTION APPROVAL. Account signup or login is not permission to create a public Pages deployment. See [release dossier](README.md), [static contract](static-only-contract.md) and [F5 evidence matrix](evidence-matrix.md).

## Chosen hosting architecture

**Cloudflare Pages Direct Upload** for a reviewed, Docker-generated static site. No Git-connected automatic deployments, no Nuxt/Nitro server, no deployed Pages Functions, no public Rust/C native process, no image upload API, no database, no paid Cloudflare plan and no initial custom-domain requirement. Existing source repository and all backends remain preserved. A `pages.dev` URL is publicly reachable once deployed; **do not** consider an unprotected preview secret.

Official vendor references: [Direct Upload](https://developers.cloudflare.com/pages/get-started/direct-upload/), [Wrangler CI/direct upload](https://developers.cloudflare.com/pages/how-to/use-direct-upload-with-continuous-integration/), [Pages rollback](https://developers.cloudflare.com/pages/configuration/rollbacks/), [Pages Functions](https://developers.cloudflare.com/pages/functions/get-started/). Recheck guidance/limits at execution time.

## Phase 0 — account and governance (no deployment)

- Owner verifies Cloudflare Free plan, email and multifactor authentication; protects account recovery codes; no secrets/PII in repo or AI logs.
- Confirm provider dashboard project type. **Direct Upload** projects are chosen deliberately; do not connect Git and accidentally enable auto-publish.
- Prefer no project/public URL until a controlled preview is approved. If a project shell is created beforehand, make sure that action cannot publish an unapproved asset. Verify account permissions, billing/product plan and user access.
- Configure GitHub branch protection/review rules requiring `CI verification gate`; do not rely on GitHub's default green badge or auto-merge behavior.
- No domain purchase, VPS, API token or production traffic is implied by creating this runbook.

## Phase 1 — static artifact candidate in Ubuntu Docker

1. Start only from reviewed release SHA/tag and prescribed Docker Ubuntu 24.04 image, Node/pnpm version, pinned lockfile and clean source; keep Windows host free of Node tooling.
2. After the F5-STATIC-01 PR passes exact-head CI and is approved, the candidate commands **inside Ubuntu 24.04 Docker** are `pnpm install --frozen-lockfile`, `pnpm run build:free-static` and `pnpm run verify:free-static` from `imgengine-saas/frontend`. The build intentionally selects only Free routes and removes generated SPA `200.html`; **do not substitute generic `pnpm run build` or generic `nuxt generate`**. As of this document, actual approved release-candidate build evidence remains NOT_RUN.
3. Inspect `.output/public` and assert all required prerendered routes/assets. Package **only** a verified copy of the files under `.output/public`, **not** the containing `.output/server`, source tree, Docker files, backend, secrets or hidden runtime.
4. Run fail-closed file/tree/secret/function scans and independently serve only this output to the complete four-project browser/privacy/a11y suite; verify negative `/api/` behavior and no server uploads. Do not change tests to hide failures.
5. Produce a **post-freeze generated** artifact manifest with source SHA, exact Node/pnpm/lockfile/build image identities, inventory, full artifact SHA-256, test/CI runs and provenance/SBOM as appropriate. Do not insert the Git SHA into the same tracked release-plan JSON whose modification would change that SHA.
6. Archive the exact artifact and manifest in a protected release record (GitHub Release assets/CI artifact with retention/provenance as reviewed), without automatically publishing a public GitHub Release or CDN site.

## Phase 2 — owner signoff and approved deployment

1. Obtain **explicit**, separate owner approval for immutable artifact digest, intended Cloudflare account/project, Free plan constraints, expected site visibility and launch/rollback operator.
2. Review remaining high-severity advisories, static privacy/legal/support documentation, Cloudflare Access preview protection when required, DNS/TLS and asset security headers. Resolve/accept **only with valid production-authorized security decision**; the current picomatch exemption is *non-production only*.
3. Create/select the **Direct Upload** Pages project under Workers & Pages, after validating its public-visibility behavior. For controlled internal rehearsal, use a protected preview if authorized; Cloudflare Pages previews may otherwise be Internet-accessible.
4. Only after signoff, upload the **already-verified** static directory/ZIP using the dashboard's Direct Upload or a narrowly scoped Wrangler session/credential. No default Git integration or automatic deploy-on-`main` pushes. Never pass bearer tokens in shell histories, GitHub commits, screenshots or logs.
5. Record actual Cloudflare deployment ID, UTC timestamp, environment, URL, asset digest and operator. Do not treat `release/free/v1.0.0` or `free-v1.0.0` as deployed until these facts are observed.

## Phase 3 — post-deployment verification

- Test every Free path and hard refresh, HTTPS/security headers, 404/nonexistent `/api/` routes and no Pages Functions or server image upload.
- Repeat representative browser-local image transformations plus privacy tests on the **actual hosted asset revision**; verify the deployed asset hashes match the approved candidate (accounting for provider asset handling).
- Confirm support/contact, privacy and terms, status/error responses and no hidden server layout feature.
- Check provider usage/limits, uptime indicators, error monitoring without image data, incident communication and cost exposure. Record PASS/FAIL with UTC time and reviewer.
- Mark `RELEASED` **only** after the authorized public site is live and these smoke tests pass; keep live `next.md` and immutable release/deployment records consistent.

## Phase 4 — rollback/withdrawal and disaster handling

For releases **after the first**, restore the **verified previous known-good** Pages deployment through Cloudflare dashboard/approved API and confirm rollback URL, assets, no-upload, TLS, routes and timing. Record a corrective issue and incident report.

For the **first** public deployment there may be **no previous known-good deployment**: preapprove an emergency traffic restriction/maintenance response or disabling/removing public availability, and practice that without leaking secrets. Deleting a Pages project is not automatically the preferred rollback; use reversible access controls whenever possible. Verify provider rollback controls on the actual account/project before publication.

No cloud or repository modification is performed by following this paper runbook; every public traffic, billing, credential and domain action requires separate owner approval.
