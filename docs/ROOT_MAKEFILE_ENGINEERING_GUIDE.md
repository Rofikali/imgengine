# IMGENGINE — Root Makefile Engineering Guide

**Status:** New contributor command interface, proposed for review. Unlike the older [native-only Makefile](../imgengine/Makefile), the **repository-root `Makefile`** provides one stable entry point for C17, focused Rust FFI/Axum tests, Nuxt/Vitest/Playwright, and optional full Linux/Docker verification. The established **CMake, Cargo, pnpm and authoritative verification script remain the owners** of their own build/test semantics.

**Source reviewed:** `main` at `8fa2cc3faa651b99599b750978662cd85ce272fb` on 2026-11-16. The existing GitHub CI workflow runs **seven jobs on every PR**, independently of this file. Recheck HEAD before treating the snapshot as current.

## 1. The point of a root Makefile

For a multi-language product, `make` should be a **command menu, not another build system**. It must not duplicate CMake rules, vendored code, Rust manifests, frontend package scripts, or CI definitions. The native-only `imgengine/Makefile` is intentionally **preserved**, including its legacy profiling commands. The recommended entry point is the new root Makefile from repository root.

- Start safely: **`make` or `make help` only prints help**; it never starts builds, containers or services.
- Isolate build outputs: `imgengine/build/make-*` and `build/make-logs/`, covered by existing repository `.gitignore` patterns.
- Bounded native compilation: `JOBS=2` by default, allowed 1–16, so small Docker/Codespaces environments do not accidentally use all available cores.
- No implicit package install, production services, `sudo` performance profiling, deployment, payment, branch reset, `git clean` or generic `rm -rf build/`.
- Risk-based checks are selected by developer/agent based on changed paths, **never by a Makefile that guesses whether a PR is safe**.
- Expensive full verification and cleanup require explicit flags, and preserve real exit codes.
- Local tests never establish public release readiness. The M1→M2 progression belongs in [the milestone continuation runbook](AGENT_MILESTONE_CONTINUATION_RUNBOOK.md), not Makefile side effects.

## 2. Target-to-scope matrix (run in Ubuntu 24.04/Linux workspace)

| Change or goal | Root command | Notes |
| --- | --- | --- |
| What commands exist? | `make help` | Safe default |
| Quick build-menu safety smoke | `make self-check` | Validates help/dry runs, invalid arguments and opt-in refusal **without compiling** |
| Baseline before work | `make status` | Local SHA/branch/dirty files; fetch latest remote separately |
| First frontend environment | `make frontend-setup` | Explicit frozen pnpm install, not automatic on every iteration |
| UI/unit code | `make frontend-test` | Vitest tests; avoid C rebuild |
| M1.2/M1.3 frontend gate | `make frontend-check` | Unit, Nuxt typecheck, production build |
| Browser interactions/a11y | `make frontend-browser` | Requires installed Playwright browsers and existing production Nuxt build |
| Live Nuxt→Rust integration | `make frontend-browser-live` | Requires separately configured API/server/credentials; no service started here |
| C17 code | `make native-test` | Native Debug CMake and CTest, does not replace full PR security CI |
| Optimized local binary | `make native-release` | Compile only; **not** release authorization |
| Older C compatibility | `make native-c11` | Isolated C11 compatibility configuration |
| Native memory safety | `make native-sanitize` | ASan/UBSan CTest |
| Native fuzz target | `make native-fuzz FUZZ_RUNS=1000` | Clang and bounded runs; not full fuzz-release evidence |
| Rust API, Rust FFI or supervisor | `make rust-test RUST_CRATE=imgengine-api` | Supported crates: imgengine-api, imgengine-supervisor, imgengine, imgengine-ffi-smoke; **builds C first** to link native library |
| Rust formatting | `make rust-fmt-check RUST_CRATE=imgengine-api` | Doesn't require C native build |
| Full Ubuntu/Linux native/FFI/security | `make verify-linux CONFIRM_FULL=YES` | Runs existing `imgengine/scripts/verify_linux.sh`; deletes its *dedicated* verification tree and rebuilds |
| Full Ubuntu 24.04 Docker | `make verify-docker CONFIRM_FULL=YES` | Builds existing `imgengine/Dockerfile.verify`, runs authoritative script, needs Docker |
| Remove only Make-owned outputs | `make clean CONFIRM_CLEAN=YES` | No removal of Cargo/third-party/user data or other native builds |

