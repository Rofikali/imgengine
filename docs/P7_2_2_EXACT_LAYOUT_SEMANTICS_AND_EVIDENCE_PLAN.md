# P7.2.2 — Exact Layout Semantics and Evidence Plan

## 1. Executive Summary

This is a design and evidence gate, not an implementation, API change, or ABI change. It turns P7.2.1 archaeological findings into an implementation-independent contract for a future layout capability.

The recommendation is conservative: requested physical photo geometry is either planned exactly, subject only to a disclosed deterministic physical-to-raster conversion, or rejected. A successful render produces exactly the requested placements. It never silently shrinks a photo, reduces copies, skips a cell, changes paper/DPI/FIT mode, omits a decoration, or substitutes an output capability.

This does not claim layout support in the current P5/P6 endpoint. Current POST /api/v1/render remains file-only JPEG/PNG-to-JPEG; ABI v1 remains unchanged and has no layout operation.

## 2. Scope

Scope is a future one-source, one-page layout capability. It does not select product limits, a paper catalogue, output formats, public wire schema, or a new ABI.

| Label | Meaning |
| --- | --- |
| **PROVEN BY CODE INSPECTION** | Directly established by cited current source. |
| **PROVEN BY TEST** | Established by a named current or future required test. |
| **OBSERVED HISTORICAL BEHAVIOR** | Current internal behavior, not a future public contract. |
| **RECOMMENDED FUTURE SEMANTIC** | Design direction required before implementation. |
| **UNDECIDED** | Product/capability decision not settled by evidence. |

## 3. Source Evidence

This plan uses [P7.2.0-B](P7_2_0_B_PRODUCT_LAYOUT_DOMAIN.md) and [P7.2.1](P7_2_1_NATIVE_LAYOUT_ARCHAEOLOGY.md) as source documents and reconfirms their material findings.

| Finding | Exact evidence | Classification |
| --- | --- | --- |
| LYT-001 | imgengine/src/pipeline/canvas_geometry.c, img_canvas_compute_geometry(), converts photo_w_cm/photo_h_cm, then proportionally scales them to maximum cell dimensions and returns success. | PROVEN BY CODE INSPECTION / OBSERVED HISTORICAL BEHAVIOR |
| LYT-002 | imgengine/src/pipeline/layout_grid_place_cells.c, img_layout_grid_place_cells(), uses continue for invalid coordinates/bounds. imgengine/src/pipeline/layout_grid.c, img_layout_grid(), stores the smaller count and returns IMG_SUCCESS. | PROVEN BY CODE INSPECTION / OBSERVED HISTORICAL BEHAVIOR |
| A4/float/pixels | include/pipeline/canvas.h defines IMG_A4_W_CM, IMG_A4_H_CM, and float img_cm_to_px; include/pipeline/job.h stores float centimetres plus pixel decoration fields. | PROVEN BY CODE INSPECTION |
| FIT/FILL | layout_prepare_fit.c uses the smaller scale and backgrounds remaining cell area. layout_prepare_fill.c uses the larger scale and crops a scaled image. | PROVEN BY CODE INSPECTION; exact rounding/crop oracle not proven |
| Existing layout tests | src/tests/test_layout_properties.c covers 1,000 benign A4 cases; tests/regression/geometry_regression.sh observes FIT/FILL/decorations differ. Neither proves exact size/count. | PROVEN BY TEST, limited |
| Current public boundary | include/imgengine/api/v1/imgengine.h exposes encoded-image-to-JPEG only; rust/imgengine-api/src/lib.rs exposes file-only /api/v1/render. | PROVEN BY CODE INSPECTION |
| SIMD-001 | AVX2 grayscale bulk behavior in src/arch/x86_64/avx2/color_avx2.c differs from scalar BT.601 in src/arch/x86_64/scalar/color_scalar.c; it is outside the audited sheet-layout path. | PROVEN BY CODE INSPECTION |

The retained architecture remains:

    Nuxt (advisory UX)
      -> Rust validation, plan compilation, admission, lifecycle
      -> future safe layout capability
      -> C scheduler and C execution plane

## 4. Semantic Decision Matrix

