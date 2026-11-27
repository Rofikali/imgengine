# CI-EFF-01 — Efficient development batching and trusted scope refinement

**Owner override — 27 November 2026, GO AHEAD ONLY:** `go ahead` is the sole manual continuation keyword (case-insensitive, optionally followed by IMGENGINE). Retire `next` as a shortcut; `next.md` remains the live board, not a command. Apply the five-step execution cycle and maximum five opened PR attempts per stable feature ID in [continuation policy](../IMGENGINE_FIVE_STEP_CONTINUATION_POLICY.md). Count all authors, including closed/failed PRs; same-PR fixes and ordinary development pushes do not consume another slot. Finish earlier if accepted, stop on required blockers, and do not reset counts by renaming. Preserve explicit merge/deploy/spend approval boundaries, no new autopilot, no security bypass. This owner instruction supersedes any older shortcut aliases.

2026-11-27. Owner requested quicker frontend development. Work state: IMPLEMENTED candidate on an unmerged development branch; verification NOT RUN. Base PR #127 head 9937aeeeb80eec7f04e35a327f6800fd832f7108; main at 1f57493bb0d0b6545bc03da12aed195bc5a87b1e. PR #127 must not be reset or have its running checks cancelled. Branch is dependent on that roadmap; reconcile ancestry after owner merge (especially squash). No release or deployment approval.

## Evidence and root cause

[Run 45967764812](https://github.com/Rofikali/imgengine/actions/runs/45967764812) selected full CI for a docs-only PR. Existing trusted selector treats AGENTS, skills, scoped AGENTS and several mandatory planning/log files as full. Browser job runs normal Nuxt build and local-tool matrix, then Free static build, static/publication verifier and full static browser matrix. This is correct under existing policy, not a test failure. Actual elapsed durations and resource costs not measured.

## Proposed workflow

1. Develop on an ordinary non-main branch without an open PR. The current main workflow triggers on push to main, pull_request and merge_group. Other workflow triggers must be checked independently. Commit bounded changes with synchronized docs. Mark tests NOT RUN if not executed.
2. At a coherent integration checkpoint, open one PR, run trusted-base impacted jobs and policy gate. Do not repeatedly push while checks are in progress unless fixing a verified issue; new pushes may cancel superseded CI. No skip-ci tokens, branch protection change, direct main push or owner-approval bypass.
3. Before any production authorization, full exact-source and exact-artifact tests, dependency/security/privacy/legal/hosting/cost evidence and rollback. Main push/merge_group still select all nine jobs. Release candidate evidence remains independent.
4. If approved after full CI, refine classification using exact allowlisted planning Markdown and four known frontend shell files. Planning-only gets always-on scope/policy/doc-sync/aggregate; shell plus mandatory docs gets both Nuxt checks. Unknown paths, security/release docs, workflow/selector, package/config/lockfiles, native or ambiguous changes remain full. Existing trusted base means this PR must run full CI before the change applies. No broad markdown glob or exception for arbitrary frontend scripts.
5. Next potential improvements: measure browser step durations; investigate sharding and safe dependency cache only after profiling. Keep every required browser/profile, static security check and failure gate. Do not postpone all testing until production.

## Review and tests

New selector fixtures cover mandatory docs, known frontend shell plus mandatory docs, PR #127 path set, high-risk mixed diffs and deletion. Existing gate truth-table and historical dry-run tests remain unchanged. Run in Ubuntu 24.04 approved environment: node --test scripts/ci-scope-select.test.mjs scripts/ci-scope-gate.test.mjs scripts/ci-scope.test.mjs scripts/docs-sync-check.test.mjs; git diff --check. NOT RUN locally; no Docker evidence. CI workflow unchanged. Full CI on the integration PR is mandatory.

L1–L7: bounded classifier, exact path coverage, testable fail-closed behavior and integration/release separation. CISSP: trusted-base/unknown full behavior, no bypass. CCIE: no network boundary change. Linux/SRE: reproducibility and job timings. CA/MBA: reduce duplicate runner work, no actual cost claim; no paid runner. Management: staged work, bounded integration checkpoint and truthful status. Native/ABI, Rust, privacy and release contracts reviewed — no contract change. Domain, finance and monetization deferred.
