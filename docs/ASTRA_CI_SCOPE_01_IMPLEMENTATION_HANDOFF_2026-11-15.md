# Astra / Codex / human engineer — CI-SCOPE-01 safe selective verification handoff

**Status: IMPLEMENTATION PROPOSAL / READ-ONLY UNTIL SEPARATELY AUTHORIZED.** Written 2026-11-15 from `main` at `39e7099fbb31a2b879c4a9b2c22fca73de42419d`. This is not a CI implementation, smoke test, required status protection change or release waiver. The caller may currently be implementing frontend M1.2: **do not put CI refactoring into the M1.2 feature PR**.

**Read:** `AGENTS.md`, `docs/CHANGE_SCOPED_VERIFICATION.md`, [test lanes, milestones and agent budget](TEST_LANES_CI_OPTIMIZATION_AND_AGENT_BUDGET_2026-11-15.md), `.github/workflows/ci.yml`, [seven-level operating model](SEVEN_LEVEL_ENGINEERING_OPERATING_MODEL.md), [next.md](../next.md), and all affected scoped AGENTS. Verify latest HEAD, branch rules and required checks before proposing a diff.

## Business and technical objective

Reduce repeated GitHub Actions runner compute on pure Nuxt/CSS, isolated future Rust/Wasm or docs PRs **without** weakening native ABI/FFI, C17/C11, ASan/UBSan, fuzz, Rust API/container, SaaS legacy integration or actual release gates. Save AI context by never reading all successful logs.

Today, `ci.yml` triggers for every PR and `main` push with **seven unconditionally executed jobs**:
1. `native-build-test`;
2. `native-sanitizers`;
3. `native-fuzz`;
4. `saas-integration`;
5. `frontend-verification` (display name `Nuxt frontend verification`);
6. `rust-api-container` (display name `Rust API container verification`);
7. `docker-linux-verification` (display name `Ubuntu 24.04 Docker verification`).

**Do not remove the legacy integration test** until the separate legacy-retirement/preservation decision is approved. Do not confuse runner minutes with ChatGPT token consumption.

## Required technical design and proof

### Phase A — design and dry-run implementation (still runs all seven real jobs)

- Retain always-on `pull_request` and `push main` workflow triggers with **no workflow-level paths filter**.
- Propose a low-privilege read-only changed-file classifier (use ordinary Git and shell/JS where appropriate; **no new Python**), including PR merge base, push range, renames, deletions and all changed paths. Its output is a set of affected-layer booleans plus reasons.
- Define path ownership and **transitive affected contracts**:
  - docs-only (normal text) -> cheap docs validation (special review for API/ABI/policy docs);
  - frontend visual/UI -> frontend check + browser/a11y tests where present;
  - browser-facing API types/form/nuxt proxy -> frontend **plus Rust/proxy integration**;
  - isolated future Wasm -> Rust/Wasm + Worker/browser tests; no native full suite if truly isolated;
  - Axum/supervisor -> Rust API/container test and integration as appropriate;
  - C/Rust FFI or native ABI/codec/scheduler/allocators -> **native-build-test, native-sanitizers, native-fuzz, docker-linux-verification and Rust API integration** as applicable;
  - legacy, infra, Dockerfile, scripts/workflows, lockfiles, dependency updates, root build config, shared images and unknown/mixed -> **conservative broader or full suite**.
- Prefer a versioned, **unit-tested classifier** with a table of expected booleans. Leave CI job selection *inactive* during Phase A so the baseline full seven-job CI still runs while classifier decisions are compared to expected paths.
- Add a stable planned required aggregate gate that will eventually inspect selected-job **actual success** and detects classifier error, selected-job skipped/cancelled/missing, or ambiguous changes. A skipped conditional job may display green; this must never satisfy a *selected* gate.
- Test forks and least-privilege permissions: no `pull_request_target` execution of untrusted PR code, no write-token requirement to classify paths, no secrets in PR feature tests.

### Path fixtures / design review cases

