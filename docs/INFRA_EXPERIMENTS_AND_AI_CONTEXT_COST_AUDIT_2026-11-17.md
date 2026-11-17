# IMGENGINE — Infrastructure experiments and AI-context cost audit (2026-11-17)

**Status: verified static GitHub-source audit and recorded Actions metadata; DOCUMENTATION ONLY.** No Docker engine, PowerShell runtime, real VPS, secrets, GitHub personal billing dashboard, Copilot conversation/session token counter or paid service was accessed. Do **not** interpret a recommendation as a passed experiment, a measured model token charge, or permission to change production/CI. Source base `main`: `1523d82f9549bcc3e54b57976e7cd5c539900e27`. [Infra README](../infra/README.md), [infra AGENTS](../infra/AGENTS.md), [F1 existing browser-local feature](F1_BROWSER_LOCAL_RESIZE.md), [P8-SIM-02 runbook](P8_SIM_02_SHEET_CROP_RESOURCE_EVIDENCE.md), [P14 actual-host gates](P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md) and [short live milestone board](../next.md) define the boundaries.

## 1. What can be tested in the existing root infra/ directory?

| Track | Source-backed configuration | First low-cost/low-risk check | What it cannot establish |
| --- | --- | --- | --- |
| **P7 local development** | `docker-compose.p7-frontend-dev.yml`, local dev Dockerfile and `scripts/verify-p7-frontend-stack.sh` | `make self-check` (safe, no build), then **after local test-key setup and Docker approval** invoke documented P7 verification to test Nuxt↔Rust/P5/P8, 413/503 and no leaked browser API key. | Public TLS, resource isolation, native race freedom, stable Docker Desktop CPU/RAM or hosting economics |
| **P8 bounded VPS *simulation*** | `docker-compose.p8-vps-sim.yml`, workload container, `run-p8-vps-sim.ps1`, P12/P13 smoke | **STOP: PowerShell harness source is malformed** as of this SHA. Parse/fix in a dedicated small PR before any Docker workload; then record cgroup peak/OOM and workload evidence with an isolated Docker host. | Real 1 GiB VPS capacity or successful run simply because PR #52 merged |
| **P12 disposable local candidate** | `scripts/run-p12-local-candidate.ps1`, separate native/Rust/Nuxt images | Review script and use a test-only disposable checkout/key after actual Docker approval. Collect health, response codes, bound ports, cleanup and revision. | Public Caddy/TLS or P14 release approval |
| **P14 static production policy** | `docker-compose.production.yml`, `Caddyfile.production`, `production.env.example`, existing static verifier | Inspect production Compose/policy and run existing CI-compatible **dummy values only** static checks in an authorized isolated environment; verify published ports and internal Rust networking. | Actual DNS/TLS/cert renewal, host secret permissions, whole-machine RSS, overload/abuse, monitoring, rollback, supported 1 GiB host or live release |

Do not accidentally use the separate legacy `imgengine-saas/infra/` Python/Celery/Redis/PostgreSQL stack. No cloud account is needed for P7/P8/P12 local rehearsals, although Docker image downloads and builds still consume local electricity, data and disk.

### Safe first experiments on Ubuntu 24.04 / Docker Desktop Linux engine

1. In an isolated local checkout at an identified commit: `git status --short`, `git rev-parse HEAD`, `make self-check`. This checks Makefile menu/guardrails without compiling or cleaning. **NOT RUN by this audit.**
2. Inspect Compose without starting containers: `docker compose -f infra/docker-compose.p7-frontend-dev.yml config --quiet` requires a **local test-only** `IMGENGINE_DEV_API_KEY` value set outside committed files; P8 config requires `IMGENGINE_VPS_SIM_API_KEY`. Keep keys out of source, shell transcripts, model context, screenshots and PR comments.
3. For P7, follow [infra README](../infra/README.md) and existing `infra/scripts/verify-p7-frontend-stack.sh` only after reviewing its operations: it **builds images, starts/stops services**, tests existing APIs, then cleans up. Never run against a production Docker context; confirm no important user containers share the project name.
4. For P8, **do not run `./infra/scripts/run-p8-vps-sim.ps1 -Build` yet**. First check syntax with the PowerShell AST parser and repair as **INFRA-P8-01**. Then verify the original FIT P8 workload plus P12 omitted crop, P13 explicit crop, negative inputs and cgroup OOM fields. Collect exact SHA, Docker version, images, workload JSON, measured `memory.peak` and `memory.events`. **Do not call Docker RAM caps host capacity.**
5. For P14, remain static until real-host project/cost/secret authorization. File-backed Compose secrets can ignore requested `uid/gid/mode` at mount time; verify actual host ownership and *metadata only* if ever approved for deployment.

