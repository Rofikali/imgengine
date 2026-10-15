# IMGENGINE Product / Layout / Render Domain Model v1

**Status:** P7.2.0-B architecture and product-domain checkpoint. This document defines vocabulary and a future direction; it does not expand an HTTP, Rust, ABI, C, or frontend capability.

## 1. Executive Summary

IMGENGINE has two different histories that must not be conflated.

- The verified Rust API is intentionally narrow: one JPEG or PNG file in, one completed JPEG out, synchronously and ephemerally.
- Legacy SaaS code and native CLI/internal pipeline documents describe a print-sheet product: physical photo size, an A4 grid, fit/fill, border, bleed, crop marks, and named templates.

Those historical layout concepts are product requirements to recover, not capabilities of `POST /api/v1/render`, `RequestLifecycleService`, or ABI v1. ABI v1 exports only `image.jpeg_encode`; it has no public layout request. The native internal `img_job_t` and CLI are evidence of an implementation history, not authorization to expose their fields through the current Rust endpoint.

This checkpoint establishes explicit terms, separates user intent from derived raster geometry, assigns ownership, and recommends a validated server-side Render Plan before any layout feature is exposed.

| Status | Meaning |
| --- | --- |
| **Current / verified** | Available through the current Rust HTTP contract and Ubuntu Docker evidence. |
| **Historical / internal** | Present in legacy SaaS or native CLI/internal code; not a current Rust API capability. |
| **Target product** | Required domain concept for a future print-layout product. |
| **Future** | Needs product decision, implementation, and capability-specific verification. |

## 2. Current vs Target Capability

### Current / verified

`POST /api/v1/render` accepts exactly one `file` multipart field, accepts JPEG or PNG, and returns a completed JPEG. It is authenticated, synchronous, ephemeral, body/admission bounded, and has no layout, preset, paper, output format, job, polling, or durable-output field. The provisional 1 MiB input limit, queue capacity one, and ten-second post-dequeue deadline are not product layout limits.

### Historical / internal

The legacy `POST /api/generate` job API accepts `cols`, `rows`, `gap`, `padding`, `width`, `height`, `dpi`, `border`, `bleed`, crop-mark controls, and three named presets. Its worker invokes the native CLI. Native internal `img_job_t` likewise contains centimetre photo dimensions, DPI, grid and decoration fields. The current native canvas implementation is A4-oriented.

This is useful evidence, but it is ambiguous: legacy `width`/`height` are centimetres of a photo cell, `padding` is an outer page margin in pixels, and `rows * cols` implicitly becomes the number of placements. It does not model copies independently, paper selection, or a public capability-backed layout ABI.

### Target product and future

The target is a print-layout service for photo studios, passport agents, travel agencies, and similar operators. It should express physical photo size, paper, copy count, fit/crop intent, and a reproducible sheet plan. It must not make passport-compliance claims merely because a named size exists.

A future version may add versioned render-plan input, A5/A6/Letter/custom paper, PDF/PNG output, crop positioning, multiple images, multi-page output, and streaming. None is implemented or implied here.

## 3. Domain Model

| Domain | Definition | Source of truth |
| --- | --- | --- |
| **Input** | Untrusted uploaded encoded image and metadata actually found while decoding. | Decoder/native validation, surfaced through Rust. |
| **Photo specification** | Intended physical size and image-fitting policy for one repeated photo cell. | User/preset intent, normalized and validated by Rust. |
| **Paper specification** | Physical sheet selected by preset or custom dimensions, plus orientation. | Product registry/request, normalized by Rust. |
| **Canvas** | Concrete raster surface calculated from paper dimensions and DPI. | Rust Render Plan; executed by C. |
| **Layout specification** | Copy count, auto/manual grid mode, spacing, margins, and placement order. | User/preset intent, resolved by Rust. |
| **Print specification** | DPI and conversion policy used to rasterize physical intent. | Rust Render Plan. |
| **Decoration** | Border, bleed and crop-mark semantics applied to a cell/sheet. | Product intent and C raster capability. |
| **Output** | Encoded artifact and its dimensions/format. | Derived Render Plan and C encoder. |

`copies` means requested repeated instances of the source image. It is not a synonym for `rows * columns`: a 4 by 5 grid has capacity 20, while a request for 13 copies has 13 occupied cells and seven deliberately empty cells. The first future single-page contract should reject `copies > capacity`; multi-page is a separate product capability.

