# IMGENGINE — Test lanes, CI selection, milestone gates and AI-agent budget

**Status: PROPOSED OPERATING RUNBOOK; documentation only (2026-11-15).** Not a change to `.github/workflows/ci.yml`, branch protection, release gates, test implementation, or product behavior. Reviewed remote `main` at `39e7099fbb31a2b879c4a9b2c22fca73de42419d`. Work from current HEAD, merge base and applicable scoped AGENTS, not this dated snapshot. **This document does not authorize disabling required checks, merging, or deploying.**

**Companion authority:** [root rules](../AGENTS.md), [change-scoped verification policy](CHANGE_SCOPED_VERIFICATION.md), [Astra engineering start](ASTRA_ENGINEERING_START_HERE.md), [seven-level operating model](SEVEN_LEVEL_ENGINEERING_OPERATING_MODEL.md), [current work](../next.md), [frontend AGENTS](../imgengine-saas/frontend/AGENTS.md), [native AGENTS](../imgengine/AGENTS.md), [Rust AGENTS](../backend/AGENTS.md), [P14 release runbook](P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md).

## 1. Executive decision: fast feedback without false assurance

**Golden rule: test the smallest affected layer while developing, then run the complete set demanded by *changed contracts, risk and release stage*.**

1. **Developer / AI-agent loop (fast):** inspect diff; test changed code and its immediate consumers; capture logs to artifacts/files and report one short summary. Do **not** manually rerun the whole native verifier after a Nuxt CSS or isolated browser-Wasm change.
2. **PR review (integration):** run scoped tests plus risk-appropriate integration/security checks. **Today, GitHub CI still runs all seven jobs for every pull request**. Let current CI finish; agents must not manually duplicate it.
3. **Milestone acceptance:** require feature-level user journey, negative tests, a11y/privacy/resource assertions, exact-head reviewed PR and required passing CI. A *UI-only milestone* does not inherently require developers to run native fuzz again manually.
4. **Shared boundary change (high risk):** native C, C ABI/public headers, Rust FFI, codecs, scheduler/allocator, native container flags or security-sensitive changes require the relevant full native tests/sanitizers/fuzz/ABI and Rust integration on an exact revision.
5. **Release candidate / production:** full appropriate end-to-end and security/release gates against the **exact candidate**; real-host operations, secrets, TLS/alerts/RSS/rollback and human approval remain mandatory. A previous green PR, nightly, or docs-only run is not a release certificate.

**Separate budgets:** ChatGPT/Astra context or tool usage is **not** the same as GitHub Actions minutes or wall-clock build time. Repeatedly reading giant logs, rerunning equivalent suites and posting thousands of passing lines consumes AI context; CI can often do heavy work independently. Keep full evidence in the CI run/artifact; send the agent only concise status, failed step(s), bounded error excerpts and links.

## 2. Verified GitHub Actions state — this is why seven jobs run

At the reviewed revision, `.github/workflows/ci.yml` has:
- `on: push` for `main` and `on: pull_request` for all branch patterns; **no path-based job selection**.
- `native-build-test`: CMake/native builds, portable/scalar, CTest, Rust supervisor, regressions, export ABI, benchmark *build*.
- `native-sanitizers`: native sanitizer builds and regressions.
- `native-fuzz`: native input-validator and decoder bounded libFuzzer jobs.
- `saas-integration`: **retained legacy** Python/FastAPI/Celery/Redis/PostgreSQL Docker integration; this is an existing verification gate, **not authorization to create new Python product work**.
- `frontend-verification`: pnpm dependency audit, Nuxt unit tests, typecheck and production build.
- `rust-api-container`: production-shape Rust API + Nuxt container integration, Compose/security policy and HTTP smoke tests.
- `docker-linux-verification`: Ubuntu 24.04 Docker run of `imgengine/scripts/verify_linux.sh`, including C17/C11, ABI/Rust, sanitizer/sandbox and fuzz verification.

Six other workflows exist: native coverage, native performance and native/SaaS/P12 release-candidate workflows. The inspected release/coverage/performance workflows are **manual `workflow_dispatch`**, not automatic seven-job PR runs. Verify their trigger state on current HEAD before using.

**Finding:** repeated manual full native testing *on top of* `ci.yml` is usually redundant for an isolated frontend change. **Fact:** the seven PR CI jobs will still execute until a separately reviewed CI selection implementation changes the workflow. This docs PR cannot lower runner minutes itself.

