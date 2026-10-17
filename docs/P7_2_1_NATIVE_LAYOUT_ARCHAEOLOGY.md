# P7.2.1 Native Layout Archaeology and RenderPlan Design

**Status:** Planning and source archaeology only. No ABI, scheduler, renderer, Rust API, or Nuxt UI change is made by this document.

## 1. Executive Summary

The repository contains a substantial historical native print-sheet pipeline, but it is not the current public Rust capability. The path is centered on internal `img_job_t`, an A4-only `img_canvas_t`, RGB24 resize/placement, and JPEG/PDF file output. ABI v1 instead exposes only the encoded-image-to-JPEG capability used by the verified P5/P6 path.

The investigation confirms reusable native building blocks—A4 raster canvas allocation, fit/fill resize, cell placement, border/bleed/crop-mark post-processing, bounded slab allocation, and JPEG encoding—but also finds semantic and boundary gaps that prohibit direct SaaS exposure:

1. `img_canvas_compute_geometry` silently shrinks requested photo raster dimensions when the selected grid cannot fit A4.
2. `img_layout_grid_place_cells` skips an invalid placement and the caller still returns `IMG_SUCCESS`; `layout->count` can therefore be less than `rows * cols`.
3. The internal layout model has no independent copy count.
4. A4, centimetre floats, DPI-to-pixel rounding, and pixel-valued decoration are embedded in historical implementation details.
5. The native grayscale AVX2 implementation is not semantically equivalent to the scalar grayscale reference; it is not part of the current render-sheet path, but it must not be treated as correct if such a pipeline operation is enabled.

The future boundary must be: Nuxt collects intent; Rust resolves presets, units, geometry, resource budgets and an immutable exact-placement RenderPlan; C executes that validated pixel plan and retains defensive native checks. Neither `img_job_t` nor any current ABI-v1 type is suitable as the SaaS request model.

## 2. Scope / Non-Goals

In scope: historical native layout data flow, unit/geometry semantics, allocation and overflow checks, test evidence, ABI gap, and conceptual RenderRequest/RenderPlan design.

Out of scope: implementation, ABI v1 changes, a new public ABI, scheduler changes, Rust lifecycle changes, Nuxt controls, product-limit selection, commits, pushes, and merges.

## 3. Repository Evidence

| Evidence | Repository location | What it establishes |
| --- | --- | --- |
| Internal job descriptor | `imgengine/include/pipeline/job.h` | `img_job_t` contains cm floats, DPI, rows/cols and pixel decorations. |
| CLI-to-job mapping | `imgengine/src/cmd/imgengine/job_builder.c` | CLI options directly overwrite an internal job descriptor. |
| A4 canvas | `imgengine/include/pipeline/canvas.h`; `src/pipeline/canvas.c` | A4 physical constants are converted to canvas pixels from `job->dpi`. |
| Geometry | `src/pipeline/canvas_geometry.c` | Grid-fit calculation and silent photo-cell shrink policy. |
| Layout | `src/pipeline/layout_grid*.c` | Scale once, allocate one descriptor per grid cell, blit all feasible cells. |
| Decorations | `src/runtime/job_postfx.c`; `src/pipeline/{border,bleed,crop_marks}.c` | Post-processing order is border, bleed, crop marks. |
| Memory | `src/memory/{arena*,slab*}.c`; `include/core/buffer.h` | Arena holds transient metadata/scratch; slab owns pixel blocks. |
| Current ABI | `imgengine/include/imgengine/api/v1/imgengine.h` | ABI v1 exposes encoded JPEG conversion, not a layout job. |
| Current Rust HTTP | `imgengine/rust/imgengine-api/src/lib.rs` | File-only JPEG/PNG-to-JPEG endpoint; no layout request. |
| Layout evidence | `src/tests/test_layout_properties.c`; `tests/regression/geometry_regression.sh` | A4 geometry and visual-difference checks, not full product semantics. |
| SIMD evidence | `src/tests/test_resize_kernel_equivalence.c` | AVX2/scalar resize equivalence test exists; no equivalent grayscale test found. |

## 4. Historical Native Layout Pipeline

The active historical file/CLI route is:

