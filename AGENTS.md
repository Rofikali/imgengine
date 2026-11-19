# AGENTS.md — IMGENGINE Engineering Constitution

> **FIVE-PRODUCT RELEASE DESIGN — PROPOSED DOCS ONLY (2026-11-19):** Review [ADR-PORTFOLIO-005](docs/ADR_PORTFOLIO_FIVE_PRODUCT_RELEASE_STRATEGY_2026-11-19.md) and [five-family release/compatibility matrix](docs/IMGENGINE_FIVE_PRODUCT_RELEASE_COMPATIBILITY_MATRIX.md) alongside the existing four-product portfolio. The proposed fifth SDK/Desktop family contains **separately releasable** native SDK, API client SDK and desktop packaging work; it is **not implemented, approved for commercial launch, or a new Free v1 feature**. Prefer one `main` and short-lived feature branches, not five permanent product branches; product/app SemVer, C ABI, HTTP API contract and installer versions must remain distinct. No agent may treat a merged docs/feature PR as a public release, claim SDK/.a availability from internal builds, enable billing/uploads/API keys, bypass required CI/C-A0/P14, or tag/deploy without the separate required owner go/no-go. [next.md](next.md) remains the status authority.

> **F2c CI QUALITY CONTRACT (2026-11-18):** [F2c 12-category test and acceptance matrix](docs/FRONTEND_F2C_CI_QUALITY_GATES.md) is the single detailed specification for Compress coverage, privacy and browser evidence. The existing seven unconditional GitHub CI jobs are **not** twelve tested categories; the 12-gate workflow is **not implemented** by this documentation change. Agents must report each applicable gate against an exact SHA; never convert NOT RUN, an old job result, or a user-reported local implementation to PASS. Current state only in [next.md](next.md).

> **POST-MERGE STATUS (2026-11-18):** [Live `next.md`](next.md) + actual GitHub PR/CI facts outrank dated notes below. **F1 Resize #57, F2a Crop #59, F2b Rotate #62 MERGED; F2c Compress NEXT; Free v1 3/5 features merged, NOT RELEASED.** [PR #65](https://github.com/Rofikali/imgengine/pull/65) merged only a temporary **frontend CI, non-production picomatch GHSA-fc5q-39xc-vjpf risk acceptance** plus native Ubuntu 24.04 runner labels. Vulnerability still exists; [decision record](docs/FRONTEND_PICOMATCH_TEMPORARY_NONPROD_RISK_ACCEPTANCE_2026-11-18.md) expires **2026-12-02 00:00 UTC**; no production release, no other high/critical waiver, no weakened native/Rust/Docker gates. [Real Codex Skill/lean AGENTS #63](https://github.com/Rofikali/imgengine/pull/63) remains **DRAFT, unmerged**. Milestone PRs must propose `next.md` plus relevant documentation; after external merge, create a *reviewable* small docs sync PR for true merge SHA and affected README/skills/AGENTS/docs indexes. Do not claim Markdown auto-updates, merge, deploy or spend without approval. [Runbook](docs/AGENT_MILESTONE_CONTINUATION_RUNBOOK.md).

> **CODEX VS CODE — SIX-RULE COST POLICY (owner-requested 2026-11-17):** Use [local-first six-rule Codex operating policy](docs/CODEX_VSCODE_LOW_CONTEXT_SIX_RULES.md): (1) local VS Code/Git for source/edit/tests, (2) short `next.md` plus relevant docs only, (3) GitHub tool only for remote PR/CI/merge and concise metadata—not repeated full local-source downloads, (4) short PASS/FAIL checks and failure-only diagnostics while retaining every existing required CI/security gate, (5) select available standard/lower-effort mode for routine tasks *only when suitable and supported*, higher reasoning for native/FFI/security, and (6) privately measure account allowance after reset instead of guessing token bills. **No silent model/account setting changes, no bypassed CI, no private Usage records in public repo.** Follow [Skill 22](skills.md) and [CODEX-COST-01](next.md). Root/scoped security and approval instructions still take precedence.

