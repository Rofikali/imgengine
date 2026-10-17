# P7.5 Branch Consolidation Record

## Scope

This record captures the controlled integration of the verified P7.5 grayscale
dispatch and diagnostic-observability work into the post-recovery `main`
baseline. It is an integration and evidence record, not a production-readiness
claim.

## Baseline and integrated history

- Integration baseline: `origin/main` at
  `7313fbaf6f1885e381be20c2104f15e082574ce3`.
- Integrated branch tip before consolidation:
  `origin/codex/p7-5-grayscale-observability` at
  `f2ef76d7616311129293a2113591d74df418f28c`.
- P7.5 commits preserved by the integration:
  - `2b213e0 fix(native): restore grayscale dispatch equivalence`
  - `f2ef76d fix(native): silence raw ABI diagnostics`
- Review vehicle: GitHub pull request #6,
  `codex/p7-5-grayscale-observability` into `main`, merged normally as
  `def28ea9972cd34efce2dea70a72271cbb758af3`.

The semantic conflict resolution deliberately retains `main`'s current scalar
and AVX2 grayscale implementation while retaining P7.5's independent,
pixel-aware dispatch coverage. No public ABI, scheduler contract, or
production resource policy is changed by this consolidation.

## Verification evidence

Focused integration verification passed in an isolated worktree using the
Ubuntu 24.04 verifier image:

```text
format-check
resize_kernel_equivalence
abi_public_consumer
```

The unchanged authoritative command also passed in Ubuntu 24.04 Docker:

```text
bash scripts/verify_linux.sh
```

Its evidence includes normal native CTest (13/13), native security/output /
geometry/progressive regressions, Rust FFI, supervisor tests (17/17), Axum API
tests (11/11), C ABI and Rust FFI real-image processing, ASAN/UBSAN/leak
verification, sandbox lifecycle verification, and both libFuzzer targets at
5,000 runs each. The terminal result was:

```text
Linux verification passed: /workspace/build-linux-verify
```

## Branch hygiene follow-up

After PR #6 merged, each candidate below was checked against the resulting
`origin/main`: it was an ancestor, had zero unique commits, had no open pull
request, and had no containing tag. The verified obsolete remote refs were
then deleted. This does not delete their local branches or worktrees.

| Deleted remote ref |
| --- |
| `chore/professional-repository-hardening` |
| `codex/fix-cli-output-contract` |
| `codex/fix-saas-candidate-fixtures` |
| `codex/native-release-candidate` |
| `codex/p7-5-grayscale-observability` |
| `codex/p7-frontend-foundation-audit` |
| `codex/p7-recovery-integration` |
| `codex/rust-admission-control` |
| `codex/rust-request-lifecycle` |
| `codex/rust-supervisor-foundation` |
| `codex/scheduler-characterization` |

The protected rescue evidence and the dirty canonical checkout remain out of
scope for this remote-ref cleanup.

## Status

P7.5 grayscale dispatch evidence is integrated for review. Production
readiness remains **not claimed**.