```text
CLI options
  -> img_build_job() in src/cmd/imgengine/job_builder.c
  -> img_job_t
  -> img_api_run_job_impl() in src/api/api_job_run.c
  -> img_api_begin_job() / img_api_begin_job_with_photo()
  -> scheduler render-stage task when an engine scheduler is active
  -> img_runtime_prepare_render_stage()
  -> img_canvas_init() + img_canvas_compute_geometry()
  -> img_layout_grid()
  -> FIT or FILL resize; arena cell descriptors; AVX2/scalar blit
  -> img_runtime_apply_job_postfx()
  -> JPEG buffer encoder or PDF file encoder
  -> canvas/slab and arena cleanup
```

`img_api_begin_job()` decodes a file into an `img_buffer_t`; the RGB24 route in `src/api/api_job_run_rgb24.c` starts with caller-owned raw bytes. Both enter `img_api_begin_job_with_photo()`, which submits an `IMG_TASK_KIND_RENDER_STAGE` to the retained C scheduler if one is active, otherwise invokes `img_api_prepare_render_stage()` directly. Rust does not use this historical job route today.

## 5. Input / Data-Flow Archaeology

| Field | Created | Validation/transformation | Consumer and effect | Invalid behavior observed |
| --- | --- | --- | --- | --- |
| `photo_w_cm`, `photo_h_cm` | CLI parser -> `img_build_job`; defaults/template | `img_cm_to_px(float, dpi)` then possible shrink in `img_canvas_compute_geometry` | Cell raster width/height, resize, grid footprint | No explicit finite/positive/range validation at geometry boundary. |
| `dpi` | CLI parser/default/template | Used in `img_cm_to_px` for A4 and photo dimensions | Canvas dimensions, cell dimensions, PDF physical size | No historical product ceiling in this path. |
| `cols`, `rows` | CLI/default/template | Nonzero check; `rows * cols` checked before arena allocation | One cell descriptor and attempted blit per grid index | No independent copies value. |
| `gap`, `padding` | CLI/default/template | 64-bit gap/padding reservations in geometry | Available area and placement step | Pixel units; semantics differ from physical fields. |
| `border_px`, `bleed_px`, crop fields | CLI/default/template | Little geometry validation; postfx clips drawing to canvas | Alters pixels after placement | Can consume/overlap existing whitespace without preflight clearance. |
| FIT/FILL/background | Job defaults/CLI mode | Float scale calculations; `uint8_t` RGB background | Resize and unused FIT area | Crop position is always centre; not externally modeled. |

For all layout fields, the historical native path is internal behavior, not a supported SaaS request contract.

## 6. Physical Dimension Model

Historical conversion is the inline function in `include/pipeline/canvas.h`:

```text
pixels = (uint32_t)(cm * (float)dpi / 2.54f + 0.5f)
```

`img_canvas_init()` applies it to `IMG_A4_W_CM` (21.0) and `IMG_A4_H_CM` (29.7). `img_canvas_compute_geometry()` applies it separately to requested `job->photo_w_cm` and `job->photo_h_cm`.

```text
requested cm float + DPI
  -> rounded uint32 photo pixels
  -> row bytes = photo_width_px * 3
  -> scaled-photo allocation = row bytes * photo_height_px
  -> A4 canvas bytes = page_width_px * page_height_px * 3
```

### Confirmed silent-size shrink — LYT-001

If a requested photo cell does not fit the A4 grid after padding and gaps, `img_canvas_compute_geometry()` computes `s = min(max_pw / photo_w, max_ph / photo_h)` as a `float`, then replaces both requested cell dimensions with truncated `photo * s`. It subsequently clamps each to its maximum. It returns `IMG_SUCCESS`.

Therefore historical execution can turn a requested physical 35 x 45 mm cell into a smaller raster cell to fit the selected grid, without returning a distinct status or a resolved physical-size report. This preserves aspect ratio approximately, but not the requested physical size. It is a **HIGH product-semantic finding**, not a demonstrated memory-safety defect.

The future contract must choose and name one behavior: reject an infeasible exact-geometry request, or expose an explicit documented auto-fit-to-paper policy and return the resolved geometry. It must never silently choose either.

### Float and conversion observations

