# P7.2.8 Implementation Audit

## 1. Audit scope

Independent audit of the C-private `layout.render.v1` implementation against
P7.2.0-B through P7.2.7. No implementation, API, ABI, scheduler, Rust, HTTP, or
Nuxt changes were made by this audit.

## 2. Baseline

- Branch: `codex/p7-2-render-ui`
- Starting/current commit: `90cedaf fix(frontend): return safe proxy errors`
- Intended P7.2.8 tracked changes: `imgengine/CMakeLists.txt`,
  `include/runtime/task.h`, and `src/runtime/worker_loop.c`.
- Intended P7.2.8 untracked additions: private layout header/executor/test and
  P7.2.8 documentation.
- Pre-existing untracked P7.2.0-B through P7.2.7 planning documents remain
  uncommitted.
- Unrelated tracked changes: none observed.
- `git diff --check`: PASS.

## 3. Evidence reviewed

Reviewed the P7.2.0-B–P7.2.7 planning set, P7.2.8 private header/executor/test,
CMake wiring, worker dispatch, decoder wrapper, resize, canvas, blit, encoder,
and scheduler paths. Focused Ubuntu build/test and ASAN/UBSAN/leak test were
reported PASS before this audit.

The authoritative Ubuntu verifier completed **FAIL** at `format-check`:
`layout_render_private.c` and `test_layout_render_private.c` require
clang-format. Because `verify_linux.sh` stops at that point, its normal CTest,
ASAN/UBSAN, sandbox, and fuzz stages were not executed for this change.

## 4. Requirement-to-implementation matrix

| Requirement | Implementation evidence | Test/evidence | Status |
| --- | --- | --- | --- |
| Non-owning decode | `img_layout_render_decode_nonowning` uses `img_api_decode_prepared` / `g_io_vtable.decode`, not `img_api_decode_input` | JPEG/PNG/malformed focused cases | Implemented |
| C-owned queued state | State owns decoded `img_buffer_t` and copied `calloc` rectangle array before submit | Source trace | Implemented |
| Scheduler extension only | One `IMG_TASK_KIND_LAYOUT_RENDER` payload and worker switch branch | Source diff | Implemented |
| FIT, no crop/stretch | Integer branch computes one uniform scale then calls existing resize | Source trace | Implemented; oracle evidence missing |
| Explicit RGB background | Executor fills entire canvas before rendering | Source trace | Implemented; pixel assertion missing |
| Rectangles | Positive dimensions, checked ends, bounds, pairwise overlap before submit | Source trace; rejection smoke cases | Implemented; boundary coverage incomplete |
| Exact completion | Expected/planned initialized from request/count; encoder is reached only after equality check | Source trace | Implemented; forced failure-path evidence missing |
| JPEG ownership | Existing encoder allocation; failure frees; success transfer clears state pointer; release uses `img_encoded_free` | Source trace; repeated release smoke test | Implemented |
| Independent RGB oracle | No test-only scalar fixed-point reference exists | Test source | **Mismatch** |
| Mandatory fixture semantics | Test mostly checks status/count/output dimensions | Test source | **Mismatch** |

## 5. Ownership/lifetime audit

Encoded input is passed only to `img_api_decode_prepared`, which calls the
registered decoder and does not call the historical mmap-owning
`img_api_decode_input`. The input pointer is not stored in state or task
payload. The state copies placements with `calloc` before submission. The
stack `img_task_t` is waited to `IMG_TASK_DONE` before state destruction.

Decoded source is slab-owned through `owner_pool`; cleanup uses
`img_runtime_release_raw_buffer`. Worker canvas and scaled buffers are slab
owned and released on normal and error exits. Encoder output is cleared/freed
on failure, transferred only after successful wait, and released by the private
result release function. The audit found no direct new use-after-free or wrong
allocator path.

## 6. Decoder audit

