# IMGENGINE — Five-family artifact, versioning and release compatibility matrix

**Status:** PROPOSAL / DOCUMENTATION ONLY (2026-11-19). This is a future control matrix, not evidence of shipping SDKs, a desktop app, public API or release automation. Read [ADR-PORTFOLIO-005](ADR_PORTFOLIO_FIVE_PRODUCT_RELEASE_STRATEGY_2026-11-19.md), [existing four-product portfolio](IMGENGINE_FOUR_PRODUCT_PORTFOLIO.md), [current next.md](../next.md) and [ABI](ABI.md) first. **The latest live board and exact GitHub revisions override dated samples.**

## 1. Two levels: commercial families and technical release artifacts

| Family (portfolio parent) | Example release artifact | Independent version dimension | Target status for this proposal |
| --- | --- | --- | --- |
| `PRODUCT-FREE-01` | Browser web application | Free site/version and deployment revision | Existing in-progress product; first five features only |
| `PRODUCT-PRO-01` | Browser Pro capability/entitlement pack | Pro product version + recipe schema | Planned, not paid/released |
| `PRODUCT-STUDIO-01` | Studio operator workflows/templates | Studio version + print preset schema | Planned, not released |
| `PRODUCT-API-01` | HTTPS API specification/server | Stable HTTP API contract `v1` + independently deployed server build | Private backend foundation only; public API planned |
| `PRODUCT-SDK-01` **new candidate** | Installable C library/headers, Rust wrapper, sample apps | Native ABI + each published package SemVer | **Proposed; public installable `.a`/SDK not established** |
| `PRODUCT-DESKTOP-01` **new candidate** | Signed desktop installer, local Studio/Pro workflows as separately licensed | Desktop app/installer SemVer + target platform build | **Proposed; not implemented/released** |
| API client packages (sub-deliverable of Developer API) | TypeScript/Rust HTTP clients and examples | Client SemVer independent of API contract | Proposed, not assumed published |

**Counting policy:** five families; SDK and Desktop are **two milestone parents and potentially separate SKUs** inside the fifth portfolio family. This avoids mislabeling the HTTP SDK, public native SDK and GUI installer as one file. A package may consume the same governed native source but requires its **own** compatibility proof.

## 2. Capability and entitlement scope — hypotheses, not implemented access controls

| Capability | Free | Pro | Studio | Developer API | SDK/Desktop |
| --- | --- | --- | --- | --- | --- |
| Five basic transforms | Initial Free v1 goal | Base value | Base value | Only if in published contract | Depends on shipped package |
| Reusable recipes and bounded batch | Not in initial Free v1 | Candidate differentiator | Candidate, if owner-approved | Client/workflow-managed; paid requests separate | Platform/SKU dependent |
| Physical print sheet/presets | Outside first five-tool launch | Optional later | Core candidate | Only if a separately released API capability | Studio Desktop candidate |
| Per-operator shop workflow | No | No | Candidate | No | Desktop Studio SKU candidate |
| Hosted compute and usage billing | No | Not automatically | Separately approved when chosen | Planned metered API | Requires separate API plan |
| Offline workflow | Local browser tools where supported | Possible local workflows | Local-first research candidate | No | Desktop discovery objective |

Do not implement browser-only hard entitlements as if they were cryptographically enforceable. In all cases, forbid implicit cross-product unlimited API credits and prevent unconsented server upload.

## 3. Proposed Git and release naming examples

```text
main                                # one governed integration branch
feat/free/*                         # short-lived reviewed changes
feat/pro/*  feat/studio/*           # short-lived reviewed changes
feat/api/*  feat/sdk/*              # short-lived reviewed changes
feat/desktop/*  docs/*              # short-lived reviewed changes
release/<artifact>/vX.Y.Z           # only for actual bounded release freeze
support/<line>                      # only for supported legacy versions

free-v1.0.0                         # illustrative artifact tags only
pro-v1.0.0
studio-v1.0.0
api-contract-v1.0.0
sdk-c-v0.1.0
sdk-rust-v0.1.0
desktop-v0.1.0-beta.1
platform-v1.0.0                     # optional umbrella compatibility family
```

No claim that these branches/tags exist; no branch-per-product model. A commercial `Platform 1.x` label does not set ABI 1.x, dictate Pro/Studio versions, or force an HTTP schema breaking change. Version contracts separately for C ABI (existing 2.3 references), Rust crate/wrapper, API schema/route, Pro recipe JSON, Studio physical preset schema, entitlement/catalog, and desktop application/installer.

## 4. Compatibility matrix template — must be populated by real builds

| Published artifact | Build SHA and digest | Core ABI allowed | API/schema dependencies | Target OS/CPU | Test/CI evidence | Status |
| --- | --- | --- | --- | --- | --- | --- |
| Free web build | TBD | N/A when browser-local | Local transform/export contract | Browsers/devices tested | Browser privacy/a11y/correctness | NOT RELEASED |
| Pro browser package | TBD | N/A for purely local work | Recipe/schema version, entitlement decision | Browsers/devices tested | Batch/ZIP/cancel/security | PLANNED |
| Studio browser/host package | TBD | Record if Rust/C17 path used | Preset/physical-sheet contract | Host/printer/browser tested | Calibrated print proofs + P14 if hosted | PLANNED |
| Developer API | TBD | ABI version pinned | OpenAPI route/schema version | Linux server architecture | Tenant/billing/security/host evidence | PLANNED |
| Native C/Rust SDK | TBD | Exact ABI/header ownership | Package API + feature capability list | Linux/Windows/other proven targets only | External consumer/symbol/ABI tests | PROPOSED |
| Desktop installer | TBD | Bundled native ABI and signing | Local document/preset schemas; offline entitlement | Explicit Windows/Linux target matrix | Clean install/update/uninstall/offline/print tests | PROPOSED |
| HTTP API client SDK | TBD | No local native ABI requirement | Documented API contract range | Per-language runtime | Auth/retry/error/compatibility tests | PROPOSED |

