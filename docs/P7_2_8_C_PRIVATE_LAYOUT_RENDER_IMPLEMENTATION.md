# P7.2.8 — C-Private `layout.render.v1` Implementation

## Status

Implemented as a C-private native vertical slice. It is not exposed through
ABI v1, Rust FFI, HTTP, or Nuxt, and is not a production-readiness claim.

## Implemented path

`img_layout_render_v1` accepts one borrowed encoded JPEG or PNG only for its
synchronous decode boundary. It uses the existing non-owning prepared-decoder
path (`img_api_decode_prepared` and the registered decoder vtable), then retains
only C-owned decoded pixels in copied private task state.

The state is submitted through the retained C scheduler as
`IMG_TASK_KIND_LAYOUT_RENDER`; the worker calls the private executor and the
submitter waits through the existing `img_runtime_wait_task` path. This is a
task-kind/dispatch addition, not a queue, worker, or scheduler-policy change.

The executor validates all ordered half-open `(x, y, width, height)` rectangles
before scheduling: counts, non-zero dimensions, addition overflow, canvas
bounds, and overlap. It computes FIT dimensions using checked integer/rational
arithmetic, fills the entire RGB canvas with the explicit RGB background,
resizes with the existing bilinear primitive, and blits only validated regions.
Success requires `expected == planned == executed` before JPEG encoding.

The existing JPEG encoder and `img_encoded_free` remain the only output
allocation/release family. A failed execution exposes no output; a successful
result can be released once through the C-private result release function.

## Evidence

`test_layout_render_private` uses deterministic generated JPEG and embedded PNG
fixtures. It exercises scheduler execution, JPEG and PNG decode, concrete
canvas output dimensions, exact placement accounting, malformed input,
overlapping/count-mismatched/overflow rectangles, and repeated result release.

The P7.2.7 scalar fixed-point RGB reference and expanded pixel-level oracle
coverage remain required verification work before this private capability can be
called fully validated. JPEG comparisons remain semantic rather than bytewise.

## Deferred

FILL, crop, AUTO_FIT, public ABI changes, Rust/HTTP/Nuxt integration, PDF/PNG
output, production resource ceilings, and production readiness remain deferred.
