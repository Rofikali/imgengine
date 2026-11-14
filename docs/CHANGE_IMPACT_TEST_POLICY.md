# IMGENGINE — Change-impact testing and agent token budget

**Status:** Engineering policy proposed for future milestones; documentation alone does NOT alter the existing GitHub Actions jobs. The current `.github/workflows/ci.yml` still runs native build, sanitizers, fuzz, legacy SaaS integration, frontend verification, Rust API container and Docker verification on PR/push. Implement selective CI in a separate reviewed change with reliable path-detection, required-checks and fail-closed behavior.

## Goal

Reduce AI tokens, compute time, and duplicated tests without sacrificing security, correctness, release confidence, or public ABI stability. **Tests run in tools/CI, not by asking the model to print full terminal logs.** Use minimal failures/summaries; expand logs only for diagnostics. Never use shortcuts to bypass a failing relevant test.

## Three levels of verification

1. **Edit loop (fast, local):** lint/type/unit/checks related to edited paths; incremental compilation/caches; smallest deterministic fixture. Report command, exit status, and concise failure excerpt. No automatic full native rebuild.
2. **Boundary/PR gate:** integration tests for affected API/FFI, stable contract, browser vs server, ownership and error mapping. Build impacted native target only if the changed boundary needs it. Run full native suite when native code, ABI/headers, build scripts, codec handling, scheduler, C/Rust unsafe FFI, security admission, or release configuration changes.
3. **Release/periodic gate:** complete Ubuntu 24.04 authoritative C17+C11, public ABI, optimized/scalar, ASan/UBSan, fuzz, Rust, frontend and container/security checks plus real-host P14 evidence as applicable. Run on release-candidate revisions, manual dispatch or scheduled security verification; no claim of completion until evidence is captured.

## Change-impact matrix

| Change kind | Fast edit loop | PR/integration gate | Heavy native testing |
| --- | --- | --- | --- |
| Docs / roadmap text only | Markdown/link/diff checks | Documentation consistency, secrets review | **No** native build/san/fuzz unless instructions or release contracts change execution in a risky way |
| Pure Nuxt UI/SEO/CSS | Frontend unit, typecheck, lint, local browser check | Frontend build, accessibility/E2E for changed flows | **No** C rebuild/sanitizers/fuzz by default |
| Browser-only Rust/Wasm/Worker | Wasm unit tests, `wasm-pack` or target-specific build *if available*, worker/UI tests | Browser compatibility, memory budgets, CSP, output fixtures/parity; Rust/Wasm dependency audit | **No** native full suite by default; run cross-boundary parity fixtures if native contract is consumed |
| Nuxt proxy ↔ Axum transport | Typed schema/API tests, Rust route/adapter test | HTTP, auth, limits, malformed input, relevant FFI integration | Targeted integration; **full native only** if ABI/native or security boundary affected |
| Rust safe FFI / `unsafe` / native ABI | Rust safety/unit tests, targeted ABI/public consumer | Full native ABI, ASan/UBSan, fuzz as relevant, Rust, Docker | **Required** |
| C17 core / codec / SIMD / scheduler / memory allocation | Native targeted tests and portable/optimized parity | Full authoritative Linux verification, sanitizer/fuzz/ABI | **Required** |
| Compose, Docker base, toolchain, release config, deps | Changed configuration/security checks | Relevant build, integration, supply-chain and release gate | Risk-based; release candidate requires full agreed suite |
| Final release/canary | Focused smoke | Full authoritative evidence plus operational validation | **Required for native/server shipping candidate** |

**Escalate to the stronger gate** whenever the change affects an untrusted-input trust boundary, shared exported contract, memory ownership, new codec/decoder, build-chain safety, security policy, or fails a parity check. A nominal frontend PR that changes Rust or native contracts is not frontend-only.

## Agent cost discipline

- Inspect changed files and dependency boundaries first; name the chosen test tier and why.
- Run small, reusable, cache-friendly commands; do not delete build directories or invalidate caches unless required.
- Capture stdout/stderr into an artifact/log; report status, test counts, duration, errors, relevant reproduction, and log path. Avoid pasting 10,000 lines into the conversation.
- Do not rerun passing tests after a prose-only change unless the change invalidates them.
- Never ask Astra to watch giant CMake/fuzz/sanitizer logs; use Astra for architectural tradeoffs or a **small diff, failing test, and focused evidence**.
- Parallelize independent tests only within available RAM/CPU; especially on 1 GiB VPS, never oversubscribe compilation.
- Fuzz and sanitizers are required at the relevant boundary/release gate, but not on every keystroke. Never replace fuzzing with a model assertion.
- CI optimization must preserve a stable required aggregate status check (if branch protection is configured), avoid silent skipped required jobs, use safe base/head diff on PRs, and default to the **full** gate on path-detection failure.
- Token saving is not a reason to delete coverage, relax test assertions, or merge on failing CI.

## Before implementing selective CI

1. Map all jobs in `.github/workflows/ci.yml` and all branch protection/rulesets.
2. Propose `changes` classification for native, Rust backend, frontend, browser-Wasm, infra and docs; account for shared contracts, lockfiles, headers, and workflow modifications.
3. Keep one always-running summary job that fails if required impacted jobs fail, and clearly marks unneeded jobs skipped by policy.
4. Run negative tests of the classifier (unknown path, rename, deleted file, workflow change, unavailable diff, forks).
5. Compare PR-minutes/cost before/after; protect baseline by manual full-suite release dispatch and periodic checks.

**Not yet implemented:** selective CI configuration; this document authorizes a separately reviewed CI milestone, not hidden skipping of existing security gates.