The CLI's `img_cli_parse_float()` feeds `float` job fields. The conversion has a conventional `+0.5f` rounding intent but does not establish a validated finite/positive input domain at `img_cm_to_px`; casting negative, NaN, infinity, or out-of-range float results to `uint32_t` is not a future-safe API boundary. The existing property test exercises benign positive values only. Source image dimensions are separate decoder-derived pixel facts and must never be replaced with requested photo dimensions.

## 7. Unit Model

The historical model mixes centimetre `float` photo dimensions, integer DPI, pixel gap/padding/border/bleed/crop settings, and a hard-coded physical A4 size. It has no mm input, no paper request, and no named unit in `img_job_t`.

P7.2.0-B's recommendation remains sufficient conceptually: Rust parses a requested decimal physical length and normalizes it to integer micrometres before any raster calculation.

```text
requested photo/paper/decorative physical length
  -> integer micrometres in Rust
  -> checked pixels = round(um * dpi / 25,400)
  -> immutable integer RenderPlan geometry
  -> C executes and defensively validates that geometry
```

Requested physical photo dimensions, calculated photo raster dimensions, and decoded source dimensions must remain separate because they answer different questions: customer intent, execution footprint, and untrusted input fact respectively.

## 8. Paper / Canvas Model

A4 is hard-coded at `IMG_A4_W_CM` and `IMG_A4_H_CM` in `include/pipeline/canvas.h`, used by `src/pipeline/canvas.c`, and duplicated for a probe in `src/runtime/job_prepare_render_stage.c`. `img_canvas_t` comments and `tests/regression/geometry_regression.sh` also explicitly call the canvas A4. The PDF encoder derives its page point dimensions from the resulting raster buffer and DPI; it is not independently hard-coded to 595 x 842 despite its comment.

The internal geometry routine itself accepts `pw` and `ph`, so its arithmetic is potentially reusable with concrete paper dimensions. But active callers derive those values exclusively from A4. A5/A6/Letter/custom paper are therefore **not current native capabilities**, and A4 must not leak into a future product request model.

Future flow:

```text
product paper preset or custom physical paper dimensions
  -> Rust resolves orientation and concrete micrometres
  -> Rust derives checked canvas pixel dimensions
  -> C receives concrete canvas geometry in a future versioned capability
```

## 9. Grid / Copies Model

Historical `img_job_t` has `rows` and `cols` only. `img_layout_grid_alloc_cells()` allocates exactly `rows * cols` `img_cell_t` descriptors; `img_layout_grid_place_cells()` iterates every grid coordinate; no field or code path represents a requested copy count smaller than capacity. Thus historical execution treats grid capacity as actual placements.

### Confirmed partial-placement-success path — LYT-002

`img_layout_grid_place_cells()` checks each calculated coordinate. On overflow or out-of-canvas bounds it uses `continue`, not an error. It increments `placed` only after a blit. `img_layout_grid()` assigns this value to `layout->count` and returns `IMG_SUCCESS` without checking it against `rows * cols`. Downstream post-processing and encoding operate on only `layout->count` cells.

Under normal geometry, placement is expected to fit; the property test checks geometry, not the actual placement count. Nonetheless the active code admits a partial-success semantic if a later inconsistency reaches this layer. This is a **HIGH correctness/product finding** for SaaS: it can produce fewer photos than the inferred request with successful output.

Future single-page invariant:

```text
requested placements == validated placements == actual placements
```

The future RenderPlan must contain an explicit `placement_count`, a fixed coordinate list or reproducible coordinate rule, and C must fail the operation if it cannot execute every planned placement. Partial rendering requires an explicitly versioned product contract; it must never be accidental.

## 10. FIT / FILL / Crop Semantics

`img_layout_grid_prepare_scaled()` selects `IMG_FIT` or `IMG_FILL`.

- FIT (`src/pipeline/layout_prepare_fit.c`) uses `min(cell_w/source_w, cell_h/source_h)`, floors the scaled dimensions through cast, bilinearly resizes, then centres the result on a full cell filled with `bg_r/g/b`.
- FILL (`src/pipeline/layout_prepare_fill.c`) uses `max(...)`, floors through cast, bilinearly resizes, then crops the centre rectangle in `img_layout_prepare_fill_commit()`.

Both preserve aspect ratio subject to float/cast rounding. There is no user-selectable crop anchor or focal point: FILL is centre crop. Neither behavior is current Rust HTTP functionality. Future APIs need a closed fit/crop enum and must say whether any crop anchor is supported; a browser preview cannot become authoritative rendering math.

