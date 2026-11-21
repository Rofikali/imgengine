# IMGENGINE — Agent-owned documentation and engineering activity protocol

**Effective on adoption (2026-11-21).** This is the controlling documentation rule for Codex, other AI agents, human developers, reviewers and maintainers. **Documentation is authored by the engineer doing the work, while that work happens. It is never generated from PR events or inferred from PR state.**

## 1. Mandatory updates during real work (not after a PR)

Every meaningful engineering action—creating/editing/removing source, tests, CI, infra, designs, contracts, security controls, dependencies, or project docs—requires a truthful *contemporaneous* written record. Record at least one entry per bounded action or work session; do not generate empty reports for trivial keystrokes.

**At the start, before changes:** inspect current code, open work, applicable scoped `AGENTS.md`, and active status. Create/update a task ID in `next.md` and `docs/engineering/live-status.md` with responsible agent, baseline, goal, affected paths, timestamp/date, planned tests, remaining risks, and status `IN_PROGRESS`. Record `PLANNED` only for not-yet-started work.

**While changing anything:** after each meaningful change batch, update the same work entry: actual files changed, what changed, why, progress, new failures/decisions and remaining steps. In the *same working tree and implementation commit*, synchronize these four roots:
- `AGENTS.md` — current binding/agent-operating implications and a short dated task handoff/reference (preserve durable rules);
- `next.md` — actual `IN_PROGRESS`, `BLOCKED`, `IMPLEMENTED`, `VERIFIED`, or `COMPLETED` task status with NOW/NEXT/blockers;
- `README.md` — current user/operator-facing behavior, scope, limitations or concise task checkpoint;
- `skills.md` — repeatable implementation/verification commands or precise link to procedure and status.

Do not blindly rewrite large documents. A concise and *meaningful* update, reference and evidence belongs in each. Update affected scoped `AGENTS.md`, HLD/LLD/ADR, API/ABI, C/Rust FFI, tests, security, threat model, SRE/observability, Linux/Docker, CCIE networking, privacy, financial/CA-MBA, management, and release notes only where the work actually changes their contracts. If a document is unaffected, explicitly say `reviewed — no contract change` in the task entry instead of inventing facts.

**Before stopping, handing off or committing:** update all records to the *actual current* state, document tested commands/environment/commit SHA (or label `uncommitted; no immutable SHA yet`), and name one next action. If stopped early, leave `IN_PROGRESS` or `BLOCKED`, do not fabricate a finish. If tests were not run, mark `NOT RUN`. Never defer this documentation to a future PR or a separate documentation-cleanup PR.

## 2. Truthful engineering states

| State | Meaning; evidence required |
| --- | --- |
| `PLANNED` | An authorized goal exists; implementation not yet started |
| `IN_PROGRESS` | Actual work begun; record changed paths and outstanding steps |
| `BLOCKED` | Real obstacle; include error/evidence, owner and unblock condition |
| `IMPLEMENTED` | Scoped code/docs written, but required verification incomplete; not a PASS |
| `VERIFIED` | Required scoped tests have actually passed for the recorded SHA/environment; broader gates must be explicitly marked pending if so |
| `COMPLETED` | Task acceptance criteria, required verification and documentation handoff all satisfied; record evidence and next work |

**Work completion is not PR merge or public release.** `RELEASED` additionally requires explicit independent legal/privacy, security, supply-chain, infrastructure/rollback, business/CA-MBA and owner approval. A merged PR, passing partial test or CI status alone never changes a work item to `COMPLETED` or a product to `RELEASED`.

## 3. Explicitly retire PR-driven documentation

The former `.github/workflows/milestone-status-sync.yml`, its unit workflow, and `scripts/milestone-sync*` / `scripts/milestone-status-publication*` are retired. No schedule, PR lifecycle event, bot branch or generated milestone dashboard shall edit `next.md` or `docs/engineering/live-status.md`. Remove the `BEGIN/END AUTO-GENERATED PR MILESTONES` region from `next.md`; both are now **agent-maintained, human-readable task records**.

Pull requests may still be used for normal code review and merge authorization. **They do not create, overwrite, publish or update documentation and never determine task status.** The engineer must write docs as part of the work *before* a PR exists, if a PR will be used at all.

A read-only local/CI diff checker may **reject omitted documentation**, but it is never a documentation writer, PR-status reader or autonomous agent. It checks changed paths; it cannot prove the content truthful. Human review and actual tests remain essential.

## 4. Mandatory review lenses (proportionate to change)

- **L1–L2:** understand requirement, bounded diff, unit/edge tests, usage and errors.
- **L3–L4:** architecture HLD/LLD, API/ABI, C/Rust lifetime/FFI, integration, security and rollback.
- **L5–L7:** product-family compatibility, dependency sequencing, system trade-offs, evidence-backed performance, maintainability and long-term strategy.
- **CISSP/security:** threats, privacy, secrets, supply chain, permissions, incident and risk ownership.
- **CCIE/networking:** DNS/TLS, ingress/egress, trust boundaries, concurrency/backpressure, availability.
- **Linux/SRE:** reproducible Docker/Linux, deployment safety, resource ceilings, observability, recovery, SLO/test evidence.
- **CA/MBA/management:** cost ceilings, unit economics, user impact, opportunity cost, release decisions and approvals.

Apply only relevant lenses, noting `not applicable` when so. No unsupported micro/nanosecond benchmark claims, security waivers, new Python server path, involuntary costs, automatic merge, tag or deployment.

## 5. Agent task template

```text
Task ID: IMG-...       Owner/agent: ...
Updated (UTC): ...     Status: PLANNED | IN_PROGRESS | BLOCKED | IMPLEMENTED | VERIFIED | COMPLETED
Goal and scope: ...
Actual files changed + reason: ...
Documentation synchronized: AGENTS / next / README / skills / affected scoped docs
Architecture/security/network/SRE/finance impact: ...
Commands and evidence: PASS/FAIL/NOT RUN, environment and SHA (if committed)
Risks/blockers and rollback: ...
NEXT exact bounded action: ...
```

Read and update [live work board](../../next.md), [agent-maintained engineering log](live-status.md), and the relevant scoped AGENTS on **every** work handoff. Existing Free v1.0.0 remains **NOT_RELEASED** until independent owner authorization.
