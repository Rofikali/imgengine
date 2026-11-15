# IMGENGINE — Agent Milestone and Engineering Handoff Template

**Status:** Reusable task template, not a record of completed implementation (2026-11-15). Use with [Seven-Level Engineering Operating Model](SEVEN_LEVEL_ENGINEERING_OPERATING_MODEL.md), [root AGENTS](../AGENTS.md), [Astra start guide](ASTRA_ENGINEERING_START_HERE.md), [technology ADR](TECHNOLOGY_AND_REPOSITORY_POLICY.md), [change-scoped verification](CHANGE_SCOPED_VERIFICATION.md) and affected scoped AGENTS.

**Purpose:** Give Astra, Codex or another coding/review agent one bounded task at a time. It is **not** an automatically installed agent or skill; the operator must direct the model to read this file. Source, contracts and tests at the current SHA are authoritative, not the contents of an old prompt.

## A. Fill this before delegation

| Field | Fill with actual evidence; do not guess |
| --- | --- |
| Task ID and short name | [e.g. P13-M1.1; preserve canonical next.md ID] |
| Requested outcome and customer value | [what changes for whom?] |
| Mode | [read-only audit / docs-only / code + tests / release review] |
| Exact repo, base branch and starting SHA | [actual SHA; do not copy an outdated example] |
| In-scope directories and files | [minimal list with scoped AGENTS] |
| Explicitly out of scope | [including infrastructure/legacy where not involved] |
| Authoritative contracts and API/ABI versions | [links to exact docs and installed headers] |
| Existing behavior and measured baseline | [commands/evidence or UNKNOWN] |
| Acceptance tests and negative paths | [specific assertions and fixtures] |
| Security/privacy/resource invariants | [attack surface, memory budget, lifetime and logging] |
| Release/rollout/rollback impact | [none / review-only / separately approved] |
| Business/CA-MBA question | [cost per success, support, value, opportunity cost] |
| Human decision required | [none until review / explicit approval needed for X] |
| Deliverable and max changed scope | [one small PR, no unrelated refactor] |

**Hard exclusions:** no new Python/FastAPI/Django/DRF/Flask, Celery, Redis, PostgreSQL, Python scripts, automatic database/queue adoption or unapproved paid infrastructure. Retained legacy files and established verification scripts are not invitations to extend the legacy product or delete necessary checks.

## B. Ordered agent execution instructions

1. **Inspect first:** report current HEAD, merge base, directory/changed-path list, existing tests and any uncommitted work. Never erase, clean, reset or rewrite user changes.
2. **Read minimal authority:** root AGENTS and next.md, this task, affected scoped AGENTS, binding technology ADR, current API/ABI contracts and impacted source. Historical design notes are evidence of intent, not implementation truth.
3. **State status:** implemented / recorded at an older revision / reproduced now / partial / blocked / planned / legacy. Distinguish hypothesis from verified security defect.
4. **Design before edit:** compact HLD+LLD for material boundaries, failure paths, input/ownership/clock/thread invariants, alternative options and an acceptance matrix. Do not implement imagined HTTP or ABI fields.
5. **Apply seven-level review lenses:** include the relevant items from C below. Scale review depth with change risk; do not generate ceremonial role reports for trivial diffs.
6. **Implement smallest bounded change only when task mode authorizes editing.** Keep public ABI and P12 existing behavior stable unless an explicitly versioned and approved contract changes.
7. **Verify with relevant gates:** use change-scoped testing for developer loop; never skip existing required CI, security or release evidence. If the environment cannot run a test, mark it **NOT RUN**, not PASS.
8. **Business/security review:** detail attack surfaces, confidentiality, operational cost per success and customer value, leaving unknown inputs explicitly unknown.
9. **Review own diff:** scan unrelated changes, broken links, unintended secrets/images, new dependencies, portability and backwards compatibility; explain rollback/revert.
10. **Hand off:** report SHA, changes, exact tests/results, fresh vs inherited evidence, open risks, updated next.md item (only after genuine completion) and next single milestone. Stop before merge, deployment, legacy removal, purchases or secret/DNS changes without separate explicit approval.

## C. Seven-level quality checklist (use proportionately)

- **Junior:** Can I reproduce the problem and name the file, function, test and expected output?
- **Mid-Level:** Does the bounded feature work through the expected layers, including errors and cleanup?
- **Senior:** Is HLD/LLD correct on security, ownership, reliability, tests and operational recovery?
- **Staff:** Are API/ABI contracts, module responsibilities, cross-layer parity and integration sequencing consistent?
- **Senior Staff:** Are concurrency, end-to-end capacity, deployment/rollback and multi-track dependencies correctly managed?
- **Principal:** Is this the simplest safe architecture and an evidence-backed performance/business trade-off?
- **Distinguished:** Does the choice scale to a genuine differentiated product and portable long-term capability, or is it unnecessary complexity?

**Cross-functional checks:** Security (threat model, privacy, abuse, native memory, secrets); SRE (latency distribution, RSS and isolation, logs and recovery); product/UX (accessible task completion); CA/finance (margin, cash, total cost); MBA/management (priority, ROI and opportunity cost). These perspectives advise one accountable human decision-maker rather than overriding project consent.

## D. Required final report for each agent

Use concise factual headings and complete every field; a blank or UNKNOWN item is acceptable but unsupported PASS is not.

### Task and baseline
- ID / goal / mode:
- Base SHA, new SHA, platform/tool versions:
- Scoped files and current contract:
- Known legacy and out-of-scope boundaries:

