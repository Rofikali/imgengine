# IMGENGINE — Milestone Continuation and New-Session Recovery Runbook

**Status:** Proposed standing procedure (2026-11-16); documentation only. This is not a running agent service, an installed automation, a workflow change or a grant to merge/deploy. It supplements [root AGENTS](../AGENTS.md), [skills](../skills.md), [next.md](../next.md), [milestone handoff](AGENT_MILESTONE_HANDOFF_TEMPLATE.md), [minimal-context CI policy](CI_FIRST_MINIMAL_AI_CONTEXT_POLICY.md), and [seven-level engineering model](SEVEN_LEVEL_ENGINEERING_OPERATING_MODEL.md).

## The one crucial distinction

**A persistent instruction is not a persistent process.** Writing "continue with the next milestone" in `AGENTS.md` helps any agent *that is running* choose correctly. **It cannot wake a finished Astra/Codex chat, launch another model invocation, monitor GitHub in the background or authorize a merge.** A stopped session only restarts via:
1. **Manual continuation:** start a new agent session and give the one-line bootstrap command below; the agent reconstructs state from repository files and GitHub, not pasted prior chat.
2. **Actual orchestration:** separately configure an authorized coding-agent runner or external workflow trigger (e.g. issue assignment, PR merged event, scheduled backlog triage), with appropriate credentials, spending limits, isolation, security review and a working resume/delegation mechanism. Merely adding a GitHub Actions YAML file does not supply an AI coding agent. A CI job that runs tests is **not** an autonomous coding agent.
3. **Same running session:** if the chosen agent product supports multi-task continuation, execute only the tasks in its authorized scope until a stop condition/resource limit; this capability depends on the agent runner.

**Normal result:** The owner does not repeat engineering standards. They may still click **Continue / Start task / Approve** when the actual runner or permission boundary requires it.

## 1. One durable task source of truth (not another chat transcript)

- `next.md` is the project-wide *work register*. Its current factual status must be reconciled against latest merged PRs and exact-SHA CI; old notes are historical evidence, not the current state.
- Each task has **ID, priority, outcome/customer value, prerequisites, status, scope, acceptance tests, risks, owner, evidence/PR SHA and next action**.
- Recommended task states: `PLANNED`, `READY`, `IN_PROGRESS`, `PR_OPEN`, `CI_PENDING`, `READY_FOR_REVIEW`, `BLOCKED`, `MERGED`, `RELEASE_CANDIDATE`, `RELEASED`. A separately marked `FAILED` or `CANCELLED` is possible. Do not call `CI_PASS` automatically `MERGED` or `RELEASED`.
- Completed code reviews/PRs and run IDs belong in evidence links, not repeated long agent chat messages. Record `last_verified_sha` separate from `implementation_sha`, `merge_sha` and release candidate.
- A task is **READY** only when all prerequisites are truly met, its scope/contract is understood, and it does not require an undecided owner/security/ABI decision.
- Priorities: confirmed P0 safety/security defects and release-blocking risk first where relevant; then critical in-progress customer workflows and dependencies; then measurable customer value. A speculative feature does not displace a known blocker by default.
- Keep `next.md` a dashboard, not a new duplicate HLD/LLD. Detailed requirements and fixtures go in the versioned contracts/docs.

## 2. On agent start or after any milestone

1. **Read latest state**: `git status --short`, `git branch --show-current`, `git rev-parse HEAD`, remote `main` tip, PR list/CI statuses; do not wipe local dirty worktrees.
2. Read root and affected scoped AGENTS, `next.md`, contract, evidence, [change-scoped verification](CHANGE_SCOPED_VERIFICATION.md), and [CI-first log policy](CI_FIRST_MINIMAL_AI_CONTEXT_POLICY.md). Read only relevant source, not the full repository.
3. **Reconcile state**: identify merged PRs not yet reflected in `next.md`, declined/deferred PRs, unpushed local changes and the exact tested revision. Do **not** overwrite the owner's uncommitted work or present stale docs as current.
4. **Choose one next eligible task** by dependency and risk. Give it a bounded branch/PR, acceptance criteria, negative cases, out-of-scope and effort budget. If several workstreams exist, choose independent approved tasks only; never conflate native security work and frontend parity in one PR.
5. Plan → small code/tests/docs change → run L0 focused checks → open review PR **only if publication is authorized** → GitHub CI L1 → check only job conclusions and failure excerpts → report exact-SHA evidence and residual risks.
6. **When CI passes:** set `READY_FOR_REVIEW` for the correct PR head and note the required human decision. **Do not assume PR merged.** If the owner explicitly approved merging that PR, recheck current head/reviews/checks and apply the approval to that specific operation; approvals are not blanket permissions.
7. **When PR is actually merged:** refresh `main`; update milestone to `MERGED` with merge SHA, exact check-run identity, unresolved risks and new task. A docs-only handoff update is not proof of feature implementation.
8. **Continue only within current active-runner scope**: if allowed and a next independent task is READY, start its separate bounded branch/PR. If the runner stops, leave a checkpoint and use one-line bootstrap later. Never self-trigger forever or create an infinite CI/PR loop.