> **PRODUCT MILESTONE ≠ FEATURE PR (owner-requested 2026-11-17):** Track four separate product parents in [short live next.md](next.md): `PRODUCT-FREE-01` (five tools only: Crop, Resize, Compress, Rotate, Convert), `PRODUCT-PRO-01`, `PRODUCT-STUDIO-01`, `PRODUCT-API-01`. Feature merges update only their child; a parent can become `READY_FOR_RELEASE_REVIEW` only after all scope and gates pass, and `RELEASED` only after a real, explicitly approved public launch. F1 #57, F2a #59 and F2b #62 are merged; **F2c Compress is next**, not repeating earlier code. Independently track `INFRA-P7/P8/P12/P14` by exact host, commit and test evidence—no automatic propagation from Docker simulation to production. [Product/infra hierarchy + Codex usage protocol](docs/IMGENGINE_PRODUCT_INFRA_MILESTONE_AND_CODEX_USAGE_PROTOCOL.md) and [Skill 21](skills.md). **Codex Usage after allowance reset:** measure read-only local-only vs GitHub-tool-only sessions privately with matched model/effort; do not store personal usage percentages/screenshots in a public repository or claim automatic measurement. Preserve all CI, safe privacy, native/FFI and release gates. Select the next READY task from current GitHub evidence, not stale historic paragraphs.

> **COST-01: short-context first (2026-11-17):** Before any “go ahead” read the compact [live `next.md`](next.md) and current GitHub PR/CI facts; its full former text is archived [on demand](next-history.md), **not a live task queue**. Limit other reads to the changed files, contract, relevant scoped AGENTS and focused tests. Avoid wholesale re-reading `skills.md`, all AGENTS/PRDs/history, entire CI/Docker success logs, verbose seven-persona narratives and repeated CI polls. Show exact SHA and short check summaries; examine only failed steps. Keep every binding security, native/ABI, seven-job CI, reviewer and public-release gate. **This guidance controls discretionary reads, not automatic context loaded by a provider; no guaranteed AI credit savings are claimed.** [Infra/AI usage audit](docs/INFRA_EXPERIMENTS_AND_AI_CONTEXT_COST_AUDIT_2026-11-17.md) and [Skill 20](skills.md) contain the scope/economics recipe. Do not confuse GitHub Actions runtime with paid AI/Copilot credits or GitHub Pro subscriptions.

> **OWNER-SELECTED FOUR-PRODUCT PORTFOLIO (2026-11-16, planning only):** Treat [IMGENGINE Four-Product Portfolio](docs/IMGENGINE_FOUR_PRODUCT_PORTFOLIO.md) as the product-scope reference once its review PR merges: **Free** initially contains **exactly five** browser-local tools (Crop, Resize, Compress, Rotate, Convert); **Pro** offers repeatable creator workflows; **Studio** serves print/service operators; **Developer API** is a *future* independently metered B2B product. The [API billing/capacity blueprint](docs/IMGENGINE_DEVELOPER_API_USAGE_BILLING_CAPACITY_BLUEPRINT.md) and [commercial release/finance checklist](docs/IMGENGINE_COMMERCIAL_READINESS_AND_UNIT_ECONOMICS.md) are design and decision gates, **NOT** authorization for public API access, account storage, advertisements, subscriptions, billing providers, tax claims, domains or cloud spend. Never mark any paid tier IMPLEMENTED or RELEASED from a docs PR. Preserve five-tool focus: no Flip, extra photo-sheet tool, SDK or AI feature in the first Free launch without a new approved milestone. Keep [F0 accepted sequencing](docs/F0_BROWSER_FIRST_SEQUENCING_ADR.md): select the next READY browser-local task from `next.md` (**F2c Compress** at this dated checkpoint), not Rust/Wasm/API infrastructure by default. For server-backed commercial work, require explicit identity/entitlement/billing/storage ADR, C-A0 and P14 real-host security/capacity gates, actual measurements, approval and existing CI. Agents must update the live [next.md](next.md) with exact-PR evidence and not repeat merged tasks. See [Skill 19](skills.md).

> **PERMANENT MILESTONE STATUS / NO REPEATED WORK (owner-approved 2026-11-16):** At every agent start, after each bounded implementation or verification milestone, after opening a PR, after its CI conclusion, and at the next session following a merge, **reconcile `next.md` with the latest GitHub main/PR/check evidence**. Update the relevant task ID, priority, owner-decision state, implementation state, next action, PR URL, exact tested head SHA, CI status/link, merge SHA (only after confirmed merge), remaining blockers, and release state (only after actual release). Use `APPROVED/READY` for a decision to do work; `IN_PROGRESS`, `PR_OPEN/CI_PENDING`, `READY_FOR_REVIEW`, `MERGED`, and `RELEASED` for different facts. **Do not reimplement work already present in verified merged source**, even when an old historical paragraph still says “next” or “in review.” Keep prior evidence under historical notes; update the short **NOW / NEXT / LATER** board first. A PR may carry a proposed status edit, but it **does not update `main` until merged**; after an external merge, record the new merge SHA in the next authorized reconciliation PR/checkpoint. No fabricated checks or automatic merge/deploy. Follow [milestone continuation runbook](docs/AGENT_MILESTONE_CONTINUATION_RUNBOOK.md) and [Skill 18](skills.md).