| Topic | Decision | Evidence status |
| --- | --- | --- |
| Physical size | Initial capability is EXACT: preserve requested geometry after one stated conversion; reject infeasible layout. | RECOMMENDED FUTURE SEMANTIC |
| Auto-fit | Excluded initially; a separately specified opt-in policy only. | RECOMMENDED FUTURE SEMANTIC |
| Copies | Exact positive requested count, distinct from capacity. | RECOMMENDED FUTURE SEMANTIC |
| Ordering | Row-major, top-to-bottom then left-to-right; deterministic plan contract. | RECOMMENDED FUTURE SEMANTIC |
| Partial placement | No successful partial result: requested = planned = executed. | RECOMMENDED FUTURE SEMANTIC |
| Units | Physical values normalize to integer micrometres; raster to integer pixels. | RECOMMENDED FUTURE SEMANTIC |
| Paper | Rust resolves paper identity to concrete dimensions; C receives no commercial paper name. | RECOMMENDED FUTURE SEMANTIC |
| Output | JPEG is the only current verified output; no future format selected here. | PROVEN CURRENT / UNDECIDED FUTURE |
| Limits | Overflow/feasibility mandatory; product ceilings require evidence. | RECOMMENDED / values UNDECIDED |

## 5. Physical Photo Size Semantics

35 × 45 mm means an intended rectangle of 35,000 um × 45,000 um at the plan's declared integer DPI. At 300 DPI, the conversion in section 12 resolves it to 413 × 531 pixels. It is neither “up to” that size nor a hint for the renderer to choose another size.

The plan records requested physical geometry and resolved pixels. Raster quantisation is disclosed, not scaling: layout feasibility must not alter photo_width_px or photo_height_px. Its associated rational extent is:

    effective_um(px, dpi) = px * 25,400 / dpi

It can differ by at most half a raster pixel's physical extent. A later output contract must additionally define DPI metadata; pixel dimensions do not prove physical print calibration.

- **Case A — fits:** plan/render succeeds with requested geometry and deterministic target pixels.
- **Case B — does not fit:** reject before admission as impossible layout. Do not shrink photos, reduce copies, change paper, alter margins/gaps, or lower DPI.

This resolves LYT-001 for a future capability.

## 6. Auto-Fit Semantics

AUTO_FIT is **FUTURE / UNDECIDED** and excluded from the initial capability. If introduced, it must be an explicit closed request policy that reports requested and resolved geometry. It needs separate decisions on what it may change (recommended: only explicitly permitted target geometry), what is immutable (copies, paper, DPI, margins, gaps, decorations, FIT/FILL, output), deterministic objective/tie-breakers, and response confirmation. Without those decisions, infeasible exact layout rejects.

## 7. Copies vs Capacity

For an explicit grid:

    capacity = rows * columns
    1 <= copies <= capacity

All arithmetic is checked before allocation, loops, or admission. Empty cells are permitted if copies < capacity; they show only defined canvas/background/decorations. copies = 0 rejects: it is not a meaningful one-source layout render. copies > capacity rejects before admission.

Placements are the first copies cells in row-major order, starting at the top-left. This order is public plan determinism. A future explicit placement list need not include rows/columns, but must provide ordered non-overlapping rectangles; it does not overload “capacity”.

## 8. Placement Invariants

The successful-render invariant is:

    requested_copies == plan.placement_count == native.completed_placement_count

There is no partial-success result initially. Planning errors reject before admission. Native bounds, allocation, decode, or draw errors are execution failures with no success artifact and mandatory cleanup. This resolves LYT-002: C must fail, never skip a planned placement.

## 9. Grid Geometry

## 10. Coordinate System

Resolved geometry uses pixel-edge coordinates: origin (0,0) is upper-left; +x is right; +y is down. Rectangle (x,y,width,height) is half-open [x,x+width) × [y,y+height). It is valid only if dimensions are positive, additions are checked, and its far edges are no greater than the canvas.

Initial explicit-grid semantics use four named page margins and two inter-cell gaps, resolved to pixels:

    required_w = margin_left + columns * photo_w + (columns - 1) * gap_x + margin_right
    required_h = margin_top  + rows    * photo_h + (rows    - 1) * gap_y + margin_bottom

    cell_x(row, col) = margin_left + col * (photo_w + gap_x)
    cell_y(row, col) = margin_top  + row * (photo_h + gap_y)

