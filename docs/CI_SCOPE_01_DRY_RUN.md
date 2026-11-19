# CI-SCOPE-01 Phase A: informational change classification

**Superseded when CI-SCOPE-02 merges (2026-11-19):** This is the original **Phase A informational-only** classifier with historical seven-job outputs. The [Phase B selection+gate policy](CI_SCOPE_02_SELECTION_GATE.md) adds a new independently tested trusted-base selector, preserves all nine existing job IDs and a new always-on aggregate. Do not treat this dry-run's `actualJobs` property as a live branch-protection contract.

Base: `758bedc24c9acd0ddde8a956210fd090d5bdb874`. Implementation is proposed
for review; no selective skipping is activated. All seven existing jobs retain
their names, triggers and unconditional scheduling. Two steps in the frontend
job test the classifier and print its JSON report. No job consumes its output.

The classifier uses Node built-ins and Git argv, with no shell interpolation,
network calls, packages, secrets or write permissions. The frontend checkout
fetches full history with credential persistence disabled; its token has only
contents-read permission. Fork PRs use event object SHAs, never branch names.
PRs compare merge-base(base SHA, head SHA) to head; pushes inspect before/after
and always recommend the full suite. Missing history/event, invalid SHA, Git
failure, initial pushes, unsupported events, empty or oversized diffs fail closed.
Git output is bounded to 4 MiB/30 seconds and 10,000 changed records.

NUL-delimited status/path records preserve spaces. Renames, copies, deletions,
unmerged records and unusual control/path syntax recommend all jobs. Ordinary
README/notes markdown is the only narrow docs allowlist; API/ABI/policy docs are
full. CSS can recommend frontend alone. Vue, forms, types, tests and proxies
recommend frontend plus Rust integration; component filenames do not establish
isolation. Rust API/supervisor recommend Rust integration. Native/FFI, legacy,
infra, workflow/scripts, dependencies/build config and unknown paths select full.
Mixed paths union recommendations, with full dominating. No Wasm path has yet
been approved as isolated, so future Wasm paths default to full.

JSON contains affected-layer booleans, fixed reasons, proposed jobs and all seven
actual jobs. Conservative full results set every layer true. Docs-only proposed
jobs can be empty: a future docs validator would still be required before rollout.
The report prints no filenames or raw Git errors and writes no GITHUB_OUTPUT.
Classifier failures cannot skip verification; syntax/runtime failure fails its
existing frontend job, while other jobs remain independently scheduled.

## Evidence and rollout boundary

Run `node --test scripts/ci-scope.test.mjs`. Tests cover path fixtures, mixed
changes, malicious/missing SHAs, failed Git, initial/main pushes, unsupported
events, real Git rename/deletion handling and workflow invariants. Test repository
cleanup is limited to its freshly created temporary directory. Full logs stay
outside the model conversation; exact committed SHA/results/CI belong in the PR.
Existing CI runs all seven jobs, including native safety checks; no redundant
local native/sanitizer/fuzz/Docker suite is needed for this classifier change.

Current branch-protection/ruleset settings are not established by this code or
the connected tools. They must be read and reviewed before any Phase B proposal;
no settings are changed here. Fork event handling is locally tested; an actual
external-fork CI execution remains unverified until observed.

Future separately approved Phase B should introduce a stable `verification-gate`
that runs regardless of dependency outcome, validates classifier schema/revision,
requires every selected job to have actual success, and rejects missing, skipped,
cancelled, failed or ambiguous results. Test this gate's truth table and fork/
workflow-change scenarios before adding conditions. Required checks and branch
protection must be reviewed by the owner. This document does not implement that
gate, authorize skipping, or replace release/P14 evidence.

Cost: Phase A adds a small Node test and full-history checkout; it saves no runner
minutes yet. False positives are deliberate; incomplete path ownership cannot
become permission to omit safety checks. Rollback removes the two informational
steps and classifier files; existing verification stays intact. No merge or
deployment is authorized. Next action: review dry-run results and audit proposed
classifications on real PRs before seeking any Phase B authorization.