The new path avoids `img_api_decode_input` and its `munmap` behavior.
Malformed tiny input is exercised only as a smoke rejection. Truncated JPEG/PNG,
zero input, extreme decoded dimensions, and decoder allocation failure do not
have capability-specific tests. These are missing evidence, not proof that the
underlying decoder is unsafe.

## 7. Scheduler audit

P7.2.8 extends the scheduler; it does not redesign it. The queue, selection,
worker lifecycle, and existing task branches remain unchanged. The new task
holds a single C-owned state pointer and synchronously joins through the
existing wait function. Existing scheduler synchronization/task-state semantics
remain inherited risks and were not changed by this task.

## 8. FIT arithmetic and rectangle audit

The FIT cross-products are products of two `uint32_t` values and therefore
fit `uint64_t`; source dimensions are checked non-zero before division.
Fitted dimensions are clamped to one and constrained to the rectangle.
Destination centering is deterministic and odd residual pixels remain on the
right/bottom. Request validation performs checked `x + width` and
`y + height`, validates canvas allocation arithmetic before calling the
historical allocator, and rejects overlap.

The later blit retains its historical clipping behavior, but validated
rectangles plus fitted dimensions ensure this capability does not invoke it with
an out-of-bounds copy. Tests do not prove horizontal/vertical letterboxing,
exact-aspect, one-pixel, boundary-touching, or odd-residual pixels.

## 9. Completion, rendering, and test-independence audit

The encoder is called only after `expected == planned == executed`; error
cleanup prevents the internally-held JPEG pointer escaping. However, no test
injects resize/allocation/worker failure at placement N, so no direct evidence
proves every mismatch/failure cannot produce output.

The test has generated JPEG and embedded PNG inputs, exercises scheduler
execution, output dimensions, several invalid requests, malformed input, and
idempotent result-release. It does **not** contain the required independent
scalar fixed-point RGB reference. It does not decode output and compare
background, fitted margins, placement pixels, or semantic output against such
a reference. Its success assertions are therefore insufficient to establish
the frozen rendering semantics.

## 10. Memory, concurrency, regression, and contamination audit

Focused ASAN/UBSAN/leak evidence is positive, but the authoritative sanitizer
gate has not run because formatting failed first. The private API is under an
internal include path, is not installed by the public-header install rule, and
does not modify ABI v1 symbols, Rust, HTTP, or Nuxt. No scheduler algorithm,
legacy task behavior, or public capability discovery was changed.

Historical classification:

| Item | Classification | Reason |
| --- | --- | --- |
| `img_api_decode_input` | QUARANTINE / DO NOT USE | Releases mmap input with `munmap`. |
| `img_api_decode_prepared` | PROVEN / SAFE TO REUSE | Non-owning vtable decoder call. |
| Historical float `img_layout_prepare_fit` | REQUIRES ADAPTER | Float geometry is not P7 semantic authority. |
| `img_layout_resize_bilinear` | PROVEN / SAFE TO REUSE | Production resampling primitive; needs independent oracle. |
| `img_blit_avx2` | REQUIRES ADAPTER | Silently clips; new pre-validation is essential. |
| `img_layout_grid_place_cells` | QUARANTINE / DO NOT USE | Historical grid and partial-success semantics conflict. |

## 11. Findings register

### P7-2-8-A01

- Severity: HIGH
- Area: verification / release gate
- Evidence: `verify_linux.sh` exited 1 at `format-check`; both new C files
  need clang-format.
- Impact: authoritative normal, sanitizer, sandbox, fuzz, and regression
  evidence is absent.
- Current behavior: full verifier stops before CTest.
- Expected behavior: full verifier passes unchanged.
- Recommendation: format only the reported new files, rerun full verifier.
- Blocking status: blocking.

### P7-2-8-A02

- Severity: HIGH
- Area: rendering correctness / test independence
- Evidence: `test_layout_render_private.c` has no scalar fixed-point reference
  and asserts dimensions/counts rather than RGB semantics.
- Impact: FIT, centering, background preservation, and production resize output
  are not independently proven.