All operations use checked arithmetic. Feasibility requires required_w <= canvas_w and required_h <= canvas_h. Extra canvas space remains after declared right/bottom margins under this initial rule; it is not redistributed or used to resize cells. Residual-space alignment is **UNDECIDED** and must be explicit, never inherited from historical asymmetric A4 centering.

## 11. Units

Future cross-boundary names include domain: photo_width_um, paper_height_um, gap_x_um, border_width_um, canvas_width_px, placement_x_px. Unqualified width, height, gap, margin, padding, and size are forbidden at a public/cross-language boundary.

| Concept | Canonical unit | Rule |
| --- | --- | --- |
| Photo/paper/margins/gaps | integer um | Positive dimensions; non-negative clearances. Decimal UI input normalizes exactly or rejects. |
| DPI | positive integer pixels/inch | Zero and fractional values reject. |
| Canvas/cells/coordinates | integer px | Never floats. |
| Bytes/stride | named bytes | Checked separately from pixels. |

## 12. Physical → Raster Conversion

For non-negative physical_um and positive integer dpi, use deterministic nearest-pixel, half-up conversion:

    px_from_um(physical_um, dpi) = floor((physical_um * dpi + 12,700) / 25,400)

The numerator and subsequent values are checked. A required positive dimension that converts to zero pixels rejects. This is the future replacement for historical float-centimetre conversion; it does not modify historical code here.

## 13. DPI Semantics

Mathematical DPI validity is positive integer plus representability. The SaaS maximum DPI is **UNDECIDED** and must follow resource evidence, not an incidental native constant.

## 14. Paper / Canvas

Paper identity, physical paper dimensions, and raster canvas are distinct. Rust resolves an approved preset or custom physical size (and any future orientation) into paper_width_um/paper_height_um, then canvas pixels. C receives concrete geometry rather than A4/A5/A6. The initial preset catalogue, custom paper, and orientation policy are **UNDECIDED**.

## 15. Margins / Gaps / Padding

A page **margin** is canvas-edge-to-grid clearance; a **gap** is inter-placement clearance; **cell padding** is a future inner-cell inset and is not in the initial model. Historical padding must not become an ambiguous public name.

## 16. Decoration Semantics

Historical post-processing order is border, bleed, crop marks in src/runtime/job_postfx.c. Its fields are pixels and current tests only prove visual differences. The initial capability should exclude decorations unless each is explicitly specified and preflighted.

| Decoration | Required semantic before exposure |
| --- | --- |
| Border | Define inside-cell geometry (recommended), unit, width bound, colour and order. |
| Bleed | Define physical purpose and reserved outset; no reliance on clipping/overlap. |
| Crop marks | Define referenced edge, physical length/thickness/offset, clearance and overlap policy. |
| Background | Define colour model and empty-cell treatment if configurable. |

Decoration outsets belong in feasibility/resource calculations. A required out-of-canvas pixel or illegal overlap rejects; defensive clipping is not successful rendering.

## 17. FIT / FILL / Crop

Source dimensions are decoder facts; they never redefine the requested target photo rectangle. For source (sw,sh) and target (tw,th):

- **FIT:** uniform scale min(tw/sw, th/sh), preserve aspect ratio, background the remaining target cell.
- **FILL:** uniform scale max(tw/sw, th/sh), preserve aspect ratio, crop overflow to target cell.

Historical code supports these broad facts. The future contract must choose integer resize rounding and crop anchor, then prove both with reference tests. The historical path appears centred; crop position beyond a specified centre default is **UNDECIDED**. FIT/FILL changes only cell pixels, never physical geometry, copies, or feasibility.

## 18. Source Image vs Photo Geometry

The decoded source is an untrusted input fact used by FIT/FILL; the requested photo geometry is the independent, immutable output target described in section 5. A source image may determine crop pixels, but it cannot alter cell size or physical photo semantics.

## 19. Resource Model

## 20. Resource Estimation

Before bounded admission Rust must use checked arithmetic for decoded-source facts, canvas pixels/bytes, cell pixels, placement count/descriptor bytes, resize/intermediate bytes, decoration work/outsets, estimated encoded bytes, workspace/transient bytes, and estimated resize/blit/postfx/encode work.