## 3. Three explicit test lanes

| Lane | Purpose | Typical trigger | Required outcomes | What must NOT be inferred |
| --- | --- | --- | --- | --- |
| **L0: Fast feedback** | Minutes-to-feedback where feasible; no claimed fixed duration | Each small iteration; modified files/tests | Focused tests, typecheck/build appropriate to change, failing evidence | Does not certify production, native race freedom or integration outside changed scope |
| **L1: PR/integration evidence** | Confirm changed contract and adjacent consumers | One bounded PR/milestone | Relevant component/API/ABI/security/integration, current required CI, exact SHA | Unrelated unexecuted jobs did **not** pass merely because another job passed |
| **L2: Full/release confidence** | Validate integrated candidate and operational readiness | Native/FFI safety change; release candidate; dedicated full-check cadence once implemented | C17/C11/ABI, sanitizer, fuzz, Rust, frontend, container, real-image/host gates as applicable, reviewed risk | Past CI at a different SHA or staging host does not prove current production readiness |

A **milestone passing** is not automatically the same as **publicly releasing**. Mark `next.md` precisely as implementation merged, previously verified at SHA, release candidate, or production deployed.

### Change-classification matrix — proposed developer loop and future CI lanes

| Changed files / boundary | L0 developer/agent checks | L1 / required contract checks | Native full verifier in developer loop? |
| --- | --- | --- | --- |
| **Documentation only** (ordinary README/guides) | Link integrity, diff/format, factual consistency | Existing CI stays mandatory until separately updated; API/ABI docs must be reviewed against source | **NO** |
| **Nuxt view/style/component** (`imgengine-saas/frontend/components/**`, `app.vue`, styles) | Targeted Vitest, `pnpm run typecheck`, `pnpm run build`, keyboard/touch/browser tests for changed UX | Frontend CI, relevant browser E2E; render path checks if changed workflow contract | **NO** |
| **Browser Worker/Rust-Wasm only** (in an explicitly isolated future crate, **not implemented today**) | Target Wasm compile, Rust unit tests, Vitest/Worker tests, real browser, memory and no-upload assertions | Browser/Worker integration, cross-browser fixtures, privacy/resource and bundle budget; Nuxt checks | **NO**, unless C/ABI/shared native source actually changed |
| **Browser-to-HTTP contract** (`frontend/types/api.ts`, `utils/render-form.ts`, `composables/useImgengineApi.ts`, `server/api/**`) | Unit/contract/serialization and negative tests, typecheck/build | Relevant Rust endpoint/HTTP smoke, proxy integration and secret/error mapping; host-bound image tests as needed | **NO**, unless native/FFI change or incident |
| **Rust Axum/supervisor only** (`backend/imgengine-api/**`, `backend/imgengine-supervisor/**`) | Focused `cargo test` for affected crate, native link prerequisites, request/admission regression | Rust API integration, Nuxt contract (if route changed), container smoke | Usually **NO**, but dependency/unsafe/resource changes may escalate |
| **Rust FFI, public C headers/ABI** (`backend/imgengine/**`, `imgengine/include/imgengine/api/**`, C ABI adapters) | Affected Rust/C targeted tests and exact external consumer/export tests | **Full native + C11/C17 compatibility + sanitizer/fuzz + Rust/API integration** | **YES** for changed unsafe/native contract |
| **C renderer/decoder/scheduler/allocators/SIMD** (`imgengine/src/**`, CMake build config, native dependencies) | Target CTest/regression, scalar/SIMD parity and profiler/fixture when relevant | **Full native/security/ABI/memory/concurrency tests**, Ubuntu verifier; targeted performance benchmark on controlled hardware | **YES**, no safety shortcut |
| **Infra/container/security/CI** (`infra/**`, `backend/Dockerfile`, root workflows or security policy) | Config/lint/security dry-run and targeted image/build checks | Supply chain, Compose/secret/resource/security and proper integrated candidate tests; **CI workflow changes require full validation of routing** | **RISK-BASED; fail closed** |
| **Legacy service** (`imgengine-saas/backend/**`, legacy infra/tests) | Existing legacy-compatible focused tests; no new Python target code | Preserve legacy regression/rollback evidence until explicit retirement | Escalate if native/shared paths affected |
| **Mixed, renamed, deleted, unknown, tooling/build lockfile** | Inventory transitive consumers and baseline assumptions | **Conservative superset** of impacted checks; uncertain risk means full validation | **Default to broader scope** |