> **P8-SIM-02 SIMULATION VERIFICATION:** For P8 Docker workload or P12/P13 server sheet/crop changes, use [P8-SIM-02 suite and evidence guide](docs/P8_SIM_02_SHEET_CROP_RESOURCE_EVIDENCE.md) along with infra-scoped AGENTS, [P13 M1.3 pixel parity](docs/P13_M1_3_CROP_PARITY.md) and [CI-first log policy](docs/CI_FIRST_MINIMAL_AI_CONTEXT_POLICY.md). Never confuse bounded HTTP/PNG/JPEG smoke with all-pixel browser parity, real 1 GiB VPS capacity or production approval. The simulation must be run on an actual Docker engine before claiming PASS. [Skill 17](skills.md) documents the recipe.

> **ROOT MAKEFILE COMMAND POLICY:** The repository-root [Makefile](Makefile) is a *safe GNU Make command menu* for C17/CMake, Rust/Cargo, Nuxt/pnpm/Playwright and opt-in full Ubuntu verification; it is not a new build authority or a replacement for required seven-job GitHub CI. Use [root Makefile guide](docs/ROOT_MAKEFILE_ENGINEERING_GUIDE.md) and [Skill 16](skills.md) to select focused tests from the diff. Default `make` prints help; full verification and cleanup require explicit confirmation. No implicit installs, `sudo`, deployment, public exposure or destructive generic cleanup. Agents report exact-SHA PASS/FAIL/NOT RUN and minimal logs.

> **PERMANENT MILESTONE CONTINUATION / NEW-SESSION RECOVERY:** Read [Milestone Continuation Runbook](docs/AGENT_MILESTONE_CONTINUATION_RUNBOOK.md) at every startup and milestone boundary. Do **not** make the owner repeatedly write the next task prompt: inspect current source/PR/CI/`next.md`, mark stale statuses as historical, select the next eligible approved milestone, implement a small PR if authorized, and persist a concise exact-SHA handoff. **Documentation does not restart a stopped AI session**: a new session or an explicitly configured agent runner/event trigger is required. Never autonomously merge, deploy, pay, change secrets or bypass a required human decision. [Skill 15](skills.md) provides the repeatable task-selection recipe.

> **PERMANENT CI-FIRST / MINIMAL-CONTEXT TESTING RULE (all Astra, Codex, AI and human contributors):** Read [CI-first minimal-AI-context log policy](docs/CI_FIRST_MINIMAL_AI_CONTEXT_POLICY.md) before test execution or retrieving GitHub Actions results. Let required CI execute **all relevant tests**, keep full successful logs in CI artifacts/local files, and show the AI only exact SHA, job names, `PASS/FAIL/NOT RUN/PENDING`, links and small **failed-step-only** excerpts. For unpublished work, run local focused tests; GitHub cannot test an unpushed SHA. Never manually duplicate the seven automatic PR jobs just for a summary; do not weaken native/FFI/sanitizer/fuzz/release gates, declare skipped checks passing, or merge/deploy without owner approval. Separate model tokens from CI runner minutes. [Skill 14](skills.md) is the repeatable recipe.

> **Change-scoped testing and AI token budget (2026-11-15; docs proposal):** Before running tests, classify the complete PR/worktree diff, use [Test Lanes, Milestone Gates & Agent Budget](docs/TEST_LANES_CI_OPTIMIZATION_AND_AGENT_BUDGET_2026-11-15.md) and [Change-Scoped Verification](docs/CHANGE_SCOPED_VERIFICATION.md), and report only concise PASS/FAIL/NOT RUN evidence at exact SHA. For isolated Nuxt UI or future browser-only Rust/Wasm changes, run targeted frontend/Worker/browser/privacy tests **without manually rebuilding all C17 native/sanitizer/fuzz/Ubuntu suites** unless a shared native/ABI/FFI boundary changes. **Today, `.github/workflows/ci.yml` still launches seven jobs on every PR; never skip its required checks or confuse AI context tokens with CI runner minutes.** A [separate CI-SCOPE-01 proposal](docs/ASTRA_CI_SCOPE_01_IMPLEMENTATION_HANDOFF_2026-11-15.md) plans conservative path-aware CI with required-check safety; it is not yet implemented or authorization to change workflow/branch rules. Release gates and human merge/deploy approval remain mandatory.