### Evidence and technical decision
- Observed source behavior and reproduction:
- HLD + LLD, data flow, lifetime and failure cases:
- Options and trade-offs; why the chosen solution:
- DSA/complexity/design pattern only if material:
- API/ABI compatibility and changes (none if not changed):
- Performance and resource measurements, or NOT MEASURED:

### Quality, security, economics
- Threat/privacy/abuse risks and mitigations, with source evidence:
- Tests: command | environment | result | exact SHA | limitations:
- Known failures / skipped checks / dependencies:
- Customer impact and financial assumptions, actual vs forecast:
- Support, operational and rollback effects:

### Deliverable and next gate
- Changed paths / commit(s) / PR:
- next.md status/evidence update (or WHY NOT YET):
- Residual risk register with owners:
- Required human approval:
- Next one bounded milestone:

## E. Concrete current examples (not authorizations or fresh test results)

### Example 1: P13-M1.1 — Rust crop transport contract

**Read first:** [P13 contract](P13_INTERACTIVE_CROP_CONTRACT.md), [P12 compatibility](P12_PHYSICAL_SHEET_CONTRACT.md), [Rust FFI](RUST_FFI.md), [HTTP API](api.md), [backend AGENTS](../backend/AGENTS.md), [frontend AGENTS](../imgengine-saas/frontend/AGENTS.md), native installed ABI 2.3 header, current Rust source and tests.

**Goal:** identify/freeze the exact already supported C ABI 2.3 crop parameters and safely carry them through Rust ownership/error handling, Axum and typed Nuxt proxy. Preserve P12 ABI 2.2 centered-fill semantics, size/resource limits and confidentiality.

**First deliverable:** **read-only** interface/compatibility matrix and minimal implementation/test design, explicitly listing current fields rather than inventing names. **Follow-on implementation:** bounded M1.1 PR only after contract is reviewed. **Not in this task:** frontend drag/zoom gestures, Wasm, static .a SDK, payments, VPS deployment or legacy deletion.

**Acceptance criteria for follow-on:** invalid normalized values rejected consistently; no unsafe pointer lifetime; deterministic input/output/error mapping; P12 regression and old consumer compatibility; Axum and Nuxt proxy tests; no secret leak or unbounded request memory; exact-revision tests recorded. This example does **not** claim M1.1 has already been implemented.

### Example 2: C-A0 — native concurrency/security audit (read only)

**Read first:** [C17 source audit](C17_CORE_PRINCIPAL_SOURCE_LLD_DSA_SECURITY_AUDIT_2026-11-15.md), [native AGENTS](../imgengine/AGENTS.md), actual SPSC queue/worker stealing/metrics source, caller graph, test files and relevant previous evidence.

**Goal:** determine whether owner and stealing workers can consume the same SPSC queue concurrently; inspect metrics ordinary vs atomic accesses and affected reader/writer reachability; derive reproducible schedule and invariants. Treat suspected races as **unproven** until traced/tested.

**Deliverable:** exact function/file paths and revision, producer/consumer/thread identity table, a proposed deterministic 2–8 worker test harness, issue severity conditional on confirmed reachability, minimal remediations with behavioral/ABI risks, and proof requirements. **No code changes** in this first step, no scheduler rewrite, no claim that tests were run.

### Example 3: P14 — actual-host release gate (review only until authorized)

**Read first:** [infra scoped rules](../infra/AGENTS.md), [infra audit](INFRA_SECURITY_OPERATIONS_AUDIT_2026-11-15.md), [actual-host runbook](P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md), [MVP release checklist](MVP_RELEASE_CHECKLIST.md).

**Goal:** list the evidence missing before releasing a single-host Caddy -> Nuxt -> private Rust -> native C image service. Require actual file-backed secret ownership/mount proof (Compose mode fields alone are insufficient), CPU/RSS/abuse ceilings, TLS, observable error/latency metrics, recovery and rollback. Report **NOT VERIFIED on real VPS** until actual authorized tests are performed. No deploy, purchase, DNS or secret action in an audit.

## F. Copy/paste task prompt for Astra, Codex or another agent

> Work on the IMGENGINE repository as a careful engineering implementation/review assistant, not as an unrestricted autonomous team. First inspect current git SHA/status and read root AGENTS.md, next.md, docs/TECHNOLOGY_AND_REPOSITORY_POLICY.md, docs/SEVEN_LEVEL_ENGINEERING_OPERATING_MODEL.md, docs/AGENT_MILESTONE_HANDOFF_TEMPLATE.md, docs/ASTRA_ENGINEERING_START_HERE.md, and the selected path's scoped AGENTS + current source/contract/tests. Apply Junior, Mid, Senior, Staff, Senior Staff, Principal and Distinguished review lenses proportionally to actual risk, plus Security, SRE, product, CA/MBA and management perspectives. Focus **only** on the assigned task ID and mode. Preserve C17/C ABI -> safe Rust FFI/Axum -> Nuxt/Vue/TS and legacy boundaries. Do not add new Python, Redis, PostgreSQL, a new database, queue or unapproved service. Before editing, report precise baseline, HLD/LLD/trust/resource and business risks, smallest safe diff and acceptance tests; distinguish hypotheses from confirmed defects. After work, report exact changed paths/SHA, commands/results, residual risks, rollback and next.md update only if supported by tests. Do not misrepresent old CI as new proof and do not merge, deploy, change secrets/DNS, retire legacy or spend money without explicit human approval.

**Then append your actual Task ID, scope/mode and acceptance criteria from section A.** Never send this generic prompt alone as authorization for a broad rewrite.