## 11. Decoration Model

| Field | Existing meaning/unit | Code path and footprint | Risk / ambiguity |
| --- | --- | --- | --- |
| `border_px` | Pixel thickness, black, inside cell | `img_draw_borders()` after placement; does not grow grid | Thickness greater than cell dimensions is clipped by bounds but has no product preflight. |
| `bleed_px` | Pixel expansion of edge pixels outside each cell | `img_apply_bleed()` after border; draws into existing canvas/gap/padding only | Does not allocate a bleed area; may overlap neighbours or decorations if spacing is insufficient. |
| `crop_mark_px` | Pixel line length outside cell | `img_draw_crop_marks()` after bleed | Clipped at canvas edges; clearance is not preflighted. |
| `crop_thickness` | Pixel line thickness; zero becomes one | Crop-mark draw helpers | Large values increase loops; no explicit product ceiling. |
| `crop_offset_px` | Pixel offset from cell/bleed edge | Crop-mark coordinate arithmetic | Terminology must not be confused with image crop position. |
| `gap`, `padding` | Pixels; inter-cell space and A4 outer start | Geometry and placement | They are not physical units and the current vertical origin is `padding`, while horizontal is centred. |
| `bg_r/g/b` | RGB byte values | FIT background and canvas fill | Safe byte range in struct, but no product color model. |

Postfx order is border -> bleed -> crop marks (`src/runtime/job_postfx.c`). Border is intentionally inside a cell. Bleed and marks draw only within the allocated canvas because their helpers clip coordinates, but clipping is not an assurance that requested print geometry exists. A future plan must resolve decoration clearance before rendering and specify whether decorations are inside/outside the cell footprint.

## 12. Memory / Buffer Model

`img_buffer_t` (`include/core/buffer.h`) records `data`, `owner_pool`, `width`, `height`, `channels`, and `stride`. Layout paths use RGB24 (`channels == 3`) and derive stride as width times three.

- **Arena:** `img_runtime_prepare_render_stage()` creates or resets a 1 MiB arena. It stores layout-cell descriptors and may store resize-axis tables / scratch resize data. `img_arena_alloc_aligned()` checks alignment and remaining capacity. It is not the primary canvas allocation.
- **Slab:** `img_slab_create()` makes fixed-size, aligned blocks. Canvas, intermediate resize buffer, FIT backing cell, and cache canvas obtain one slab block each only when `required <= img_slab_block_size(pool)`. `img_slab_alloc()` returns null when exhausted; recycle returns a valid block to its origin pool.
- **Ownership:** buffers allocated from a slab set `owner_pool`; temporary buffers are recycled only if owned by that pool. RGB24 ingress buffers in `img_api_run_job_rgb24_impl()` have `owner_pool = NULL` and are caller-owned. Encoded JPEG output is TurboJPEG allocation and released via `img_encoded_free()` / `free` in historical internal code; ABI v1 has a distinct `imgengine_output_release` API for its narrow output object.

The arena/slab design is worth preserving as an execution-plane bounded-memory mechanism. It does not replace Rust product admission: slab block configuration and free-block availability do not establish SaaS per-request, aggregate workspace, or output budgets.

## 13. Resource / Overflow Analysis

| Calculation | Existing defense | Gap relevant to future plan |
| --- | --- | --- |
| Decoded width * height | `img_security_validate_request()` uses `uint64_t`; dimension/pixel/compression guards | Product pixel ceiling remains undecided. |
| RGB stride * height | RGB24 ingress uses `uint64_t`; resize validates `new_w * 3`, then `row_bytes * new_h` | Canvas allocator directly computes `(size_t)pw * ph * 3` without an explicit staged overflow check. |
| A4 canvas allocation | Must fit one slab block | DPI enters A4 conversion without a clear product range; `pw * 3` assignment to `uint32_t` is unchecked. |
| rows * cols | Arena allocation uses `size_t` overflow checks | No exact-placement success check after blitting. |
| placement x/y | Uses `size_t`, then skips invalid cells | Skip-and-success is semantically unsafe. |
| resize output | Checks row bytes, total bytes, slab block before allocation | Float scale results are not a public safe request boundary. |
| decoration loops | Canvas coordinate clipping | No preflight policy for overlap/clearance or cost. |
| encoded output | Encoder returns owned JPEG bytes | No output-size ceiling in current HTTP/lifecycle contract. |

