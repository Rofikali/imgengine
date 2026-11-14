# IMGENGINE — Project inventory and decision register

**Snapshot:** 2026-11-14, `main` at `14d1a8683f492ec2c1128a2014442cc85cd832e9`. This is a living index, not a release certificate. Status is based on repository contracts, tracked code/PRs, and recorded verification, not fresh end-to-end execution.

## Status vocabulary
- **Implemented:** tracked code exists; may not have complete evidence.
- **Validated (recorded):** tests or review evidence exists for a specified revision/environment; not necessarily reproduced today.
- **Partial:** a lower layer exists, but the customer-visible workflow is incomplete.
- **Planned:** proposal only, no implementation claim.
- **Blocked:** prerequisite or operational evidence missing.
- **Legacy:** retained during controlled migration; do not delete without parity and rollback.
- **Historical:** superseded design/evidence; retain for traceability.

## Built and/or validated so far

| Area | Status | Source / evidence | Remaining work |
| --- | --- | --- | --- |
| C native engine, scalar/SIMD, TurboJPEG, scheduler | Implemented; recorded native tests | `imgengine/`, `docs/ENGINE_SPEC.md`, `docs/SCHEDULER_CHARACTERIZATION_REPORT.md` | Preserve correctness, measure workload-specific performance |
| C17 shipping baseline and C11 compatibility | Validated (recorded) in Ubuntu 24.04 Docker | PR #29, `docs/M0_C17_PORTABILITY_AND_WASM_ADR.md` | Keep C11 regression and ABI gates on native changes |
| Public C ABI v1 and additive v2.0–2.3 | Implemented; recorded consumer/export/layout checks | `docs/ABI.md`, `docs/P13_INTERACTIVE_CROP_CONTRACT.md` | Maintain compatibility on future native changes |
| Rust FFI, supervisor, bounded admission, Axum | Implemented; recorded integration tests | `docs/RUST_FFI.md`, `docs/RUST_SUPERVISOR.md`, `docs/api.md` | Production operational evidence |
| P8 FIT layout | Implemented | `docs/P8_PRODUCT_CONTRACT.md` | Keep unchanged while adding new tools |
| P11 geometry planning | Implemented | PR #13, `docs/P12_PHYSICAL_SHEET_CONTRACT.md` | Preserve deterministic parity |
| P12 physical JPEG sheet and Nuxt UI | Implemented; local workflow demonstrated, not production | `docs/P12_PHYSICAL_SHEET_CONTRACT.md`, PRs #16–19, #18 | VPS/resource evidence, usability release gates |
| P13 normalized native crop, ABI 2.3 | **Partial** | PRs #24–26, `docs/P13_INTERACTIVE_CROP_CONTRACT.md` | Rust/Axum/Nuxt transport, accessible UI, end-to-end parity |
| Nuxt 4 frontend and typed proxy | Implemented | `imgengine-saas/frontend/`, frontend `AGENTS.md` | Crop UI, browser-local engine, broader tools |
| Docker / Caddy private candidate | Implemented local candidate; production blocked | `docs/P12_DEPLOYMENT.md`, `docs/LOCAL_DOCKER_REHEARSAL.md` | Real VPS, TLS/DNS/firewall, rollback, monitoring |
| Native sanitizer/fuzz/ABI verification | Validated (recorded), not run for this documentation change | `imgengine/scripts/verify_linux.sh`, PR #29 | Re-run on relevant native changes and release gates |
| P7.5 worktree changes | **Separate local dirty worktree reported by operator** | `docs/P7_5_BRANCH_CONSOLIDATION.md` records older integrated P7.5 history | Reconcile local uncommitted work separately; remote GitHub cannot inventory local dirt |
| Python/FastAPI/Celery/Redis/Postgres path | **Legacy** | `AGENTS.md`, `docs/migration.md` | Retire only after parity, rollback window and explicit decision |
| Browser-first Rust/Wasm processing | **Planned, not implemented** | M0 ADR | Architecture review, bounded proof, browser tests |
| Tailwind | **Not established as implemented** in the 2026-11-14 audit | Audit snapshot / frontend files | Choose deliberately; avoid asserting installed |
| Paid tenancy, quotas, metering, B2B | **Planned** | `docs/PRD.md`, roadmap | Demand validation, privacy and payment architecture |

## Unfinished, interrupted or deferred work
1. **P13/M1 interactive crop:** native ABI complete, transport/UI incomplete. Next implementation boundary.
2. **P12/P14 public release:** local candidate exists; actual-VPS RAM/CPU/disk, TLS, monitoring, canary and rollback evidence missing. Do not claim production-ready.
3. **P7.5 separate worktree:** preserve as-is; inspect locally before any reconciliation. A remote branch or PR does not prove its uncommitted state.
4. **Browser-local processing:** no first-party Worker/Wasm engine. Transitive Wasm dependencies are not product functionality.
5. **Image product catalog:** crop, resize, compression, conversion, signature tools, presets, batch, professional editor, photo sheets in browser are a proposed expansion, not shipped features.
6. **Legacy retirement:** explicitly deferred until replacement parity and rollback evidence.
7. **Nuxt dependency advisories:** verify current advisory state and record accountable disposition; historical audit mentioned moderate/low findings.
8. **Branch protection:** at last repository inspection `main` was unprotected; establish required checks/reviews according to repository capabilities.

## Decision log
| ID | Decision | Reason / revisit |
| --- | --- | --- |
| D-001 | Keep C scheduler and native processing plane | Existing investment and characterization; revisit only on measured evidence |
| D-002 | Shipping C17, C11 regression, isolated C23 experiments | PR #29; preserve ABI |
| D-003 | Browser-first local processing is a product goal, not a shipped capability | Minimize uploads and server compute; prove actual browser resource envelope |
| D-004 | Start Wasm architecture review after crop M1, before production Wasm code | Avoid parallel architecture rewrite and premature codec port |
| D-005 | Prefer browser-native codecs plus bounded Worker geometry prototype first | M0 ADR; compare parity, licensing, memory and bundle size |
| D-006 | No automatic server fallback for sensitive images | Require explicit consent and clear privacy disclosure |
| D-007 | Change-scoped verification locally; full relevant gates on integration/release | Save developer time/tokens without sacrificing release evidence |
| D-008 | No blanket promise of 30 ms, 60 FPS, 1 GiB VPS capacity, passport compliance, or guaranteed target-KB quality | Require workload-specific evidence |
| D-009 | No production or public release without explicit human go/no-go | Security, legal, cost and operational risk |

## Keeping this register current
For each PR: update relevant status, source links, owner/decision, blockers, next action, and exact revision tested. Do not mark an item validated from an AI summary alone. Historical records remain immutable; corrections should be dated. Cross-check `docs/PRODUCT_ROADMAP_BROWSER_FIRST.md` and `docs/CHANGE_SCOPED_VERIFICATION.md`.