**P8 source defect (actionable):** [`infra/scripts/run-p8-vps-sim.ps1`](../infra/scripts/run-p8-vps-sim.ps1) at `main`, **lines 126–148**: the OOM-event regex/PowerShell `if` at line 127 has an unterminated string/condition and a duplicated `finally` cleanup block at lines 128–133 and 143–148. It is not safe to call the harness runnable. This was identified by **static line inspection**, not a PowerShell AST parser or runtime test. The code PR should: repair *only* the OOM test and one `try/finally`; add deterministic syntax-parser regression; prove nonzero `oom`/`oom_kill` rejects and `unavailable` is not zero; run full P8 Docker smoke in a disposable environment; keep cleanup and non-secret reporting; preserve original P8 and P12/P13 tests. Avoid mixing this repair with production Compose changes.

On a machine with PowerShell installed, a **parse-only preflight**, requiring **no Docker**, is:

~~~powershell
$tokens = $null
$errors = $null
$path = (Resolve-Path './infra/scripts/run-p8-vps-sim.ps1').Path
[void][System.Management.Automation.Language.Parser]::ParseFile($path, [ref]$tokens, [ref]$errors)
if ($errors.Count) {
  $errors | Select-Object Message
  throw 'P8 PowerShell parser found syntax errors; do not run the harness.'
}
'PowerShell parser PASS'
~~~

This parser command was **not executed here**. Do not pretend it proves any workload.

## 2. Which consumption is actually happening?

**Three distinct meters, not one “token” counter:**