Canvas/descriptor facts can be exact for the chosen representation. JPEG size, CPU time, allocator overhead, compression scratch, and future streaming/disk usage are estimates unless independently bounded. Estimates are not safety guarantees.

## 21. Output Resource Model

Current native input validation/slab checks are defense-in-depth, not product layout policy. P5/P6 bounds for current JPEG conversion do not establish layout canvas/output/disk/capacity limits. Current verified output is JPEG only. PDF, PNG output, streaming, output byte ceiling, temporary files, and production budgets are **UNDECIDED**.

## 22. Failure Semantics

| Condition | Boundary/class | Outcome |
| --- | --- | --- |
| Invalid authentication | Rust security | Existing fail-closed rejection. |
| Invalid file/unsupported JPEG/PNG | Rust/native input/capability | Existing safe 4xx; no fallback. |
| Invalid unit/DPI/enum/zero copies/grid | Rust plan validation | Reject before admission. |
| Impossible geometry/copies > capacity | Rust geometry validation | Reject before admission; no shrink/reduce. |
| Resource policy failure | Rust resource validation | Reject before admission with future stable resource code. |
| Queue full | Existing admission | 429 overloaded; unchanged. |
| Deadline | Existing lifecycle | Existing timeout/cleanup; no native cancellation claim. |
| Native bounds/allocation/decode/encode failure | C execution | Safe failure; no success artifact; cleanup. |
| Internal invariant failure | Rust/C internal | Safe redacted failure; cleanup. |

The future layout endpoint's exact HTTP error tokens remain **UNDECIDED** until a separately versioned API contract exists. It must not leak keys, raw bodies, filenames, paths, image bytes, or native diagnostics.

## 23. No-Silent-Fallback Invariants

Unless an explicit closed policy was requested and recorded in the immutable plan, the renderer must not shrink a photo; reduce/skip copies; change paper, orientation, DPI, margins/gaps, alignment, FIT/FILL, crop anchor, decoration, background, preset, or output; substitute a capability; or treat clipped/partial drawing as success. C plan-invariant mismatch is failure, not correction.

## 24. Test Matrix

No tests are added in this design gate. A later implementation gate must run these in authoritative Ubuntu 24.04 Docker alongside native, ABI/FFI, P5/P6 lifecycle, security, sanitizer, fuzz, and API gates.

| Area | Required examples | Required assertion |
| --- | --- | --- |
| Physical size | 35×45 mm at 300 DPI; exact fit; half-pixel boundary; too-small paper; zero/extreme | Exact resolved pixels/disclosed extent or pre-admission rejection; never scaling. |
| Copies | 1, less/equal capacity, capacity+1, zero, multiplication extreme | Exact count or rejection; deterministic empties. |
| Grid | 1×1, 1×N, N×1, normal, maximum, invalid/overflow | All rectangles in bounds, non-overlap, no wrap. |
| Paper | A4 reference; future A5/A6/custom once enabled; smaller-than-cell | Rust resolves concrete paper; unsupported rejects. |
| Margins/gaps/DPI | zero/legal/impossible; zero/fractional/overflow/boundary DPI | Formula and conversion exactly match plan. |
| FIT/FILL | matched/wide/tall/extreme aspect/crop boundaries | Reference geometry and pixels. |
| Decorations | disabled/valid/excessive/overlap/boundary | Explicit clearance/order or rejection. |
| Fault injection | invalid placement, allocation/draw/native error | No successful partial artifact; cleanup. |
| Resources | large paper/DPI/copies/source and combinations | No wrap; controlled rejection/failure. |
| End-to-end | real JPEG and PNG through future capability | Exact geometry/count and P5/P6 deadline, drain, disconnect cleanup evidence. |

## 25. Property / Invariant Tests

Use deterministic property tests against a small independent reference geometry implementation:

    copies <= capacity
    plan.placements.len() == requested_copies
    native.completed_placement_count == requested_copies
    all placements are inside canvas and do not overlap
    cell pixels == px_from_um(requested_um, dpi)
    same normalized request + source facts => same immutable plan
    all arithmetic representable; resource estimates never wrap