**Do not assume all `backend/` Rust paths are browser-Wasm**: current `backend/` is the server. A browser-Wasm crate gets its own reviewed directory/manifest/CI mapping if implemented. Do not mark it isolated merely because it is written in Rust.

### Real M1.2 example (post merged PR #43)

Changed files likely include:
- `imgengine-saas/frontend/components/SheetRenderPanel.vue`
- `imgengine-saas/frontend/composables/useSheetRenderWorkflow.ts`
- `imgengine-saas/frontend/types/api.ts`
- `imgengine-saas/frontend/utils/render-form.ts`
- relevant frontend tests and status docs.

**Classification:** frontend UI **plus HTTP form contract** (not pure CSS); run form/all-or-none/400/413 tests, Nuxt unit/typecheck/build, browser keyboard/touch preview and one proxy-to-Rust smoke. Preserve P12 omission and server-only API key. Do not manually execute C17 ASan/fuzz/Ubuntu docker verifier **if no C/FFI/ABI changed**. The current seven-job GitHub CI still runs automatically on the PR. M1.3 comprehensive pixel parity is a later bounded milestone.

### Future M2/M3 browser-Wasm example

An isolated future Rust-Wasm Worker would run: its own Rust/Wasm build/tests; TypeScript Worker/protocol tests; browser codec/dimension fixtures; pointer/memory allocation limits; local no-image-upload network assertion; Safari/Firefox/Chromium representative compatibility; cold/warm latency, bundle size and screenshot/fidelity evidence. Only add C native tests if reusing/changing C ABI/geometry/decode code or shared contracts. A server-backed P13 crop editor is **not** equivalent to a local-only Rust/Wasm tool.

## 4. One safe local/test-shell recipe

Inspect repository root first, preserving local dirty changes and untracked work:

~~~bash
git status --short
git fetch origin main                    # read remote, do not modify worktree
BASE="$(git merge-base HEAD origin/main)"
git diff --name-status "$BASE" HEAD     # PR branch changes
git diff --name-status                   # unstaged changes
git diff --cached --name-status          # staged changes
git ls-files --others --exclude-standard  # untracked files
git diff --check
~~~

**Agent must classify all these paths before running expensive tests.** `git fetch` requires access; if offline, clearly mark origin/main freshness unknown instead of inventing a baseline. Branch/PR changes, deletions, renames, and root lockfiles may affect many consumers; *do not just check the most recent commit*.

### Frontend-only or M1.2 minimum (actual package scripts on reviewed HEAD)

~~~bash
cd imgengine-saas/frontend
corepack enable
pnpm install --frozen-lockfile
pnpm exec nuxi prepare
pnpm exec vitest run utils/sheet-crop.test.ts utils/render-form.test.ts
pnpm run typecheck
pnpm run build

# PR integration/final milestone: test all frontend modules too
pnpm test
~~~

These commands are examples for an environment with supported Node/pnpm. If existing dependencies/lockfile are already installed and clean, avoid repeatedly reinstalling on each local iteration. A focused Vitest command must be supplemented by new component/interaction tests as they are added. Always run applicable browser/a11y and contract tests; a production build alone does not verify drag/zoom behavior.

### Future isolated Wasm crate (placeholder: **do not run until manifest exists**)

~~~bash
# First inspect the actual, approved isolated crate and target files:
# cargo test --manifest-path <APPROVED_WASM_CRATE>/Cargo.toml
# build the intended wasm32 target and wasm-bindgen glue using its documented tooling
# run Worker/browser integration and privacy/memory tests
# do not run imgengine/scripts/verify_linux.sh for a truly isolated Wasm-only change
~~~

Rust backend tests may need a built native `.so` to link; scope to the minimal documented native prerequisite instead of pretending `cargo test` is independent of C when it is not. Do not replace required previous Python verification scripts with new Python code.

### Native/unsafe or release work (when actually required)

The authoritative existing command is:

~~~bash
bash imgengine/scripts/verify_linux.sh
~~~

