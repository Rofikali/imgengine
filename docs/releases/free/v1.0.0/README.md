# IMGENGINE Free v1.0.0 — Release dossier

**CURRENT VERIFIED MAIN (2026-11-20 UTC):** [PR #89](https://github.com/Rofikali/imgengine/pull/89) **MERGED** as `6c9e5ff7846d6d4e3b4ec63d62b19ac0d3af3008`, with [post-merge main CI SUCCESS](https://github.com/Rofikali/imgengine/actions/runs/44771824270). Strict zero-moderate release-only audit is implemented, not a dependency fix. The tracked frontend lockfile retains the moderate `glob@13.0.7` / `minimatch@10.2.6` findings; the #88 patched-major trial was disposable. **NEXT: F5-SUPPLY-05 permanent upstream-compatible package-manager-generated lockfile patch, exact-head Docker/CI/browser/security proof. Free v1.0.0 NOT RELEASED.**


**Historical F5-SUPPLY-03 diagnostics before #88 merged (2026-11-20):** Later baseline [CI 44740853471](https://github.com/Rofikali/imgengine/actions/runs/44740853471) passed all 12 jobs, but later [disposable patched-major trial 44740853518](https://github.com/Rofikali/imgengine/actions/runs/44740853518) failed one Firefox `DOMContentLoaded` navigation timeout despite timely static HTTP responses. The earlier dual-success revision remains historical evidence, not approval of the next HEAD. A 50 one-test-per-process Firefox CI experiment is under independent review; standard CI and probe must both pass again. The two moderate advisories remain in production lockfile and Free v1.0.0 remains **NOT RELEASED**.

**Historical #88 review candidate, before merge (2026-11-20):** Revision `25f67a65f7c3b13a0b99267da6938142e04d6ff4` passed [12/12 required CI](https://github.com/Rofikali/imgengine/actions/runs/44739663971) on the unchanged production lockfile (84/84 normal + 200/200 Free static browser tests; ten short-lived Firefox test shards). The separate [disposable compatibility trial](https://github.com/Rofikali/imgengine/actions/runs/44739664043) passed with zero audit findings under trial `glob@14.0.2`/`minimatch@11.0.1` and the same full browser test counts. This is **not** a merged production dependency fix: main still carries the two moderate advisories; intermittent Firefox root cause remains unproven. **Free v1.0.0 remains NOT RELEASED**, with publication guard, legal/operator and Cloudflare gates blocked.

**F5-TRUST-01 merged (2026-11-20):** [PR #84](https://github.com/Rofikali/imgengine/pull/84) merged as `cffe893c03bed4203e75d03703fbb4c5238c1ebb`; exact head `4a7a9698d6856053d31a953304019ceff74cf945` passed [12/12 CI](https://github.com/Rofikali/imgengine/actions/runs/44704589708) with 108/108 Free static and 84/84 normal browser tests. Draft /privacy, /terms and /support and fail-closed publication guard are merged; **none are final owner/legal notices**. No site was deployed. Production dependency audit on that PR: **critical=0, high=0, moderate=2**. The two moderate advisories require identification and applicable reviewer disposition.

**F5-HOST-HDR-01 MERGED (2026-11-20):** [PR #85](https://github.com/Rofikali/imgengine/pull/85) merged at `44c3b0f28cb916e8dbb80e9215212785187c9961`; exact head `870f255ba55363194e39af94104aac2d4d42805a` passed [12/12 CI](https://github.com/Rofikali/imgengine/actions/runs/44722531833) including 200/200 Free static and 84/84 normal browser checks. Compiled `_headers`, script-hash CSP and strict static artifact checks work in local simulation. **Actual Cloudflare response/TLS, hosting privacy, legal owner notices and release approval are NOT VERIFIED.**

**F5-SUPPLY-02 MERGED (2026-11-20):** [PR #87](https://github.com/Rofikali/imgengine/pull/87) merged as `6677029e504ef6cb122fc230bfae010c0edb138b`; exact head passed [12/12 CI](https://github.com/Rofikali/imgengine/actions/runs/44727385277), including 84/84 normal and 200/200 Free static browser tests. The withdrawn picomatch CI waiver is gone; high/critical gate remains strict. Two moderate transitive advisories are still open, with a separate disposable compatibility experiment proposed under F5-SUPPLY-03.

**State: F5 FEATURES AND SECURITY IMPLEMENTATION MERGED / FREE V1.0.0 NOT RELEASED.** This dossier is a tracked release plan, **not** a frozen release artifact, Git tag, Cloudflare deployment or public launch. Release owner go/no-go: **NOT GIVEN**.

## Customer promise (Free only)

Exactly five browser-local JPEG/PNG tools: **Resize, Crop, Compress, Rotate and Convert**. No account, server image upload, server compute fallback, or paid entitlement in Free v1.0.0. Images, metadata, source/output bytes and filenames must not leave the browser through IMGENGINE application code. Serving website assets, hosting-level HTTP request metadata and clearly documented support communications are distinct data paths, to be accurately disclosed.

The complete project (C17 native libraries, Rust/Axum backend, Nuxt server routes, Docker, CI, docs) remains versioned in the **source repository**; the public Free product receives **only an audited static artifact**. The current server-backed printable-photo-layout entrypoint must be excluded from the Free public interface, with no broken/hidden upload path. The Free static build and development-output checks are **implemented and CI verified**; the frozen production candidate, provider response/hosting configuration and actual distribution are **not verified or authorized**.

## Verified feature history, not release-candidate proof

| Milestone | Recorded fact | Scope of evidence |
| --- | --- | --- |
| Five Free tools | Resize #57, Crop #59, Rotate #62, Compress #72 and Convert #76 merged | Feature implementation only |
| F5 navigation / extended browser matrix | [PR #80](https://github.com/Rofikali/imgengine/pull/80) merged as `f65fde4f8e2794a10b78f936bb0f2c035c32301f`; exact PR head `1fce2098fe4a0ff1d6bf1e7c29581c0d0892822c` passed [12 selected checks](https://github.com/Rofikali/imgengine/actions/runs/44678857538) | Existing runtime/CI, **not** static artifact release certification |
| Native security prerequisite | [PR #81](https://github.com/Rofikali/imgengine/pull/81) merged as `a50c6e1469022eb8b0d3cbe5d76e1290f5db1d45` | Native malformed PNG hardening, not hosted Free deployment |
| Free v1.0.0 frozen release artifact | **NOT FROZEN / NOT VERIFIED** | Local static generator and CSP are merged/CI verified; immutable release SHA/digest and hosting evidence still pending |
| Cloudflare production deployment | **NOT DEPLOYED** | Separate exact-artifact owner approval required |

## Version, branch and artifact

- **Intended product version:** `1.0.0` (not milestone F5/v5).
- **Proposed release freeze:** `release/free/v1.0.0`; **not created by this documentation PR**.
- **Proposed immutable version tag:** `free-v1.0.0`; **not created**.
- **Planned source of build:** reviewed/frozen Git SHA from governed `main`, after the static contract passes.
- **Planned deployed artifact:** audited `.output/public` static assets from an Ubuntu 24.04 Docker build (implementation merged; release-candidate execution and approval pending), uploaded through **Cloudflare Pages Direct Upload** only after independent authorization.
- **Domain:** none required for initial `pages.dev` deployment; no domain registered or URL claimed by this dossier.

## Local dossier contents and release evidence

- [Release plan metadata](release-manifest.json) — intentionally contains null evidence fields and no self-referential Git SHA
- [Static-only contract](static-only-contract.md) — required architecture/negative checks
- [Evidence and sign-off matrix](evidence-matrix.md) — actual verified historical facts vs unknown production gates
- [F5-SUPPLY-05 dependency-remediation handoff](dependency-remediation-handoff.md) — permanent dependency patch still NOT RUN; no permission to weaken release gates
- [Dependency advisory review (2026-11-20)](dependency-advisory-review-2026-11-20.md) — exact audit IDs, reported patches and unresolved applicability/compatibility gates
- [F5-SUPPLY-04 release-candidate audit gate](release-dependency-security-gate.md) — a separate **no moderate/high/critical** dependency threshold for release preflight. Ordinary development CI still logs moderate findings, and a green audit alone never authorizes hosting/publication.
- [Cloudflare deployment/rollback runbook](deployment-runbook.md) — **procedure only**, no deploy

Authoritative shared policies: [release index](../../README.md), [five-family release ADR](../../../ADR_PORTFOLIO_FIVE_PRODUCT_RELEASE_STRATEGY_2026-11-19.md), [compatibility contract](../../../IMGENGINE_FIVE_PRODUCT_RELEASE_COMPATIBILITY_MATRIX.md), [Docker-only engineering](../../../IMGENGINE_DOCKER_ONLY_DEVELOPMENT_POLICY.md), [main live milestone board](../../../../next.md), [historical, retired non-production security exception](../../../FRONTEND_PICOMATCH_TEMPORARY_NONPROD_RISK_ACCEPTANCE_2026-11-18.md). Avoid copying those policies.

## Decision and remaining blockers

**Current source progress (2026-11-20):** [F5-STATIC-01 PR #83](https://github.com/Rofikali/imgengine/pull/83) merged at `f1f61cfeecc565cbf3c44de57a747d1369b11000`; exact-head [run #44686554819](https://github.com/Rofikali/imgengine/actions/runs/44686554819) passed 12/12 jobs including 100/100 static four-browser tests and artifact negative checks. This verifies development implementation, **not** a frozen, signed or Cloudflare-hosted release candidate.

**F5-TRUST-01 merged (drafts only):** static **draft-only** `/privacy`, `/terms` and `/support` routes are proposed for Free builds, with a labelled footer and a separate publication readiness guard. No legal operator identity, private contact channel or contractual jurisdiction is fabricated. Current public-page content says pre-launch draft, and `pnpm run verify:free-publication-ready` intentionally fails; it must not be used to approve deployment. The future final artifact/owner approval must be external and post-freeze. Hosting provider logs and analytics configuration must be checked in the actual Cloudflare project before privacy copy is finalized.

**Next:** F5-SUPPLY-03 disposable compatibility probe and separate owner-reviewed dependency remediation/disposition, followed by independently frozen-artifact browser/privacy/a11y evidence, reviewed privacy/legal/support contacts and notices, Cloudflare hosting/rollback verification and named owner release approval. No cloud upload or tag follows automatically from any development PR. A public site requires an additional **explicit owner approval** identifying the release artifact digest and target project.

No VPS / real C/Rust host-capacity evidence is demanded for a genuinely serverless *static* Free build; it becomes mandatory for a separately approved public Rust/C/Studio/API service. A 20-GB/1-GB local Docker allocation alone is not public-host evidence.
