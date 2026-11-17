# IMGENGINE — Live product, feature and infrastructure milestones

**Authoritative short task board.** Read this first, then current GitHub main/open PR/check summaries and only affected scoped AGENTS/contract. The previous long queue is preserved as [historical archive](next-history.md) **on demand**, not the task selector. Rules and evidence schema: [Product/Infra milestone hierarchy and Codex measurement guide](docs/IMGENGINE_PRODUCT_INFRA_MILESTONE_AND_CODEX_USAGE_PROTOCOL.md), [portfolio](docs/IMGENGINE_FOUR_PRODUCT_PORTFOLIO.md), [infra-cost audit](docs/INFRA_EXPERIMENTS_AND_AI_CONTEXT_COST_AUDIT_2026-11-17.md), [Skill 21](skills.md).

**GitHub verified at 2026-11-17:** `main` `b829155c7b74796087ef117ba2789f6c00210a19`. [F1 Resize PR #57](https://github.com/Rofikali/imgengine/pull/57) **MERGED** as `1523d82f9549bcc3e54b57976e7cd5c539900e27`; [F2a Crop PR #59](https://github.com/Rofikali/imgengine/pull/59) **MERGED** as `b829155c7b74796087ef117ba2789f6c00210a19`. [F2a PR-head CI](https://github.com/Rofikali/imgengine/actions/runs/44049109529) **SUCCESS** and all **seven checks on current main SUCCESS**. F0 [#55](https://github.com/Rofikali/imgengine/pull/55), portfolio [#56](https://github.com/Rofikali/imgengine/pull/56), and infra/AI audit [#58](https://github.com/Rofikali/imgengine/pull/58) **MERGED**. No public Free product or paid products released. **Recheck GitHub whenever the next agent session begins; this is dated, not a live subscription.**

## A. Product-level milestones (one parent per shippable version)

| Product parent | Child features / actual progress | Parent state and release proof | Next |
| --- | --- | --- | --- |
| **PRODUCT-FREE-01 — IMGENGINE Free v1** | **2/5 features merged**: Resize F1 #57; Crop F2a #59. **Remaining:** Rotate F2b, Compress F2c, Convert F4 | **IN_PROGRESS / NOT RELEASED.** Five tool correctness/integration, privacy, accessibility, browser/mobile/hosting/legal/ads gates; F5 launch with explicit owner approval. Exactly five initial tools; extras later v2. | **F2b Rotate READY** |
| **PRODUCT-PRO-01 — IMGENGINE Pro v1** | Creator saved recipes, bounded batch/ZIP, repeatable processing; customer proof | **PLANNED / NOT RELEASED.** Real users, entitlement/provider/storage/payment/refund/tax and separate approved commercial release. | PRO-00 interviews after Free evidence |
| **PRODUCT-STUDIO-01 — IMGENGINE Studio v1** | Print/operator workflow; physical presets, proof, accurate sheet, repeat customer jobs; existing P12/P13 are server-side foundation | **PLANNED / NOT RELEASED.** Real print validation/consent and, if server backed, C-A0 and P14 capacity, plus approved commercial launch. | STU-00 shop workflow interviews |
| **PRODUCT-API-01 — IMGENGINE Developer API v1** | Private stable API sandbox → tenant keys/quotas → durable usage ledger/reconciliation → beta | **PLANNED / NOT RELEASED.** The existing private Rust server is not a multi-tenant billable product; security/finance/host/owner go-no-go gates. | API-00 developer discovery + measured unit costs |

**Milestone definition:** Feature `MERGED` ≠ product `RELEASED`. When all **mandatory** child features and independent gates pass, set parent `READY_FOR_RELEASE_REVIEW`; set `RELEASED` **only after actual owner-approved release**. Parent 2/5 means feature checklist count only, not 40% of readiness. Reconcile merge SHA and exact-CI outcome before updating any child or parent.

## B. Infrastructure/environment tracks (independent from products)

| Environment | What to test | Verified state and next action |
| --- | --- | --- |
| **INFRA-P7 — Development** | Local Docker Nuxt + Rust + C17, positive/negative API results, private credential checks, cleanup | **CONFIGURED / RUNTIME NOT VERIFIED THIS REVIEW**; local-only eligible. Evidence: host + SHA + command + exit + real smoke result. |
| **INFRA-P8 — VPS simulation** | P8 FIT + P12/P13 crop, resource CPU/RAM, `memory.peak`, OOM events, cleanup | **BLOCKED**: malformed OOM condition and duplicated `finally` in `infra/scripts/run-p8-vps-sim.ps1` lines 127–148 remain in current main. **INFRA-P8-01** separate script fix + parser test FIRST; then actual isolated Docker test; not proof of 1 GiB public VPS. |
| **INFRA-P12 — Local candidate** | Disposable production-shape containers, secret/health/response limits/cleanup | **CONFIGURED / RUNTIME NOT VERIFIED THIS REVIEW**; requires real isolated Docker result. |
| **INFRA-P14 — Production configuration** | Caddy/TLS, private Rust networking, file-backed secrets, host RSS/abuse, rollback/monitoring | **CONFIGURED / ACTUAL HOST NOT VERIFIED**; real VPS release requires explicit budget/host/secret/deploy owner approval and on-host evidence. |

**Environment advancement rule:** Update a P7/P8/P12/P14 row only when that exact track's named test/host/revision has actual `PASS/FAIL/NOT_RUN` evidence. Passing an infra experiment never silently marks another environment or a product released. Do not execute the broken P8 harness, use real customer images/test credentials publicly or add new legacy Python services.

## C. Engineering tasks and priorities

| Priority | ID | Next bounded step and mandatory gates |
| --- | --- | --- |
| **NOW — product work** | **F2b Rotate READY** | New browser-local rotate route in one small PR. Actual pixel/alpha/orientation/cancel/keyboard/mobile/no-upload tests; regressions for shared Worker Resize/Crop; exact-head required CI, owner PR review. |
| **NOW — independent defect** | **INFRA-P8-01 BLOCKED UNTIL CODE FIX** | Separate minimal PowerShell parser/OOM/one-finally repair PR, followed by actual Docker simulation. No changes to production secrets/Compose. |
| **PARALLEL safety** | **C-A0 OPEN** | Evidence-led native queue/metrics/resource security review before public server-backed workloads. Preserve C17/ABI/sanitizer/fuzz gates. |
| **NEXT — Free completion** | **F2c Compress → F4 Convert → F5 launch** | One PR per feature; product Free v1 remains open until all 5 plus release gates pass. |
| **OPTIONAL** | **F3 Worker/Wasm evaluation** | Measure first; not a prerequisite for native browser APIs. |
| **LATER** | **Pro/Studio/API implementation** | Interviews first; separate feature PRs and owner-approved billing, entitlement and launch gates. |
| **RESEARCH / COST** | **CODEX-COST-01 — PROTOCOL READY, DATA NOT YET MEASURED** | After 5-hour AND weekly allowance reset, privately compare read-only local vs GitHub Codex sessions with same model/effort, both before/after usage percentages, times, sessions/tool calls; do not share or commit private billing screenshots. No automated telemetry or plan changes. |

**Permanent minimal-cost update rule:** Start with this board and current GitHub PR/check facts. Only inspect exact source/contract needed; avoid huge historical docs, full successful CI logs, repeated remote diffs and redundant status-commit pushes. Required seven CI checks remain intact. No owner-account/balance/payment screenshots in repo. No implicit merge, deployment, spending, secrets, discounts or provider purchase. The **next user feature is F2b**, not reopening already merged F1/F2a, and not starting P14 production.