> **STRAT-00/F0 status (verified 2026-11-16):** Browser-first priority was owner-approved and **F0 architecture/sequencing PR #55 MERGED** as `e26a9194a3c9f58059fcd81c8bd78cffd2199c33` with successful exact-PR-head CI at `073a16ab84901f67d78f5964622aed524adbc089`. **F1 Resize #57, F2a Crop #59 and F2b Rotate #62 MERGED**; F2c Compress and F4 Convert remain unmerged, F3 optional measured Worker/Rust-Wasm is not a prerequisite, and F5 public release has separate owner approval. The other three product identities are owner-selected, but pricing, billing, customer accounts and live products remain unimplemented. No new Python or unapproved C/ABI, deployment or spending; preserve P0 C-A0 and P14 security gates.

> **Seven-level engineering and cross-functional reviews (2026-11-15):** Use the [Seven-Level Engineering Operating Model](docs/SEVEN_LEVEL_ENGINEERING_OPERATING_MODEL.md) for proportional Junior → Mid-Level → Senior → Staff → Senior Staff → Principal → Distinguished review, plus security/SRE/product/CA-MBA management checks. For bounded Astra/Codex tasks use the [Agent Milestone Handoff Template](docs/AGENT_MILESTONE_HANDOFF_TEMPLATE.md). This adds a decision/review process, **not** seven autonomous agents or permission to change architecture, merge, deploy, spend money or bypass existing tests. Root and deeper scoped AGENTS, current contracts, and [next.md](next.md) remain authoritative.

> **Binding product-stack decision (2026-11-14):** For **all new IMGENGINE product/backend development**, use **C17 native core + Rust/Tokio/Axum HTTP/application layer + safe C FFI + existing Nuxt/Vue/TypeScript frontend**. **No new Python, FastAPI, Django/DRF, Flask, Celery, Redis, PostgreSQL, or Python-based tooling** for the target architecture; do not substitute a new queue/database automatically. Existing legacy Python/services and Python verification scripts remain temporarily only for safe maintenance and migration. **Changing this decision requires the user's explicit approval and a new ADR.** See [approved technology and repository structure](docs/TECHNOLOGY_AND_REPOSITORY_POLICY.md) and [Rust-scoped AGENTS](backend/AGENTS.md). A new Rust service belongs under `backend/`, not the legacy `imgengine-saas/backend/` tree.