| Meter | What is counted | What GitHub source proves here | What it does NOT prove |
| --- | --- | --- | --- |
| **GitHub Actions runner compute** | Sum of individual concurrent job executions, not just the 3–4 minute workflow wall time | [`ci.yml`](../.github/workflows/ci.yml) runs seven jobs on **pull requests and pushes to main**, including docs-only commits. Latest completed F1 PR-head and merge-push runs were both **7/7 success**. | **Not AI token usage.** Public standard GitHub-hosted Actions execution is free under [GitHub Actions billing rules](https://docs.github.com/en/billing/concepts/product-billing/github-actions); this does not cover paid larger runners or other services. |
| **GitHub Copilot / coding-agent AI credits** | Model/context/tool/service consumption according to the user's exact provider/plan | Repository text sizes and frequent code/doc/CI turns can indicate **waste risks**. | Repo permission **cannot read personal AI billing/credit records**, exact agent prompt tokens, model attribution, invoice or subscription. Copilot credits cannot be inferred from Actions job counts. |
| **GitHub Pro / Copilot plan subscription, Codespaces, caches/storage** | Separate plan/license charges and metered products | GitHub repo metadata says **public**. Standard public Actions does **not**, by itself, justify buying GitHub Pro. | The user's actual subscribed product, charges, reason for purchase, workspace entitlement, refund eligibility and cloud billing cannot be determined from this repo. |

**Current official billing guidance (verify at use):** [Monitoring AI usage](https://docs.github.com/en/copilot/how-tos/manage-and-track-spending/monitor-ai-usage), [budget controls](https://docs.github.com/en/billing/how-tos/set-up-budgets), [AI usage optimization](https://docs.github.com/en/copilot/tutorials/optimize-ai-usage), [GitHub Actions billing](https://docs.github.com/en/billing/concepts/product-billing/github-actions). Differentiate **GitHub Pro** (repository/account plan), **Copilot Pro/Pro+** (AI product), and any separate ChatGPT/Astra subscription or API credits. A repository cannot identify which plan was purchased.

### Repository-backed usage observations, 2026-11-17

- GitHub Actions reports **262 total workflow runs** at inspection; **100 most recent examined** (2026-10-31 through 2026-11-17): **73 success** (37 PR + 36 push), **14 cancelled** (12 PR + 2 push), **13 failed** (3 PR + 10 push). This is a **sample, not a month-to-date bill**.
- Of those sampled runs, **79 occurred 2026-11-13 through 2026-11-17** (10 + 26 + 16 + 23 + 4). Multiple commits/pushes to a PR cause repeated seven-job executions and cancellations; merge commits trigger another seven-job push run. Those are real resource/time costs but not evidence of paid AI tokens.
- Example final F1 PR-head [run #44043494900](https://github.com/Rofikali/imgengine/actions/runs/44043494900) completed all seven jobs. Their individual elapsed durations totaled approximately **12.3 runner-job minutes**. Post-merge [run #44043512607](https://github.com/Rofikali/imgengine/actions/runs/44043512607) totaled **11.9 runner-job minutes**. Earlier [cancelled PR run #44043076217](https://github.com/Rofikali/imgengine/actions/runs/44043076217) had already consumed approximately **10.8 elapsed runner-job minutes**. These are sums of recorded job start/end timestamps, **not a GitHub invoice**, model tokens or additional cash cost for standard public runners.
- Typical heaviest *observed* CI job in those runs was Ubuntu Docker verification (~3–4 minutes per job); native fuzz and native build may also take minutes. But these checks protect native image processing and **must not be silently deleted**. Optimize push churn and AI transcript reading first.
- Agent text size on this base: `AGENTS.md` **30,036 characters**, `skills.md` **35,874**, old `next.md` **41,119**: **107,029 characters total**. Dividing by ~4 characters/token gives an **illustrative 26,800-token context-equivalent** if all are loaded in full, before any code, nested AGENTS, links or logs. **This is NOT measured usage or a precise tokenizer result**. Previous `next.md` is now preserved *exactly* in [`next-history.md`](../next-history.md) and the new live board is approximately **5.5 KB** (86.5% smaller) so the agent can read the board without the archive.
- If an agent system automatically preloads root `AGENTS.md` or other files, documentation instructions **cannot prevent that injection**. A separate scope-reviewed simplification of the long AGENTS/skills files may be worthwhile; keep essential security/CI/approval rules and move detailed examples to on-demand references rather than deleting important policies.

## 3. COST-01 — default low-context, high-confidence agent workflow

**Before EVERY “go ahead” / Astra run:**

1. Read **only the short live** `next.md`, inspect current `main`, open PR list and exact SHA/check summaries. Select **one** READY task. Follow the **root AGENTS essential rules** and *only the scoped* `AGENTS.md` for files touched. Do not load `next-history.md`, every other AGENTS, all 19 skills, product strategy, full repo tree or past successful logs by default. If system automatically loads instructions, do not claim a saved token count from these reading choices.
2. Budget a small **read-set**: target task contract, directly changed files, immediate test/dependency code. Use file ranges/diff/grep before whole files; record any needed exceptions. For unusual cross-cutting security/ABI work, widen the scope deliberately.
3. For **AI/Copilot work**, select an appropriate affordable model for routine scoped implementation and escalate only for real uncertainty, unsafe native C/FFI decisions or security review. Do not automatically run seven separate simulated “engineer personas,” duplicate agents, huge unbounded searches or copy entire test output. Establish provider-specific session credit limits where supported (for example Copilot CLI/SDK); use short fresh sessions or a compact handoff rather than dragging an unrelated long chat history. Avoid unnecessary model/tool-setting changes that invalidate cache where relevant.
4. **One bounded PR at a time**. Make coherent local changes and focused local tests, then publish a reviewable head to CI. Avoid pushing a doc status update every minute while the prior seven-job CI still runs; cancellation and reruns consume runner time. Every PR needs exact-head CI and review; a finishing status commit *after* CI means the head changed and must be rechecked. Do not merge while required checks are pending.
5. **Test fully, read cheaply.** Use focused local checks by changed boundary. GitHub CI still executes all seven jobs; do **not** disable native sanitizer/fuzz/Docker/security gates. Ask for seven outcome rows (PASS/FAIL/PENDING/CANCELLED), tested SHA and links. Only on failure fetch the failing step and **40–60 relevant lines**, never full success logs. CI checking/polling should be bounded, not minute-by-minute repeated token calls. Never use AI to continuously tail a Docker build.
6. After completion, provide **at most 8–12 concise handoff lines** with task, head SHA, PR, CI link, local tests, security/release risks, remaining work, and one next task. Update live `next.md` factually in the same bounded PR when possible; after an external merge, next session reconciles the merge SHA, rather than doing the same implementation twice.
7. Check user-specific [GitHub billing → AI usage](https://github.com/settings/billing) to find actually consumed AI credits and model breakdown, then **Budgets and alerts** for a **hard usage stop** where supported. GitHub's official guides warn alert-only budgets may not stop usage. Do not automatically raise budgets, switch/upgrade a subscription, purchase higher plans or expose credentials.
8. Keep low-cost developer experiments separate from product work. A future CI-SCOPE-01 *selective job scheduling* change requires its own robust dependency/branch-protection/security review and explicit approval. Avoid weakening required checks as a token-saving technique: **CI executions and AI context consumption are different meters**.

**Measurement plan:** record one week's per-session model/AI-credit usage from authorized user billing exports if accessible, number of context-file reads, full-log reads, tool calls, cancel/retry PR runs, successful completed milestones and substantive review defects. Compare **credits per merged useful milestone** rather than counting model messages. No cost reduction should be advertised until real before/after provider readings exist.

## 4. Next bounded PRs and ownership

- **F2a Crop** remains the next free user feature. This docs audit must not become a reason to delay it for a wholesale tooling rewrite.
- **INFRA-P8-01**: isolate and repair the parser/duplicated-finally defect; add parser and representative OOM validation; run real Docker P8 workload at the fixed SHA before claiming runtime success. The present PR does **not** modify PowerShell or fake a test.
- **COST-02 optional**: after this compact `next.md` is adopted, examine root/scoped AGENTS and `skills.md` for safely extracting verbose historical examples into reference files while keeping binding security/CI/approval rules discoverable. Use before/after actual AI usage, not ungrounded absolute token savings.
- **CI-SCOPE-01 Phase B optional**: risk-reviewed path-aware CI if **GitHub runner/queue cost** matters; do not conflate with Copilot credits and do not bypass required statuses/branch rules.
- **P14** stays blocked until separately approved live host and measured capacity/security/recovery evidence.

**Review/merge permissions:** documentation PR only; no cloud changes, CI workflow edits, server deployments, PowerShell execution, subscriptions/billing actions, secrets or paid services. A merged audit does not repair INFRA-P8-01, run Docker, enable AI budgets or establish actual billing usage.
