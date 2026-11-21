# IMGENGINE Free v1.0.0 — draft release notes

**DRAFT / NOT PUBLISHED / NOT APPROVED.** These notes describe implemented features, not an available public download, tagged release or hosted service.

## What the Free version includes

- **Resize:** change dimensions of supported images locally in your browser.
- **Crop:** choose image regions without sending source image data to an IMGENGINE application server.
- **Rotate:** apply supported orientation changes in the browser.
- **Compress:** adjust supported output compression for local image processing.
- **Convert:** perform supported JPEG/PNG image conversions in the browser.

All five tool implementations were merged through the Free F1–F4 program. Their final behavior, supported formats and privacy boundary must be reconfirmed on the exact frozen source and live static Cloudflare deployment before these claims appear on a public site.

## Hosting and privacy promise (subject to verification)

The intended Free distribution is a static, browser-local Nuxt build on Cloudflare Pages, with **no IMGENGINE server-side image processing**, account, telemetry of image bytes, Rust/C service or paid entitlement. Asset delivery and hosting request metadata are separate from IMGENGINE image processing and must be addressed accurately in the approved privacy notice.

## Verification and release evidence — not complete

**Release tag:** `free-v1.0.0` — NOT CREATED
**GitHub Release:** NOT PUBLISHED
**Cloudflare Pages public URL:** UNKNOWN / NOT VERIFIED
**Frozen source commit:** NOT APPROVED
**Immutable static artifact digest:** NOT RECORDED
**Dependency audit:** BLOCKED — two moderate transitive findings pending source-lockfile remediation
**Public notices and detached publication attestation:** NOT APPROVED
**Provider HTTPS/CSP/privacy/rollback and owner financial/launch checks:** NOT RUN

See the [Cloudflare operator playbook](CLOUDFLARE_PAGES_RELEASE_PLAYBOOK.md) and [release evidence matrix](evidence-matrix.md). Change this file to final release notes **only after** the full release check and an explicit owner go/no-go.