## 3. Decision table: exactly when to continue or stop

| Event | Recorded state | Safe next action | Ask owner? |
| --- | --- | --- | --- |
| Local focused tests pass | `IN_PROGRESS` | Finish feature acceptance, prepare PR if allowed | Only if publishing is not authorized |
| PR opened, CI pending | `CI_PENDING` | Allow CI to run; retrieve compact statuses when available; do not claim PASS | No, unless approval needed to publish |
| CI success at exact PR SHA | `READY_FOR_REVIEW` | Report PR and risks; select other independent **authorized** task if possible | **Yes** before merge |
| CI failed | `IN_PROGRESS` or `BLOCKED` | Inspect failed step only, diagnose and fix; bounded retries | If repeated failure requires new design/authority |
| PR merged externally | `MERGED` | Verify merge event and main SHA, reconcile `next.md`, choose next READY task | No for read-only reconciliation; yes for new restricted action |
| Contract/security decision unresolved | `BLOCKED` | Record options/evidence; select another independent READY task | **Yes** for consequential contract/risk choice |
| No READY task | `BLOCKED` or `GOAL_COMPLETE` | Summarize missing decisions or prove agreed goal gates | **Yes** for next priority/scope |
| Public release requested | `RELEASE_CANDIDATE` | Verify P14/CI/host/security/rollback criteria on exact candidate | **Yes** before deploy |
| Agent context/time/runner limit | `CHECKPOINTED` (session status, not feature state) | Write checkpoint, stop safely; resume only with real trigger/new session | No repeated explanation required |

**IMPORTANT: passing all seven CI jobs for PR #47 does not permit the next agent to merge any future PR automatically.**

## 4. Concrete IMGENGINE example — M1.1 → M1.2 → M1.3

