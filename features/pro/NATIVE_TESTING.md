# IMGENGINE Pro — native-first Docker Desktop / Ubuntu 24.04 / CI-CD contract

**State: PRO-00 documentation candidate, not tested/released.** The owner's current direction is **C17 core and Rust verification first; Pro frontend later**. This does not remove existing required repository-wide CI checks, which may independently include frontend jobs.

## 1. Runtime separation

| Part | Environment | Status |
| --- | --- | --- |
| Windows 11 host | VS Code, Git and Docker Desktop/Compose orchestration ONLY | Development host |
| Development execution | Docker Desktop Linux engine; Ubuntu 24.04 container | Required |
| Native C17 + public ABI/C11 compatibility | CMake/Ninja, CTest, sanitizer/fuzz/ABI | In scope |
| Rust | Separate Cargo packages, safe FFI, supervisor and lifecycle | In scope |
| Nuxt/Vue/TS or browser testing for Pro | Separate later milestone | **DEFERRED** |
| API accounts, billing, public deployment | Independent security/financial decision | **NOT AUTHORIZED** |

Read root [Docker-only development policy](../../docs/IMGENGINE_DOCKER_ONLY_DEVELOPMENT_POLICY.md), [stack ADR](../../docs/TECHNOLOGY_AND_REPOSITORY_POLICY.md), [native AGENTS](../../imgengine/AGENTS.md), and [Rust AGENTS](../../backend/AGENTS.md). The existing C verifier invokes some tracked Python helpers; this does not authorize **new** Python work.

## 2. Files to use, not duplicate

- Existing base: [imgengine/Dockerfile.verify](../../imgengine/Dockerfile.verify) (Ubuntu 24.04, C/CMake/Ninja/Cargo dependencies); **not** a dedicated production Pro image. Rust package version/toolchain compatibility must be checked in actual Docker execution.
- New local test wrapper: [features/pro/compose.native-test.yml](compose.native-test.yml), one C/Rust-only test service; no ports, frontend service, database or new runtime dependencies.
- Native and Rust entry points: [root Makefile](../../Makefile), [imgengine/scripts/verify_linux.sh](../../imgengine/scripts/verify_linux.sh), [backend/README.md](../../backend/README.md). Cargo packages are separate; do **not** assume a root Cargo workspace.
- CI: existing mandatory [ci.yml](../../.github/workflows/ci.yml), and an **optional manual native-only** [pro-native-smoke.yml](../../.github/workflows/pro-native-smoke.yml).
- The current backend under `backend/` is Rust; the legacy Python backend is outside new-Pro scope.

## 3. Windows 11 + Docker Desktop commands

From repository root, a Windows host PowerShell terminal runs **only Git/Docker orchestration**, not local `cmake`/`cargo`/`node`:

```powershell
git status -sb
git branch --show-current
git rev-parse HEAD
docker info --format "{{.OSType}}"
docker compose -f features/pro/compose.native-test.yml config --quiet
docker compose -f features/pro/compose.native-test.yml build native-tests
docker compose -f features/pro/compose.native-test.yml run --rm --no-deps native-tests
```

Expected Docker OSType: `linux`. Compose builds existing `imgengine/Dockerfile.verify` from repo-root context, mount current working tree, then runs C17 CTest, Rust FFI facade and consumer tests, and Rust supervisor tests. It is **not a Pro feature test** until actual Pro code/fixtures are added. Docker will require dependencies on initial build; perform a supply-chain and version review before relying on an offline workflow.

Use a separate clean worktree when another agent is editing Free; the bind mount can write build output under `imgengine/build/`. Avoid concurrently running tests into the same build directory. For low-memory machines, tune limits with an evidence-backed Compose change, not by quietly disabling safety checks.

### Full existing native release-candidate verifier (separate, expensive)

Inside an **approved Ubuntu 24.04 container** at repository root:

```bash
make verify-linux CONFIRM_FULL=YES JOBS=2 FUZZ_RUNS=1000
```

The full verifier covers C11 compatibility, sanitizers, fuzz, ABI checks and Rust crates. The repository also has `make verify-docker CONFIRM_FULL=YES` for a Docker-managed invocation, as documented in the root Makefile. Run full gates when native/ABI/security changes warrant them, not for docs-only edits. Do not claim PASS unless exact commands actually ran.

## 4. Evidence and gates

| Change scope | Test expectation |
| --- | --- |
| Pro docs only | Broken links, status truth, `git diff --check`, Compose YAML/static review |
| Pro native verification-only | Existing C17 CTest, Rust safe facade, external FFI consumer, supervisor |
| PRO-01 recipe code | JSON version, malformed/overflow/unknown-op, deterministic ordering and round trip |
| PRO-02 batch code | Bounded admission, partial failures, cancellation, cleanup and peak RSS |
| PRO-03 operation semantics | Golden image fixtures, orientation/alpha/ICC, C11 header/API compatibility |
| PRO-04 multi-output | Output identities, total work cap, failure isolation, naming collision |
| PRO-05 manifest | Stable structured errors, hash/dimensions, safe path and export limits |
| Native/unsafe/security changes | Relevant sanitizer/fuzz, ABI and full required GitHub CI |
| Pro frontend | **NOT NOW**; later browser privacy, a11y, browser/device memory and visual correctness |

Record all test outcomes as `PASS`/`FAIL`/`NOT_RUN`, with exact SHA, environment, image digest, commands, elapsed time and failure excerpts. Existing required GitHub Actions checks at each exact PR head remain mandatory; optional local/manual jobs cannot replace them. Benchmark speed/throughput/peak RSS with repeated representative image sets, not unverified nanosecond claims.

## 5. Security, CI and CD

- **Native CI:** optional manual workflow on GitHub Ubuntu 24.04; one local Ubuntu container via Compose. The workflow may require review/merge to default branch before it can be dispatched. No new required check/protection change is automatic.
- **Main CI:** existing `.github/workflows/ci.yml` unchanged. It may include frontend jobs per trusted selection policy; Pro-only local verification does not bypass them.
- **CD:** none authorized for Pro. No deployment, release tag, container push, billed storage, registry secrets, account setup or pricing activation.
- **Container:** no privileged mode, host network, Docker socket, public port or production secret; minimal permissions and bounded process resources. File inputs must be synthetic or explicitly approved. Clear image bytes, sensitive filenames and secrets from logs.
- **Supply chain:** the inherited Dockerfile uses an Ubuntu 24.04 tag rather than a digest and apt packages whose versions may change. Version/digest pinning, SBOM, provenance and licensing remain future release gates.
- **Product boundaries:** C17 owns pixel geometry/codec/scheduling; Rust owns application orchestration/validation/FFI; no unsupported `Send/Sync` promise and no silent mid-call native cancellation guarantee.

## 6. Next bounded milestone

After this review-only PRO-00 PR, open **PRO-00-NATIVE-01**: inspect ABI, C17/Rust functionality and missing primitives; run Docker verification and document actual outcomes at exact SHA. Then select the smallest PRO-01 recipe-contract implementation. Defer Nuxt/Pro frontend until a later owner conversation. See [requirements](README.md) and [status](STATUS.md).