Release metadata minimum: product, component/version, source commit, build environment, artifact digest/provenance, licensing/SBOM, public contract/ABI version, supported targets, test pass/fail/NOT_RUN, outstanding vulnerabilities, consent/data retention, rollback, support window, release approval, publishing timestamp and actual download/API endpoint. Do not fabricate future values.

### Machine-readable release manifest — **schema sketch only**

```json
{
  "artifact": "desktop",
  "version": "0.1.0-beta.1",
  "git_commit": "<full-tested-sha>",
  "artifact_sha256": "<digest-after-build>",
  "native_abi": "<verified-bundled-version>",
  "api_contract": null,
  "target": "<verified-os-and-architecture>",
  "verification": "<immutable-CI-evidence-link>",
  "release_state": "NOT_RELEASED"
}
```

This is an illustrative record, not a manifest generated from an actual build. In a real machine-readable format, use a clearly typed schema, actual pinned dependency versions, validated digest fields and a release-state transition controlled by the human owner.

## 5. Go/no-go matrix by discipline

| Discipline | Release gate (all apply when relevant) |
| --- | --- |
| Engineering levels 1–3 | Correct user outcome, deterministic tests, type/build coverage, error/resource/cancel paths |
| Staff/Principal/Distinguished | Reused architecture, compatibility policy, support/backport window, ABI ownership and no speculative dependencies |
| Security/CISSP | Threat model, malformed/huge inputs, native C-A0 disposition, supply-chain/license, scoped keys, image privacy, cross-tenant isolation, no stale high-vulnerability exception |
| Networking/CCIE/SRE | TLS/private service boundaries, real-host P14 tests, quotas, backpressure, observability, rollback and capacity on actual approved hardware |
| CA/finance | Net revenue versus cash, variable cost per successful unit, failure/egress/support cost, durable usage reconciliation, approved tax and refund rules |
| MBA/product/operations | Validated customer jobs, saved time/error reduction, price sensitivity, sales/support owner, launch readiness and channel economics |
| Release owner | Exact release candidate SHA + all required checks + signoffs, public access/publishing verification, incident rollback owner |

Free/browser-local does not need P14 merely because the repository contains Rust. Any **public server-side** Studio/API operation needs the relevant C-A0/P14 host/capacity/security approvals. A successful documentation CI job is **not** a release signoff.

## 6. SDK/Desktop discovery checklists — do not start coding by default

**SDK-00 (proposed):** inventory public symbols, ownership, allocation/free and thread-safe guarantees; confirm `.so` installation, public `.a` prerequisites, Windows toolchain/link/runtime choices, Rust crate SemVer, C header packaging, codec licensing, binary distribution rights, minimal external consumer tests, performance and portability evidence. Do not call an internal static target a distributable SDK. A small HTTP client may be published separately under API milestones only after its contract is stable.

**DESKTOP-00 (proposed):** observe operator use cases and offline failure modes; compare Tauri or other feasible shell against existing Nuxt UI and native Rust/C17 packaging; define app sandbox/permissions, print accuracy, customer-photo consent and deletion, signing/update/rollback/recovery, antivirus compatibility, reproducible installers, offline entitlements (and limits), supported Windows/Linux target list and support workload. No assumption that a desktop wrapper automatically makes server-side Nuxt code offline or that a browser owns filesystem access.

## 7. Near-term work, without changing existing priority

1. On this documentation PR, add proposed `PRODUCT-SDK-01` and `PRODUCT-DESKTOP-01` to [live board](../next.md), explicitly **LATER / NOT READY**. Do not raise parent Free scope from five tools or mark portfolio implementation complete.
2. Keep **F2c Compress -> F4 Convert -> F5 Free release review** as the current feature sequence unless verified newest GitHub state changes it.
3. Research Pro/Studio and API customers/cost evidence independently; do not silently enable payment providers, account stores, public API keys or cloud services.
4. Later, when approved, make a **separate implementation PR** for manifest/tag CI, compatibility tests, SDK packaging and desktop proof-of-concept; this document does not create that infrastructure.
5. When a future actual release occurs, update this table with **real artifact hashes, supported targets, verification links, license decision, signoff and published location**. Use `RELEASED` only for completed owner-approved customer availability.

**Reference policies:** [root AGENTS](../AGENTS.md) · [skills](../skills.md) · [technology ADR](TECHNOLOGY_AND_REPOSITORY_POLICY.md) · [commercial readiness](IMGENGINE_COMMERCIAL_READINESS_AND_UNIT_ECONOMICS.md) · [API billing/capacity](IMGENGINE_DEVELOPER_API_USAGE_BILLING_CAPACITY_BLUEPRINT.md) · [milestone hierarchy](IMGENGINE_PRODUCT_INFRA_MILESTONE_AND_CODEX_USAGE_PROTOCOL.md) · [seven-level model](SEVEN_LEVEL_ENGINEERING_OPERATING_MODEL.md).