**Not provided on purpose:** `make deploy`, `make merge`, auto-push, `make prod`, `make benchmark` with guessed workloads, root `make test` that silently tests an arbitrary subset, or any target that modifies image/customer data. Benchmarking requires a separate workload-specific, controlled runner/evidence plan.

## 3. Two important usage examples

**M1.3 frontend/browser parity (C/FFI unchanged):**

~~~bash
make status
make frontend-test
make frontend-check
make frontend-browser
# Live browser+API only when the environment is ready:
# make frontend-browser-live
~~~

A real browser/native pixel-parity milestone also requires known-good fixtures, decoded-pixel comparison/rounding tolerances and the P12 omission path; these checks may require the live Rust/C integration and do not become satisfied merely by `make frontend-check`. Existing full GitHub CI runs after publication.

**Native C17/ABI security change:**

~~~bash
make native-test JOBS=2
make native-c11 JOBS=2
make native-sanitize JOBS=2
make native-fuzz JOBS=2 FUZZ_RUNS=1000
# For full native/release boundary gate when appropriate:
make verify-linux CONFIRM_FULL=YES JOBS=2
~~~

The root wrapper never replaces C ABI C11/C17/export/FFI/real-image and full security gates for native changes. See [change-scoped verification](CHANGE_SCOPED_VERIFICATION.md).

## 4. Minimal AI tokens without skipping verification

**Agents should not run the full native verifier after every frontend tweak.** For relevant local checks, run the command and report outcome, test revision and only failing excerpts; full CI already executes on PRs. Full `verify-*` output is automatically stored in `build/make-logs/`; a success prints one PASS line, a failure prints only its last 50 lines and preserves a nonzero exit. Other focused commands may print output; the agent can redirect it to a private log when appropriate. Do not log secrets or raw customer-image data. `make verify-linux` is *not equivalent* to the GitHub seven-job suite (which also runs Nuxt, SaaS integration and Rust API container separately).

Follow [CI-first log policy](CI_FIRST_MINIMAL_AI_CONTEXT_POLICY.md), [test lanes](TEST_LANES_CI_OPTIMIZATION_AND_AGENT_BUDGET_2026-11-15.md) and required PR CI. ChatGPT tokens are **different from** CI runner minutes, compilation time, or code quality.

## 5. Seven-level engineering, security, CTO and CA/MBA view

- **Junior/Mid:** Discover one consistent command per task, clear prerequisites and meaningful failures.
- **Senior:** Separate native, Rust and frontend ownership; write tests for changed boundaries.
- **Staff/Senior Staff:** Avoid duplicate build logic, hidden side effects and accidental cross-boundary regressions.
- **Principal/Distinguished:** Retain authoritative ABI/test/release gates and scalable tool boundaries without premature build framework complexity.
- **Security/SRE:** No implicit `sudo`, deploy, secret propagation, image-data operations or broad cleanup; opt-in full runs and bounded resources; real exit statuses; no false green checks.
- **CTO/Product:** Small PRs, predictable onboarding and repeated developer actions, versioned contracts, clear escalation.
- **CA/MBA/Management:** Minimize duplicated CI/agent effort, hardware load and opportunity cost; measure costs separately; no claim of nanosecond speedups or free resources.

## 6. Review and verification rules for this Makefile change

- **The proposed Makefile is new execution logic.** Run `make self-check` in a real GNU Make/Ubuntu checkout; it checks help, dry-run targets, invalid arguments and refusal gates without compiling. The PR should receive real GNU Make smoke evidence, not just Markdown link checks, before being considered accepted. Verify opt-in refusal for `verify-linux`, `verify-docker` and `clean`; invalid `JOBS`/Rust crate rejection; output placement; and existing required CI. No real full verifier is necessary just to inspect help parsing, but current GitHub CI still executes automatically.
- Because the GitHub connector cannot execute arbitrary new Make targets in a local repo, **do not present unchecked `make help` as PASS**. A coding runner or reviewer with repo checkout should validate those commands and report the actual SHA.
- Full Docker verifier builds the Ubuntu verification image again; it is intentionally opt-in and expensive. Docker must be available and `$(CURDIR)` must map correctly to a Linux filesystem directory. Windows PowerShell should run commands **inside Ubuntu 24.04 Docker/WSL**, not with a native Windows-host GNU Make setup.
- Release/deployment still requires the [P14 runbook](P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md), actual security/capacity evidence, and owner approval.

**Maintenance rule:** When a new independently tested build/test task appears, prefer adding a small root target that calls an existing authoritative command. Do not copy all seven CI jobs into Make recipes, and do not infer that running one target completes a product milestone.