Source snapshot checked for this handoff: `main` **`de8b1708ba4e54cd219c93cc15f7923773ce8c9e`**, merged [PR #47](https://github.com/Rofikali/imgengine/pull/47), exact [PR-head seven-job CI](https://github.com/Rofikali/imgengine/actions/runs/43808582807) at `b3aa42c2b89b0831a48448e8c4529cd67131ed75`. This is *not* a new test run for this documentation PR.

- **M1.1:** native C ABI 2.3 → safe Rust/Axum + Nuxt-proxy transport; recorded merged previously.
- **M1.2:** accessible browser crop editor and typed serialization, **PR #47 MERGED** with seven passing PR-head jobs. Current `next.md` contains some *stale pre-merge* M1.2 text: treat it as historical and reconcile in a separate status update; **do not reimplement it**.
- **M1.3 READY for a bounded implementation/review task:** deterministic browser-to-Nuxt-to-Rust-to-C image parity, explicit stored-raster/EXIF orientation fixtures, odd sizes/rounding/edges, error/hostile cases, and expanded P12 no-crop compatibility. Respect unchanged ABI/FFI and the documented P13 rounding contract; distinguish decoded-pixel comparison from byte-identical JPEG.
- **Parallel high-priority reviews:** C-A0 owner/thief SPSC/metrics concurrency concerns remain *hypotheses requiring reproduction*, and P14 actual-host release evidence remains blocked. Neither is silently solved by M1.3.
- **After M1.3 PR CI PASS:** mark **READY_FOR_REVIEW**, **not MERGED**. An authorized review/merge decision is necessary. After confirmed merge, refresh main, choose the next approved unblocked item in `next.md`, and continue if the runner is still active; otherwise checkpoint and stop.
- **When backlog is ambiguous:** prioritize source-backed urgent risk and customer value; do not invent an "M1.4" contract or a production release without a real requirement.

## 5. Minimum persistent checkpoint (commit to appropriate docs/issue, never to a secret)

At every stop, record or link:

~~~text
PROJECT: IMGENGINE
LAST_CONFIRMED_MAIN_SHA: <actual>
TASK_ID / TASK_STATE: <e.g. P13 M1.3 / READY_FOR_REVIEW>
TASK_BRANCH / PR: <actual or NOT CREATED>
TESTED_SHA / CI: <actual link or NOT RUN>
CHANGED_PATHS: <scope>
KNOWN_OPEN_RISKS: <short factual list>
BLOCKER / OWNER_DECISION: <none or precise approval needed>
NEXT_ELIGIBLE_TASK: <ID and reason; no invented feature>
NEXT_BOOTSTRAP_ACTION: <one sentence>
~~~

Keep statuses and evidence in the canonical `next.md` row and task docs; a brief [milestone handoff](AGENT_MILESTONE_HANDOFF_TEMPLATE.md) can link the detail. Do not continuously edit `main` from a branch; a PR may include proposed changes, and merged docs may be refreshed after approval.

## 6. What is and is not automatic

| Setup | What it does | What it cannot do alone |
| --- | --- | --- |
| **Read AGENTS on each new agent session** | Resume choices without repeating project instructions | Launch a new session after it ends |
| **One agent session with task-loop support** | Work through allowed independent tasks before ending | Bypass time/tool/context limits or permission gates |
| **GitHub Actions existing CI** | Run build/security/test jobs after PR/push | Design and implement the next milestone by itself |
| **GitHub issue/PR-triggered coding-agent runner (future setup)** | Wake a configured agent for *a bounded task* | Automatically inherit all owner approvals, or guarantee unattended production-ready code |
| **Periodic task automation (optional)** | Inspect queue, remind or triage when authorized and supported | Replace a coding runner if no agent-tool integration exists |

**Safe way to add genuine continuation later:** choose a supported agent runner and event source; define a **task selector** that fetches `next.md`/issues and verified PR state; grant read-only repo initially, then limited branch/PR writes; sandbox execution; limit per-run tasks, runtime, retries and budgets; enforce PR review and CI; record exact-SHA checkpoints; test cancellation, duplicate events and replay; never allow unapproved merge/deploy/spend. Validate that the agent product supports such unattended triggers before declaring it autonomous. No CI-YAML or agent integration has been installed by this docs PR.

## 7. One-line commands — owner does NOT repeat milestones

**A. If Astra/Codex is still running and supports another task:**

> Continue from the latest repository state under `AGENTS.md` and `docs/AGENT_MILESTONE_CONTINUATION_RUNBOOK.md`. Reconcile merged PRs and exact-SHA CI, update `next.md` as evidence warrants, choose the next READY approved milestone, and prepare one bounded PR. Do not merge/deploy or ask me to restate established policies.

**B. If the old session stopped or a new chat is opened:**

> Resume IMGENGINE from latest `main`: read root `AGENTS.md`, `next.md`, `skills.md` and `docs/AGENT_MILESTONE_CONTINUATION_RUNBOOK.md`; verify current PR/CI, select next authorized READY task, and work until the next review/approval boundary. No repeated setup questions.

**C. If you want truly unattended next-task starts:** the repository document alone is insufficient. **Explicitly configure** a supported agent runner or event/issue automation to invoke instruction B at each verified milestone transition, with deduplication and the safeguards above.

## 8. Project template for future repositories

Copy the *policy*, not IMGENGINE's actual stack/status:
- `AGENTS.md`: mission, technology/forbidden dependencies, permissions, security, review and test discipline.
- `skills.md`: runnable stack-specific task and verification recipes.
- `next.md`: prioritized READY/BLOCKED/MERGED tasks with evidence and next action.
- `docs/AGENT_MILESTONE_CONTINUATION_RUNBOOK.md`: general state machine (replace IMGENGINE example with the new project).
- `docs/QUALITY_GATES.md`, `docs/RELEASE_RUNBOOK.md`, `docs/PRODUCT_VISION.md` as appropriate, not ceremonial boilerplate.

For a new product, first establish real customer problem/MVP, approved architecture, risk profile, and an executable first vertical slice. "Production-ready in hours" is not an acceptance criterion; secure configuration, actual host/runtime verification, meaningful tests, operational recovery and customer fitness require evidence.

**Permanent rule: The agent may choose the next approved task without another custom prompt; only an actual running session or configured trigger can execute it.**