Rust must decide whether a prospective plan is allowed and affordable using checked resource estimates before admission. C must continue checking native dimensions, strides, arithmetic, slab capacity and actual execution consistency. Neither layer is sufficient alone.

## 14. SIMD / Correctness Findings

Resize has meaningful evidence: `src/tests/test_resize_kernel_equivalence.c` compares `resize_scalar()` and `img_arch_avx2_resize()` across 128 deterministic randomized shape cases, byte-for-byte, when AVX2 is available. `tests/regression/scalar_equivalence.sh` additionally compares complete optimized and portable CLI outputs for one raw fixture. This is **PARTIALLY PROVEN**, not a universal proof.

The grayscale AVX2 code in `src/arch/x86_64/avx2/color_avx2.c` is materially different from the scalar reference in `src/arch/x86_64/scalar/color_scalar.c`. The scalar path computes BT.601 approximation `R*77 + G*150 + B*29`; the AVX2 bulk loop applies coefficient 77 independently to every byte, producing neither grayscale nor the scalar output for bulk pixels. Its scalar tail is correct. No grayscale scalar/AVX2 equivalence test was found.

This is **MEDIUM (SIMD-001)** because grayscale is not invoked by the audited historical render-sheet pipeline, but it is an invalid correctness premise if that operation is later enabled. Scalar/reference behavior must define the semantics; SIMD needs focused equivalence evidence before exposure.

## 15. Historical Native Capability vs ABI v1

| Surface | What it can do | Public to current Rust path? |
| --- | --- | --- |
| Historical CLI/internal job path | Decode, A4 canvas, grid, FIT/FILL, border, bleed, crop marks, JPEG/PDF file output | No. |
| Internal `img_job_t` and `img_api_run_job*` | Carries layout state and runs native job pipeline | No ABI-v1 contract; not suitable for SaaS input. |
| ABI v1 `imgengine.h` | Engine lifetime, capability query, encoded JPEG/PNG-to-JPEG output and release | Yes; `image.jpeg_encode` only. |
| P5/P6 Rust/Axum | File-only JPEG/PNG-to-JPEG lifecycle, admission and cleanup | Yes; no layout fields. |

The gap is a future versioned native operation capable of executing an already validated exact layout plan. It needs explicit input ownership, errors, output ownership, paper/canvas geometry, exact placement semantics and bounds. It must be designed as a new capability after review; ABI v1 must remain unchanged.

## 16. Proposed RenderRequest Model (Conceptual)

This is a future public API model, not a proposed wire encoding or implemented structure. It deliberately avoids generic `width`, `height`, `size`, `margin`, and `padding` names.

| Group / field | Meaning and unit | Domain / normalization | Owner and implications |
| --- | --- | --- | --- |
| `input.file` | One untrusted encoded source image | Current JPEG/PNG policy; decoded facts are authoritative | HTTP/Rust ingress; byte and decoded-image policy. |
| `preset_id?` | Stable reviewed product preset | Closed registry version; establishes override policy | Rust resolves it; Nuxt only presents it. |
| `photo.physical_width`, `photo.physical_height`, `photo.unit` | Intended one-photo physical geometry | Parse decimal -> integer um; positive bounded product range | Rust; affects cell pixels and cost. |
| `paper.preset_id` or `paper.custom_physical_width/height` | Sheet geometry | Closed preset or normalized um; mutually exclusive form | Rust; product policy and canvas budget. |
| `paper.orientation` | Portrait or landscape | Closed enum; resolves paper axes | Rust; no A4 assumption in request. |
| `layout.mode` | `auto` or `manual` | Closed enum | Rust compiles geometry. |
| `layout.copies` | Exact requested placements | Positive integer; one-page policy requires no more than capacity | Rust; direct CPU/output cost. |
| `layout.rows?`, `layout.columns?` | Manual capacity, not copies | Required only for manual; checked multiplication | Rust; bounds descriptors/loops. |
| `layout.page_margin` and `layout.cell_gap` | Named physical clearances | Normalize um, non-negative | Rust; required for fit calculation. |
| `photo.fit_mode` | `fit` or `fill` | Closed enum | Rust passes resolved mode; C rasterizes. |
| `photo.crop_position?` | Named fill anchor | Only valid for a capability that implements it | Future C capability; no historical arbitrary anchor. |
| `decoration.*?` | Border, bleed and marks with explicitly named physical meanings | Optional but normalized and preflighted | Rust calculates clearance; C draws. |
| `output.format` | Requested enabled artifact type | Closed enabled enum | Rust selects only verified capability. |

