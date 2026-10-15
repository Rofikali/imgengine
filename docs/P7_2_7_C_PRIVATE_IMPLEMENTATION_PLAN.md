# P7.2.7 — C-Private Implementation Plan

## 1. Scope

This document freezes the internal implementation path for the first native
`layout.render.v1` vertical slice. It preserves the P7.2.0-B through P7.2.6
terminology and decisions: Rust remains responsible for exact physical plan
construction; C receives concrete geometry and executes FIT-only rendering.

This is an investigation and planning gate only. It makes no source, ABI,
Rust, HTTP, Nuxt, scheduler-algorithm, or product-policy change.

The capability is deliberately narrow:

```text
validated encoded JPEG/PNG -> C decode -> RGB canvas -> explicit RGB background
-> ordered, non-overlapping rectangles -> FIT -> JPEG -> owned result
```

## 2. Source archaeology

| Concern | Existing source and entry point | P7.2.8 disposition |
| --- | --- | --- |
| Decode | `src/api/api_job_decode_input.c`: `img_api_decode_input`; `include/api/io_vtable.h`: `g_io_vtable.decode` | Reuse the existing decoder through a new non-owning C-private wrapper. |
| Scheduler submit/wait | `src/runtime/scheduler_submit.c`: `img_runtime_submit_task`, `img_runtime_wait_task` | Reuse. |
| Worker dispatch | `src/runtime/worker_loop.c` | Add one private task dispatch branch only. |
| Historical render stage | `src/api/api_job_begin.c`: `img_api_begin_job_with_photo`; `src/runtime/job_prepare_render_stage.c` | Do not reuse as an executor; it is `img_job_t`/A4/grid-specific. |
| Resize | `src/pipeline/layout_resize_bilinear.c`: `img_layout_resize_bilinear` | Reuse as the RGB resampling primitive. |
| Existing FIT preparation | `src/pipeline/layout_prepare_fit.c`, `layout_prepare_fit_commit.c` | Reuse only its RGB-cell commit behavior if suitable; do not reuse its float geometry calculation. |
| Canvas allocation | `src/pipeline/canvas_alloc_buffer.c`: `img_canvas_alloc_buffer` | Reuse after a new checked precondition guard. |
| Blit | `src/arch/x86_64/avx2/blit_avx2.c`: `img_blit_avx2` | Reuse only after full validation; its native contract clips. |
| JPEG encode | `src/api/api_job_render_output.c`, `include/api/io_vtable.h`: `g_io_vtable.encode` | Reuse directly via a C-private output wrapper, without historical post-effects. |
| Encoded output release | `src/api/api_encoded.c`: `img_encoded_free` | Reuse as the one release family. |
| Decoded-buffer release | `src/runtime/raw_buffer_release.c`: `img_runtime_release_raw_buffer` | Reuse for decoded source/intermediate slab buffers. |

## 3. Existing decoder path and decoded representation

`img_api_decode_input(ctx, file_data, file_size, photo)` validates its
arguments, calls `g_io_vtable.decode`, then calls `img_api_release_input` and
sets `*file_data = NULL`. `img_api_release_input` in
`src/api/api_job_release_input.c` calls `munmap(file_data, file_size)`.

This is an explicit source-level contradiction with the P7.2.6 premise that
future Rust can borrow encoded HTTP bytes for synchronous C decode: the
historical wrapper consumes an mmap-backed input allocation and must not be
called on a Rust slice. P7.2.8 must add the smallest C-private *non-owning*
decode adapter: validate arguments and call the same `g_io_vtable.decode`, but
never call `img_api_release_input`. This reuses the proven decoder without
creating a decoder, changing ABI v1, or transferring input ownership. The
decoded `img_buffer_t` contains RGB pixel data, width, height, channels, stride,
and an `owner_pool` (`include/core/buffer.h`).

The selected P7.2.8 path therefore decodes before scheduler submission, holds
the resulting slab-owned `img_buffer_t` in C-private request state, and calls
`img_runtime_release_raw_buffer` exactly once after worker completion or a
pre-submit failure. The C-private entry must arrange ownership explicitly: it
must not retain a Rust-owned encoded pointer after the non-owning decode returns.

## 4. Existing resize/FIT, canvas, and placement paths

