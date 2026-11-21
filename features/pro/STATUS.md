# IMGENGINE Pro — live evidence and status register

**Snapshot:** 2026-11-21, baseline main `d4048d7a119bab671e769bae903e56707b3674ae`. Owner selected **Pro** as the next commercial product to research and architect; first work is **C17 + Rust native verification and feature contracts**, not frontend code. This file is **proposed in an unmerged branch**, so do not present it as a main-branch status change until an owner-reviewed merge. Refresh GitHub evidence before every PR or status update.

## Truth model: status is not one boolean

- **NOW:** single highest-priority approved work item *within Pro*, independent of the existing Free release NOW queue.
- **STARTED:** discovery/specification or implementation actually initiated; record whether this is **documentation only** or code.
- **IN_PROGRESS:** active code/contract branch or PR with exact ID; never implies tests passed.
- **PLANNED:** accepted idea/requirement, no confirmed implementation.
- **BLOCKED:** a named dependency blocks work, with owner and next action.
- **COMPLETED:** specific deliverable passes defined acceptance on an exact commit; a completed document does not make a feature complete.
- **NOT_MERGED:** open/draft PR or local branch only; no claim the change exists on main.
- **MERGED:** GitHub reports a merged PR and merge commit; no automatic release or runtime verification.
- **RELEASED:** user-accessible, exact-artifact/host evidence, signed-off legal/security/finance and owner go/no-go. None for Pro.
- **NOT_RUN / PASS / FAIL:** record each verification lane separately with commands, platform, source SHA, timestamps and artifacts. CI PASS for one SHA never transfers to another.

## Current Pro board

| Work item | NOW / state | PR / evidence | Next gate |
| --- | --- | --- | --- |
| PRODUCT-PRO-01 parent | **SELECTED; PLANNED; NOT RELEASED** | Owner decision 2026-11-21, portfolio docs established | Independent security/commercial/product acceptance and release approval |
| PRO-00 documentation | **NOW / STARTED — DOCUMENTATION ONLY / NOT_MERGED** | `feat/pro/pro-00-native-first-blueprint-20261121`; PR number updated after opening | Owner review; link and status verification |
| PRO-00 customer discovery | **PLANNED / NOT_STARTED** | No recorded Pro interviews or paid pilots | Interview users and measure saved time |
| PRO-00 native C/Rust capability audit | **PLANNED / NOT_STARTED** | Existing core/FFI foundation is shared but no Pro-specific acceptance | Read ABI, fixture, API ownership; define smallest implementation |
| PRO-01 through PRO-05 | **PLANNED / NOT_STARTED** | No verified Pro implementation PR, test pass or merge | One bounded feature PR per ID |
| PRO-06 through PRO-11 | **LATER / NOT_STARTED** | No verified Pro implementation | Demand + prior core gates |
| PRO-12 and PRO-13 | **DEFERRED / NOT_STARTED** | Frontend out of scope for current discussion | Separate owner-approved frontend planning |
| PRO-14 and PRO-15 | **LATER / NOT_STARTED** | No verified implementation | Demand, quality and license proofs |
| PRO-16 through PRO-18 | **BLOCKED / NOT_STARTED** | Requires owner finance/security/account/deployment decisions | Explicit business/architecture ADRs and tests |

### Existing merged work that Pro may reuse — NOT Pro completion

- Four-product **planning documents** originated in [PR #56](https://github.com/Rofikali/imgengine/pull/56) and are present on main. This means shared portfolio planning was **MERGED**, not Pro functionality.
- Free component PRs [#57](https://github.com/Rofikali/imgengine/pull/57) resize, [#59](https://github.com/Rofikali/imgengine/pull/59) crop, [#62](https://github.com/Rofikali/imgengine/pull/62) rotate, [#72](https://github.com/Rofikali/imgengine/pull/72) compress, and [#76](https://github.com/Rofikali/imgengine/pull/76) convert are **MERGED**. Free remains **NOT RELEASED** in the recorded main release board, and browser-local Free functionality is not proof of native Pro recipes or batches.
- Existing C17 engine, safe Rust FFI, test toolchain, native Docker verifier and protected Rust server are **shared engineering foundations**; their historical existence does not satisfy a new Pro feature gate.
- No verified **Pro feature** is MERGED, COMPLETED or RELEASED as of the above baseline. Any future status update requires linked PR + exact CI evidence.

## Native-only verification lanes

| Lane | Existing source | Current Pro-specific evidence |
| --- | --- | --- |
| Native C17 Debug/CTest | root `Makefile` / `imgengine/` | **NOT RUN for Pro** |
| C11 compatibility/public ABI | native verifier and existing CI | **NOT RUN for Pro** |
| Rust facade and external FFI consumer | `backend/imgengine/`, `backend/imgengine-ffi-smoke/` | **NOT RUN for Pro** |
| Rust supervisor resource/lifecycle tests | `backend/imgengine-supervisor/` | **NOT RUN for Pro** |
| ASan/UBSan and malformed-image fuzz | existing native verification | **NOT RUN for Pro** |
| Ubuntu 24.04 Docker Desktop compose | `features/pro/compose.native-test.yml` | **CONFIGURATION PROPOSED, NOT RUN** |
| Separate optional native-only GitHub Actions | `.github/workflows/pro-native-smoke.yml` | **PROPOSED, NOT RUN** |
| Required existing repo CI checks | `.github/workflows/ci.yml` | **MUST verify for the new PR head; not inherited** |
| Frontend/Playwright Pro behavior | deferred | **NOT IMPLEMENTED / NOT RUN** |
| Real public host, billing and customer tests | future ADR | **NOT RUN / NOT APPROVED** |

## Small-PR protocol and truthful handoff

1. Never mark `PRO-01` started because a recipe JSON file is *described*; require a code change and linked PR. Likewise `PRO-02` is not done because native worker machinery exists.
2. At most one named implementation milestone per PR; include tests, code, dependent docs, exact SHA and failure/rollback notes. Avoid permanent Pro branches and avoid editing auto-generated Free milestones directly.
3. If a feature is coded but not merged: `IN_PROGRESS / NOT_MERGED`. If merged but tests/run-time acceptance incomplete: `MERGED / NOT_COMPLETED`. If merged and acceptance tested: `COMPLETED` with exact evidence; product remains `NOT_RELEASED`.
4. Track all Free, Studio, API, SDK and infrastructure prerequisites independently. A Pro status update cannot publish Free, deploy Docker, activate API billing or authorize spending.
5. Next handoff must contain date, repo/branch/head, status by feature ID, PR numbers, exact CI/test outcomes, known blockers, docs updated and one next eligible step.

See [product contract](README.md), [native verification](NATIVE_TESTING.md), root [next.md](../../next.md), and [release compatibility matrix](../../docs/IMGENGINE_FIVE_PRODUCT_RELEASE_COMPATIBILITY_MATRIX.md).