Client filenames, paths, source width/height, native strides, canvas allocation sizes, and credentials are not user RenderRequest fields. A preset must either prohibit overrides or list allowed fields; it must never silently combine unknown custom values with template defaults.

## 17. Proposed Immutable RenderPlan Model

Rust creates one plan after authentication, schema validation, preset resolution, unit normalization, checked geometry/resource calculation, and admission policy. It contains concrete execution facts, not ambiguous customer labels:

```text
RenderPlan vNext
  plan_version and required_native_capability
  source_policy: allowed encoded input / decoded source limits
  requested_physical_geometry_um: photo and paper
  resolved_canvas_px: width, height, channels, stride expectation
  resolved_cell_px: width, height
  placement_count and ordered placement rectangles
  fit/crop policy and resolved background
  resolved decoration rectangles / clearances / draw order
  output format and bounded output policy reference
  checked resource estimates: pixels, input, transient, output, workspace
```

It is immutable, has no client path, API key, raw image bytes, or hidden fallback. C receives only a plan that already promises exact placement count and consistent geometry. C must nevertheless reject malformed/inconsistent plan values and fail if an allocation or draw cannot complete.

## 18. Ownership Boundaries

| Responsibility | Nuxt | Rust | C |
| --- | --- | --- | --- |
| UX and friendly pre-validation | Yes | No | No |
| Authoritative schema/security validation | No | Yes | Defensive native checks |
| Authentication and admission | No | Yes | No |
| Unit normalization and preset resolution | No | Yes | No |
| Product resource policy / plan compilation | No | Yes | Defensive feasibility only |
| Workspace lifecycle / safe error mapping | No | Yes | No |
| Decode facts and pixel execution | No | No | Yes |
| Native arithmetic, slab/arena ownership | No | No | Yes |
| Layout placement / decoration / encoding | No | Orchestration | Yes |
| C scheduler | No | No | Yes |

Rust remains control plane. C remains the execution plane and scheduler. This design creates no second lifecycle or queue system.

## 19. Failure / Rejection Semantics

| Class | Meaning | Future response behavior |
| --- | --- | --- |
| Invalid user request | Malformed units, invalid enum, impossible manual geometry, copies above capacity | Stable 4xx validation error before admission. |
| Unsupported capability | Valid concept not enabled, such as PDF/paper/crop anchor without verified capability | Stable capability/validation rejection; never silent fallback. |
| Resource limit | Plan exceeds adopted input/pixel/canvas/memory/output/disk policy | `resource_limit` before admission where possible. |
| Overload | Valid plan cannot enter bounded admission | Existing distinct `429 overloaded`. |
| Native execution failure | Plan admitted but decoder/allocation/encoder cannot complete | Safe mapped execution failure; cleanup required. |
| Internal bug/invariant breach | Planned count/geometry is inconsistent or an unexpected state occurs | Safe internal failure; diagnostics redacted. |

Semantic invariants: exact physical geometry is either honored or explicitly rejected; requested placements equal plan placements equal completed placements; no current output format/preset is implied by a legacy implementation; source dimensions are decoder facts.

## 20. Security / Abuse Model

Every future plan field can amplify CPU, memory or output. Huge physical sizes and DPI amplify canvas pixels; rows/columns/copies amplify descriptors and blits; margins/gaps/decoration can invalidate or enlarge geometry; output format and quality affect response size; hostile images retain current decoder risks; and a large accepted plan can hold an admission slot longer. Rust needs measured product limits for all of these. C must retain checked arithmetic, decoder guards, slab-capacity checks and clipped defensive drawing, but clipping must not substitute for plan validation.

