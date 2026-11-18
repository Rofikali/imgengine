# IMGENGINE — Live product, feature and infrastructure milestones

**Reusable Codex runner:** [`$imgengine-milestone`](.agents/skills/imgengine-milestone/SKILL.md) uses this live board after the skill PR is merged and the local checkout is updated. No long copy-paste prompt for each new session. **Authoritative short task board.** Read this first, then current GitHub main/open PR/check summaries and only affected scoped AGENTS/contract. The previous long queue is preserved as [historical archive](next-history.md) **on demand**, not the task selector. Rules and evidence schema: [Product/Infra milestone hierarchy and Codex measurement guide](docs/IMGENGINE_PRODUCT_INFRA_MILESTONE_AND_CODEX_USAGE_PROTOCOL.md), [portfolio](docs/IMGENGINE_FOUR_PRODUCT_PORTFOLIO.md), [infra-cost audit](docs/INFRA_EXPERIMENTS_AND_AI_CONTEXT_COST_AUDIT_2026-11-17.md), [Skill 21](skills.md).

**GitHub checked 2026-11-18:** `main` `016e5452b69f00ca706e815c36ce269b62bf2229` **MERGED F2b Rotate [PR #62](https://github.com/Rofikali/imgengine/pull/62)**, head `d02818a7935736f3fb247dbb701be91fdbd6cb41`; **all seven current-main checks SUCCESS**. F1 Resize [#57](https://github.com/Rofikali/imgengine/pull/57), F2a Crop [#59](https://github.com/Rofikali/imgengine/pull/59), F0 strategy [#55](https://github.com/Rofikali/imgengine/pull/55), portfolio [#56](https://github.com/Rofikali/imgengine/pull/56), infra/AI audit [#58](https://github.com/Rofikali/imgengine/pull/58), milestone policy [#60](https://github.com/Rofikali/imgengine/pull/60), and six-rule Codex policy [#61](https://github.com/Rofikali/imgengine/pull/61) **MERGED**. **Next eligible customer feature: F2c Compress.** No Free v1 product/paid-tier launch. Reconcile current GitHub state at each subsequent session; this paragraph is dated evidence.

## A. Product-level milestones (one parent per shippable version)

| Product parent | Child features / actual progress | Parent state and release proof | Next |
| --- | --- | --- | --- |
| **PRODUCT-FREE-01 — IMGENGINE Free v1** | **3/5 feature implementations merged**: Resize F1 #57; Crop F2a #59; Rotate F2b #62. **Remaining:** Compress F2c, Convert F4 | **IN_PROGRESS / NOT RELEASED.** Five tool correctness/integration, privacy, accessibility, browser/mobile/hosting/legal/ads gates; F5 launch with explicit owner approval. Exactly five initial tools; extras later v2. | F2c Compress READY, then F4 Convert |
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
| **NOW — product work** | **F2c Compress READY** | Implement one bounded **browser-local Compress** PR after reading the existing frontend privacy/bounds/worker contract; define supported input/output and quality/size policy, EXIF/alpha/error behavior, cancellation and resource limits. Add pixel/size/privacy/keyboard/mobile tests and shared Worker Crop/Resize/Rotate regressions as appropriate. Preserve review/push approval and all seven PR CI checks. |
| **NOW — independent defect** | **INFRA-P8-01 BLOCKED UNTIL CODE FIX** | Separate minimal PowerShell parser/OOM/one-finally repair PR, followed by actual Docker simulation. No changes to production secrets/Compose. |
| **PARALLEL safety** | **C-A0 OPEN** | Evidence-led native queue/metrics/resource security review before public server-backed workloads. Preserve C17/ABI/sanitizer/fuzz gates. |
| **NEXT — Free completion** | **F2c Compress → F4 Convert → F5 launch** | One PR per feature; product Free v1 remains open until all 5 plus release gates pass. |
| **OPTIONAL** | **F3 Worker/Wasm evaluation** | Measure first; not a prerequisite for native browser APIs. |
| **LATER** | **Pro/Studio/API implementation** | Interviews first; separate feature PRs and owner-approved billing, entitlement and launch gates. |
| **RESEARCH / COST** | **CODEX-COST-01 — PROTOCOL READY, DATA NOT YET MEASURED** | After 5-hour AND weekly allowance reset, privately compare read-only local vs GitHub Codex sessions with same model/effort, both before/after usage percentages, times, sessions/tool calls; do not share or commit private billing screenshots. No automated telemetry or plan changes. |

**CODEX-COST-02 — OWNER-APPROVED WORKFLOW / NO ACCOUNT SETTINGS CHANGED:** Six-rule [Codex VS Code local-first policy](docs/CODEX_VSCODE_LOW_CONTEXT_SIX_RULES.md) (and [Skill 22](skills.md)) covers local source/tests, minimal milestone docs, GitHub-only remote facts, compact CI logs, risk-adjusted model/effort *if user-supported*, and private post-reset Usage comparison. **Status: documentation review; measured savings UNKNOWN.** Follow for every future bounded task; do not change model/account settings silently or let cost experiments block the next READY feature. The separate CODEX-COST-01 paired experiment remains DATA NOT YET MEASURED.

**Permanent minimal-cost update rule:** Start with this board and current GitHub PR/check facts. Only inspect exact source/contract needed; avoid huge historical docs, full successful CI logs, repeated remote diffs and redundant status-commit pushes. Required seven CI checks remain intact. No owner-account/balance/payment screenshots in repo. No implicit merge, deployment, spending, secrets, discounts or provider purchase. The **next user feature is F2c Compress**, not reopening merged F1/F2a/F2b or starting P14 production.