**Only execute it in the supported Ubuntu 24.04/Linux toolchain and when appropriate for the impacted boundary.** For full Docker verification, follow the actual `imgengine/Dockerfile.verify` workflow; do not run it **again** manually after an equivalent exact-revision successful CI run just to fill the chat with logs. Narrow CTest/ABI checks may precede the full gate for a native bug fix.

## 5. AI context and compute budget contract

**Before any test call, agents must state in their working plan:**
- Exact HEAD/base and the complete changed-path set;
- Chosen `change_class` (`docs`, `frontend-ui`, `browser-wasm`, `http-contract`, `rust-api`, `rust-ffi`, `native`, `infra`, `mixed/unknown`);
- Applicable required checks, reasons for selecting/defering each;
- Time/compute proportionality (no arbitrary promise of seconds/minutes);
- Where full logs will be saved and maximum summary output size.

**Log handling:** run a command once; redirect full output to a local log or upload the CI artifact; preserve **exit status**; show 20–60 relevant lines on failure and a compact success line on pass. A failed command **never** becomes PASS because output was truncated. Never print secrets, raw image bytes, or private user filenames in logs. Use `--quiet` only if it preserves actionable failures. If a test fails, inspect its relevant short failure log; do not rerun entire CI without diagnosis.

**Report format (<= 15 lines when everything passes):**

~~~text
SHA / baseline: <full or short hashes, branch>
Change class: frontend-ui + http-contract
Files/contract touched: <short list>
Focused: <command> — PASS/FAIL/NOT RUN
Typecheck/build: <commands> — PASS/FAIL/NOT RUN
Browser a11y/privacy: <browsers and fixtures> — PASS/FAIL/NOT RUN
PR CI: <URL> — success/pending/failed (named jobs)
Native full verifier: NOT MANUALLY RUN — why; current CI may run separately
Unresolved risks / release gates: <concise>
Next small PR/milestone: <one sentence>
~~~

If all relevant GitHub CI runs at the exact SHA, link to the check; do not paste the complete 5,000-iteration fuzz log. State recorded vs freshly run evidence accurately. For failing CI, retrieve only failed job name and useful error excerpt, not full successful jobs.

**Caches:** persist pnpm store and Cargo registry/target where supported; cache keys must account for lockfiles/toolchain/compiler/ABI changes. Reproducibility matters more than cache hits, and cache may never substitute for test execution.

## 6. Future safe GitHub Actions optimization — separate CI implementation PR

**Nothing in this docs change makes GitHub's seven jobs conditional.** A proposed next task is `CI-SCOPE-01` to make GitHub Actions themselves risk-selective. A Principal/Security reviewer and an authorized maintainer must review it before rollout.

### Proposed design, without implementing workflow YAML now

1. Keep `ci.yml` triggered on every PR / `main` push. **Do not put workflow-level `paths` filters on a required workflow**: GitHub documents that path-skipped required workflows can remain Pending and block merge.
2. Add a **small, always-running, read-only change classifier job** that compares all PR branch changes with merge base (and correctly handles main push). Output `docs`, `frontend`, `browser_wasm`, `rust_api`, `native`, `legacy`, `infra`, `unknown` booleans plus a *reason*.
3. A reviewed dependency mapping chooses *job-level* `if` conditions; for frontend HTTP proxy/type/API changes, run frontend + Rust container/proxy integration as necessary, **not just frontend Vitest**. Native/unsafe/codec/C-ABI changes turn on the full native matrix. All `.github/workflows/**`, security config, unknown/unhandled files, classifier failures, missing permissions, big diffs/renames and branch-policy changes must **fail closed** to a conservative full matrix.
4. Add a stable **required aggregate status gate**, always evaluated after classifier and dependent jobs, that checks the classifier actually completed successfully, every *selected* job **ran and passed** (rather than merely being skipped/neutral), and every omitted job was justified by the classification. Unknown or ambiguous -> fail/force full matrix; skipped selected job -> gate fails. Do not treat a skipped job's green GitHub status as equivalent to a pass.
5. Confirm or establish **branch protection/ruleset** on `main` with the stable required gate, up-to-date branch testing policy and reviews as appropriate. At the reviewed branch API `protected=false`, so a green badge alone cannot enforce the intended policy. Do not automatically change the repository settings.
6. Before enabling selection, simulate an adversarial diff-classification table: docs-only, Vue-only, form serializer, Nuxt server proxy, isolated Wasm, Rust API, Rust FFI, public C header, codec/scheduler, `Dockerfile`, infra secrets/config, workflows, native/legacy mixed, unknown new file, rename, deletion, very large PR, external fork, direct main push, cancelled/retried runs.
7. For initial deployment of this CI change, **run the complete current seven-job baseline on the workflow-change PR itself** and review CI results at exact SHA. Do not claim gate safety from reading YAML alone.
8. Adopt a controlled schedule of **full-matrix/nightly or periodic security integration** if operationally affordable, and *always* run a candidate-specific full release gate. Scheduled full checks supplement, never replace, checks necessary for a risky PR. Release evidence remains exact candidate SHA; never authorize deployment from only an older nightly pass.