Known current safety evidence includes decoded dimension/pixel/compression guards in `src/security/input_validator.c`, checked resize allocation in `src/pipeline/layout_resize_validate_alloc.c`, checked cell-array allocation, and slab exhaustion behavior. Gaps include direct staged overflow checking in `img_canvas_alloc_buffer()`, explicit DPI/float geometry validation, exact placement enforcement, output-size policy, workspace-byte policy, and product-scale capacity measurements.

## 21. SaaS Resource / Cost Implications

| Variable | Cost / abuse implication | Future measurement needed |
| --- | --- | --- |
| DPI and paper size | Canvas pixels grow roughly with DPI squared; memory and encode time grow with area | Peak RSS, latency, encoded bytes by representative paper/DPI. |
| Photo size/grid/copies | More placements increase blits, postfx work and output entropy; rows*cols also uses metadata arena | CPU, cell metadata, actual versus requested placements. |
| Fit/FILL and source aspect ratio | Resize/crop intermediate footprint and CPU vary | Worst-case intermediate allocation and latency. |
| Decorations | Bleed and crop marks add pixel loops and require whitespace | Cost/overlap and deterministic geometry fixtures. |
| Output format/quality | Drives response memory, bandwidth and temporary files | Output-size distribution and output ceiling policy. |

No capacity, profitability, or production limit is claimed from this source review.

## 22. Evidence Matrix

| Behavior | Status | Evidence / limitation |
| --- | --- | --- |
| Current Rust JPEG/PNG -> JPEG capability | PROVEN | P5/P6 Ubuntu Docker evidence; not a layout operation. |
| Historical A4 geometry remains within canvas for benign generated cases | PARTIALLY PROVEN | `test_layout_properties.c` runs 1,000 deterministic A4 cases. |
| Requested physical size stays exact | HISTORICAL ONLY — contradicted | Source inspection proves silent shrink in `img_canvas_compute_geometry()`. No explicit contract test. |
| FIT versus FILL visually differs | PARTIALLY PROVEN | Geometry regression compares outputs; no exhaustive crop/rounding oracle. |
| Border, bleed, crop marks alter output | PARTIALLY PROVEN | Geometry regression uses visual differences. Exact dimensions/clearance unproven. |
| Exact copy count separate from capacity | UNTESTED / unsupported | No `copies` field exists. |
| Partial placement cannot succeed | CONTRADICTED | Source inspection finds `continue` + `IMG_SUCCESS`; no regression proves rejection. |
| A4-only coupling | PROVEN | Active canvas and runtime callers use A4 constants. |
| Other paper sizes | UNTESTED | No active caller resolves non-A4 paper. |
| Resize scalar/AVX2 equality | PARTIALLY PROVEN | 128 kernel cases plus one full CLI optimized/portable fixture. |
| Grayscale scalar/AVX2 equality | CONTRADICTED by source | AVX2 bulk algorithm differs; no equivalence test. |
| Native input dimension guards | PROVEN for covered cases | Safety tests, fuzz/sanitizer evidence documented elsewhere. |
| Layout canvas/output resource budgets | UNKNOWN | Current block feasibility is not SaaS policy; no selected product budgets. |
| Future layout through ABI v1 | UNSUPPORTED | Header exposes no layout capability. |

## 23. Findings / Severity

### LYT-001

- **Severity:** HIGH
- **File / symbol:** `imgengine/src/pipeline/canvas_geometry.c`, `img_canvas_compute_geometry`
- **Observed behavior:** Oversized requested photo cells are proportionally reduced to fit A4 grid capacity and return success.
- **Why it matters:** A customer can receive a different physical photo size without an explicit auto-fit request or resolved-geometry response.
- **Evidence:** Direct source trace of the `photo_w > max_pw || photo_h > max_ph` scale-and-clamp branch; benign property test does not assert exact requested size.
- **Recommended architectural response:** Future Rust plan must reject infeasible exact geometry or require a named auto-fit policy; report resolved dimensions where auto-fit exists.

### LYT-002

- **Severity:** HIGH
- **File / symbol:** `imgengine/src/pipeline/layout_grid_place_cells.c`, `img_layout_grid_place_cells`; `src/pipeline/layout_grid.c`
- **Observed behavior:** Individual placement bounds failures are skipped and the layout returns success with a smaller `layout->count`.
- **Why it matters:** Successful output can contain fewer photos than the implied grid request.
- **Evidence:** `continue` branches in placement loop; no equality check against allocated `rows * cols` in caller.
- **Recommended architectural response:** Plan exact placement count and make C fail any missing planned placement; add direct regression tests.

