# IMGENGINE — PR-driven milestone synchronization (F5-DOCSYNC-01)

**Verified operational checkpoint (2026-11-21):** [PR #91](https://github.com/Rofikali/imgengine/pull/91) **MERGED** as `76d42f094e5df13d8eefe0ad8a417f9214e5a2a7`; [scheduled generator run #44840635463](https://github.com/Rofikali/imgengine/actions/runs/44840635463) **SUCCESS** with a generated bot branch. [Initial post-merge run #44780901126](https://github.com/Rofikali/imgengine/actions/runs/44780901126) **FAILED** because GitHub refused Actions PR creation. Subsequent identical-branch runs exited success without opening a PR; this is a confirmed workflow defect, not a successful end-to-end automation. The generated snapshot is under [manually opened PR #92](https://github.com/Rofikali/imgengine/pull/92). F5-DOCSYNC-02 is a **review candidate** to require an actual open review PR or explicitly fail and retry. Repository owner must enable GitHub Actions PR creation; this PR does not change that account setting. **No automatic merge, release, cloud action or spending.**

## Authority and scope

- `docs/engineering/milestones.json` is the **reviewed task registry**: milestone ID, title, declared state, priority, dependencies and an optional **explicit PR number**. Only humans/reviewed implementation PRs change the task graph. Unknown PR titles are never interpreted as project authorization.
- GitHub's authenticated PR/check APIs are the **status evidence**: open, draft, CI pending/failed, exact-head `CI verification gate` success, closed-unmerged, or actual **merged** with merge SHA. A merge is not a release.
- `next.md` contains the generated NOW/NEXT/WORKING/COMPLETED region, above older dated notes. `docs/engineering/live-status.md` is the generated dashboard. **Do not edit the generated region manually.**
- Root `README.md`, root and scoped `AGENTS.md`, `skills.md`, architecture/HLD/LLD/ADR, finance/security/legal and runbooks are durable contracts. **Do not rewrite them automatically for each PR.** A reviewed milestone PR updates those only when the actual implementation or operating rule changes.

## Runtime

Two new workflows are proposed:

1. `.github/workflows/milestone-status-tests.yml`: read-only unit tests on relevant pull requests and main commits. Covers dependency ordering, open/merged/failed states, no fabricated CI success, bad API results, history preservation and Markdown sanitization.
2. `.github/workflows/milestone-status-sync.yml`: trusted `main` checkout on main pushes, PR lifecycle events, every two hours, or manual dispatch. Uses the GitHub REST API to collect referenced PRs and other currently open PRs; changes only generated status files. If changed, it updates a **dedicated bot branch** and opens/refreshed a normal, **review-required** GitHub PR.

The sync workflow uses a write-capable `GITHUB_TOKEN` but does **not** checkout or run code from a contributing PR or fork. It runs only reviewed default-branch Node code, pins checkout, uses least feasible repository-level `contents:write`/`pull-requests:write`, refuses stale main, refuses non-bot changes on its dedicated branch, force-with-lease protects the bot branch and never pushes to `main`. It neither merges PRs nor publishes a release.

## Important GitHub account prerequisites

**For automatic PR creation, the owner may need to enable** `Settings → Actions → General → Workflow permissions → Allow GitHub Actions to create and approve pull requests`. This is a GitHub account/repository policy decision; the code cannot silently change it. It does **not** mean granting release approval.

GitHub may restrict `pull_request_target` in public repositories. If those events are disabled, the `push main`, schedule and manual dispatch remain fallbacks. Repo/organization Actions policy can independently block a workflow; inspect run logs and do not claim it is active until a real event run succeeds.

A PR created with the repository `GITHUB_TOKEN` may not automatically start normal PR CI, or may need owner approval. **Human review and relevant CI evidence are mandatory before merging the generated documentation.** If GitHub does not permit automatic PR creation, the workflow fails visibly; it does not fall back to a direct main push or weaker token.

## How to keep status genuinely current

- Each implementation PR should include a reviewed registry update mapping its **actual PR number** to the existing milestone. Avoid guessing a number before GitHub allocates it.
- PR opened/draft/updated/closed/merged triggers a fresh snapshot when supported; merge also triggers a main push. The next scheduled run catches missed events. The bot refreshes only its generated documentation PR, not every contract.
- `NOW` is the highest-priority **eligible, not completed** task declared in the registry; `NEXT` is the next outstanding record, including blocked dependencies; `COMPLETED` requires verified `merged_at` and merge SHA. Known unrelated open PRs are displayed but not assigned to a milestone automatically.
- If work is intentionally CANCELLED, blocked by a security/finance/release decision, or requires a new task, add a reviewed registry change. Do not let an AI infer intent or release authorization from a PR title.
- A generated status PR is itself unmerged until the owner reviews it. GitHub's current PR page remains the authoritative live view between snapshots.

## Releases, risk, rollback and cost

The registry hard-codes `release_state=NOT_RELEASED`; this mechanism **cannot** mark a product released. Explicit independent owner approval, clean source-locked release audit, privacy/legal/support signoff, immutable artifact, hosting/rollback and finance remain separate release gates. Neither the report nor a GitHub green check bypasses security.

Rollback: disable this single sync workflow, close the generated status PR and revert the reviewed workflow/script/registry changes; previous source and historical notes are preserved. The workflow uses hosted GitHub Actions runners, public GitHub API and no third-party paid service, but available minutes and enterprise/repository policy must be checked rather than claiming cost is always zero.

## Acceptance checklist

- [ ] Unit tests pass on the PR head.
- [ ] Required project CI and aggregate gate pass at the exact PR head.
- [ ] No PR fork code or untrusted commands execute with write credentials.
- [ ] After merge, verify one **real** workflow run, correctly opened/updated bot PR and the complete next dashboard.
- [ ] Owner reviews and merges generated docs PR only if accurate.
- [ ] Confirm no CI-suppression, merge automation, cloud spend, release tagging or deployment is introduced.
