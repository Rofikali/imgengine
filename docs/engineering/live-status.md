# IMGENGINE — Agent-maintained engineering activity log

**25 November 2026 · F5-BROWSER-VERIFY-REPAIR-01.** [Notice PR #120](https://github.com/Rofikali/imgengine/pull/120) merged. All 12 jobs passed at PR head `18f19d6e1678671ae14826d2605c9c94df30aca6` and main `080029325a7862c8dab89fa28dfdc2b1242de689`, including the full static matrix. [PR browser job](https://github.com/Rofikali/imgengine/actions/runs/45336581571/job/138250029844) · [main browser job](https://github.com/Rofikali/imgengine/actions/runs/45338597623/job/138256544053).

The [manual preflight](https://github.com/Rofikali/imgengine/actions/runs/45546232678) exposed a separate launcher defect. It inherited the caller's homepage mode. Desktop Chromium then expected the server heading on the Free homepage. The repair sets the static mode in every browser subprocess. All 23 focused checks passed in Ubuntu Docker with a conflicting caller setting. Run the manual preflight at the repair commit to verify the four projects, archive and independent guard. F5 publication remains blocked.


Historical pre-merge checkpoint. Notice review — IMPLEMENTED on a local review branch. Three focused notice and guard tests passed in Ubuntu 24.04 Docker. GitHub verification of the changed pages awaits branch publication.

PR CI evidence — IN_PROGRESS. Required local Playwright image acquisition remains unavailable. After publication, record the review commit, PR URL, job URL and browser-step conclusion for the changed notice sources.

24 November — COMPLETED. Updated the standing session-start rule and contributor entry points. Verified the root links, Linux Docker engine and Ubuntu 24.04 identity in the local P7 container.


**Billing evidence update — COMPLETED.** Recorded dated subscription and earlier project evidence in the register, matrix and worksheet. Next owner evidence: Billable Usage period and result.


**Hosting decision — 23 November 2026.** Cloudflare Pages is primary. Netlify is deferred. The release manifest and privacy draft already name Cloudflare. Workers Free subscription settings were verified by the owner on 23 November. Account inventory and RUM site count are verified. Billable Usage and production evidence remain pending. See the [consolidated matrix](../releases/free/v1.0.0/evidence-matrix.md). Use the [read-only evidence worksheet](../releases/free/v1.0.0/CLOUDFLARE_LAUNCH_GATE_AND_SMOKE_TEST.md). PR #115 records a proposal. Governance, artifact and deployment approvals remain separate and publication is BLOCKED.


**23 November 2026 — governance proposal prepared, publication BLOCKED.** The [owner compliance register](../releases/free/v1.0.0/owner-compliance-register.md) records source evidence, legal uncertainties, synthetic Gmail decisions and account checks awaiting verification. Independent legal review remains required pending a separate governance decision.


**Free browser bundle licenses (22 November 2026).** Vite emits client dependency licenses to _nuxt/third-party-licenses.txt and Terms links to them. The local review matched all 16 MIT-labelled package versions to the production inventory. Independent notice and artifact approval remain pending.


**Free notices — IMPLEMENTED for independent review.** The three draft pages share operator and pending-date metadata. The 12-month support retention proposal requires a manual rehearsal and operator approval. Three local notice/guard tests passed. See the [operator review packet](../releases/free/v1.0.0/operator-review.md) for Gmail handling, Indian commencement dates and Cloudflare verification.


**Convert picker VERIFIED and candidate review (22 November 2026).** [PR #111](https://github.com/Rofikali/imgengine/pull/111) passed all 12 jobs at 9242d51 and [merged-main CI](https://github.com/Rofikali/imgengine/actions/runs/44990709168) passed at 27ee446. The review build contains 47 static files. The detached candidate tar SHA-256 is 2fb4da2bf5c5dd18b9e70675024db1d277f33ace1d3498d2fd5edf8b2beb618b. The local production dependency inventory covers 511 package versions. Final artifact freeze, shipped-license review, approved notices and hosting gates remain open.




**Free launch notices — IMPLEMENTED, publication BLOCKED (22 November 2026).** Rofik Ali approved the operator identity, Gmail contact and intended date on the three Free notices. Before freezing the artifact, confirm actual mailbox and host retention, obtain independent legal review, and update all notice dates and their test to the publication date if it changes. The publication guard still applies.


**Firefox static privacy verification — COMPLETED (2026-11-21).** All 12 jobs passed on merged main `aa5b834f97e7e000ee86aaa17b127e66f979f1d4`, including the Nuxt browser matrix and required verification gate. Free v1.0.0 remains blocked. [post-merge CI](https://github.com/Rofikali/imgengine/actions/runs/44914244032) Ubuntu 24.04 passed the frozen install, Free static build and nine-route verifier (47 files) on the cf3d192 frontend tree. [PR-head CI](https://github.com/Rofikali/imgengine/actions/runs/44913355058) passed at 418f2e6.

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
