# IMGENGINE — Master Work Inventory and Next Actions

**Audit snapshot:** 2026-11-14. **GitHub baseline reviewed:** `main` at `f09eeae7cb8097a8c51188f64d6a0ab139ab09f2` (documentation PR #30 merge). **Scope:** tracked GitHub repository, existing design/operations contracts, PR history, and user-reported local evidence. This is a living inventory, **not** proof that every test was rerun. Local uncommitted worktrees cannot be inspected through GitHub.

## How to read this file

- **FINISHED (recorded):** implemented and supported by previous PR/test/contract evidence for its original scope; not necessarily production-ready.
- **IMPLEMENTED / RELEASE OPEN:** functioning source path; real-host/security/release evidence incomplete.
- **HALF IMPLEMENTED:** one layer exists but the end-to-end product/workflow is unfinished.
- **OPEN / BLOCKED:** an identified operational, release, governance, or security task awaiting evidence.
- **LEGACY / MIGRATION:** older code retained; do not assume it is today's target architecture or delete it abruptly.
- **NOT IMPLEMENTED / PLANNED:** roadmap intent only; do not claim customer availability.
- **UNVERIFIED / HISTORICAL:** sources disagree or only a local/old record exists; inspect before scheduling or removing.

**Sources of truth:** source and tests at a named revision → versioned API/ABI contract → revision-bound evidence → this task index → historical design notes. Existing `docs/PRD.md` (last updated 2026-08-17) still describes a FastAPI/Celery/Redis/Postgres asynchronous product and is **historical for that legacy path**, not the new browser-first Rust/Axum baseline.

## A. Finished or implemented with recorded validation

| ID | Area | Status and exact scope | Evidence / qualification |
| --- | --- | --- | --- |
| F01 | Native IMGENGINE C engine: decoding, resizing/layout/rendering, TurboJPEG, scalar/SIMD/AVX2, scheduler | **FINISHED (recorded core slice)** | `imgengine/`, `docs/ENGINE_SPEC.md`, `docs/SCHEDULER_CHARACTERIZATION_REPORT.md`; not all future codecs/tools |
| F02 | Native C build language M0 | **FINISHED / MERGED** strict C17 shipping, C11 whole-core regression, isolated C23 policy | PR [#29](https://github.com/Rofikali/imgengine/pull/29), `docs/M0_C17_PORTABILITY_AND_WASM_ADR.md`; Ubuntu 24.04 Docker test evidence recorded |
| F03 | Stable public ABI v1 plus additive ABI 2.0–2.3 | **FINISHED (existing symbols)** | ABI export, layout and consumer evidence; do not break old consumers |
| F04 | Rust FFI, supervisor, lifecycle, bounded admission, Axum endpoints | **IMPLEMENTED / VALIDATED (recorded)** | `docs/RUST_FFI.md`, `docs/RUST_SUPERVISOR.md`, `docs/api.md`; commercial tenant service not complete |
| F05 | P8 FIT-only render with prior semantics | **FINISHED (slice)** | `docs/P8_PRODUCT_CONTRACT.md`; must remain separate from crop-to-fill |
| F06 | P9.3 bounded native JPEG destination allocation | **FINISHED (narrow control)** | `docs/P9_3_BOUNDED_OUTPUT_API_DESIGN.md`, PR #9; not a total memory bound |
| F07 | P11 deterministic physical-sheet geometry planner | **FINISHED (slice)** | PR [#13](https://github.com/Rofikali/imgengine/pull/13) |
| F08 | P12 centered-fill JPEG physical-sheet implementation and browser workflow | **IMPLEMENTED / LOCAL VALIDATION** | `docs/P12_PHYSICAL_SHEET_CONTRACT.md`, PRs #16–19; not a public production release |
| F09 | P13 C-side normalized crop geometry and additive ABI 2.3 | **FINISHED (native slice only)** | PRs [#24](https://github.com/Rofikali/imgengine/pull/24), [#25](https://github.com/Rofikali/imgengine/pull/25), [#26](https://github.com/Rofikali/imgengine/pull/26); no complete UI path |
| F10 | Nuxt 4/Vue/TypeScript frontend and typed same-origin proxy | **IMPLEMENTED** | `imgengine-saas/frontend/`, frontend `AGENTS.md`; does not include browser-local Wasm |
| F11 | Local Docker/Compose/Caddy hardening, loopback binding tests | **IMPLEMENTED / LOCAL EVIDENCE** | PRs #20–22, `docs/P12_DEPLOYMENT.md`; no real VPS claim |
| F12 | Native ABI/export checks, CTest, sanitizer, fuzz, Rust smoke/integration gates | **RECORDED PASS on prior revisions** | `imgengine/scripts/verify_linux.sh`, PR #29; do not misrepresent as rerun for this docs update |
| F13 | P7.5 grayscale dispatch and ABI diagnostic remediation in historical Git history | **MERGED HISTORIC SLICE** | PR #6 and `docs/P7_5_BRANCH_CONSOLIDATION.md`; separate dirty local worktree is a distinct issue |
| F14 | Master status/roadmap/testing-policy documentation | **MERGED** | PR [#30](https://github.com/Rofikali/imgengine/pull/30) added registers and updated `next.md` |

## B. Started but half implemented / incomplete customer journeys

| ID | Feature | Completed part | Outstanding part | Next action |
| --- | --- | --- | --- | --- |
| H01 | **P13 interactive user crop** | C crop geometry + ABI 2.3 and tests | Safe Rust FFI adapter, Axum contract, Nuxt proxy, drag/zoom/keyboard frontend, parity and lifecycle tests | **M1.1 → M1.2 → M1.3**, do not change ABI 2.2 centered-fill |
| H02 | **P12/P14 server-backed MVP launch** | Local JPEG sheet, private Compose, Caddy/Rust/Nuxt paths | Exact release candidate, real host capacity, TLS/DNS/firewall, monitoring, restart/drain/rollback, canary, go/no-go | Follow `docs/MVP_RELEASE_CHECKLIST.md` and `docs/P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md` |
| H03 | **Legacy async → synchronous Rust product transition** | Rust/Axum synchronous path exists | Evaluate old feature/API parity, migration decisions, rollback, then controlled retirement | Do not delete Python/Celery/Redis/Postgres without explicit closure |
| H04 | **Security and tenancy** | API-key boundary, native limits, private proxy, Compose hardening | Paid tenants, API-key lifecycle, quotas, metering, abuse prevention, full memory accounting | Threat model per new paid boundary |
| H05 | **Operational observability** | Historical policies, logs, runbooks and local measures exist | Verify real-host alerting, log redaction, SLOs, on-call ownership and evidence | Separate release/security workstream |
| H06 | **Print/export feature breadth** | JPEG sheets with exact count and bounded geometry | Customer-controlled crop, reviewed presets, local sheets, PDF, ZIP, CMYK/prepress only after justified | Treat every added format as a separate contract |
| H07 | **Frontend experience** | Nuxt and working P12 UI | Accessible crop interaction, error/preview fidelity, broader image-tool UX | After transport contract stabilizes |
| H08 | **Repository documentation coherence** | Many detailed historical P7/P8/P10/P11/P12/P13 contracts | Old PRD and earlier AGENTS/README claims can be stale; label history and update canonical references | Documentation-only reconciliation, no automatic rewrite of history |

## C. Open, blocked, or uncertain tasks — do not mark complete yet

| ID | Finding | Evidence / action |
| --- | --- | --- |
| O01 | **Actual 1 GiB VPS capacity is unknown** | 544 MiB configured service ceilings are not whole-host peak RSS; P14 test required |
| O02 | **Production release approval not granted** | `docs/MVP_RELEASE_CHECKLIST.md` has outstanding boxes for digests/SBOM, host, canary, recovery, security |
| O03 | **Nuxt/Nitro advisories** | Prior audit reported moderate/low advisories; verify current lockfile/audit before risk acceptance |
| O04 | **Branch protection/required review governance** | At inspection `main` was unprotected; configure rules/required checks where permissions allow |
| O05 | **Root license/provenance question** | 2026-10-31 audit noted README MIT assertion but no root `LICENSE`; 2026-11-14 tree still lacks root LICENSE. Verify upstream/fork and third-party licenses before commercial release. Do not assume rights or invent license text |
| O06 | **Open older PR #27** | [P12 ABI 2.2 native sheet PR #27](https://github.com/Rofikali/imgengine/pull/27) remains open despite main containing P12; compare unique changes and decide close/supersede, not blind merge |
| O07 | **Separately dirty P7.5 local worktree** | User reported local uncommitted files. Remote GitHub cannot see them; preserve and inventory locally before cleanup |
| O08 | **C23 production enhancement** | Intentionally not implemented. C17 remains shipping; review only isolated C23 experiments with evidence |
| O09 | **Native cancellation** | Established C path is not mid-operation cancellable; response abandonment is not native cancellation. Revisit only if workload evidence justifies |
| O10 | **Decoded/aggregate memory, disk and workload ceilings** | Output byte caps alone insufficient; characterize real malicious and worst-case images |
| O11 | **Tooling economics** | Existing PR CI may run native suite on frontend docs; scoped local test policy exists but CI path-based optimization is **not implemented** |
| O12 | **Local vs remote history gaps** | Current GitHub inventory cannot claim exhaustive status of unpushed experiments/worktrees |

## D. Not implemented / future plans — comprehensive image-only product direction

| ID | Proposed area | Scope / caveat |
| --- | --- | --- |
| P01 | **Browser Worker + Rust/Wasm** | No first-party browser compute path today. Start **after M1**, Astra-led architecture review and bounded prototype |
| P02 | Browser crop/rotate/resize/convert | Local JPEG/PNG/WebP, browser codec fallback, no-upload assertion and real-device tests |
| P03 | Target-KB smart compression | 20 KB, 50 KB, 100 KB, custom sizes; report impossible constraints rather than promise universal 30 ms |
| P04 | Browser physical photo sheets | A4, 4×6, DPI, exact counts, margins and print fidelity; reuse contracts where applicable |
| P05 | Photo/application/visa presets | Versioned dimensions with official requirements, review owner, provenance, disclaimers |
| P06 | Signature/image document tools | Resize, background extraction, ID front/back layout; privacy-first without permanent image retention |
| P07 | Local batch workflows + ZIP | Bounded concurrent processing, partial failures, memory backpressure, per-image results |
| P08 | Advanced reusable presets | Save/organize dimensions, crop, DPI, format, compression and naming settings |
| P09 | Professional studio | Watermark, text, borders, masks, layers, non-destructive edit history, responsive render budgets |
| P10 | Paid B2B server API | Tenant identity, quotas, accounting, API auth, HMAC where appropriate, billing reconciliation |
| P11 | Advanced PDF/CMYK/prepress and very large sheets | Only if demand, licensing, accuracy, security and VPS cost justify |
| P12 | SEO/customer image routes | `/crop-image`, `/resize-image`, `/compress-image-to-20kb`, `/photo-sheet-maker`, `/studio` etc.; all proposed, not shipped |
| P13 | Tailwind/design system | Not proven installed; explicit design/dependency decision required |
| P14 | Studio Pro subscriptions/priority features | Pricing and paid tiers are **hypotheses**, not implemented transactions |
| P15 | Performance objectives | SIMD and native optimizations must be benchmarked; no universal sub-30-ms/60-FPS promise |
| P16 | Hosting/scale strategy | Minimize server processing for browser-local work; first prove retention, real demand, cost per job and margins |

For precise dependencies, routes, business model and milestones, use `docs/PRODUCT_ROADMAP_BROWSER_FIRST.md`.

## E. Next execution order

1. **Documentation reconciliation**: review this inventory and `skills.md` if adopted; preserve old worktrees and historical contracts.
2. **M1.1** native crop ABI 2.3 → Rust FFI → Axum → Nuxt proxy, with focused interface tests.
3. **M1.2** accessible manual crop UI, drag/zoom/reset/keyboard and safe local preview.
4. **M1.3** image parity, invalid cases, P12 backward compatibility; review and merge.
5. **M2** Astra-led Worker/Wasm design and small geometry prototype; first decide Rust-only, C17-Wasm, browser-native codec, or hybrid using measurements.
6. **M3–M5** local crop/resize/convert, target-KB compression, local photo sheets and presets, each in small PRs.
7. **Commercial workflow**: validate customer usefulness, local batch and paid studio convenience, then B2B only if economics work.
8. **Parallel P14**: actual VPS operational tests only on an explicitly approved candidate; public release stays blocked meanwhile.
9. **Repo hygiene**: review stale PR #27, reconcile uncommitted P7.5 locally, licensing provenance, dependency advisory disposition and branch protection.

## F. Tests, tokens and agent behavior

**Read `AGENTS.md`, affected directory `AGENTS.md`, `docs/CHANGE_SCOPED_VERIFICATION.md`, and optional workflow guidance in `skills.md`.**

- For documentation-only work: links/consistency/diff; **no manual C build**.
- For isolated Nuxt UI / Worker / Rust-Wasm: frontend tests, typecheck/build, browser/Worker/Wasm correctness, memory and no-upload security tests. **No manual full CMake, ASan, UBSan or native fuzz** unless shared native C/ABI/FFI boundary changed.
- For C, ABI, native codec/scheduler or unsafe FFI: focused native/ABI tests and necessary sanitizer/fuzz gates, escalating to full native verification.
- For release candidates: all required revision-bound release/security and real-host tests. Never suppress required CI to save tokens.
- To save model context: keep full command logs in files/artifacts and report only command, SHA, pass/fail and actionable errors; avoid duplicate manual runs already performed by CI.
- Astra reviews high-risk Wasm architecture, native/FFI changes, adversarial security and release gates; Terra/Sol implement bounded steps. An AI model is not a replacement for test evidence.
- Do not merge or deploy until the applicable review/go-no-go. Do not alter dirty local worktrees.

## G. Historical original next.md backlog (retained without deleting intent)

The original file described print-debug cleanup, sanitizer CI, Windows PowerShell/make verification, JSON observability and ELK/Loki, Redis/Postgres/worker SLOs, CMYK/JPEG security fixtures, key rotation and tenancy. These were **ideas for an older architecture**, not an automatically active mandate. Existing CI includes native sanitizer/fuzz jobs, so "add sanitizer CI" is not an unimplemented task without a documented gap. Legacy queue/database observability and migrations must not be imposed on the new ephemeral Rust path.

<details>
<summary>Original next.md text, preserved verbatim</summary>



</details>
