# IMGENGINE — Five-step GO AHEAD continuation and five-PR feature ceiling

Owner amendment, 2026-11-27. Adapted from [Messy-File-Cleaner feature delivery policy](https://github.com/Rofikali/Messy-File-Cleaner/blob/02b1de63057abf997d1bde0c3e8190a77221910a/docs/FEATURE_DELIVERY_AND_ACCEPTANCE_POLICY.md). Applies to all authorized human and AI contributors. Documentation candidate; not an automation or additional merge authority.

## One manual shortcut

`go ahead` (case-insensitive; optional IMGENGINE qualifier) is the sole manual continuation shortcut. Do not offer a Next button or treat bare next/next milestone/continue as aliases. Retain next.md as live task selector and historical documentation; explicit natural-language requests remain valid. Earlier aliases are superseded. No scheduler created by Markdown.

## Five-step cycle per invocation

1. **Reconcile:** fetch main, all relevant open/recent PRs and branches, exact-head/base CI, live next.md, scoped AGENTS and ledger. Resume work by any author. Never repeat verified work from old snapshots.
2. **Select and scope:** choose highest-priority eligible bounded stable feature. Fix existing relevant PR first. Check dependencies, PR budget, costs and security; if blocked, report exact prerequisite and choose only independent approved work.
3. **Implement:** make smallest substantive change plus tests and manual documentation updates. Batch safe development commits on non-main branches without open PRs where appropriate. Do not restart active CI without a justified fix. No premature dependency, paid service, deployment or security change.
4. **Verify and review:** use Ubuntu 24.04 Docker or exact-head CI; distinguish pass, fail, running, skipped and NOT RUN. Review L1–L7, CISSP, CCIE, Linux/SRE, CA, MBA and management proportionately. Do not merge without existing owner authorization and complete applicable gates. No fabricated test runs.
5. **Checkpoint:** synchronize AGENTS.md, skills.md, README.md, next.md, live-status and affected scoped docs with actual evidence, stable feature/PR count, remaining gaps, rollback and one next eligible action. Push only authorized branches. Report what changed, pending and blocked; offer GO AHEAD only.

Five steps are the execution cycle, not five artificial commits or PRs per message. Complete as much as safely possible; stop early when blocked. No indefinite retries or background promise.

## Maximum five opened PR attempts per stable feature

Count every opened feature-specific PR across contributors, including closed/failed/abandoned/merged PRs. Multiple commits and reruns on the same PR do not count again. Five is a hard ceiling, not a quota. Maintain actual PR numbers in [feature ledger](engineering/feature-budget.json); add the assigned number to the same PR when known. Declare stable Feature-ID(s) in review description; a combined feature PR counts once against each actual feature it materially changes. A stacked PR carrying unmerged ancestor work is counted conservatively until its independent diff is reconciled; never erase a real attempt to obtain another slot. Historical pre-adoption work not fully inventoried must be reconciled before opening further feature PRs; no fabricated counts. After verified acceptance or five attempts, mark DONE/PARTIAL/BLOCKED with gaps and move to an independent READY feature; no renaming to evade budget. Security/release gates never expire. Confirmed safety incidents require a separately justified traceable scope and existing approvals, not a sixth disguised attempt.

This ledger is initially agent-maintained; CI enforcement equivalent to Messy-File-Cleaner is a future bounded governance task, not implemented by this policy. Existing CI policy/doc-sync gates remain. No five-PR limit overrides owner approvals or authorizes merge, release, paid infrastructure, domain, ads, analytics, secrets, destructive operations or branch protection changes.

## Current handoff

PR #127 merged at cfb788635b8ff553794b90707c24d93946f93fdf; planning only. PR #128 head 95b85f038dacfd1f37f4aee3814eacf806e362e1 has 10 successful jobs, browser job running and aggregate pending (run 45969914525). Dependent PR #129 head 919fbef80af48711941480ae1785d0a8323d349d has 4 successful jobs and remaining gates pending (run 45970668283). No failures observed in returned jobs. Both PRs target main; review dependency and exact diff after #128 merge before #129 merge. Do not restart their checks merely to record this instruction. Governance changes staged on separate dependent branch. Actual tests NOT RUN for governance candidate. Domain, finance and monetization deferred; F5 live verification and F6-00 baseline outstanding.
