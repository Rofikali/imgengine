# IMGENGINE — Change-scoped testing and token/time budget policy

**CI-SCOPE-02 implementation candidate (2026-11-19):** [Actual selector/gate design and conservative impact matrix](CI_SCOPE_02_SELECTION_GATE.md) supersedes the *future implementation* wording below **only after owner-approved merge**. This phase adds an always-on status verifier; skips are not accepted as evidence for required checks. GitHub currently reports main unprotected, so an owner must require `CI verification gate` before relying on selective PR results. Local Docker-only engineering and milestone-specific test scope remain unchanged.

**Status:** Proposed contributor policy, expanded by [Test Lanes, Milestone Gates & AI-Agent Budget (2026-11-15)](TEST_LANES_CI_OPTIMIZATION_AND_AGENT_BUDGET_2026-11-15.md) and the proposed [Astra CI-SCOPE-01 handoff](ASTRA_CI_SCOPE_01_IMPLEMENTATION_HANDOFF_2026-11-15.md). This document does **not** change existing GitHub Actions workflows, release gates, or authoritative `imgengine/scripts/verify_linux.sh`. No tests are disabled by this documentation PR. The actual `.github/workflows/ci.yml` currently runs **seven unconditional jobs** for all PRs, including docs/UI-only PRs; do not duplicate them manually.

**Permanent log/output execution rule:** [CI-first minimal-AI-context policy](CI_FIRST_MINIMAL_AI_CONTEXT_POLICY.md). Run scoped local tests and preserve all required CI; store full logs in files or CI artifacts, present concise exact-SHA statuses (including NOT RUN/PENDING), and inspect failed steps only when needed. This guidance reduces AI log-reading tokens, **not** the number of GitHub tests or the validity of failures.

## Goal
Reduce repeated local compilation, sanitizer/fuzz time, and AI-agent context/token consumption **without** suppressing required checks for changed boundaries or shipping unverified code. The AI agent must not execute a full native rebuild merely because a frontend-only milestone begins.

## Change classification and minimum developer-loop verification
| Changed scope | Fast local checks | Additional checks when relevant | Mandatory pre-merge/release evidence |
| --- | --- | --- | --- |
| Docs only | Link/format/diff checks; no compilation | Contract consistency review | Existing branch protections/checks still apply |
| Nuxt UI, styling, pages | Frontend unit/component tests, typecheck, targeted build | Accessibility and browser E2E for changed flow | Required frontend CI; existing required workflows remain authoritative |
| Browser Worker / Rust-Wasm (isolated, no native C files) | Wasm package compile, focused Rust unit tests, Worker/browser tests, memory/performance fixtures | Browser compatibility, CSP, no-upload assertions, codec parity | Relevant Wasm/frontend CI plus required repository checks; **no manual native C rebuild** if no native boundary changed |
| Nuxt proxy / Axum HTTP contract | Targeted proxy/Rust tests, validation/security fixtures | End-to-end request test; FFI tests if invocation changed | Required integration/security CI; native ABI checks if boundary affected |
| Rust FFI, C headers, C ABI, native core, scheduler, codec | Targeted native/Rust tests, ABI/export checks, C17/C11 as applicable | ASan/UBSan, fuzz, scalar/SIMD parity, full Ubuntu verifier | Full native/security/ABI matrix for changed native boundary |
| Docker/CI/deployment/resource limits | Config/lint, image build, policy checks | Controlled host characterization and rollback | Release/deployment gates appropriate to risk |
| Release candidate or dependency/security change | All relevant gates for the exact release revision | Actual host and canary evidence | Never waive security/release checks to save tokens |

**Practical risk split:** Distinguish (a) isolated Nuxt UI/CSS, (b) browser-facing typed form/proxy/HTTP contract, (c) isolated future Rust/Wasm Worker, (d) Rust FFI/C ABI/native C, (e) Docker/infra/CI policies and (f) release candidates. A frontend **form/proxy** edit must run relevant Rust/proxy contract tests, even if it touches only `imgengine-saas/frontend/`. Browser-local Rust/Wasm is not implemented yet, so do not assume an isolated manifest/build target exists. Native/ABI/FFI changes require full relevant native safety evidence. If classification is uncertain, expand validation rather than optimizing it away. The detailed commands, examples and future CI required-gate design live in the [test-lanes runbook](TEST_LANES_CI_OPTIMIZATION_AND_AGENT_BUDGET_2026-11-15.md).

## AI-agent operating rules
1. Start with `git diff --name-only` (against merge base), classify impacted boundaries, and name the exact commands **before running** them.
2. For a pure frontend/Wasm change, **do not manually run CMake, full native build, ASan, UBSan, or native fuzz**. Reuse existing exact-SHA CI evidence where applicable; never claim a new SHA is validated by old SHA results.
3. Do not rebuild C merely because a Wasm source uses Rust. If a new Wasm component shares C code or headers, treat the changed shared boundary as native-impacting and run the appropriate compatibility tests.
4. Run a fast focused loop first, then broaden on failures, changed boundaries, release gates, or security concerns. Never delete, skip, or weaken existing tests to make a PR green.
5. If existing CI automatically runs native tests for all PRs, let it run; **do not duplicate that suite manually**. Any future path-filtering of CI requires a separate reviewed workflow PR with conservative change detection, fail-closed unknown paths, and required-check safety. A path-filter change must never make required GitHub checks silently disappear.
6. Prefer one consolidated test summary: command, environment, revision, pass/fail, failure cause. Do not paste thousands of repeated test log lines into agent context. Store full logs as CI artifacts and report concise failures.
7. Use compiler caches, pnpm/Cargo caches, incremental builds, and immutable test fixtures when safe; do not trade away deterministic release verification.
8. Never infer that a browser worker is secure merely because it runs locally. Test untrusted inputs, resource ceilings, cancellation, and object URL/buffer cleanup.
9. On a failing test, report failure and stop the affected gate; never present an incomplete run as passing.
10. Preserve separate dirty worktrees and ask before modifying or cleaning them.

**Current vs future:** The existing PR workflow still launches **native-build-test, native-sanitizers, native-fuzz, saas-integration, frontend-verification, rust-api-container and docker-linux-verification** on every PR. Selective CI with a change detector and fail-closed required aggregate gate is a **separate future implementation milestone**. Do not add top-level `paths` filters to required workflows: skipped required workflows can remain Pending; conditionally skipped jobs can appear green without running. Follow [CI-SCOPE-01](ASTRA_CI_SCOPE_01_IMPLEMENTATION_HANDOFF_2026-11-15.md) only after appropriate security/Principal review and owner approval.

## Suggested staged workflow (future CI optimization, not implemented)
- PR fast lane: change classification, frontend/Wasm focused checks, contract tests when affected.
- Required security lane: native/FFI checks for native-impacting changes, conservative unknown-path fallback.
- Release/nightly lane: full C17/C11 native matrix, sanitizers, fuzz, Rust integration, real-image, container and operational verification.
- Each release uses evidence from the **exact candidate revision**. Nightly evidence alone is not a release substitute.

## Token economics
Token savings primarily come from **smaller task scopes and summarized logs**, not skipping safety tests. Compile time and GitHub Actions runner minutes are separate from AI context tokens. Agent should execute targeted commands with bounded output, capture logs to files, and summarize the last relevant error lines rather than reading every success line.

## Review gate
Astra reviews architecture, new trust boundaries, native ABI/FFI changes and pre-release security decisions. Terra/Sol handles bounded implementation and focused tests. Model selection is an engineering-cost preference, not evidence of correctness.