`img_layout_prepare_fit` currently computes FIT dimensions with floating-point
scaling, clamps dimensions to one pixel, calls `img_layout_resize_bilinear`,
then calls `img_layout_prepare_fit_commit`. The latter allocates an RGB cell,
fills it with the supplied RGB background, and centers the resized image using
integer `(cell - fitted) / 2`; this places an odd residual pixel at the
right/bottom. `img_layout_resize_bilinear` is the selected resampling primitive.
Its scalar fallback in `layout_resize_resample_bilinear.c` uses fixed-point
bilinear interpolation; optimized dispatch may select another implementation.

P7.2.6's exact FIT geometry is not satisfied by the current float calculation.
P7.2.8 must add a C-private checked-integer FIT geometry adapter implementing
the frozen rational rule before calling the existing resize primitive. It may
use `img_layout_prepare_fit_commit` for a one-rectangle RGB intermediate only
after that geometry is supplied. A new private direct background-fill/blit
helper is also acceptable when it uses the same slab ownership model; it must
not create a separate renderer or resampler.

`img_canvas_alloc_buffer` can allocate a concrete RGB canvas but calculates
`pw * ph * 3` internally. It is reusable only after a P7.2.8 guard proves every
product and `stride = width * 3` representable, and proves the required size
fits the selected slab block. `img_canvas_init` is not reusable because it
derives historical A4/job geometry.

`img_layout_grid_place_cells` is not reusable: it derives a historical grid,
continues past invalid positions, and returns a partial placement count.
`img_blit_avx2` is reusable as a hot copy primitive, but its documented
contract silently clips at a canvas boundary. P7.2.8 must validate every
rectangle before rendering and then call it only for wholly in-bounds sources
and destinations. The same validated input can use the scalar blit fallback on
non-AVX2 builds.

## 5. Existing JPEG/output path

`img_api_render_job_output` demonstrates the native JPEG allocation contract:
it zeroes `out`/`out_size`, calls `g_io_vtable.encode`, and on encoder failure
releases any allocation with `img_encoded_free`. `img_encoded_free` calls the
native `free` family. That is the selected output family.

The P7.2.8 C-private executor must call `g_io_vtable.encode` after exact
completion, without `img_apply_job_postfx` or an `img_job_t`. It initializes its
result to empty; on every failure before handoff it calls `img_encoded_free` if
needed and leaves `{data = NULL, size = 0}`. Only a successful result transfers
the exact pointer and size to the future owner, which must release it once via a
native release wrapper. Rust must not call a guessed allocator or raw `free`.

## 6. Scheduler entry selection

**Selected scheduler entry:** `img_runtime_submit_task()` and
`img_runtime_wait_task()` in `src/runtime/scheduler_submit.c`, with execution
dispatch in `src/runtime/worker_loop.c`.

**Why:** `img_api_begin_job_with_photo` already establishes the intended
decode-before-submit, submit-and-wait execution-plane pattern. `img_task_t` in
`include/runtime/task.h` is the actual queued-work representation.

**Required minimum scheduler change:** add `IMG_TASK_KIND_LAYOUT_RENDER`, a
C-private payload pointer, and one `worker_loop.c` switch branch that invokes a
C-private layout-render executor. No queue, worker, work-stealing, admission,
or scheduling algorithm changes are required.

This narrow extension is necessary because the only current render task kind,
`IMG_TASK_KIND_RENDER_STAGE`, carries `img_job_t`, canvas/layout/arena pointers
and invokes `img_runtime_prepare_render_stage`. That executor derives A4/grid
geometry and cannot honor explicit rectangles or the no-partial-success
invariant. The change is a new task capability, not a scheduler redesign.

Before submit, the C-private entry creates request state and deep-copies the
placement list and all execution metadata it needs. The queued task refers only
to that C-owned state and decoded C-owned pixels. The submitter retains the sole
join/wait responsibility and destroys that state only after the worker writes a
terminal status.

## 7. C-private request and result semantics

The internal request should have a private version marker (initial value `1`),
and the following semantic fields:

- decoded source `img_buffer_t` owned by C after synchronous decode;
- canvas `uint32_t width`, `height`, and explicit RGB background;
- owned ordered list of `(x, y, width, height)` placements plus count;
- FIT-only mode (no mode selector exposed for FILL/crop/AUTO_FIT);
- `expected_placements`, required to equal the copied list count; and
- JPEG output configuration limited to the minimum existing encoder inputs.

The private completion result should contain terminal `img_result_t`, expected,
planned, and executed counts, output pointer, and output size. Output fields are
non-null/non-zero only for success. This is C-private versioning, not an ABI v1
or future public contract.

## 8. Rectangle representation and completion invariant

Rectangles are ordered `uint32_t` `(x, y, width, height)`, with upper-left
origin, x increasing rightward, y increasing downward, and half-open coverage
`[x, x + width) x [y, y + height)`. Width and height must be positive.

P7.2.8 validates pointer/count consistency, `x + width` and `y + height` with
checked arithmetic, canvas bounds, and pairwise overlap before allocating a
partial artifact or scheduling. No clipping, shrinking, skipping, reordering,
or partial success is allowed. Existing `img_layout_grid_place_cells` and
`img_blit_avx2` demonstrate why these guards belong in the new adapter.

Success requires, exactly:

```text
expected == planned == executed
```

Check it (1) when validating the copied request, (2) after each successful
placement in the executor, and (3) immediately before encoder invocation and
result handoff. Any mismatch destroys intermediates/canvas and exposes no JPEG.

## 9. Ownership and allocator strategy

| Object | Owner/allocation family | Release point |
| --- | --- | --- |
| Encoded caller bytes | Rust-owned borrowed bytes, consumed synchronously by the new non-owning adapter | No native free or `munmap`; the caller retains ownership. |
| Decoded source/canvas/intermediates | Existing slab pool / `img_buffer_t.owner_pool` | `img_runtime_release_raw_buffer` or canvas release on every terminal path. |
| Copied placements and private request/result state | Checked-size native heap allocation | Submitter after `img_runtime_wait_task`; worker never frees state still observed by submitter. |
| JPEG bytes | Existing encoder allocation | `img_encoded_free` on failure or one future caller handoff release. |

Private metadata uses the ordinary native heap family: checked-size `calloc` for
the private request/result and copied rectangle array, released with `free` in
one private destroy routine. This matches existing scheduler-owned metadata
practice (`src/runtime/queue_create.c` uses `malloc`/`calloc`) and avoids using a
worker scratch arena whose lifetime is not the queued request's lifetime. It is
not an output allocator: JPEG output remains exclusively in the existing
`g_io_vtable.encode`/`img_encoded_free` family.

## 10. FIT reference oracle

The initial geometry oracle is an independent small checked-integer test helper
that implements P7.2.6's frozen rational rule and centering rule. It compares
fitted width, fitted height, and destination offsets exactly.

The pixel oracle is the deterministic fixed-point scalar bilinear behavior in
`src/pipeline/layout_resize_resample_bilinear.c`, not an optimized SIMD path.
P7.2.8 tests must use a tiny test-only RGB reference implementing that inspected
formula (or expose the existing scalar branch only for tests), then compare
pre-JPEG RGB canvas pixels exactly: background, fit margins, placement pixels,
and placement count. Production optimized output must be checked for equivalence
to that reference before any SIMD claim. JPEG evidence should decode the output
and validate dimensions/semantic image behavior; no byte-for-byte JPEG rule is
selected, and any lossy tolerance must be measured and documented rather than
invented.

## 11. Deterministic fixture strategy

Generate tiny deterministic RGB patterns in tests rather than add large binary
assets: e.g. 2x1 and 1x2 primary-color sources for letterboxing, a 2x2 distinct
corner-color source for interpolation/placement, and a generated valid JPEG and
PNG encoding of those patterns. The test table must cover:

1. single placement;
2. multiple ordered placements;
3. fewer copies than a separately planned capacity;
4. exact capacity;
5. requested copies greater than the supplied plan (failure);
6. horizontal letterbox;
7. vertical letterbox;
8. exact aspect match;
9. infeasible/overflow/out-of-bounds/overlap geometry;
10. malformed encoded input; and
11. output ownership/release, including failure cleanup.

Each case declares source dimensions, canvas dimensions, RGB background,
rectangles, expected/planned/executed count, exact pre-JPEG oracle pixels where
practical, and expected result category. JPEG results are decoded for dimension
and semantic validation. The first implementation must support both generated
JPEG and PNG decode fixtures; JPEG is the only output.

