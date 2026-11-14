# IMGENGINE — Project status and unfinished-work register

**Snapshot:** 2026-11-14. **Baseline:** `main` at `14d1a8683f492ec2c1128a2014442cc85cd832e9` (merged M0 PR #29).  
**Purpose:** One navigable inventory of started, partial, deferred, and proposed work. This is an *evidence index*, not a claim that every historical file was revalidated. Recheck each item against the current source and exact-head CI before changing its status.

## Status vocabulary

- **Implemented:** relevant code/configuration exists; release behavior not necessarily verified.
- **Validated (specified environment):** tests/evidence recorded for a named revision and environment.
- **Partial / in progress:** some work exists; the customer-visible or deployment contract is incomplete.
- **Planned / proposed:** design or intent; code is not established.
- **Legacy retained:** existing historical path; must not be deleted without contract parity, rollback, and explicit approval.
- **Blocked by evidence:** release cannot be claimed until the evidence gate is closed.

## Evidence-backed inventory

| Workstream | Current state | Next dependency / action | Source of truth |
| --- | --- | --- | --- |
| Portable native C core, SIMD/AVX2, TurboJPEG, native scheduler | Implemented; native regression/security/performance evidence recorded | Keep scheduler; benchmark changes before optimization claims | `imgengine/`, `docs/ENGINE_SPEC.md`, `docs/SCHEDULER_CHARACTERIZATION_REPORT.md` |
| C17 shipping profile, C11 compatibility, C23 isolation | M0 **merged** in PR #29; strict C17 and a whole-core C11 regression documented; Wasm deferred | Preserve compatibility matrix; maintain ABI tests | `docs/M0_C17_PORTABILITY_AND_WASM_ADR.md`, `imgengine/CMakeLists.txt` |
| C ABI v1 plus additive v2.0–2.3 | Implemented, consumer/export/layout regression evidence recorded | Freeze compatibility; don't rewrite existing requests | `docs/ABI.md`, `docs/P13_INTERACTIVE_CROP_CONTRACT.md` |
| Rust FFI, lifecycle, bounded admission, Axum | Implemented and integration-tested in recorded local/Linux environments | Continue validated cross-boundary changes in small slices | `docs/RUST_FFI.md`, `docs/RUST_REQUEST_LIFECYCLE.md`, `docs/api.md` |
| P8 FIT and P12 physical sheet, JPEG result | Implemented; P12 has local Docker/browser evidence, not public production approval | User-controlled crop and release evidence | `docs/P8_PRODUCT_CONTRACT.md`, `docs/P12_PHYSICAL_SHEET_CONTRACT.md` |
| P13 user crop | Native ABI 2.3 implemented; **partial** product path: Rust HTTP/Nuxt transport and accessible browser editor outstanding | M1.1 FFI/HTTP/proxy, M1.2 UI, M1.3 E2E | `docs/P13_INTERACTIVE_CROP_CONTRACT.md` |
| Nuxt frontend | Implemented: typed proxy and existing physical-sheet UI; no first-party image Wasm path established | M1 editor; client-local processing later | `imgengine-saas/frontend/`, `imgengine-saas/frontend/AGENTS.md` |
| Browser Rust/Wasm | **Planned, not implemented** as a first-party image engine | Architecture/security review, constrained prototype, parity/browser budget | `docs/M0_C17_PORTABILITY_AND_WASM_ADR.md`, `docs/BROWSER_FIRST_PRODUCT_ROADMAP.md` |
| Tailwind visual system | Not established in M0 repository audit; do not assume installed | Separate dependency/value decision; avoid coupling to Wasm | `imgengine-saas/frontend/package.json` |
| Legacy FastAPI/Celery/Redis/Postgres | **Legacy retained**, historical PRD still refers to queue-backed jobs | Contract parity, rollback window, explicit retirement decision | `docs/PRD.md`, `docs/SAAS_CONTRACT.md`, `docs/migration.md` |
| P7.5 older/dirty worktree | Independent local worktree was reported preserved, not represented by remote `main` cleanliness | Owner should reconcile local changes separately; never overwrite | `docs/P7_5_BRANCH_CONSOLIDATION.md` and local worktree audit |
| Hardened production Compose (Caddy → Nuxt → Axum) | Implemented configuration; constrained Docker characterization only | Real host capacity, certificate/DNS/firewall, rollback/monitoring | `docs/P14_PRODUCTION_BASELINE.md` |
| Actual VPS/public launch | **Blocked by evidence**; 1 GiB capacity not proven | P14 real-host runbook and explicit go/no-go | `docs/P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md`, `docs/MVP_RELEASE_CHECKLIST.md` |
| Passport jurisdiction presets | Planned; **no compliance claims** | Source/version/legal review and acceptance fixtures | `docs/P13_INTERACTIVE_CROP_CONTRACT.md`; future preset contract |
| Paid tenants, billing, quotas, bulk/PDF | Planned; no proven current demand/economics | Customer research, unit economics, security model | `docs/BROWSER_FIRST_PRODUCT_ROADMAP.md` |
| Full layered studio / undo / masks | Proposed later product | Validate simpler tools and workflow demand first | `docs/BROWSER_FIRST_PRODUCT_ROADMAP.md` |

## Important historical/active distinction

`docs/PRD.md` (last marked 2026-08-17) describes the earlier asynchronous FastAPI/Redis/Celery/Postgres product and is **not** the authoritative contract for the present synchronous Rust/Axum MVP. Its original requirements remain historical until individually migrated, superseded, or retired by an approved decision. The current synchronous release gate is `docs/MVP_RELEASE_CHECKLIST.md`. Do not silently delete the old PRD.

## Open decisions / gaps

1. Finish P13's crop transport and usable editor without changing ABI 2.2 centered-fill semantics.
2. Choose a narrow browser geometry/Wasm experiment after an independent architecture and privacy review; measure codecs and mobile memory first.
3. Decide whether and when real-host P14 release evidence occurs (independent of browser roadmap).
4. Review current Nuxt/Nitro advisories and assign a time-bounded risk owner.
5. Decide legacy component retirement only after parity, backup/rollback, and explicit approval.
6. Confirm feature pricing and useful paid differentiation with customer interviews and real cost measurements.
7. Audit stale status labels and paths in historical architecture/PRD/AGENTS documents as *separate scoped documentation work*.

## Maintaining this register

For each merged milestone: update row status, exact evidence reference (PR/SHA/test environment), remaining gap, owner, and dependency. A design proposal is not implementation; a passing local test is not production evidence. Do not state that uninspected local or unpushed work is complete. Review this register at each milestone and before release.