- Current behavior: production output is only decoded for dimensions.
- Expected behavior: deterministic scalar-reference RGB canvas comparison plus
  semantic JPEG comparison under a measured/documented rule.
- Recommendation: add the frozen test-only reference and targeted fixtures;
  do not use production resize as expected output.
- Blocking status: blocking.

### P7-2-8-A03

- Severity: MEDIUM
- Area: fixture coverage
- Evidence: mandatory letterbox, exact-aspect, one-pixel, odd residual,
  boundary-touching, fewer/exact capacity semantics, and injected
  partial-execution failure cases are absent or lack meaningful assertions.
- Impact: frozen contracts lack sufficient regression evidence.
- Recommendation: add deterministic explicit fixture cases and failure
  injection/equivalent controllable failure evidence.
- Blocking status: required before freeze; can be combined with A02.

### P7-2-8-A04

- Severity: LOW
- Area: decode-path evidence
- Evidence: only a tiny malformed string is exercised.
- Impact: truncated format and zero-length/extreme cases lack direct capability
  regression evidence.
- Recommendation: add bounded negative cases while retaining existing decoder
  security gates.
- Blocking status: non-blocking once A01–A03 are resolved.

## 12. Freeze decision and next gate

**FAIL — FIX REQUIRED.**

No implementation defect was proven by source review, and ownership/scheduler
scope are consistent with the frozen architecture. Nonetheless, a failed
authoritative verifier and missing independent rendering oracle are blocking
evidence failures. P7.2.8 must not be frozen or exposed through a public native
ABI/Rust boundary until A01–A03 are resolved and the full Ubuntu verifier passes.

Required fixes before freeze: A01, A02, and A03.

## 13. Remediation history

### A01 — formatting

Original finding: the authoritative verifier stopped at format-check because
the two new C files were not clang-formatted.

Remediation: only `layout_render_private.c` and
`test_layout_render_private.c` were formatted with the repository formatter.

Focused verification: `format-check` now reports all checked files clean.

Status: **partially resolved**. The complete authoritative verifier must still
pass before this finding is closed.

### A02 — independent scalar RGB oracle

Original finding: no independent scalar fixed-point RGB oracle existed.

Remediation attempted: the test now has an independent checked-integer FIT
geometry reference covering horizontal, vertical, exact-aspect, and odd-residual
geometry. It does not call production FIT or resize code.

Status: **open / blocking**. Geometry evidence is not a scalar fixed-point RGB
oracle; the required independent RGB canvas comparison remains absent.

### A03 — semantic fixture evidence

Original finding: semantic FIT/background and controlled failure coverage was
insufficient.

Remediation: deterministic geometry cases now cover horizontal/vertical
letterboxing, exact aspect, and odd residual placement; a deliberately
oversized canvas request proves planned work can fail with no output ownership
transfer.

Status: **partially resolved**. Pixel-level background and rendered RGB
assertions remain blocked on A02's independent reference.

Explicit non-goals remain public ABI/FFI/API work, FILL/crop/AUTO_FIT,
PDF/PNG output, resource policy, scheduler redesign, and production-readiness
claims.

## 14. Remediation re-audit — A01 through A03 closure

This section preserves the original findings above and records the evidence
added after them. It does not revise their historical status at the time of the
first audit.

### A01 — formatting: PASS

The two new C files were formatted with the repository formatter. The complete
authoritative Ubuntu 24.04 verifier subsequently reached and passed
`format-check`:

```
[fmt-check] 413 file(s) checked — all clean.
```

The verifier container exited with status zero. A01 is closed.

### A02 — independent scalar fixed-point RGB oracle: PASS

`src/tests/test_layout_render_private.c` now contains the test-only
`reference_resize_rgb` implementation. It uses direct RGB buffers and a
separately written 16.16 fixed-point bilinear equation: endpoint sampling
ratios are computed with integer division, horizontal and vertical samples are
weighted in two stages, and the final 32-bit fixed-point result is rounded with
`2^31` before the shift. Its FIT dimensions and placement origins are computed
by the existing test-only checked-integer reference, not the production FIT
adapter.