## 12. Failure and cleanup matrix

| Failure point | Required terminal behavior |
| --- | --- |
| Invalid pointer/count, rectangle, bounds, overlap, or arithmetic | Reject before scheduling; release private copied state and decoded source; no output. |
| Malformed/unsupported encoded input | Non-owning decoder failure before submit; the caller retains input bytes; no C-owned source or output survives. |
| Canvas/intermediate allocation or FIT failure | Worker reports failure; submitter releases all slab buffers/private state; no output. |
| Scheduler rejection | No worker ownership transfer; submitter destroys request/source/state. |
| Worker/render failure or count mismatch | Destroy canvas/intermediates; no JPEG; submitter performs final request/source cleanup. |
| Encoder failure | Release encoder allocation with `img_encoded_free`; release native buffers/state; no output. |
| Successful handoff | Release all non-output native state; retain only JPEG pointer/size for exactly-one release by future owner. |

P7.2.8 must make task state, terminal status, and request-state ownership
linear: no worker access after wait returns; no submitter destruction before a
submitted task reaches `IMG_TASK_DONE`; no output release by both native and
caller. Sanitizer runs and output-release regression tests are mandatory.

## 13. Native safety checks and deferred policy

Native invariants for P7.2.8 are: non-null and consistent pointer/counts;
encoded-length representability; decoded dimension/channel/stride validity;
checked canvas and stride byte arithmetic; slab-block fit; checked rectangle
ends; positive dimensions; overlap rejection; allocation checks; terminal task
status; encoder pointer/size consistency; and cleanup on all paths.

These are safety invariants, not product limits. Selecting production body,
pixel, output, workspace, queue, deadline, or capacity limits remains deferred.
This plan does not introduce arbitrary thresholds.

## 14. P7.2.8 implementation scope

Expected native files to change:

- `include/runtime/task.h` — one private layout-render task kind/payload;
- `src/runtime/worker_loop.c` — one dispatch branch;
- new C-private layout-render request/executor/validation source and private
  header under the existing native API/runtime/pipeline internal structure;
- build manifest(s) for those new private files; and
- focused native tests and deterministic fixture generator/test helpers.

Existing functions to reuse: `g_io_vtable.decode` through a non-owning private
adapter, `img_runtime_submit_task`, `img_runtime_wait_task`,
`img_layout_resize_bilinear`, optionally
`img_layout_prepare_fit_commit`, `img_canvas_alloc_buffer` after guard,
`img_blit_avx2`/scalar fallback after validation, `g_io_vtable.encode`,
`img_runtime_release_raw_buffer`, and `img_encoded_free`.

Scheduler changes are limited to the proven task-type/dispatch extension. ABI,
Rust, HTTP/API, Nuxt, public headers, C scheduler algorithms, and legacy paths
remain unchanged. Tests add the fixture categories above plus scheduler
rejection, worker failure, exact-count, and allocator-release coverage.

## 15. Remaining blockers and explicit non-goals

There is no architectural blocker to P7.2.8. The historical decoder wrapper's
`munmap` ownership is a resolved implementation constraint: P7.2.8 must use the
documented non-owning wrapper around `g_io_vtable.decode`, never
`img_api_decode_input` directly for Rust-borrowed input. It must verify the
actual encoder's minimal private configuration at the call site and add the
scalar-reference and sanitizer evidence described here before claiming
capability validation.

Non-goals remain FILL, crop, AUTO_FIT, decorations, paper/preset catalogs,
PDF/PNG output, metadata, streaming, asynchronous native jobs, a public native
ABI, Rust FFI, HTTP fields, frontend fields, production resource limits, and
production-readiness claims.

## 16. Gate conclusion

P7.2.8 has one unambiguous implementation route: synchronous non-owning native
decode through the existing decoder vtable,
deep-copy C-private plan state, submit/wait through the existing execution-plane
scheduler with one new task kind, execute strict integer FIT over explicit
rectangles using existing decode/resize/blit/encode primitives, and hand off the
existing encoder-owned JPEG allocation exactly once. No parallel rendering
engine and no scheduler replacement are warranted.