> **Current repository-status checkpoint (2026-11-14):** The Rust backend migration [PR #37](https://github.com/Rofikali/imgengine/pull/37) is **merged** and [7/7 CI checks passed](https://github.com/Rofikali/imgengine/actions/runs/43618598771) at its PR-head revision. C17 remains in `imgengine/`, Rust/Axum in `backend/`, Nuxt in `imgengine-saas/frontend/`, and Python backend in retained legacy `imgengine-saas/backend/`. Previous “complete/validated” milestone claims are scoped to their dated test evidence, **not blanket production/security approval**. Begin bounded Astra work from [the engineering start guide](docs/ASTRA_ENGINEERING_START_HERE.md) and [next.md](next.md); do not assume P13 crop or P14 VPS release is finished.

> **Infrastructure scope checkpoint (2026-11-15):** For `infra/` configuration, scripts, Caddy, Compose and P14 deployment governance, follow [infra/AGENTS.md](infra/AGENTS.md), [infra security/operations audit](docs/INFRA_SECURITY_OPERATIONS_AUDIT_2026-11-15.md), [P14 runbook](docs/P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md), and [next.md](next.md). Static hardening is **not** real-host deployment evidence. The separate `imgengine-saas/infra/` tree is retained legacy. No new Python tooling; existing `infra/scripts/verify-production-compose.py` remains until parity-preserving replacement. Do not deploy, rotate secrets or change DNS without explicit approval.

> **Native C17 principal audit checkpoint (2026-11-15):** The [C17 source/LLD/DSA/security audit](docs/C17_CORE_PRINCIPAL_SOURCE_LLD_DSA_SECURITY_AUDIT_2026-11-15.md) and [native README](imgengine/README.md) describe implemented C ABI/algorithms, source-backed concurrency/metrics review findings and the first bounded Astra C-A0 task. For C work also follow [native AGENTS](imgengine/AGENTS.md). Do not mistake historical design-pattern sketches, 2-ms targets or previously passing limited tests for blanket race-freedom, cross-platform portability or production release. No C source change is authorized solely by this docs audit.

# 1. Purpose

This file defines the global engineering rules for the IMGENGINE repository.

These instructions apply to the entire repository unless a deeper `AGENTS.md` provides more specific rules for a directory.

IMGENGINE is being developed as a production-grade image-processing SaaS with:

* a high-performance native C engine,
* a stable C ABI,
* a safe Rust control/orchestration layer,
* a Rust/Axum backend,
* a Nuxt/Vue/TypeScript frontend,
* ephemeral image processing,
* low-cost infrastructure,
* strong security boundaries,
* measurable performance,
* and a business model capable of becoming profitable.

The goal is not merely to make the code compile.

The goal is:

> Correct + Secure + Fast + Observable + Maintainable + Testable + Economically Viable.

---

# 2. Current Repository Architecture

The proven architecture and implemented first-slice transport boundary are:

```
Nuxt 4 + Vue + TypeScript (P5/P8/P12 current)
                |
                v
Rust HTTP/application layer (implemented P5 Axum adapter; production not claimed)
                |
                v
      Rust request lifecycle
                |
                v
 Rust bounded admission/control
                |
                v
           C scheduler
                |
                v
        libimgengine C ABI v1
                |
                v
         IMGENGINE C core
                |
                +--> SIMD / AVX2
                +--> TurboJPEG
                +--> decoding
                +--> layout/rendering
                +--> JPEG output; PDF encoder remains internal/not public P12
```

The C scheduler and C engine remain the performance-critical execution plane.

Rust owns the control plane: request/application lifecycle and bounded
admission/control. The supervisor is not a scheduler replacement.

The frontend remains a separate presentation layer.

---

# 3. Technology Direction

## Keep

* Strict ISO C17 for the public C ABI and portable native core; C23 only as
  an isolated, capability-gated enhancement after separate evidence.
* C for proven performance-critical image-processing code.
* SIMD/AVX2 where benchmarks justify it.
* TurboJPEG and existing native codec investment.
* Rust for infrastructure, orchestration, validation, lifecycle, concurrency, and FFI safety.
* Axum/Tokio for the target HTTP backend.
* Nuxt/Vue/TypeScript for the frontend.
* Linux as the production target.
* Docker for reproducible environments.

## Retire progressively

The following are legacy technologies and must not be reintroduced into the target MVP architecture:

* Python
* FastAPI
* Celery
* Redis
* PostgreSQL for any new target-product persistence (not just job-only persistence)
* permanent image storage
* S3/MinIO for the MVP image path
* unnecessary ELK/Jaeger infrastructure

Existing legacy code and historical Python-based verification helpers may remain temporarily during migration; this is not authorization to use them for new backend work. Do not remove required verification tools until an equivalent non-Python replacement has proven CI/release parity.

Do not remove legacy components merely because the target architecture says they will eventually disappear.

Retirement requires:

1. replacement,
2. contract parity,
3. verification,
4. rollback window,
5. explicit retirement decision.

---

# 4. Current Migration State

Priority 1 — Security and Correctness:

```
COMPLETE / VALIDATED
```

Priority 2 — Stable C ABI v1:

```
IMPLEMENTED / VALIDATED
```

Priority 3 — Safe Rust FFI:

```
IMPLEMENTED / VALIDATED
```

Rust supervisor foundation:

```
COMPLETE / VALIDATED
```

Rust bounded admission/control:

```
COMPLETE / VALIDATED
```

Scheduler characterization:

```
COMPLETE / VALIDATED
```

Rust request-lifecycle boundary:

```
IMPLEMENTED / VALIDATED
```

The measured architectural decision is:

> Rust owns request/application lifecycle and bounded admission/control. The
> existing C scheduler remains the execution-plane scheduler unless future
> measured evidence explicitly justifies revisiting this decision.

This is an evidence-based decision, not a permanent prohibition on future
investigation. Queue saturation must produce explicit overload behavior. ABI
v1 has no mid-operation native cancellation; client/request cancellation is
application-level response abandonment, not a claim that native work stopped.
The transport-neutral request lifecycle remains independently testable from
HTTP.

Implemented: the smallest authenticated Axum adapter around the lifecycle
contract, with P6 transport/limit evidence. Planned: production-service
evidence and contract expansion, legacy retirement after a rollback window,
P13 accessible crop completion, and measured production economics/scale. Do not mark those
planned phases as implemented merely because the first vertical slice exists.

---

# 5. Engineering Philosophy

## 5.1 Correctness before optimization

Never sacrifice correctness for a benchmark result.

The order is:

```
Correctness
    ->
Security
    ->
Reliability
    ->
Performance
    ->
Cost optimization
```

A faster incorrect image engine is a failed product.

---

## 5.2 Evidence over assumptions

Do not say:

* "this should be faster"
* "this should be safe"
* "this should scale"
* "this should use less memory"
* "this should be production ready"

unless the statement is backed by appropriate evidence.

Prefer:

* tests,
* benchmarks,
* sanitizer results,
* fuzzing,
* profiling,
* load tests,
* security tests,
* resource measurements,
* CI evidence,
* production telemetry.

---

# 6. Business Engineering Principle

IMGENGINE is not only a programming project.

Every major architectural decision must eventually answer:

1. What problem does this solve?
2. Who benefits?
3. What does it cost?
4. What does it make faster?
5. What does it make safer?
6. What operational complexity does it introduce?
7. Can the business afford it?
8. Does it improve the product?
9. Can it scale economically?

Engineering metrics should eventually include:

* requests/second,
* images/second,
* processing latency,
* P50/P95/P99 latency,
* CPU per image,
* memory per request,
* peak RSS,
* failure rate,
* timeout rate,
* crash rate,
* cleanup failure rate,
* infrastructure cost,
* cost per processed image,
* revenue per processed image,
* gross margin,
* active users,
* paid users,
* conversion,
* churn,
* MRR/ARR.

Do not optimize for "millions of users" before measuring the economics of one request.

First establish:

```
cost(request)
cost(image)
capacity(server)
revenue/customer
```

Then model scale.

---

# 7. Security Rules

Treat uploaded images and request data as untrusted.

Never assume:

* file extensions are trustworthy,
* MIME types are trustworthy,
* dimensions are trustworthy,
* metadata is trustworthy,
* paths are trustworthy,
* client-generated filenames are trustworthy,
* request sizes are safe,
* image dimensions are safe.

Validate at the correct boundary.

Never trust client-provided dimensions when actual decoding can establish dimensions.

Never expose:

* filesystem paths,
* internal diagnostics,
* stack traces,
* secrets,
* environment variables,
* internal identifiers,
* image bytes,
* private filenames.

Logs must be sanitized.

---

# 8. Ephemeral Data Policy

The target MVP must not permanently store user images or generated outputs.

Each request should use an isolated temporary workspace.

Target policy:

* random private request directory,
* restrictive permissions,
* server-generated filenames,
* bounded temporary storage,
* cleanup on success,
* cleanup on failure,
* cleanup on timeout,
* cleanup on cancellation,
* cleanup on client disconnect,
* startup stale-directory sweeper,
* stale retention no greater than five minutes.

Do not introduce a database merely to track temporary image jobs.

Metrics and carefully redacted logs are separate from image retention.

---

# 9. Observability

Production behavior must be measurable.

Lifecycle events should use structured logging.

Where appropriate, record:

* request ID,
* trace ID,
* route,
* operation,
* result class,
* duration,
* safe size buckets,
* safe pixel buckets,
* engine result.

Never log raw image content.

Never log secrets.

Never log arbitrary filesystem paths.

Never expose internal error details to clients.

Metrics should include:

* request count,
* success count,
* failure count,
* timeout count,
* overload count,
* processing duration,
* queue/concurrency utilization,
* memory pressure,
* native failures,
* cleanup failures.

---

# 10. Codex Operating Model

Codex is an implementation/review assistant.

The human developer remains responsible for:

* architecture,
* requirements,
* acceptance criteria,
* security decisions,
* business decisions,
* release decisions,
* final merge/push authorization.

Codex must not silently redefine product architecture.

For significant work, follow:

```
Understand
   ->
Inspect
   ->
Plan
   ->
Implement
   ->
Test
   ->
Review
   ->
Adversarial review
   ->
Fix
   ->
Verify
   ->
Commit
   ->
Ask before push
```

Do not immediately edit a large system without first understanding the current implementation.

---

# 11. Codex Planning Rules

For non-trivial tasks, Codex should first identify:

* current behavior,
* affected files,
* architectural boundary,
* risks,
* dependencies,
* tests,
* rollback strategy,
* documentation impact.

A good implementation task should contain:

```
Problem
Current behavior
Desired behavior
Constraints
Acceptance criteria
Tests
Documentation
Git requirements
```

---

# 12. Definition of Done

A task is not complete merely because compilation succeeds.

For production-impacting work, Definition of Done normally includes:

* implementation complete,
* relevant unit tests,
* relevant integration/regression tests,
* sanitizer verification where applicable,
* security verification where applicable,
* benchmark comparison where performance matters,
* documentation updated,
* migration notes updated where applicable,
* no unrelated changes,
* git diff reviewed,
* commit created when requested/appropriate,
* push performed only after explicit user authorization.

---

# 13. Git Policy

The current branch is **not fixed by this document**. Inspect `git status -sb`, current HEAD SHA, worktree cleanliness, remotes and open PRs before making changes.

Remote:

```
origin -> GitHub repository
```

Git policy:

> Codex may create commits, but Codex must NEVER push to a remote repository without explicit user approval.

Before committing:

1. inspect `git status`,
2. inspect the diff,
3. verify tests,
4. ensure no secrets are staged,
5. ensure no unrelated files are included,
6. use a clear commit message.

After committing:

* report commit hash,
* report working tree state.

Before pushing:

* stop and ask for permission.

Never force-push unless explicitly instructed.

Never rewrite published history without explicit authorization.

---

# 14. Linux and Docker Environment

Production target:

```
Ubuntu 24.04 Linux
```

Development/verification environment:

```
Windows 11
  +
Ubuntu 24.04
  +
Docker Desktop
```

Linux/Docker is the authoritative environment for Linux release verification.

Do not waste migration work attempting to make POSIX-specific native code behave like a Windows-native build unless portability is explicitly part of the task.

Native Windows compatibility may be treated as separate work.

---

# 15. Testing Philosophy

Tests must validate behavior, not merely implementation details.

Prefer layered verification:

```
Unit tests
    ->
Integration tests
    ->
Regression tests
    ->
Sanitizers
    ->
Fuzzing
    ->
Security tests
    ->
Performance tests
    ->
End-to-end tests
```

When changing a boundary, add a regression test for the boundary.

When fixing a bug, preserve the bug as a regression test whenever practical.

---

# 16. ABI and API Stability

Public interfaces are contracts.

Never expose internal implementation structures merely for convenience.

Public native interfaces should prefer:

* opaque handles,
* explicit lifecycle,
* explicit ownership,
* fixed-width types where appropriate,
* explicit error/status codes,
* documented allocation/free rules,
* versioning,
* symbol visibility,
* compatibility tests.

The installed public header must not accidentally depend on private implementation headers.

---

# 17. Rust Safety

Rust FFI is an unsafe boundary.

Keep `unsafe`:

* small,
* localized,
* documented,
* reviewed.

Do not automatically mark native handles as `Send` or `Sync`.

Only implement thread-safety traits when the underlying C object is proven safe for that usage.

Never hide ownership ambiguity behind convenient wrappers.

The safe Rust API must make invalid states difficult to represent.

Request/application code must preserve ABI v1 process scope, explicit output
ownership/release, and no-mid-operation-cancellation semantics. A dropped
client response must not be represented as cancellation of native execution.

---

# 18. Performance Rules

Do not rewrite C code in Rust merely because Rust is preferred architecturally.

Keep C when:

* it is performance-critical,
* benchmarks justify it,
* the implementation is well-tested,
* the code has established codec/SIMD integration,
* migration risk exceeds expected benefit.

Consider Rust when:

* ownership is difficult to prove in C,
* orchestration dominates,
* filesystem/process management is involved,
* HTTP/API handling is involved,
* concurrency safety is improved,
* lifecycle management becomes clearer.

Every performance migration must compare:

* correctness,
* latency,
* throughput,
* CPU,
* memory,
* binary size where relevant,
* operational complexity.

---

# 19. No Premature Infrastructure

Do not introduce:

* Kubernetes,
* Kafka,
* Redis,
* PostgreSQL,
* service meshes,
* complex distributed queues,
* expensive observability stacks

without a measured requirement.

The initial system should be capable of operating on a low-cost VPS.

Scale architecture should be designed intelligently, but infrastructure should be introduced according to measured demand.

---

# 20. Documentation Rules

Canonical product/engineering behavior belongs in `docs/`. Read [the Astra start guide](docs/ASTRA_ENGINEERING_START_HERE.md) for the current three-track frontend/Rust/C17 source/contract/test handoff. Legacy `docs/PRD.md`, `docs/SAAS_CONTRACT.md`, `docs/OBSERVABILITY.md`, and `docs/RELEASE_CHECKLIST.md` belong to the older asynchronous Python product generation; the current Rust/Nuxt first-MVP follows versioned [API](docs/api.md), [P12](docs/P12_PHYSICAL_SHEET_CONTRACT.md), [P13](docs/P13_INTERACTIVE_CROP_CONTRACT.md) and [release checklist](docs/MVP_RELEASE_CHECKLIST.md).

Important documentation currently includes:

* `docs/PRD.md`
* `docs/ENGINE_SPEC.md`
* `docs/SAAS_CONTRACT.md`
* `docs/ENGINEERING_PLAYBOOK.md`
* `docs/architecture.md`
* `docs/migration.md`
* `docs/security.md`
* `docs/threat-model.md`
* `docs/api.md`
* `docs/deployment.md`
* `docs/performance.md`
* `docs/LINUX_VERIFICATION.md`
* `docs/NATIVE_DEVELOPMENT.md`
* `docs/NATIVE_BENCHMARKING.md`
* `docs/RELEASE_CHECKLIST.md`

When behavior changes, update the relevant contract.

Clearly distinguish:

* Planned
* Implemented
* Validated

Do not describe planned functionality as implemented.

Do not advertise benchmark numbers without reproducible evidence.

---

# 21. Migration Discipline

Migration must remain reversible until the retirement phase.

Prefer:

```
Add new path
   ->
Verify
   ->
Compare
   ->
Canary
   ->
Rollback window
   ->
Retire old path
```

Do not delete a working legacy path before replacement parity is proven.

---

# 22. Change Scope

Every task should minimize unrelated changes.

Do not combine:

* ABI migration,
* backend rewrite,
* frontend redesign,
* database removal,
* infrastructure redesign,
* unrelated refactoring

into one uncontrolled change.

One architectural boundary at a time.

---

# 23. Required Engineering Mindset

Think simultaneously as:

### Software Engineer

Does the code work?

### Systems Engineer

What happens under load, failure, concurrency, and resource pressure?

### Security Engineer

What happens when the input is malicious?

### SRE

How will we detect and recover from failure?

### Product Manager

Does this improve the actual product?

### CA / MBA / Business Owner

What does this cost, what does it earn, and what is the economic model?

### Philosopher / Logician

What assumptions are we making?

What evidence supports them?

What would prove us wrong?

---

# 24. Final Rule

Do not optimize for code volume.

Optimize for:

```
Trust
Correctness
Simplicity
Security
Performance
Evidence
Maintainability
Profitability
```

The best implementation is not the one with the most technology.

It is the smallest system that satisfies the real requirements with strong evidence.

For the native C17 and Rust implementation, the [contract-to-evidence matrix](docs/NATIVE_RUST_CONTRACT_EVIDENCE_MATRIX.md) provides a source-linked subsystem/ownership/algorithm/security/performance checklist. It supplements the existing [documentation map](docs/NATIVE_C_RUST_DOCS_MAP.md) and [audit](docs/NATIVE_C_RUST_DOCUMENTATION_AUDIT_2026-11-14.md), and does not authorize code changes or waive required CI.

## Frontend and legacy backend documentation handoff (2026-11-14)

For files inside `imgengine-saas/frontend/` and `imgengine-saas/backend/`, read their **respective scoped AGENTS.md** and the [current frontend/legacy backend audit](docs/FRONTEND_LEGACY_BACKEND_AUDIT_2026-11-14.md). The legacy backend AGENTS.md describes Rust as a *target design* while its directory contains Python/Celery code; the current Rust code lives under `backend/` and follows [backend AGENTS](backend/AGENTS.md) plus root policy; native C ABI rules are under [native AGENTS](imgengine/AGENTS.md). Review [skills.md](skills.md) and [next.md](next.md) for bounded testing and status. Do not confuse source presence, prior CI evidence, browser abort, or local Docker rehearsal with production security or native cancellation.

## Native C/Rust documentation audit handoff (2026-11-14)

For tasks touching `imgengine/` native C, the installed public ABI or `backend/`, first read [the native/Rust documentation map](docs/NATIVE_C_RUST_DOCS_MAP.md) and [dated audit](docs/NATIVE_C_RUST_DOCUMENTATION_AUDIT_2026-11-14.md) **after** this policy and the relevant nested AGENTS.md. Use [next.md](next.md) for task state and [skills.md](skills.md) for optional task recipes. These are **evidence/navigation aids**, not a replacement for actual source review, current API contracts or release checks. Do not treat historical PRD, HLD, LLD or v2 RFC proposals as shipped features or measured performance. No native source change is authorized by a documentation finding alone.
