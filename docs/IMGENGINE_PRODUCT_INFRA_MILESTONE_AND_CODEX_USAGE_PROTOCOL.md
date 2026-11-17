# IMGENGINE milestone hierarchy, infrastructure experiments and Codex usage measurement

**Status: proposed documentation policy; not a feature release, infrastructure test, Codex billing measurement, or deployment approval.** Reconciled 2026-11-17 against `main` `b829155c7b74796087ef117ba2789f6c00210a19`, merged [F1 PR #57](https://github.com/Rofikali/imgengine/pull/57), merged [F2a PR #59](https://github.com/Rofikali/imgengine/pull/59), and seven passing check runs at the latest `main` SHA. Read the compact [live status board](../next.md) first. [Product portfolio](IMGENGINE_FOUR_PRODUCT_PORTFOLIO.md), [P8 audit](INFRA_EXPERIMENTS_AND_AI_CONTEXT_COST_AUDIT_2026-11-17.md), [commercial gates](IMGENGINE_COMMERCIAL_READINESS_AND_UNIT_ECONOMICS.md), and [release checklist](MVP_RELEASE_CHECKLIST.md) retain detailed contracts. This file defines the **hierarchy and reporting process** rather than replacing those contracts.

## 1. Definition of a milestone — products versus features versus infrastructure

| Level | Example | Completion evidence | Meaning of 'passed' |
| --- | --- | --- | --- |
| **Portfolio / product milestone** | `PRODUCT-FREE-01`, `PRODUCT-PRO-01`, `PRODUCT-STUDIO-01`, `PRODUCT-API-01` | Every required child feature implemented and merged; product integration, privacy/security/UX/legal/economic/release gates; explicit launch approval and actual deployment where relevant | **PRODUCT RELEASED** only after the real product is usable by intended users. Plans and isolated feature merges don't qualify. |
| **Feature / engineering milestone** | `FREE-RESIZE (F1)`, `FREE-CROP (F2a)`, `FREE-ROTATE (F2b)` | Reviewable PR, exact merged SHA, required CI, focused unit/browser/security/privacy tests | **FEATURE MERGED / VERIFIED**; not a released product. |
| **Infrastructure / environment milestone** | `INFRA-P7`, `INFRA-P8`, `INFRA-P12`, `INFRA-P14` | Run against the defined actual environment; results/report, SHA, resources, failure modes and evidence | **ENVIRONMENT VERIFIED** only for that named environment. P8 Docker Desktop success cannot certify P14 real VPS. |
| **Business / operational gate** | `COMM-01`, account/billing ADR, support readiness | Specific reviewer decision, customer/price evidence, owner approval and actual service status | A gate can be approved without meaning a product is implemented or released. |

**State machine per row:** `PLANNED → READY → IN_PROGRESS → PR_OPEN/CI_PENDING → READY_FOR_REVIEW → MERGED → VERIFIED → RELEASED` *only where applicable*. An `INFRA` test can be `TESTED/VERIFIED` without any deployment; a product is `RELEASED` only with explicit approval and actual availability. Other states include `BLOCKED`, `DEFERRED`, `FAILED`, `NOT_RUN` and `SUPERSEDED`. A product with 2/5 features merged is **not 40% production-ready**: the fraction describes feature-count progress only, not security, user acceptance or launch readiness.

### Products to track separately

| Parent | Initial scope | Verified child facts (at source baseline) | Remaining release gate |
| --- | --- | --- | --- |
| **PRODUCT-FREE-01 — IMGENGINE Free** | **Exactly five:** Crop, Resize, Compress, Rotate, Convert, all browser-local by default | **F1 Resize merged #57; F2a Crop merged #59.** Both are features, not a publicly released Free site. | F2b Rotate, F2c Compress, F4 Convert, integration/a11y/privacy/browser/hosting/legal/ads review; F5 real launch and owner go/no-go. No six-tool creep. |
| **PRODUCT-PRO-01 — IMGENGINE Pro** | Creator local presets/recipes, bounded batch and repeatable workflow; paid plan only after business decisions | Roadmap only | Interviews, feature PRs, entitlement/payment/refund/tax review and actual accepted launch; no Pro subscriptions implied by docs. |
| **PRODUCT-STUDIO-01 — IMGENGINE Studio** | Computer-shop / print/photo studio operator, physical presets, print proof and validated sheets | Existing P12/P13 server rendering is engineering foundation **not a Studio product release** | Operator tests, sourced/dated presets, output measurements, consent, independent server/native/P14 safeguards where relevant, business plan/launch approval. |
| **PRODUCT-API-01 — IMGENGINE Developer API** | Publicly integrated, customer-scoped metered Rust/C17 API | Existing private Rust/Axum endpoints are **not** a paid public multi-tenant API | Developer sandbox, keyed tenant quotas, durable ledger/billing and reconciliation, measured host capacity, C-A0/P14, security/business approvals, actual release. |

**Product completion event:** when **all** mandatory child features and scoped gates are verified, set parent `READY_FOR_RELEASE_REVIEW`; only after an approved, successful real launch set `RELEASED`. Later optional tools are separate subsequent **product versions** (e.g. `PRODUCT-FREE-02`); they do not reopen the first five-tool version as unfinished. If a gate fails, leave the parent open regardless of the number of passing PRs. Progress reporting should say: “Free v1: 2/5 feature implementations merged; release NOT READY; next F2b Rotate.”

## 2. P7 / P8 / P12 / P14 are four *independent* environment tracks

| Track | Experiments and owner benefit | Current fact | Required proof to become VERIFIED |
| --- | --- | --- | --- |
| **INFRA-P7 — Local development** | Nuxt↔Rust↔C17 dev stack; valid/invalid render, oversized upload, backend outage, no leaked browser API key | Implemented local Compose/scripts; eligible for local-only smoke, no present-run claim | Actual test Docker machine, synthetic keys, target commit, command/exit results, local smoke and cleanup |
| **INFRA-P8 — VPS simulation** | Bounded Rust/Nuxt resource and P8/P12/P13 image workloads; cgroup peak/OOM/cleanup | Compose/workload scripts exist, but **PowerShell harness has malformed OOM expression and duplicate `finally` in lines 127–148**; runtime **BLOCKED/NOT VERIFIED** | Separate `INFRA-P8-01` syntax fix, PowerShell parser test, Docker Desktop Linux engine run of P8+P12/P13+negative cases; measured cgroup/events and no OOM/leftovers |
| **INFRA-P12 — Disposable local candidate** | Production-shaped ephemeral Rust/Nuxt rehearsal on localhost; test secrets and response limits | Script/candidate configuration exists; present-run not verified by this audit | Real approved isolated Docker candidate run, exact SHA/image, secure synthetic keys, health/HTTP/security/cleanup and rollback notes |
| **INFRA-P14 — Public production readiness** | Caddy/TLS, Docker networks, file-backed secrets, PIDs/RSS/cpu, host firewall, monitoring/rollback, API abuse and cost | Hardened **configuration only**; real VPS **NOT VERIFIED** | Explicit host/spend/deploy permission plus actual host TLS/DNS/cert, secret mount metadata, full host RSS/limits, overload, monitoring/recovery/rollback and owner go/no-go |

**Checkpoints and separation:** after each independent infrastructure verification, update only its `INFRA-P*` status and link a dated evidence record. Mark `CONFIGURED`, `PARSER_PASS`, `DOCKER_SMOKE_PASS`, `HOST_VALIDATED` and `RELEASED` distinctly. Docker/CI/build passing does **not** propagate automatically to another environment or product milestone. The P8 script defect is **not fixed by this documentation PR**, and no PowerShell/Docker work is claimed.

**Experiments without spending on a VPS:** start with Makefile `make self-check` in Ubuntu/Linux (no build), then isolated P7 smoke with a synthetic key when appropriate. Use P12 as a disposable private rehearsal. Fix P8 script **before** `run-p8-vps-sim.ps1 -Build`; only then test measured cgroup envelopes. Keep P14 static until a real VPS and separate approval. Never run legacy `imgengine-saas/infra/` accidentally or use production keys.

## 3. Codex VS Code Plus usage: controlled before/after comparison

**Privacy and account boundary:** This procedure applies to Codex in VS Code signed in with a ChatGPT plan, not an API-key billing arrangement. **Never commit the owner's account, subscription price, live remaining percentages, credit balance, screenshots, email, identifiers or payment settings to this public repository.** The account owner privately checks their Usage screen; any previous chat screenshot is historical, not a live usage API. Usage History can be delayed or incomplete, and this repository cannot read private plan-usage records. Do not enable credit auto-reload, buy credits or change account settings without explicit permission.

### Test only after both relevant allowances have enough room

1. Open ChatGPT **Settings → Usage** and write down the exact local timestamp/timezone, **5-hour and weekly % remaining**, next reset time, model and reasoning effort. If numbers still show 0%/near zero, **defer**; don't buy credits to test.
2. Ensure **no other Codex, Work, Workspace Agents or Excel agent sessions** run during the comparison, as the limits are shared. Avoid a reset crossing either measurement interval; if a reset occurs, discard that comparison.
3. **Test A (local only)**: start a fresh VS Code Codex session; same model/reasoning as Test B; ask for a **read-only, bounded** evaluation of a named local IMGENGINE PR's changed files using local `git` and working tree—no GitHub connector, network, edits, pushes, PR, deploy, or full logs. Take a screenshot of Usage **immediately before and after**, recording any update delay.
4. **Test B (GitHub connector)**: fresh session, same model/reasoning and **comparable read-only scope**, but ask to inspect the **same existing PR** via GitHub metadata, changed-file names, concise CI outcomes and *only* the required targeted diff; no writes, full successful logs, repeated fetches or network beyond the requested GitHub access. Capture Usage before and after.
5. Prefer **two or three paired trials** under the same conditions once an initial low-cost trial suggests a real difference. Use small/medium PRs and **end trials if usage is too costly**. Record number of tool calls, approximate returned text sizes, elapsed time and optional exact Codex session token/credit information if actually available.
6. Compute `used 5-hour percentage points = before remaining - after remaining`; repeat for weekly. This is an **allowance delta, not exact tokens or money**. If the difference is smaller than UI rounding or the account refresh delay, mark it **INCONCLUSIVE**. Don't claim GitHub itself charges Codex tokens: model processing of connector output is the relevant potential driver.
7. Send **two before/after pairs** here, or share screenshots with account IDs, email, payment details, addresses and session secrets redacted. Ask for diagnosis of local-only vs GitHub-heavy. **Do not commit screenshots or account-usage observations to the public repository** without separate, privacy-conscious consent. An agent may link a redacted, owner-approved *aggregate* benchmark conclusion later, not private balances.

**Copyable report format for owner-to-assistant discussion (not repository metadata):**

~~~text
Codex usage experiment (no code changes)
Model / reasoning: ...
Timestamp/timezone: ... ; any reset during test? yes/no
A local-only: 5-hour remaining ...% -> ...%; weekly ...% -> ...%
A calls / returned text size / elapsed: ...
B GitHub:     5-hour remaining ...% -> ...%; weekly ...% -> ...%
B calls / returned text size / elapsed: ...
Other agentic sessions active? yes/no
Delay until usage page refreshed: ...
~~~

### Current evidence and cost rules

- Reading ~38 KB of PR #59 diff is roughly 9,500 tokens using **an illustrative 4-character/token heuristic**. It is **not** the measured bill: tokenization, caching, tool definitions, hidden reasoning and repeated context matter. The minimal-response GitHub metadata API avoids large diff bodies when not needed.
- Repo's root `AGENTS.md`, `skills.md` and archived `next-history.md` contain large instructions/history. Read **compact live `next.md` + scoped sources only** where discretionary; tool/agent automatic instruction injection may still add tokens.
- GitHub-hosted Actions **compute minutes** and GitHub Pro/Copilot subscriptions are **not** Codex Plus consumption. PR/merge CI is still required. Keep successful logs out of model context, fetch failures only, use coherent one-PR changes and avoid repeated status-commit CI restarts.
- Any model/effort optimization should be benchmarked on useful completed work, **not** a claim that tools themselves consume a fixed token fee. No script can query private ChatGPT Usage from public repository secrets or bypass plan limits.

## 4. Update triggers and automation boundaries

| Event | What to update in the live board | What **not** to infer |
| --- | --- | --- |
| PR opened | Feature ID, `PR_OPEN/CI_PENDING`, URL/head SHA | Not `MERGED` or `RELEASED` |
| All required exact-head CI pass and reviewed | `READY_FOR_REVIEW`, exact SHA/check link | Not permission to auto-merge |
| GitHub confirms a merge | Child feature `MERGED`, merge SHA and next bounded feature; recalculate parent *feature count* | Parent product not complete automatically |
| Real P7/P8/P12 experiment finishes | Only that environment's `TESTED/PASS/FAIL`, host/commit/report, blockers | No P14 transfer, no cost/token conclusions |
| Real P14 host gate finishes | `HOST_VALIDATED` or precise `BLOCKED`, evidence and go/no-go | No launch without human approval |
| Product's final feature and launch gate finishes | Parent `READY_FOR_RELEASE_REVIEW`, then after actual owner-approved public launch `RELEASED` | A docs PR or green unit tests are not a shipped product |
| Codex allowance reset / owner shares measurement | **Private chat analysis** and optional sanitized **aggregate** conclusion only if separately approved | Never store the owner's live billing state, screenshots or account details in a public repo |

Keep a **single short live table** in `next.md`; keep expanded contracts and history in dedicated docs. Astra/Codex should choose one next **READY** milestone from the board, verify latest GitHub reality, run only relevant focused local tests and preserve existing seven CI/release gates. The next customer feature at this checkpoint is **F2b Rotate**. A docs/roadmap PR must not block this independent implementation or silently fix P8, modify CI, merge, deploy, buy hosting or change user usage settings.

