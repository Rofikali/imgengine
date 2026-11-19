# CI-SCOPE-02 — Selective PR verification with fail-closed aggregate gate

**Status: implementation candidate in review, NOT MERGED or release-authorized (2026-11-19).** This document is the operative CI-SCOPE-02 contract after an owner-approved merge. The previous [CI-SCOPE-01 informational classifier](CI_SCOPE_01_DRY_RUN.md) remains a historical dry run. The current implementation lives in `scripts/ci-scope-select.mjs`, `scripts/ci-scope-gate.mjs` and `.github/workflows/ci.yml`.

## Why

A browser-only Nuxt change does not normally require another native C fuzz/sanitizer or legacy database integration run. Conversely, C headers, codecs, Rust FFI, CI/security configuration, shared dependencies and unknown files require broader evidence. A skipped GitHub Actions job can be shown as successful, so the **always-running `CI verification gate`** must prove every *selected* job actually succeeded.

## CI workflow and trust boundary

- Keep `pull_request` (all branches), `push: main`, and `merge_group` triggers; **no workflow-level `paths` filters or `pull_request_target`**. No new credentials or paid infrastructure.
- `CI scope selection (trusted base)` checks out the **PR base SHA**, not feature-branch code. The selector uses only Node built-ins, read-only Git, numeric GitHub `refs/pull/N/head`, and verified webhook SHAs. A fork head cannot supply the selector script. Git errors, untrusted path/status, renames/deletes, unknown/locked/workflow/security paths and missing history **select all nine jobs**.
- During this first rollout, the selector script does not yet exist in trusted `main`; the workflow **explicitly runs all nine original jobs** and labels it `bootstrap-full-v1`. After merge, future PRs can use `ci-scope-02-v1` from their trusted base. **Every push to main and merge-queue invocation always runs all nine**, independently of changed paths.
- `CI policy and diff validation` always runs Node unit tests for the selector, truth-table gate and retained dry-run classifier, plus `git diff --check`. Docs-only PRs still get this real check.
- Existing nine job IDs and test bodies are preserved. Each has `needs: ci-scope` and an explicit Boolean output condition. No job interprets a skipped check as executed test evidence.
- The **`CI verification gate`** is an additional always-run job with `if: always()`, depending on selector, policy and **all nine** existing jobs. It checks selector version/mode, exact per-job required flags and `needs.<job>.result`: selected must be `success`; unselected must be `skipped`; missing, cancelled, failed, malformed or unexpected results fail. Initial bootstrap likewise demands all nine actual successes. For PRs, the gate code is loaded from **base main**, not the untrusted PR; bootstrap inline logic demands the full nine.
- Both read-only selector/gate and test dependencies execute on GitHub Linux runners. This is **CI infrastructure**, not permission for agents to install development tooling on the user's Windows host; [Docker-only local policy](IMGENGINE_DOCKER_ONLY_DEVELOPMENT_POLICY.md) still applies.

## Approved impact matrix for Phase B

| Diff boundary (A/M only) | PR selected jobs |
| --- | --- |
| Root `README.md`, `next.md`, `next-history.md`, `docs/README.md`, ordinary `docs/notes/*.md` only | Always-on policy/diff checker + final verification gate; no application runtime tests |
| Nuxt source/UI/routes/local tools and tests under `imgengine-saas/frontend/{pages,components,composables,utils,tests,types,assets,public}` | **Nuxt frontend verification** + **Nuxt browser quality** (real four-project Playwright) |
| Browser-facing `frontend/server/**`, `types/api.ts`, `utils/render-form.ts`, `composables/useImgengineApi.ts`, or `docs/api.md` | Above frontend/browser + **Rust API container** for cross-boundary checks |
| Scoped frontend specification `docs/FRONTEND_*.md` and `docs/IMGENGINE_FRONTEND_*.md` | Nuxt + browser |
| Rust Axum under `backend/imgengine-api/**` | Rust API container; affected HTTP contract files escalate to frontend/browser too |
| Rust supervisor under `backend/imgengine-supervisor/**` | Rust API container + native build job (supervisor links native) |
| C core, public ABI, FFI, legacy service/infra, lockfiles, scripts, CI/security/AGENTS/build/ops, unknown/Wasm path, malformed diff, rename/delete | **All nine existing jobs** |

**Wasm is not yet independently classified:** there is no approved isolated browser-Wasm package/test lane today. Do **not** infer isolation from a filename and silently skip C/FFI checks. Create a separately reviewed WASM contract + actual test lane before narrowing its impact.

## Verification evidence and limits

The implementation includes **54 standalone Node unit tests** for path ownership, mixed changes, malicious/truncated input, forks, changed refs, push/merge-queue fallback, schema validity, missing/failed/cancelled/skipped selected jobs and unexpected skipped-job results. These tests passed in an isolated Linux sandbox before publication; that is **not** GitHub Actions or Docker product verification. The PR must additionally pass its actual **full nine-job bootstrap CI** plus `ci-policy` and `ci-gate` at its final head SHA. After merge, validate a separate **docs-only or Nuxt-only PR** demonstrating expected skips and successful gate, and confirm a full main push still executes nine jobs. Record actual run IDs and cost/time measurements; **do not invent savings**.

## Repository governance and owner's required action

**Verified GitHub state when authored:** the `main` branch API reports `protected: false` and no required checks, and the branch-protection endpoint returned 403. No rulesets were returned; this is not evidence of an active merge gate. The workflow adds a named `CI verification gate` check, **but it cannot prevent a user from merging a failing PR without branch rules**.

After the new PR creates a real `CI verification gate` check, the owner should configure **Settings → Branches / Rules → main → Require status checks to pass**, requiring **`CI verification gate`** from GitHub Actions, plus appropriate review/approval rules and the latest-head requirement. Keep existing required checks as desired; the aggregate gate is the authoritative proof for conditional jobs. If protected branch settings are inaccessible, **do not treat a green skipped-job badge or GitHub's mergeable flag as permission to merge**. Do not change branch rules, secrets, deployments, billing, or approvals from an agent without explicit approval.

## Rollback and safety

If the gate is not trustworthy, classifier encounters an unknown state, a selected job is missing, or branch protection cannot be enforced, revert to unconditional nine-job checks rather than waiving any test. A release/production candidate requires the full independent native/FFI, privacy/browser, infra/host, compliance and owner go/no-go review. A completed feature PR ≠ a public Free release. No automatic merge, deployment, billing, account change or security waiver is authorized.

**Docs impact:** `next.md` is the live task authority; this is the canonical CI selection policy. Root `AGENTS.md`, `skills.md`, `README.md`, docs index, change-scoped runbook and PR template carry **links/concise rules only**. Historical experiment reports remain dated; HLD/LLD, tax/CA/MBA and marketing docs are not rewritten where the CI-only change has no effect.