Invalid generated cases must reject at the documented boundary; crash, wrap, clipping, or a smaller output is never acceptance.

## 26. Golden / Reference Tests

Machine-checkable golden fixtures must assert canvas dimensions, ordered rectangles, capacity/count, FIT/FILL resize/crop rectangles, and decoration geometry. Pixel goldens are required for enabled FIT/FILL/decorations because geometry alone cannot prove sampling/crop/order. Screenshots are supplementary; fixtures must be project-owned or documented/licensed.

## 27. Rust Contract

**Rust** authoritatively authenticates, validates schema, resolves presets, normalizes units, converts physical values, calculates geometry/resources, builds an immutable plan, admits bounded work, owns workspace lifecycle, and safely maps errors. It never trusts Nuxt, user dimensions, filenames, or native defaults.

## 28. C Contract

**C** remains scheduler/execution plane. It receives validated concrete raster geometry, ordered placements, fit/crop/decoration instructions, and output execution parameters. It still validates dimensions, stride, arithmetic, allocation, plan consistency, bounds, ownership, and exact count; it fails an unexecutable placement rather than correcting/skipping it.

## 29. Nuxt Contract

**Nuxt** may format input, offer UX validation/preview/presets, and present errors. It is never the authoritative validator and no layout UI is created here.

## 30. ABI Evolution Requirements

ABI v1 remains unchanged. Before a separate capability/version is proposed, evidence must cover capability discovery/version negotiation; fixed-width struct size/version rules; input/output ownership and all error cleanup; stable/redacted errors; threading/engine/scheduler interaction; exact-count/resource accounting; P6-compatible no-mid-operation-cancellation semantics; ABI/export/FFI/sanitizer and real-image tests. Internal img_job_t is not a public candidate because it embeds float centimetres, A4/template assumptions, and ambiguous pixel fields.

## 31. Security Test Plan

Treat every geometry field/source image as untrusted. Test huge DPI/paper/photo/rows/columns/copies/gaps/margins/decorations, pathological source dimensions, output amplification, invalid enums and number parsing, preset conflicts, unsupported formats, FFI malformed plans, allocation faults, and combinations such as large DPI + paper + copies. Assert pre-admission rejection where possible, no wrap/unsafe allocation/client path/secrets logging, cleanup after admitted failure, and healthy later admission. Threshold values remain measured product decisions.

## 32. SIMD-001 Scope

SIMD-001 is outside the layout path and this gate. Before grayscale is exposed, define scalar reference semantics and prove scalar/AVX2 equivalence across dimensions, alignment, and tails. This gate neither fixes nor masks it.

## 33. Evidence Classification

The classification vocabulary in section 2 applies to every decision above. In particular, LYT-001 and LYT-002 are code-inspected historical facts; exact rejection, exact count, integer units, and no silent fallbacks are recommended future semantics; paper catalogue, decorations, AUTO_FIT and resource ceiling values remain undecided.

## 34. Open Questions

Open questions requiring explicit later decisions:

1. Need for an opt-in AUTO_FIT product policy.
2. Preset/custom paper/orientation catalogue.
3. Decoration subset, units, clearance and exact visuals.
4. Crop-position model beyond centre.
5. One-source/one-page boundary versus multi-source/page work.
6. Output formats, JPEG DPI metadata, streaming and byte limits.
7. Measured layout budgets for input, pixels, memory, workspace, output, concurrency, queue, deadline.
8. Explicit rectangle-list versus deterministic-grid plan representation.
9. Native capability/version, ownership and error design.
10. Residual-space alignment options.

## 35. Recommended Next Gate

**P7.2.3 — RenderPlan Contract (design only).** Freeze a minimal exact-layout request/plan vocabulary from these semantics, retain the open product choices explicitly, and define Rust-to-native capability inputs without modifying ABI v1. P7.2.4 can then design the native capability; only P7.2.5 may implement it.

## Gate Result

This document defines recommended future semantics, not verified layout behavior. The repository proves historical A4/float/pixel-field behavior and LYT-001/LYT-002; it does not prove a layout SaaS capability, physical-print calibration, production limits, AUTO_FIT, a paper catalogue, or layout capacity.