### LYT-003

- **Severity:** MEDIUM
- **File / symbol:** `imgengine/include/pipeline/canvas.h`, `img_cm_to_px`; `src/pipeline/canvas.c`
- **Observed behavior:** Public-looking historical job fields use `float` centimetres and arithmetic/casts with no SaaS-grade finite/range contract.
- **Why it matters:** Precision, conversion, exceptional float, and product-unit ambiguity cannot safely become HTTP semantics.
- **Evidence:** `img_job_t` fields and conversion implementation.
- **Recommended architectural response:** Normalize explicit request units to checked integer micrometres in Rust; pass concrete raster plan values to a future capability.

### LYT-004

- **Severity:** MEDIUM
- **File / symbol:** `imgengine/src/pipeline/canvas.c`, `src/runtime/job_prepare_render_stage.c`
- **Observed behavior:** Active canvas construction is A4-coupled in two locations.
- **Why it matters:** Exposing A4 as a generic paper model would make later paper support a breaking rewrite.
- **Evidence:** Both callers use `IMG_A4_W_CM` and `IMG_A4_H_CM`.
- **Recommended architectural response:** Rust resolves paper to concrete geometry; a future native capability consumes that geometry without commercial paper naming.

### SIMD-001

- **Severity:** MEDIUM
- **File / symbol:** `imgengine/src/arch/x86_64/avx2/color_avx2.c`, `img_arch_grayscale_avx2`
- **Observed behavior:** AVX2 bulk grayscale multiplies every interleaved channel byte by 77, unlike scalar BT.601 luma.
- **Why it matters:** If grayscale is enabled, optimized output diverges from the scalar correctness reference.
- **Evidence:** Direct source comparison with `src/arch/x86_64/scalar/color_scalar.c`; no grayscale equivalence test found.
- **Recommended architectural response:** Keep outside current layout scope; before enabling grayscale, define scalar semantics and add equivalence tests before any optimization fix.

### LYT-005

- **Severity:** LOW
- **File / symbol:** `imgengine/src/pipeline/canvas_alloc_buffer.c`, `img_canvas_alloc_buffer`
- **Observed behavior:** Canvas byte calculation is a single `(size_t)pw * ph * 3` expression before slab comparison.
- **Why it matters:** A future parameterized canvas must make staged checked arithmetic explicit, not rely on historical type widths/configuration.
- **Evidence:** Direct source inspection; no demonstrated overflow under current accepted configuration.
- **Recommended architectural response:** Future C capability must retain/add staged checked multiplication and Rust preflight estimates.

### INFO-001

- **Severity:** INFO
- **File / symbol:** `imgengine/include/imgengine/api/v1/imgengine.h`
- **Observed behavior:** ABI v1 has no layout request/plan; only encoded JPEG conversion is exposed.
- **Why it matters:** Prevents accidental leakage of internal job structs into Rust HTTP.
- **Evidence:** Public symbol/header inspection.
- **Recommended architectural response:** Preserve ABI v1; design a separate versioned capability only after this gate is approved.

## 24. Open Questions

1. Is initial layout MVP exact-size-only, or does it include an explicit auto-fit policy?
2. Which reviewed paper/preset catalogue and owner are commercially justified first?
3. Do initial decorations use physical units, raster defaults, or a restricted preset-only policy?
4. Is a one-source, one-page, JPEG-only layout MVP sufficient?
5. What numerical product limits follow actual sheet memory, output and capacity measurement?
6. Does a plan encode placement rectangles, or a deterministic grid rule with C validation of every generated rectangle?
7. What native capability/ABI versioning and output ownership shape is least disruptive after detailed design?

## 25. Recommended Next Gate

**P7.2.2 — Exact Layout Semantics and Evidence Plan (design + test plan only).** Resolve exact-size versus auto-fit semantics; select one small product preset/paper scope; define placement-count and decoration-clearance invariants; specify RenderPlan-to-native-capability requirements; and write a test matrix for rejection, exact geometry, exact placements, A4/non-A4 capability boundaries, FIT/FILL, output validation, memory, scalar/SIMD, sanitizer and Ubuntu Docker evidence. Do not implement the capability until that gate is approved.
