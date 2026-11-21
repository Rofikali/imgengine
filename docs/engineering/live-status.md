# IMGENGINE — Agent-maintained engineering activity log

**F5 browser failure investigation — IMPLEMENTED, verification pending.** The Firefox landing-page privacy audit timed out waiting for load in shard 12/55 on main 70d6e61. The candidate test waits for a committed navigation and then for the document navigation timing entry to report loadEventEnd. The 30-second deadline and request, cookie and storage checks still apply.

**This file is written by the engineer or Codex agent doing actual work.** It is no longer a GitHub-PR-derived dashboard and is never overwritten by GitHub Actions. Its `STATUS` values mean **engineering work**, not `OPEN/MERGED` pull requests. Root [next.md](../../next.md) is the compact current task board; [protocol](DOCUMENTATION_SYNC_CONTRACT.md) defines required checkpoint behavior.

**READY-board reconciliation — COMPLETED (2026-11-21).** F5-SUPPLY-05 landed in [PR #97](https://github.com/Rofikali/imgengine/pull/97). The frontend lockfile at main revision `70d6e610635e63795db2a3fb9f7b01b594483f06` matches the merge revision `c1141f1`. [Main CI](https://github.com/Rofikali/imgengine/actions/runs/44909116504) failed the Nuxt browser matrix and its required verification gate. Free v1.0.0 remains blocked.

## Active work entry: DOCSYNC-ACTIVITY-01

| Field | Actual recorded information |
| --- | --- |
| Owner | Current engineering agent implementing the 2026-11-21 owner-requested governance correction |
| Last updated | 2026-11-21 UTC |
| Status | **COMPLETED** — source change, documentation handoff and exact-main verification are complete |
| Starting source | `main` inspected at `c6f1c90dfff73edc77b821a1881b6b9cdd120500` |
| Goal | Remove all PR/scheduled milestone-status document generation and make every agent directly update task status/docs when working |
| Changed boundaries | `AGENTS.md`, `next.md`, `README.md`, `skills.md`, this log and the mandatory work-protocol doc, retired milestone workflow/scripts, prior PR-oriented guard and related handoff/runbook |
| Scope | Documentation governance and CI read-only missing-doc guard; **no** C, Rust, Nuxt runtime implementation or production deployment |
| Review lenses | L1–L7 for reliable work handoff; CISSP for GitHub permissions; CCIE not applicable (no network changes); Linux/SRE for Actions run; CA/MBA for ensuring no unapproved spend/publication |
| Completion evidence | Source change merged through #106 at `ad8b4a0a3ca5db71af86702b02cec2c8c8acaf92`; [exact-main run 44906583851](https://github.com/Rofikali/imgengine/actions/runs/44906583851) passed CI policy/scope, native build/fuzz/sanitizers, Nuxt frontend and browser quality, P8 parser, Rust API container, SaaS integration, Ubuntu Docker verification and the CI verification gate. |
| Automation cleanup | Stale bot-authored PR #107, based on pre-#106 `main` and proposing only PR-derived status edits, was closed unmerged. No bot branch, scheduled workflow or PR lifecycle event may update task records. |
| Local verification | **NOT RUN** — this checkpoint records an already merged governance-only change; Docker Desktop Linux availability and approved P7 definitions were inspected, and no Windows runtime was used. Exact-main GitHub CI above is the authoritative verification. |
| Remaining risk / next action | Free v1.0.0 remains **BLOCKED — NOT_RELEASED** pending independent legal/privacy/support, frozen artifact/supply-chain, live host/TLS/rollback, financial and owner go/no-go evidence. The next substantive task must create its own agent-authored `IN_PROGRESS` entry before changes. |
| Test evidence | Ubuntu 24.04 Docker / Node `v24.22.0`: `node --test scripts/docs-sync-check.test.mjs` **PASS (4/4)** and `node scripts/docs-sync-check.mjs HEAD^ HEAD` **PASS** (`engineer-updated`, 5 changed paths). #106's exact-main CI evidence is recorded above. |
| Known limitation | AGENTS instructions govern compliant agents; a static instruction does not asynchronously run Codex or guarantee an untrusted agent follows rules. Read-only CI can check files, not accuracy |
| Next action | Maintain these records directly on every future substantive task. Free v1.0.0 has no eligible autonomous release action while its independent owner/legal/hosting gates remain blocked. |

### Change history — activity-based

- **2026-11-21 — IN_PROGRESS:** Owner clarified that documentation must be authored **during each engineer/agent task**, not derived from PR state; this change replaces PR-status workflow with manual-in-code documentation and evidence-based engineering states
- **2026-11-21 — IN_PROGRESS:** Previous bot run `44904194415` had failed at PR creation permission; this is historical evidence of why PR-derived status was stale, not a live task status or a reason to enable bot permissions

## Future entries — template

For each meaningful task, create or update a distinct `Task ID` with: owner, UTC checkpoint, state (`PLANNED / IN_PROGRESS / BLOCKED / IMPLEMENTED / VERIFIED / COMPLETED`), source baseline, **actual files changed**, changes/why, tests with commands and outcome plus SHA/environment, risks/rollback, docs changed/reviewed, and one concrete next action. Update on start, after meaningful changes, and before stop/handoff. Do not create a separate PR for status.

**Free v1.0.0:** `NOT_RELEASED` until independent security, immutable artifacts, privacy/legal/support, Cloudflare live verification, rollback, cost and explicit owner release approval.