**GitHub semantics that make this necessary:** [required workflow path-skip leaves Pending](https://docs.github.com/en/actions/how-tos/write-workflows/choose-when-workflows-run/trigger-a-workflow); [skipped conditional job may be reported as success](https://docs.github.com/en/pull-requests/how-tos/merge-and-close-pull-requests/troubleshooting-required-status-checks); [jobs with failed/skipped dependencies are themselves skipped without a suitable status condition](https://docs.github.com/en/actions/reference/workflows-and-actions/workflow-syntax). Design the aggregate gate to handle all three; do not use empty green checks as proof.

**Permission boundary:** implementation PR for `ci.yml` and status-policy rollout requires separate owner approval; `pull_request_target`, arbitrary unreviewed PR scripts and write-token access are not warranted by this task. Path classification cannot be an attacker-controlled bypass to secrets or safety tests.

## 7. Escalation by engineering level, security and management

| Lens | Responsibility |
| --- | --- |
| Junior | Correctly collect changed files; run smallest named test with meaningful failure output |
| Mid-Level | Deliver targeted tests and interface regressions for the complete user workflow |
| Senior | Map changed files to consumers, exceptions, accessibility, error and memory tests |
| Staff | Own cross-boundary contract dependency graph and integration correctness |
| Senior Staff | Set milestone exit/release gates, unknown-path and mixed-PR fallback |
| Principal | Review risk-adjusted validation architecture and safe required-check aggregator; no unmeasured skipping |
| Distinguished | Preserve long-term trust and release confidence as repo gains SDK/Wasm/desktop channels |
| Security/SRE | Verify supply-chain, C/FFI, permissions, logging, host and fail-closed CI semantics |
| CA/MBA/Product | Reduce wasted CI/agent effort and cost without increasing undetected customer failures; track runner minutes, failure causes and customer impact |

**Accepted outcome of THIS DOCUMENTATION PR:** a real agent can choose efficient *manual* tests today and can hand off a safe CI implementation design. It does **not** yet reduce automatic GitHub Actions job count.

## 8. Copy/paste Astra/Codex instruction

> **IMGENGINE — TEST BUDGET / CHANGE-SCOPED VERIFICATION POLICY.** Before testing, read `AGENTS.md`, `docs/CHANGE_SCOPED_VERIFICATION.md`, `docs/TEST_LANES_CI_OPTIMIZATION_AND_AGENT_BUDGET_2026-11-15.md` and affected scoped AGENTS. Inspect latest HEAD/base and the full changed file list including untracked changes. Classify the change into docs, frontend UI, browser Rust-Wasm, HTTP/proxy, Rust API, C/Rust FFI, native C/ABI, infra/CI, legacy or mixed/unknown. Choose focused L0 tests and impacted L1 integration gates; **do not manually run the 7-job Ubuntu/native/sanitizer/fuzz/Docker suite for an isolated Nuxt or future browser-Wasm change**. For frontend HTTP serializer/proxy changes, run frontend + relevant proxy/Rust integration, not only visual tests. For C, C ABI, unsafe FFI, decoder/scheduler or native resource/security changes, keep the complete applicable native/ABI/C11/C17/sanitizer/fuzz verification. Release candidate remains full and exact-SHA. **Current GitHub CI still automatically runs all seven jobs**; let it run and do not duplicate by manual commands. Capture full logs as artifacts, return one short command/SHA/PASS/FAIL summary, and inspect only failed excerpts. Never mark skipped or NOT RUN tests as PASS. A future path-aware CI classifier and required aggregator need a separate independently reviewed PR and human approval; do not silently alter CI, branch protections, merge rules, secrets or live deployment. Use seven engineering perspectives plus Security/SRE/CA-MBA proportional to the risk.
