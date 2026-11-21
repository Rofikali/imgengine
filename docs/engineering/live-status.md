# IMGENGINE — Agent-maintained engineering activity log

**This file is written by the engineer or Codex agent doing actual work.** It is no longer a GitHub-PR-derived dashboard and is never overwritten by GitHub Actions. Its `STATUS` values mean **engineering work**, not `OPEN/MERGED` pull requests. Root [next.md](../../next.md) is the compact current task board; [protocol](DOCUMENTATION_SYNC_CONTRACT.md) defines required checkpoint behavior.

## Active work entry: DOCSYNC-ACTIVITY-01

| Field | Actual recorded information |
| --- | --- |
| Owner | Current engineering agent implementing the 2026-11-21 owner-requested governance correction |
| Last updated | 2026-11-21 UTC |
| Status | **IN_PROGRESS** — implementation is being revised; latest full exact-head verification is pending |
| Starting source | `main` inspected at `c6f1c90dfff73edc77b821a1881b6b9cdd120500` |
| Goal | Remove all PR/scheduled milestone-status document generation and make every agent directly update task status/docs when working |
| Changed boundaries | `AGENTS.md`, `next.md`, `README.md`, `skills.md`, this log and the mandatory work-protocol doc, retired milestone workflow/scripts, prior PR-oriented guard and related handoff/runbook |
| Scope | Documentation governance and CI read-only missing-doc guard; **no** C, Rust, Nuxt runtime implementation or production deployment |
| Review lenses | L1–L7 for reliable work handoff; CISSP for GitHub permissions; CCIE not applicable (no network changes); Linux/SRE for Actions run; CA/MBA for ensuring no unapproved spend/publication |
| Test evidence | Previous PR revision passed policy unit checks (98/98), **but those checks do not verify this revised task**. Current head: `NOT RUN / PENDING` until actual test or Actions result |
| Known limitation | AGENTS instructions govern compliant agents; a static instruction does not asynchronously run Codex or guarantee an untrusted agent follows rules. Read-only CI can check files, not accuracy |
| Next action | Run docs checker tests against the updated task, verify removed workflow triggers, review exact-head CI; record actual outcome here before marking `VERIFIED` or `COMPLETED` |

### Change history — activity-based

- **2026-11-21 — IN_PROGRESS:** Owner clarified that documentation must be authored **during each engineer/agent task**, not derived from PR state; this change replaces PR-status workflow with manual-in-code documentation and evidence-based engineering states
- **2026-11-21 — IN_PROGRESS:** Previous bot run `44904194415` had failed at PR creation permission; this is historical evidence of why PR-derived status was stale, not a live task status or a reason to enable bot permissions

## Future entries — template

For each meaningful task, create or update a distinct `Task ID` with: owner, UTC checkpoint, state (`PLANNED / IN_PROGRESS / BLOCKED / IMPLEMENTED / VERIFIED / COMPLETED`), source baseline, **actual files changed**, changes/why, tests with commands and outcome plus SHA/environment, risks/rollback, docs changed/reviewed, and one concrete next action. Update on start, after meaningful changes, and before stop/handoff. Do not create a separate PR for status.

**Free v1.0.0:** `NOT_RELEASED` until independent security, immutable artifacts, privacy/legal/support, Cloudflare live verification, rollback, cost and explicit owner release approval.