The oracle never calls production FIT, production resize, production blit, the
private renderer, or the scheduler. `reference_render_case` invokes the
production `img_layout_resize_bilinear` only as the system under test, then
compares its assembled RGB canvas byte-for-byte with the independent reference
canvas. Thus a change to production bilinear sampling changes the compared
production pixels and fails the test; the expected pixels cannot be derived
from the production resize implementation.

Focused evidence:

```
cmake --build build-p7 --target test_layout_render_private -j2
./build-p7/test_layout_render_private
[layout-render] JPEG/PNG, scheduler, ownership, and rejection cases passed
```

The same test passed under focused ASAN, UBSAN, and leak detection. A02 is
closed.

### A03 — semantic fixture evidence: PARTIAL / BLOCKING

The RGB oracle fixtures cover a horizontal letterbox, a vertical letterbox, an
exact-aspect placement, and two placements on one test-assembled canvas. They
use distinct small RGB source pixels and explicit background `{17, 31, 47}`.
Exact canvas comparison verifies the production bilinear output against the
independent RGB reference and verifies the test composition's margins and
out-of-rectangle pixels. The separate checked-integer geometry test covers the
one-pixel/odd-residual centering rule. Existing private-capability cases retain
JPEG and PNG decode, scheduler completion, result-release, malformed-input,
request-rejection, and output-dimension evidence.

`test_partial_execution_has_no_output` retains the controlled oversized-canvas
failure case: planned work is one, execution is not successful, no JPEG pointer
or size is transferred, and result release remains safe. This is evidence for
the frozen `expected == planned == executed` success invariant without adding a
new failure-injection mechanism.

JPEG bytes are intentionally not compared. The frozen documents do not select
a JPEG loss tolerance. More importantly, the private renderer currently does
not expose its pre-encoder canvas: the exact RGB comparison therefore does not
yet execute the renderer's own canvas-fill and validated-blit calls. The
capability path verifies its JPEG semantically through decode and dimensions,
but that is not an exact renderer-level RGB oracle. A03 remains blocking until
a narrowly scoped test seam or equally independent renderer-level observation
compares that actual pre-encoder canvas with the reference; no JPEG tolerance
should be invented to bypass this requirement.

## 15. Authoritative Ubuntu 24.04 verification after remediation

The complete verifier was run from a fresh `imgengine-p7-2-8:local` Ubuntu
24.04 container with:

```
IMGENGINE_VERIFY_JOBS=2 IMGENGINE_FUZZ_RUNS=5000 bash scripts/verify_linux.sh
```

Result: exit status `0`, including format-check, normal native CTest (9/9,
including `layout_render_private`), native regression/security checks, Rust FFI
smoke, Rust supervisor and API suites, real-image verification, ASAN/UBSAN
verification, sandbox checks, and both 5,000-run fuzz gates. The verifier
reported:

```
Linux verification passed: /workspace/build-linux-verify
```

## 16. Final re-audit and freeze decision

Ownership, decoder lifetime, scheduler state ownership, checked FIT arithmetic,
rectangle validation, completion accounting, JPEG ownership/release, and ABI /
API isolation were re-reviewed. The new code remains C-private: no public ABI
v1 header, Rust FFI, HTTP/API, Nuxt, or scheduler-algorithm change was made.

The historical classifications remain unchanged:

| Item | Classification | Current conclusion |
| --- | --- | --- |
| `img_api_decode_input` | QUARANTINE / DO NOT USE | Still not used. |
| Historical float FIT | REQUIRES ADAPTER | Still not selected as semantic authority. |
| Historical grid placement | QUARANTINE / DO NOT USE | Still not used. |
| Production bilinear resize | PRODUCTION CANDIDATE / requires independent oracle evidence | The required independent oracle evidence now exists. |
| Validated blit | Production path only after validation | Validation remains required and unchanged. |

