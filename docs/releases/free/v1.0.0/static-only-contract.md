# IMGENGINE Free v1.0.0 — Static-only public distribution contract

**Status:** F5-STATIC-01 IMPLEMENTATION MERGED AND CI VERIFIED ([PR #83](https://github.com/Rofikali/imgengine/pull/83), [12/12 exact-head CI](https://github.com/Rofikali/imgengine/actions/runs/44686554819)), **NOT** a frozen/public release. F5-TRUST-01 adds static draft notices and negative publication checks; no Cloudflare production authorization.

## Build and distribution boundary

| Retained in complete Git source | Permitted in Cloudflare Pages deployment |
| --- | --- |
| C17 `.a`/`.so`, public headers, native test harnesses | **Not** published or executed |
| Rust/Axum, Rust FFI, supervisor, server APIs | **Not** published or executed |
| Nuxt source, server routes, Docker and GitHub CI | **Not** published as source/runtime |
| Browser-local Resize/Crop/Compress/Rotate/Convert | Compiled static HTML/CSS/JS, browser Web Worker scripts if required, icons/fonts/assets, legal/support/help pages |
| Server-backed FIT/physical-sheet page and Rust proxy | **Not** present as usable public Free feature; no broken call-to-server, hidden upload flow or fallback |

Static generated output candidate: `imgengine-saas/frontend/.output/public/` from `pnpm exec nuxi generate` running **inside Ubuntu 24.04 Docker**. Nuxt `pnpm build` success and Playwright discovery alone do **not** demonstrate a viable static artifact. Direct-link/hard-refresh routes must work from the distribution; verify actual output shape rather than assuming it.

Static routes under F5-TRUST-01 are landing `/`, five browser-local tool routes (`/resize-image`, `/crop-image`, `/compress-image`, `/rotate-image`, `/convert-image`) and **three pre-launch draft** routes (`/privacy`, `/terms`, `/support`). Do **not** treat these drafts as approved terms, contact or privacy assurance. A legal-review and private support channel are required before public publication. Any redirects and security-header files are included only after review.

## Prohibited public files, functionality and secrets

- **No application server**: no Node/Nitro server runtime, Cloudflare Pages Functions/`functions` deployment, Cloudflare advanced-mode `_worker.js`, backend `server/api` handlers, runtime Rust/C services, executable binary/image-processing endpoints, or `_routes.json` that activates Functions. Browser **Web Workers for local transforms are allowed** and must not be confused with Cloudflare edge workers.
- **No server-bound image data**: no `fetch`, XHR, `sendBeacon`, WebSocket, hidden form, crash-reporting or analytics paths carrying image bytes, filenames, source metadata/EXIF, canvas pixels or downloads. No automatic fallback from browser transforms to `/api/` or any external host; no uploads to the former printable-photo-layout proxy.
- **No secrets/internal configuration**: no `IMGENGINE_RENDER_API_KEY`, `IMGENGINE_API_KEYS`, API bearer tokens, internal Rust hostnames, .env secrets, credential files, server source maps, application source files, telemetry of filenames or bearer/session identifiers. Public assets may necessarily contain non-sensitive application JavaScript and source maps only if explicitly reviewed.
- **No unsupported feature promise**: Pro, Studio, Developer API, SDK/Desktop, physical print layouts, accounts, batch/paid services or server render are not offered as working Free v1 features.
- **No hidden price or platform dependency**: Free v1 cannot require billed Workers, Functions, R2/D1/KV, private servers, domains, paid accounts or server-side compute. CDN asset delivery may still expose normal hosting-layer access metadata and is not equivalent to zero provider data processing.

## F5-STATIC-01 implementation surface (candidate, not evidence of PASS)

- `imgengine-saas/frontend/nuxt.config.ts`: opt-in `IMGENGINE_FREE_STATIC_BUILD=1`; maps only six Free paths, replaces the normal homepage with a Free-only landing component, and selects an empty Nuxt `serverDir` so normal `server/api` handlers do not join the build.
- `imgengine-saas/frontend/free-static/landing.vue` and `free-static/empty-server/.gitkeep`: Free entrypoint and explicit isolated server scan root.
- `pnpm run build:free-static`: Nuxt static generation inside Ubuntu Docker followed by explicit removal of Nuxt's catchall `200.html`. The Free artifact requires a real `404.html`.
- `pnpm run verify:free-static`: fail-closed file and text scanner, six prerendered route assertions, no symlinks, cloud-functions/server/lockfiles/hidden assets, limit and secret checks; prints a deterministic SHA-256 inventory of included files. It is **not** a substitute for human privacy/security assessment or final immutable release attestation.
- `scripts/serve-free-static.mjs`: local **test-only** HTTP server serving just `.output/public`, with real HTTP 404s for nonexistent/API routes. The test server itself must never be uploaded.
- `playwright.free-static.config.ts`: Chromium desktop/mobile, Firefox and WebKit run **all existing local-tool tests and static-specific security/navigation tests** against generated files. The normal server-backed Playwright job remains intact.
- Existing tool pages' back links now lead to the Free tools landing navigation; original backend, C/Rust/ABI and private print functionality remain in the repository.

No workflow deploy step, Cloudflare credentials or public host is introduced. The existing `Nuxt build` and `Nuxt generate` CI checks must both pass before treating this implementation as review-ready.

## Evidence demanded from F5-STATIC-01

1. Build and prerender inside pinned Ubuntu Docker from exact source SHA; validate output and retain lockfile/toolchain versions, artifact manifest/digest, allowlist of entrypoint routes and negative scan of **all** emitted paths and assets.
2. Fail CI on unexpected `_worker.js`, executable Pages Functions, `server/` / `.output/server` packaged in the *upload artifact*, unapproved dynamic API endpoints, secrets, internal hostnames or blocked routes. Beware false security claims from a string-only grep: include an actual file tree inspection, URL/network tests and independent review.
3. Serve **only generated static assets** (no running Nuxt/Rust app) in an isolated test server/container; verify each route by direct navigation, refresh and 404 behavior, including that `/api/render`, `/api/v2/sheet-render` and other server APIs do not become working endpoints or misleading HTTP 200 SPA fallback.
4. Run the **full relevant four-browser** quality matrix from the static build: Chromium desktop/mobile, Firefox and WebKit; preserve EXIF/orientation/crop/resizing/alpha, output limits, security, accessibility/keyboard, resource release, navigation and failure-path tests.
5. Assert no image bytes, filenames, EXIF/GPS or output data leak via network/browser storage, including Worker failures/cancellation; allow only reviewed asset GETs and approved disclosures.
6. Check HTTPS/TLS, CSP, security headers, caching/404 behavior, accessibility, SEO route metadata, privacy/support/terms pages and browser support on a **controlled provider preview** before owner public approval.
7. Link proof to the release [evidence matrix](evidence-matrix.md). A test not executed against the static artifact remains **NOT_RUN**, even if the earlier feature PR passed.

## Known current blocker

The repository currently includes `pages/index.vue` rendering `ServerRenderApp` and a Nuxt server proxy under `server/api`. This is retained for non-Free product work. The Free static public composition and static navigation **must be implemented separately**, not merely visually hidden. Do not delete the server implementation from Git to achieve the static boundary.

The existing [picomatch temporary exception](../../../FRONTEND_PICOMATCH_TEMPORARY_NONPROD_RISK_ACCEPTANCE_2026-11-18.md) is **non-production-only**, expires 2026-12-02 UTC and is **not** a production release authorization. Supply-chain and actual production exposure must be independently verified before deployment.

## Future products

**F5-TRUST-01 publication hold:** The source-controlled plan remains `PLANNED` and may not carry its own final Git SHA. `pnpm run verify:free-publication-ready` deliberately returns failure until a future, separately reviewed external owner approval/attestation process is built and all draft copy is approved. Success of ordinary build tests is not permission to bypass this guard.

**Cloudflare provider-specific security remains F5-HOST-01:** the service must validate actual `_headers`/CSP, page behavior, analytics configuration, HTTPS and privacy disclosures against the final chosen Pages project before production. Cloudflare may automatically enable Web Analytics, so do not claim no analytics without checking the dashboard. [Cloudflare Pages headers](https://developers.cloudflare.com/pages/configuration/headers/) and [analytics guidance](https://developers.cloudflare.com/pages/how-to/web-analytics/) are reference sources; no provider settings were changed here.

Pro/Studio may reuse the same source through governed `main` and short-lived `feat/pro/*` / `feat/studio/*` branches, but have independent release artifacts, security boundaries, support and pricing decisions. The public Free static artifact is a selected **output**, never a forked/deleted codebase.