## 4. Dimension Model

No future API, UI label, or metric should use bare `width` or `height` when one of these terms is intended.

| Term | Example | Origin | Meaning |
| --- | --- | --- | --- |
| `source_width_px`, `source_height_px` | 1200 x 1600 px | Decoded input | Discovered; never accepted from browser form fields. |
| `photo_width_um`, `photo_height_um` | 35,000 x 45,000 um | User/preset | Physical size of one intended photo, not source size. |
| `photo_width_px`, `photo_height_px` | about 413 x 531 px at 300 DPI | Calculated | Rounded once in the Render Plan. |
| `paper_width_um`, `paper_height_um` | 210,000 x 297,000 um for A4 | Preset/custom request | Physical sheet after orientation resolution. |
| `canvas_width_px`, `canvas_height_px` | about 2480 x 3508 px at 300 DPI | Calculated | Raster surface allocated by native execution. |
| `output_width_px`, `output_height_px` | Render-plan canvas dimensions | Derived output | For a future sheet; current re-encode has no sheet plan. |

The current P5 response's dimensions are decoder/encoder output facts, not photo, paper, canvas, or print dimensions.

## 5. Unit Model and Canonical Representation

The UI may accept `mm`, `cm`, or `in`; these are presentation units only. A future request should identify its selected unit, parse decimal input without binary floating-point ambiguity, and normalize physical lengths to integer micrometres (`um`) in Rust:

```text
1 mm = 1,000 um
1 cm = 10,000 um
1 in = 25,400 um
pixels = round(length_um * dpi / 25,400)
```

DPI is a positive integer. Pixel dimensions, offsets, thicknesses, and buffer sizes are derived integers. Conversion must use checked arithmetic and one documented rounding rule at the Rust Render Plan boundary. Do not repeatedly convert mm -> px -> mm.

This is a future-model recommendation. Historical C `img_job_t` stores photo dimensions as `float` centimetres and several print values as pixels. It is not a suitable public wire representation. A future native capability should receive validated concrete raster geometry or an explicitly versioned native plan, rather than untrusted user floats.

## 6. Layout Model

### Modes

- **Automatic:** Rust computes feasible row/column capacity from paper canvas, photo footprint, margins, gap, orientation, and requested copies. The UI presents the result; it does not own rendering arithmetic.
- **Manual:** User supplies rows and columns; Rust verifies that the grid fits and that `copies <= rows * columns`.

For a single page, capacity is checked multiplication:

```text
capacity = rows * columns
occupied_cells = copies
```

Cells use documented row-major order unless a later version adds a named ordering. Empty capacity is background, not implicit extra copies. The footprint calculation includes any decoration that consumes or needs clearance outside a photo cell.

### Spacing, orientation and paper

- `page_margin_*_um`: paper edge to outer layout footprint. Historical `padding` maps here; a future contract should stop using that ambiguous name.
- `cell_gap_x_um`, `cell_gap_y_um`: clear separation between cell footprints. A single `gap` is a UI shorthand only where both axes are equal.
- `cell_padding_*_um`: optional inside-cell clear space, distinct from page margin. It is not a current API feature.
- `border_thickness_um`: visible line inside the defined cell footprint unless a future capability explicitly chooses another rule.

Paper presets are product metadata with physical dimensions and stable IDs. `portrait` and `landscape` resolve the final axes before automatic layout. A custom paper request supplies named physical paper dimensions, never generic `width` and `height`.

The historical C path currently calculates an A4 canvas. A5, A6, Letter, Legal, and custom paper remain future C-capability work.

## 7. Photo, Crop, and Decoration Semantics

- `fit`: preserve all source pixels inside the photo cell; unused cell area is background.
- `fill`: scale to cover the cell and crop excess source area.
- `crop_position`: a future named position (`center`, `top`, `bottom`, `left`, `right`, or normalized focal point) for `fill`. Historical native behavior is centre crop; no current Rust API exposes crop positioning.

| Concept | Meaning | Status |
| --- | --- | --- |
| Border | Visible line around a photo cell. | Historical/internal native support; future Rust capability. |
| Bleed | Extra image/print-safe area for cutting; not generic page margin. | Historical/internal native support; future Rust capability. |
| Crop marks | Cutting guides with defined per-cell or sheet attachment, length, thickness and clearance. | Historical semantics are per-photo; future contract must preserve or version that. |
| Crop-mark offset | Distance from defined cell/bleed edge, not a source-image crop offset. | Historical/internal native support; future Rust capability. |