Blocking evidence gap: A03 still lacks an exact RGB comparison of the private
renderer’s own pre-encoder canvas, including its fill and validated-blit path.
A minimal test-only/internal observation seam is required; it must not become
public ABI, Rust, or API surface. Non-blocking evidence gaps also remain:
capability-specific zero-length, truncated JPEG/PNG, and extreme-dimension
negative fixtures are not yet exhaustive; the underlying repository
decoder/security gates remain the current evidence for those classes. No
production resource-policy or public exposure claim follows from this private
capability evidence.

**FAIL.** A01 and A02 are closed and the authoritative Ubuntu 24.04 verifier
passed, but A03 is still a required renderer-level semantic-evidence gap. P7.2.8
must not be frozen until that exact, independent observation exists. This is not
a production-readiness claim.

## 17. A03 final closure — renderer-owned pre-JPEG canvas observation

The former A03 gap is now closed with a private verification-only seam. The
private request carries an optional synchronous canvas observer and opaque
context. After the real worker has completed FIT, production resize, validated
blit, and exact completion accounting, but before JPEG encoding, the executor
calls that observer with a `const img_buffer_t`. The canvas remains owned by the
renderer/slab and is valid only for the duration of the callback; no pointer or
allocator ownership is transferred to the test.

The focused test now follows the real path:

```text
encoded JPEG or PNG -> non-owning decoder -> private scheduler task ->
renderer FIT/resize/fill/blit -> renderer-owned pre-JPEG RGB canvas -> observer
```

It independently decodes the deterministic input, builds the expected canvas
with the test-only checked-integer FIT and 16.16 scalar RGB bilinear oracle,
then byte-compares that expected canvas inside the renderer callback. The
oracle does not call production FIT, resize, blit, renderer, scheduler, or
encoder. Fixtures cover JPEG and PNG decode paths, horizontal and vertical
letterboxing, exact aspect, odd residual centering, explicit RGB background,
outside-rectangle preservation, multiple placements, and exact
expected/planned/executed counts. The pre-existing controlled canvas-allocation
failure continues to prove no JPEG ownership transfer or successful artifact on
failed planned work.

Focused evidence:

```text
cmake --build build-p7 --target test_layout_render_private -j2
./build-p7/test_layout_render_private
[layout-render] JPEG/PNG, scheduler, ownership, and rejection cases passed

cmake --build build-p7-asan --target test_layout_render_private -j2
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
  ./build-p7-asan/test_layout_render_private
[layout-render] JPEG/PNG, scheduler, ownership, and rejection cases passed
```

## 18. Final authoritative verification and decision

The complete fresh Ubuntu 24.04 verifier was run with:

```text
IMGENGINE_VERIFY_JOBS=2 IMGENGINE_FUZZ_RUNS=5000 bash scripts/verify_linux.sh
```

It exited `0` and reported `Linux verification passed: /workspace/build-linux-verify`.
It reached format-check, normal CTest/regressions, Rust FFI/supervisor/API,
real-image verification, ASAN/UBSAN CTest/regressions, sandbox, and both
5,000-run fuzz targets. Normal and sanitizer CTest each included
`layout_render_private` and passed. `git diff --check` passed after the final
source and audit changes.

Final re-audit: A01 formatting is closed; A02 has an independent scalar RGB
oracle; A03 now compares the actual private renderer canvas before JPEG
encoding. The observer is private-only, synchronous, const/borrowed, and does
not affect encoder, scheduler, ABI v1, Rust, HTTP, or Nuxt behavior. No
blocking ownership, lifetime, completion, or public-interface contamination was
found. Historical poison-code classifications in section 10 remain unchanged.

Remaining non-blocking gaps are the earlier bounded capability-specific
zero-length/truncated/extreme-dimension fixture expansion and the deliberately
deferred public ABI/Rust/HTTP exposure and production resource policy. They do
not invalidate this private rendering evidence.

**PASS — FREEZE. P7.2.8 FROZEN.** This freezes only the verified private native
capability. It does not claim production readiness, a public layout API, or
permission to proceed to P7.2.9 without a separately approved scope.
