# IMGENGINE Free v1.0.0 — Release dossier

**State: PLANNED / NOT RELEASED.** This dossier is a planning artifact proposed under `F5-RELEASE-DOCS-01` on 2026-11-20; it is **not** a release candidate, Git tag, hosted website or claim of public availability. Release owner go/no-go: **NOT GIVEN**.

## Customer promise (Free only)

Exactly five browser-local JPEG/PNG tools: **Resize, Crop, Compress, Rotate and Convert**. No account, server image upload, server compute fallback, or paid entitlement in Free v1.0.0. Images, metadata, source/output bytes and filenames must not leave the browser through IMGENGINE application code. Serving website assets, hosting-level HTTP request metadata and clearly documented support communications are distinct data paths, to be accurately disclosed.

The complete project (C17 native libraries, Rust/Axum backend, Nuxt server routes, Docker, CI, docs) remains versioned in the **source repository**; the public Free product receives **only an audited static artifact**. The current server-backed printable-photo-layout entrypoint must be excluded from the Free public interface, with no broken/hidden upload path. The Free static build and distribution have **not yet been implemented or verified**.

## Verified feature history, not release-candidate proof

| Milestone | Recorded fact | Scope of evidence |
| --- | --- | --- |
| Five Free tools | Resize #57, Crop #59, Rotate #62, Compress #72 and Convert #76 merged | Feature implementation only |
| F5 navigation / extended browser matrix | [PR #80](https://github.com/Rofikali/imgengine/pull/80) merged as `f65fde4f8e2794a10b78f936bb0f2c035c32301f`; exact PR head `1fce2098fe4a0ff1d6bf1e7c29581c0d0892822c` passed [12 selected checks](https://github.com/Rofikali/imgengine/actions/runs/44678857538) | Existing runtime/CI, **not** static artifact release certification |
| Native security prerequisite | [PR #81](https://github.com/Rofikali/imgengine/pull/81) merged as `a50c6e1469022eb8b0d3cbe5d76e1290f5db1d45` | Native malformed PNG hardening, not hosted Free deployment |
| Free v1.0.0 release build | **NOT BUILT / NOT VERIFIED** | F5-STATIC-01 pending |
| Cloudflare production deployment | **NOT DEPLOYED** | Separate exact-artifact owner approval required |

## Version, branch and artifact

- **Intended product version:** `1.0.0` (not milestone F5/v5).
- **Proposed release freeze:** `release/free/v1.0.0`; **not created by this documentation PR**.
- **Proposed immutable version tag:** `free-v1.0.0`; **not created**.
- **Planned source of build:** reviewed/frozen Git SHA from governed `main`, after the static contract passes.
- **Planned deployed artifact:** audited `.output/public` static assets from an Ubuntu 24.04 Docker build (pending static build implementation), uploaded through **Cloudflare Pages Direct Upload** only after independent authorization.
- **Domain:** none required for initial `pages.dev` deployment; no domain registered or URL claimed by this dossier.

## Local dossier contents and release evidence

- [Release plan metadata](release-manifest.json) — intentionally contains null evidence fields and no self-referential Git SHA
- [Static-only contract](static-only-contract.md) — required architecture/negative checks
- [Evidence and sign-off matrix](evidence-matrix.md) — actual verified historical facts vs unknown production gates
- [Cloudflare deployment/rollback runbook](deployment-runbook.md) — **procedure only**, no deploy

Authoritative shared policies: [release index](../../README.md), [five-family release ADR](../../../ADR_PORTFOLIO_FIVE_PRODUCT_RELEASE_STRATEGY_2026-11-19.md), [compatibility contract](../../../IMGENGINE_FIVE_PRODUCT_RELEASE_COMPATIBILITY_MATRIX.md), [Docker-only engineering](../../../IMGENGINE_DOCKER_ONLY_DEVELOPMENT_POLICY.md), [main live milestone board](../../../../next.md), [temporary unpatched non-production security exception](../../../FRONTEND_PICOMATCH_TEMPORARY_NONPROD_RISK_ACCEPTANCE_2026-11-18.md). Avoid copying those policies.

## Decision and remaining blockers

**Current source progress (2026-11-20):** [F5-STATIC-01 PR #83](https://github.com/Rofikali/imgengine/pull/83) merged at `f1f61cfeecc565cbf3c44de57a747d1369b11000`; exact-head [run #44686554819](https://github.com/Rofikali/imgengine/actions/runs/44686554819) passed 12/12 jobs including 100/100 static four-browser tests and artifact negative checks. This verifies development implementation, **not** a frozen, signed or Cloudflare-hosted release candidate.

**F5-TRUST-01 in review:** static **draft-only** `/privacy`, `/terms` and `/support` routes are proposed for Free builds, with a labelled footer and a separate publication readiness guard. No legal operator identity, private contact channel or contractual jurisdiction is fabricated. Current public-page content says pre-launch draft, and `pnpm run verify:free-publication-ready` intentionally fails; it must not be used to approve deployment. The future final artifact/owner approval must be external and post-freeze. Hosting provider logs and analytics configuration must be checked in the actual Cloudflare project before privacy copy is finalized.

**Next:** owner-reviewed merge of the docs PR, followed by a **separate** F5-STATIC-01 implementation PR, release-level browser/privacy/a11y tests against the actual generated static site, security/advisory disposition, privacy/legal/support owner review, real Cloudflare preview/rollback readiness and exact artifact provenance. A public site requires an additional **explicit owner approval** identifying the release artifact digest and target project.

No VPS / real C/Rust host-capacity evidence is demanded for a genuinely serverless *static* Free build; it becomes mandatory for a separately approved public Rust/C/Studio/API service. A 20-GB/1-GB local Docker allocation alone is not public-host evidence.