Each decoration needs an explicit unit. Product input should use physical units where it represents printed distance; Rust derives pixels at plan time. The future contract must say whether it is included in cell footprint or consumes additional canvas clearance.

## 8. Validation and Security Model

Frontend validation improves UX only. Rust must parse, normalize, validate, calculate and limit a request before admission; C retains defence-in-depth checks before allocation and raster work.

```text
authenticated request
  -> schema and finite-value validation
  -> physical-unit normalization
  -> preset resolution / allowed override policy
  -> checked raster and grid calculation
  -> product resource-policy checks
  -> admission
  -> immutable validated Render Plan
  -> C execution and defensive validation
```

| User-controlled value | Required validation | Resource/security concern |
| --- | --- | --- |
| Photo/paper dimensions | Positive finite decimal, allowed range, checked conversion | Huge canvas and overflow. |
| DPI | Integer allowlist/range selected by product policy | Quadratic pixel and memory growth. |
| Rows/columns | Positive bounded integers, checked product | Cell allocation and CPU loop growth. |
| Copies | Positive bounded integer; `copies <= capacity` for one page | Millions of placements or accidental output. |
| Margins/gaps/padding | Non-negative; footprint fits paper | Underflow, overlap and arithmetic overflow. |
| Border/bleed/crop values | Non-negative bounded physical values; geometry consistency | Out-of-canvas drawing and raster cost. |
| Fit/crop/orientation/output | Closed known enum | Invalid dispatch and ambiguous rendering. |

The plan must check all add/multiply operations for pixel counts, stride, RGB/RGBA estimates, cell count, canvas bytes, workspace bytes, and expected output bytes. It must reject rather than wrap, saturate, silently shrink, or allocate optimistically.

Existing native hostile-dimension guards (including dimensions over 16,384 and very large pixel counts) are safety guards, not an adopted product sheet policy. Current P5 has no post-encode output ceiling or workspace byte accounting. This document does **not** choose production numeric limits; those need representative-sheet memory, latency, output-size and capacity evidence.

## 9. Ownership Boundaries

| Concept | Browser / Nuxt | Rust | C core | Calculated | Security limit | Current support |
| --- | --- | --- | --- | --- | --- | --- |
| Source width/height | Display only | Classify/report safely | Decode/validate | Decoder | Native decoded-image guards | Current decode only |
| Photo physical size/unit | Collect future intent | Normalize/validate | Consume geometry | Rust plan | Rust | Historical/internal |
| DPI | Collect future intent | Normalize/validate | Use geometry | Rust plan | Rust | Historical/internal |
| Paper preset/size | Present choices | Resolve/version/validate | Receive canvas geometry | Rust plan | Rust | Historical A4 internal only |
| Orientation | Collect intent | Resolve axes | Render canvas | Rust plan | Rust | Future |
| Rows/columns | Manual UX only | Validate/resolve | Place cells | Rust plan | Rust + C defence | Historical/internal |
| Copies | Collect intent | Validate against capacity | Render occupied cells | Rust plan | Rust | Future |
| Gap/margin/padding | Collect intent | Normalize/validate | Apply geometry | Rust plan | Rust + C defence | Historical/internal (`padding` ambiguous) |
| Border / thickness | Collect intent | Validate footprint | Rasterize | Rust plan | Rust + C defence | Historical/internal |
| Bleed | Collect intent | Validate footprint | Rasterize | Rust plan | Rust + C defence | Historical/internal |
| Crop mode/position | Collect intent | Validate enum | Fit/fill/crop pixels | Rust plan | Rust + C defence | Centre fit/fill historical; position future |
| Crop marks / thickness / offset | Collect intent | Validate geometry | Rasterize | Rust plan | Rust + C defence | Historical/internal |
| Canvas/output dimensions | Display plan only | Produce authoritative plan | Allocate/render/encode | Rust plan | Rust + C defence | P5 has no sheet plan |
| Output format | Offer enabled capability only | Select capability/map response | Encode | Rust plan | Rust + C capability | JPEG current; PNG/PDF future HTTP |

Nuxt owns interaction and presentation, never authoritative rendering math, resource authorization, API credentials, workspace paths, or engine policy. Rust owns request policy, plan construction, resource governance, admission, lifecycle and safe errors. C owns decoder truth, raster execution, layout kernels, encoding and defensive arithmetic/memory checks. The C scheduler remains the execution-plane scheduler.

