# imgengine

**Free v1.0.0 release checkpoint (2026-11-21):** F5-SUPPLY-05 [PR #97](https://github.com/Rofikali/imgengine/pull/97) **MERGED** into main `c1141f13c4b0b2742013a39493244c8ab9b4a1e9`. Its [PR-head full CI](https://github.com/Rofikali/imgengine/actions/runs/44876698219) and [post-merge zero-moderate source-lockfile audit](https://github.com/Rofikali/imgengine/actions/runs/44877676779) **PASSED**. **NOW:** [F5-RC-EVID-01 PR #98](https://github.com/Rofikali/imgengine/pull/98) is preparing [detached source-bound Free static candidate evidence](docs/releases/free/v1.0.0/F5_RC_EVID_01.md). Final source freeze, archive/attestation, approved legal notices, Cloudflare and tag `free-v1.0.0` remain **NOT RELEASED**.

**Free v1.0.0 Cloudflare launch preparation (2026-11-21):** A [read-only manual release preflight](.github/workflows/free-v1-release-preflight.yml), [Cloudflare Pages Direct Upload runbook](docs/releases/free/v1.0.0/CLOUDFLARE_PAGES_RELEASE_PLAYBOOK.md) and [draft release notes](docs/releases/free/v1.0.0/DRAFT_RELEASE_NOTES.md) are proposed for review. The source plan's final version tag is `free-v1.0.0`, **NOT CREATED**. Free is **NOT RELEASED**: two moderate production dependencies remain, notices are draft, detached publication attestation is unimplemented, Cloudflare is not connected/deployed. This prep must not bypass `verify:free-publication-ready` or publish from `main` automatically.

**Milestone synchronization — F5-DOCSYNC-01 merged via [PR #91](https://github.com/Rofikali/imgengine/pull/91):** GitHub Actions has generated the [review-only status snapshot](https://github.com/Rofikali/imgengine/pull/92), but the owner must enable GitHub Actions PR creation before future review PRs can be opened autonomously. F5-DOCSYNC-02 repairs the missing-PR retry path; [runbook](docs/engineering/MILESTONE_AUTOMATION.md). Durable README/AGENTS/skills content is maintained by reviewed engineering changes, not rewritten from PR titles.

**CURRENT VERIFIED MAIN (2026-11-20 UTC):** [PR #89](https://github.com/Rofikali/imgengine/pull/89) **MERGED** as `6c9e5ff7846d6d4e3b4ec63d62b19ac0d3af3008`, [post-merge main CI](https://github.com/Rofikali/imgengine/actions/runs/44771824270) **SUCCESS**. The zero-moderate release-candidate audit is now implemented, but the source lockfile still resolves moderate-advisory `glob@13.0.7` / `minimatch@10.2.6`. #88 merged as `ec213cfc1814c04cd4e2ac693eaf28d9f0015186`; its latest [12/12 CI](https://github.com/Rofikali/imgengine/actions/runs/44742258560) and [temporary patched-major trial](https://github.com/Rofikali/imgengine/actions/runs/44742258511) passed 84 local + 200 static browser tests, but did **not** patch production dependencies. **NEXT READY: F5-SUPPLY-05 — package-manager-generated production dependency remediation in approved Ubuntu 24.04 Docker, full exact-head CI and four-browser/privacy verification. Free v1.0.0 NOT RELEASED.**

**Historical F5-HOST-HDR-01 (#85):** Static `_headers` and script-hash CSP were merged and tested locally; live Cloudflare TLS, rollback and owner go-live remain independent gates.

**Free v1 F5-TRUST-01 draft notices (2026-11-20, #84 merged):** The static-only Free `/privacy`, `/terms`, `/support` pages and fail-closed publication preflight are merged. Operator/private contact, applicable legal review, Cloudflare analytics/TLS/security, dependency audit and owner approval remain outstanding; no live site or release tag is authorized.

**F5 release documentation (owner-requested 2026-11-20, review candidate):** [versioned Free v1.0.0 dossier](docs/releases/free/v1.0.0/README.md) and [shared release-documentation templates](docs/releases/README.md) define a **static-only** Cloudflare Pages Direct Upload target. This is a documentation/governance milestone, **not** a static build, release tag, product launch or authorization to deploy. All C/Rust/Nuxt/server source remains in Git; Free public assets must exclude executable server/runtime functionality.

**CI-SCOPE-02 conditional PR checks (review candidate):** [Versioned change-impact matrix and fail-closed verification gate](docs/CI_SCOPE_02_SELECTION_GATE.md). Frontend-only PRs can avoid unrelated native/FFI jobs while retaining Nuxt and all four browser projects; native, shared/unknown, CI or security changes run the full matrix, as do pushes to main. CI job selection is secure only with the always-running aggregate gate **and owner-configured required check protection**; no protection/account settings are silently changed.

**F2c / F4 reconciliation (2026-11-19):** [Compress PR #72](https://github.com/Rofikali/imgengine/pull/72) and [Convert PR #76](https://github.com/Rofikali/imgengine/pull/76) are **MERGED**; #76 merge `9216306c93dd21c5011992b11ed68dc1051ac12d` had [9/9 required CI jobs PASS at exact head](https://github.com/Rofikali/imgengine/actions/runs/44495703308). Free now has all five tools merged, but is **not released**; F5 remains a separate owner-approved release review. [Live next.md](next.md) is authoritative.

**Frontend Excellence F6 (owner-requested plan, 2026-11-19):** [post-Free frontend strategy](docs/IMGENGINE_FRONTEND_EXCELLENCE_PROGRAM_F6.md) + [F6 acceptance and verification matrix](docs/IMGENGINE_FRONTEND_EXCELLENCE_F6_ACCEPTANCE_MATRIX.md) cover navbar, design system, accessible five-tool UX, SEO, browser speed and memory, measured Worker/Rust-Wasm decisions, dev tooling, security and CA/MBA/product gates. **F6 is a proposal, not implemented or released.** All Free v1 F5 safety/quality gates remain mandatory; [next.md](next.md) retains F5 release review as the current priority.

**Five-product platform extension (documentation proposal, 2026-11-19):** [ADR-PORTFOLIO-005: shared core, five product families, independent branches/releases](docs/ADR_PORTFOLIO_FIVE_PRODUCT_RELEASE_STRATEGY_2026-11-19.md) and [SDK/Desktop + compatibility/release matrix](docs/IMGENGINE_FIVE_PRODUCT_RELEASE_COMPATIBILITY_MATRIX.md) extend — not replace — the [existing four-product portfolio](docs/IMGENGINE_FOUR_PRODUCT_PORTFOLIO.md). SDK/Desktop is a **proposed fifth family**, not a released SDK or installer. No permanent Free/Pro/Studio/API/SDK branches, source refactors, tags, billing or production changes are authorized. [Live next.md](next.md) prioritizes F5 Free release readiness; no public launch is authorized.

**Developer/Codex efficiency:** [Six-rule local-first Codex VS Code guide](docs/CODEX_VSCODE_LOW_CONTEXT_SIX_RULES.md) — use local Git for code and tests; read the small [live milestone board](next.md), not full archives; use remote GitHub integration only for concise PR/CI/merge facts; keep full success logs outside AI context; choose model/effort appropriate to task when supported; compare allowance privately after reset. This is a **workflow recommendation**, not automated model settings, skipped CI checks or a measured token-saving promise.

**2026-11-19 milestone reconciliation:** Free v1 has **5/5 individual tools MERGED**: Resize [#57](https://github.com/Rofikali/imgengine/pull/57), Crop [#59](https://github.com/Rofikali/imgengine/pull/59), Rotate [#62](https://github.com/Rofikali/imgengine/pull/62), Compress [#72](https://github.com/Rofikali/imgengine/pull/72), Convert [#76](https://github.com/Rofikali/imgengine/pull/76). Feature merges are **not** a product release. [PR #65](https://github.com/Rofikali/imgengine/pull/65) MERGED a **temporary, non-production frontend-only picomatch audit waiver** and three native Ubuntu 24.04 runner labels: [security decision and 2026-12-02 expiry](docs/FRONTEND_PICOMATCH_TEMPORARY_NONPROD_RISK_ACCEPTANCE_2026-11-18.md). **picomatch remains vulnerable**; production is not approved. [PR #63](https://github.com/Rofikali/imgengine/pull/63) for a real reusable Codex Skill/lean AGENTS is **DRAFT/UNMERGED** and is not installed on main. [Live next.md](next.md) + current GitHub state outrank dated README status; Pro/Studio/API release/business/security gates remain independent.

**Product direction (2026-11-16):** Owner-selected [IMGENGINE Free, Pro, Studio and Developer API portfolio](docs/IMGENGINE_FOUR_PRODUCT_PORTFOLIO.md). **Free first** ships only **Crop, Resize, Compress, Rotate and Convert**, browser-local where supported and no image upload by default. Pro (creator recipes/batch), Studio (print/operator workflows) and Developer API (future metered Rust/C17 service) are **planned product lines, not live subscriptions or APIs**. See [current F1 and later NOW/NEXT/LATER milestones](next.md), [future API billing/capacity design](docs/IMGENGINE_DEVELOPER_API_USAGE_BILLING_CAPACITY_BLUEPRINT.md) and [business/security/release gates](docs/IMGENGINE_COMMERCIAL_READINESS_AND_UNIT_ECONOMICS.md). F0 strategy is merged; all five Free tool implementations are merged, while paid tiers and public release still need independent approval.

**New repository-root command menu (review proposal):** Run `make help` and `make self-check` from the project root in Ubuntu 24.04/Linux for safe C17/Rust/Nuxt/Docker build and test targets. See [Root Makefile Engineering Guide](docs/ROOT_MAKEFILE_ENGINEERING_GUIDE.md) for focused commands, prerequisites, full-verifier opt-in, log budgeting and risks. Existing `imgengine/Makefile` remains native-only; `make` at root is deliberately help-only. This does not replace seven-job GitHub CI or production release gates.

> **Technology decision (2026-11-14):** All **new** IMGENGINE backend code uses **Rust/Axum**, with C17 native execution, safe Rust FFI and the existing Nuxt/Vue/TypeScript frontend. **No new Python/FastAPI/Flask/Django/DRF, Celery, Redis, PostgreSQL, or Python tooling.** The tracked Python-based backend and older helper scripts are legacy/existing and retained only until verified migration or tooling replacement. The current Rust backend lives in `backend/`; `imgengine-saas/backend/` is **legacy Python**. Read [approved technology/folder ADR](docs/TECHNOLOGY_AND_REPOSITORY_POLICY.md) before proposing new infrastructure or folder moves.

**Infrastructure reviewers:** [infra AGENTS](infra/AGENTS.md), [infra README](infra/README.md), and [2026-11-15 infrastructure audit](docs/INFRA_SECURITY_OPERATIONS_AUDIT_2026-11-15.md) define current Caddy/Docker/secret/observability and VPS-release evidence boundaries. This does not authorize production changes.

**C17 native-core deep audit:** [Principal source/LLD/design patterns/DSA/security review](docs/C17_CORE_PRINCIPAL_SOURCE_LLD_DSA_SECURITY_AUDIT_2026-11-15.md) and [native core README](imgengine/README.md) identify implemented ABI/algorithms and high-priority concurrent SPSC consumer/metrics and resource-budget questions. These require exact-revision proofs, **not** broad rewrites or unverified nanosecond claims.

**Starting Astra or another engineering agent?** Read [Astra engineering start guide](docs/ASTRA_ENGINEERING_START_HERE.md) before choosing frontend, Rust backend or C17 native work. [next.md](next.md) is the living status board; current scoped [Rust AGENTS](backend/AGENTS.md), [native AGENTS](imgengine/AGENTS.md) and [frontend AGENTS](imgengine-saas/frontend/AGENTS.md) define boundaries. Merged PR #37 relocated Rust to `backend/` with [seven passing CI jobs](https://github.com/Rofikali/imgengine/actions/runs/43618598771), **not** a production release.

## Current application path

The verified synchronous, ephemeral application path is:

```text
Browser -> Nuxt server proxy -> Rust/Axum API
        -> RequestLifecycleService -> bounded Rust admission
        -> safe Rust FFI -> libimgengine C ABI -> C scheduler -> C engine
        -> completed JPEG response -> cleanup
```

Implemented Rust endpoints cover basic render, FIT-only layout render, and the
physical-sheet render slice. Rust owns authentication, validation, lifecycle,
bounded admission, response abandonment, graceful draining, and cleanup; C
retains native scheduling and image execution.

The older FastAPI/Celery/Redis/PostgreSQL application remains in the repository
as a legacy/rollback path. It is not the target first-MVP deployment path and
has not been silently deleted or declared retired. See
[`docs/api.md`](docs/api.md), [`docs/architecture.md`](docs/architecture.md),
and [`docs/MVP_RELEASE_CHECKLIST.md`](docs/MVP_RELEASE_CHECKLIST.md) for the
current contract, migration status, and open release gates.

The scoped frontend and retained Python backend engineering records are indexed in the [frontend/legacy backend audit](docs/FRONTEND_LEGACY_BACKEND_AUDIT_2026-11-14.md). Follow the respective `imgengine-saas/frontend/AGENTS.md` and `imgengine-saas/backend/AGENTS.md`; the active Rust implementation follows `backend/AGENTS.md`.

For a local Windows 11/Docker Desktop rehearsal of the current Rust/Nuxt P12
candidate, use [`docs/LOCAL_DOCKER_REHEARSAL.md`](docs/LOCAL_DOCKER_REHEARSAL.md).
It is local integration evidence only, not actual-VPS or production evidence.

> A native, high-performance image layout engine for print-ready photo sheets — built in C with a stable public API and a Rust-backed application path.

**License/provenance:** No root `LICENSE` file was present in the audited Git tree. The earlier MIT badge was removed pending an explicit, owner-approved license and third-party provenance decision; this statement grants no license. See [project status](docs/PROJECT_STATUS_AND_DECISION_REGISTER.md).
[![Language](https://img.shields.io/badge/core-C17-00599C.svg)](https://en.wikipedia.org/wiki/C17_(C_standard_revision))
[![Build](https://img.shields.io/badge/build-CMake-064F8C.svg)](https://cmake.org/)
[![Focus](https://img.shields.io/badge/focus-systems%20%2B%20image%20processing-111827.svg)](#)

## Why imgengine?

`imgengine` is designed around a simple engineering boundary:

**the native engine owns image processing; the application layer owns product concerns.**

The native core handles decoding, geometry, layout, rendering and encoding. Authentication, tenant policy, uploads, persistence, retries and billing belong outside the engine.

That separation makes the project useful both as a native library/CLI and as the processing core of a future SaaS product.

## What it does

- Decode supported source images
- Convert physical print dimensions (cm + DPI) into pixel dimensions
- `FIT` or `FILL` image placement
- Grid-based sheet composition
- Borders, bleed and crop marks
- PNG/JPEG/PDF output paths
- CLI and public C API
- RGB24/raw ingress for already-decoded frames
- SIMD-accelerated paths with portable fallbacks
- Bounded resource validation for untrusted input

## Architecture

```text
                         Product / SaaS Layer
                                  │
                         ┌────────▼────────┐
                         │ Rust Backend     │
                         │ API / policy /   │
                         │ lifecycle /      │
                         │ observability    │
                         └────────┬────────┘
                                  │ stable FFI
                         ┌────────▼────────┐
                         │ imgengine Core   │
                         │ C17             │
                         ├─────────────────┤
                         │ decode          │
                         │ resize / crop   │
                         │ layout          │
                         │ render          │
                         │ encode          │
                         │ SIMD dispatch   │
                         └─────────────────┘
                                  │
                    ┌─────────────┴─────────────┐
                    ▼                           ▼
                    JPEG                      PDF
```

The native specification defines the processing contract and acceptance matrix in [`docs/ENGINE_SPEC.md`](docs/ENGINE_SPEC.md).

## Processing pipeline

```text
Input
  │
  ▼
Decode / validate
  │
  ▼
FIT or FILL
  │
  ▼
Grid placement
  │
  ▼
Border
  │
  ▼
Bleed + crop marks
  │
  ▼
Encode
  │
  ▼
Print-ready output
```

## Native API

The public C API exposes an explicit engine lifecycle and job execution boundary, including file-to-file, raw encoded output and RGB24 ingress paths.

The project treats ABI stability as an engineering contract: public symbols, ownership, lifecycle, threading and release semantics are tested rather than left implicit.

## Build

### Requirements

- CMake 3.16+
- GCC or Clang
- Linux (primary deployment target)
- NASM for JPEG SIMD dependency builds where required

```bash
git clone https://github.com/Rofikali/imgengine.git
cd imgengine

cmake -S . -B build
cmake --build build -j
```

Run the CLI:

```bash
./build/imgengine_cli --help
```

## Example

```bash
./build/imgengine_cli \
  --input photo.jpg \
  --output sheet.jpg \
  --cols 6 \
  --rows 6 \
  --width 4.5 \
  --height 3.5 \
  --dpi 300 \
  --bleed 10 \
  --crop-mark 20
```

## Engineering quality

Correctness is treated as a first-class feature. The native acceptance matrix covers:

- deterministic geometry and layout fixtures
- FIT vs FILL behavior
- border, bleed and crop-mark behavior
- layout-boundary/property testing
- malformed-input and unsafe-dimension handling
- SIMD vs scalar equivalence
- ASan/UBSan validation
- bounded fuzzing of untrusted parsing/decoder paths
- memory and arithmetic boundary tests

See [`docs/ENGINE_SPEC.md`](docs/ENGINE_SPEC.md) for the detailed contract.

**F2c Compress CI acceptance (planned):** [12-category frontend quality-gate specification](docs/FRONTEND_F2C_CI_QUALITY_GATES.md) covers browser privacy, metadata/EXIF, a11y, Worker cleanup, cross-browser support, performance and regressions. Existing GitHub CI runs seven jobs and currently covers frontend unit/typecheck/build, **not** the full twelve F2c categories or F2c Playwright tests. Specifications are not tests; follow [next.md](next.md) for actual implementation, PR and CI status.

## Project direction

The project is evolving from a native image-processing engine toward a low-cost, secure SaaS architecture:

```text
C native engine
      ↓
stable C ABI
      ↓
Rust FFI boundary
      ↓
Rust service / lifecycle orchestration
      ↓
Nuxt + TypeScript frontend
```

The design deliberately avoids making the native engine responsible for authentication, billing, persistence or product policy.

## Repository structure

```text
imgengine/
├── include/          # public/internal C interfaces
├── src/              # native implementation
├── tests/             # native correctness and security tests
├── docs/              # specifications and verification evidence
├── imgengine-saas/    # application/SaaS layer
├── CMakeLists.txt
└── README.md
```

## Status

The native ABI, safe Rust FFI, Rust lifecycle/admission boundary, Axum adapter,
and initial Nuxt server proxies are implemented and verified in Ubuntu 24.04
Docker CI. Production readiness, actual-VPS evidence, canary operation, and
legacy-stack retirement remain open release decisions. A passing repository or
Docker gate is not a production-readiness claim.

## License

MIT