| Example PR change | Required proposed lane |
| --- | --- |
| `docs/README.md` only | docs validation; recorded reason, no manual native tests |
| `imgengine-saas/frontend/components/SheetRenderPanel.vue` | Nuxt + interactive browser/a11y where affected |
| `imgengine-saas/frontend/types/api.ts` and `utils/render-form.ts` | Nuxt + Rust/proxy contract integration, not only visual tests |
| `imgengine-saas/frontend/server/api/sheet-render.post.ts` | Nuxt proxy + Rust integration, authentication/validation/limits |
| Newly introduced **approved isolated Rust/Wasm** crate | Target crate/Worker/browser; check it does not import or change native C |
| `backend/imgengine-api/src/lib.rs` | Rust API/container and endpoint tests |
| `backend/imgengine/src/sys.rs` | Full native/ABI/C11/C17/ASan/fuzz and Rust FFI |
| `imgengine/include/imgengine/api/v2/sheet.h` | Full native/ABI/FFI/consumer tests |
| `imgengine/src/runtime/worker_loop.c` | Full native/security/concurrency/sanitizer/fuzz tests |
| `infra/docker-compose.production.yml` | Infra/security plus appropriate Rust/web integration and release-safety policy |
| `.github/workflows/ci.yml` | All seven CI jobs and classifier/gate simulation |
| `Cargo.lock`, Nuxt lockfile, Dockerfile or root build config | Re-evaluate impacted dependencies/consumers; unknown -> full |
| Renames/deletes/unknown paths, diff failure, incomplete checkout/permissions, unusually large diff | **Fail closed**; run full or report blocker, never silently skip |
| Mixture of UI and native scheduler | Full native plus frontend; union of impacted lanes |
| Release candidate | Full required release checks on exact candidate; separate P14 live-host gates |
| Main push (initial rollout) | Keep full required matrix until approved otherwise |

### Phase B — separately reviewed rollout (requires explicit owner decision)

After accurate Phase A evidence, a separate change may add **job-level conditions** and a **mandatory aggregate policy gate**. Preserve stable job names and check status semantics. **Do not enable conditional execution before checking branch protections/rulesets** and required context naming; a workflow skipped by path filtering can remain Pending, while a conditionally skipped job may report Success. Final gate must reject false-success cases. Simulate failure, skipped jobs, cancellations, classifier injection, external PRs, changed-workflow PRs, missing outputs and direct main pushes.

Run the full prior seven-job matrix at the exact workflow-change PR head as a release-quality gate. Restore full behavior on uncertainty and maintain fast rollback. Do not edit repository settings without owner approval.

### Phase C — operations / economics

Record before/after workflow runner minutes, failed checks by layer, false-negative/false-positive classification audits, UI lead time, C safety findings, queue/concurrency risks and maintenance burden. A schedule-based full native run can supplement but **never** replace exact-head C/ABI/FFI security checks and candidate release verification. If complexity/risk rises, revert to universal verification.

## Compact per-turn agent result format

~~~text
SHA: <latest head>
Changed paths: <count, classification>
Actual CI behavior today: all seven jobs / proposed selection not enabled
Proposed job matrix: <selection reasons>
Risk and branch-rule review: <outstanding>
Tests: <commands, PASS/FAIL/NOT RUN, exact SHA>
Logs: <CI URLs; failed excerpts only>
User approval needed: workflow change/branch rules/merge/deployment
Next bounded PR: <one name>
~~~

## Copy/paste Astra task — CI-SCOPE-01

> **IMGENGINE CI-SCOPE-01 — design and dry-run safe test selection (no automatic gate removal).** Inspect current `main`, branch protection and `.github/workflows/ci.yml` (seven unconditional jobs). Read `docs/TEST_LANES_CI_OPTIMIZATION_AND_AGENT_BUDGET_2026-11-15.md` and `docs/ASTRA_CI_SCOPE_01_IMPLEMENTATION_HANDOFF_2026-11-15.md`. Design a conservative, unit-tested changed-file classifier for docs, Nuxt UI, Nuxt proxy/API contract, isolated future Rust/Wasm, Rust API, Rust FFI, native C/ABI, legacy, infra/security and unknown paths. Union mixed changes; fail closed on missing base, renamed/unknown/shared files, workflow/lockfile/secret changes. **First PR must be dry-run: keep all seven checks running**, publish classification/expected lanes, verify sample PRs and adverse cases; propose a stable required aggregate gate whose selected jobs must actually succeed. Do not turn on conditional skipping, add workflow paths filters, change required checks or branch protection, or treat skipped-success as verification without explicit human approval and a separate rollout review. Preserve C17/C11/ABI/sanitizer/fuzz/release evidence; no new Python/Redis/PostgreSQL. Make a scoped draft PR, report exact SHA and test evidence, and do not merge or deploy.

**Decision:** Agents can apply the change-scoped *manual testing* policy immediately. Actual automatic GitHub job reduction is a later, high-risk, independently reviewed CI implementation milestone.