## 10. Product UX Model

The first future UI should have a narrow **Simple mode**:

```text
Upload -> purpose/preset -> photo size -> paper -> copies -> Generate
```

It should lead with common studio tasks, use reviewed product data, explain the selected physical result, and show server-derived capacity only after the capability exists. A preset must declare whether fields may be overridden; silently mixing it with legacy defaults is unsafe.

**Advanced options** may later expose orientation, auto/manual grid, margins, gaps, fit/fill, border, DPI, bleed and crop marks. They remain hidden until the backend supports a typed Render Plan. No fake canvas preview, progress percentage, PDF selector, or job UI belongs in the current P7 render page.

## 11. Business Considerations

Core near-term value is predictable physical sizing, a small reviewed preset catalogue, correct copy count, paper choice, deterministic fit/fill, and print-ready JPEG output. Those reduce operator error for studios and agents.

Manual grid tuning, crop marks, bleed, border, custom paper, PDF and multi-page output are useful advanced workflows, but each increases validation surface, render cost, support burden and verification scope. Passport preset names are not legal/compliance assertions; country-specific presets need a versioned source, effective date, owner, review process, and customer-facing policy.

The low-cost design remains one bounded, ephemeral process path. No database, distributed queue, durable image storage, Redis, or rate limiter is required merely to introduce a validated single-page layout capability.

## 12. Future API Shape (Conceptual Only)

```text
RenderRequest vNext
  input: file
  photo: { physical_size, fit_mode, crop_position? }
  paper: { preset_id | custom_physical_size, orientation }
  layout: { mode: auto | manual, copies, rows?, columns?, page_margin, cell_gap }
  print: { dpi }
  decoration: { border?, bleed?, crop_marks? }
  output: { format }

Rust validation -> immutable RenderPlan vNext
  { source facts, resolved pixel geometry, occupied cells,
    resource estimates, native-capability version }
  -> C capability-backed execution
```

`RenderPlan` is built once by Rust, contains no client path or credential, and is not re-derived independently in Nuxt and C. Introducing it requires a separate contract, API-version, native-capability and test decision.

## 13. C-Core Implications

Future native work needs a versioned capability beyond ABI-v1 `image.jpeg_encode` to consume a validated sheet plan. At minimum it needs concrete canvas/cell geometry and deterministic placement; fit/fill and crop position; bounded rendering of requested copies rather than unconditional full grid replication; defined decoration footprint/order; paper parameterization beyond A4; format-specific evidence for PNG/PDF; and ABI/ownership review if it crosses the public C boundary.

Do not expose native internal `img_job_t` directly through a future Rust HTTP request. It carries floats and internal layout semantics, while ABI v1 does not expose it. A public capability needs an intentional versioned contract, isolated headers, error mapping, ownership rules, sanitizer and geometry regression evidence.

## 14. Open Questions

1. Which markets and sources define the first reviewed passport/photo presets, and who owns legal/commercial updates?
2. Which paper presets, custom ranges, DPIs and output formats justify MVP after customer and cost evidence?
3. Are decoration dimensions physical values, fixed raster defaults, or both?
4. Does a first layout request support only one source image and one page?
5. What resource budgets follow representative-sheet RSS, CPU, output, workspace, latency and concurrent-admission measurements?
6. Which crop controls beyond centre fit/fill are needed, and how can preview remain visibly non-authoritative?
7. Should response delivery remain completed-buffer JPEG or gain a separately designed bounded streaming/output policy?

## 15. Recommended Next Step

**P7.2.1 — Render Plan Contract and Native Capability Gap Design (planning only).** Define a versioned, single-page `RenderRequest`/`RenderPlan` boundary, choose a small MVP preset and paper set through product evidence, map every plan field to a native capability gap, and establish proposed resource-budget measurements. Do not implement UI controls, change ABI v1, or migrate the C scheduler.

## Evidence and Related Documents

- Current P5/P6 HTTP contract and resource evidence: `docs/api.md` and `docs/API_CONTRACT_PARITY_GATE.md`.
- Historical native layout semantics: `docs/ENGINE_SPEC.md`, native `img_job_t`, CLI and pipeline sources.
- Historical durable SaaS fields/presets: `docs/SAAS_CONTRACT.md` and legacy FastAPI schemas/routes. They are not the Rust API contract.
