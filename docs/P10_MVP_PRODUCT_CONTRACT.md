# P10 — IMGENGINE MVP 1.0 Product Contract

**Status:** Product contract draft for review  
**Scope:** First usable browser-based custom photo-sheet generator

> P8 defines the first implemented FIT-only API slice. P9.3 PR #9 is merged and adds a bounded JPEG destination allocation for that existing path. This P10 contract defines the intended customer-facing MVP and includes crop-to-fill behavior that is not yet implemented by the P8 FIT-only path. This document is a target contract, not evidence that the complete MVP exists or is production-ready.

## 1. Product statement

IMGENGINE MVP 1.0 lets a customer turn one JPEG or PNG photograph into one custom-dimension, print-ready JPEG photo sheet.

The customer controls the printed photo size, sheet size, number of copies, margins, gaps, and print DPI. The browser shows a reviewable preview. The customer explicitly downloads the completed sheet.

The product is deliberately a single-purpose tool. It is not a general-purpose image editor, template marketplace, document-management system, or asynchronous job platform.

## 2. Frozen MVP decisions

### 2.1 Explicitly confirmed by the product owner

These three decisions are fixed and must not be changed by implementation convenience.

| Decision | Contract |
|---|---|
| Copy overflow | **One sheet only.** Never create another sheet, silently reduce copies, or silently omit copies. Reject an infeasible request and report maximum capacity. |
| Measurement unit | **Millimetres (mm) only** in customer-facing dimension controls. Pixels are an internal rendering unit, not a customer measurement mode. |
| Aspect ratio | **Crop-to-fill.** Preserve proportions, crop excess image area, and never stretch or distort the source. |

### 2.2 Proposed MVP defaults to review before implementation

These choices keep the first release narrow and align with the current API capability. They are explicit proposals, not additional decisions the product owner has already confirmed.

| Proposed default | Contract |
|---|---|
| Source images | Exactly one JPEG or PNG per render. |
| Sheet dimensions | Customer-entered custom width and height. Do not force A4/A5 or a fixed paper catalogue. |
| Output | One complete JPEG sheet download. Do not advertise PDF or PNG output in MVP 1.0. |
| Crop anchor | Deterministic centred crop; no manual crop handles or crop-anchor control in the first release. |
| Grid | Regular row-major grid, automatically calculated from the physical sheet capacity; no freeform placement. |
| DPI | Explicit positive DPI in the request; proposed editable UI default is 300 DPI. |
| Persistence | Uploads and generated output are ephemeral. No permanent server-side image or output storage. |

A user chooses a custom sheet ratio by entering its width and height. There is no automatic paper-size substitution.

## 3. Customer workflow

1. Open the IMGENGINE generator.
2. Select one JPEG or PNG image.
3. Enter the printed width and height of one photo in mm.
4. Enter the custom sheet width and height in mm.
5. Enter the required number of copies.
6. Set horizontal/vertical gap and outer margin in mm.
7. Select print DPI. The UI may start at **300 DPI as an editable default**; the API must receive an explicit positive DPI. This is a product default, not a legal or printer guarantee.
8. Review the calculated grid, capacity, and crop preview.
9. Confirm and generate the sheet.
10. Download the resulting JPEG.

The customer must see the important dimensions and copy count before generation. A preview is a review aid, not a security or correctness boundary; the server independently validates and computes the authoritative plan.

## 4. Input contract

### 4.1 Image

- Accept one JPEG or PNG image.
- Treat filename extension and browser MIME type as hints only. Validate actual content and decode the image on the trusted processing side.
- Reject malformed, unsupported, truncated, or unsafe images with a safe user-facing error.
- Never use a customer filename as a filesystem path.
- Do not accept SVG, animated formats, multi-frame formats, archives, or arbitrary URLs in MVP 1.0.

### 4.2 Dimensions and layout fields

All customer-entered physical dimensions are millimetres.

| Field | Meaning | Validation |
|---|---|---|
| Photo width | Printed width of one photo cell | Positive finite value |
| Photo height | Printed height of one photo cell | Positive finite value |
| Sheet width | Full output sheet width | Positive finite value |
| Sheet height | Full output sheet height | Positive finite value |
| Copies | Number of photo cells requested | Positive bounded integer |
| Outer margin | Minimum clear distance from sheet edge to grid | Finite, non-negative |
| Horizontal gap | Horizontal spacing between adjacent cells | Finite, non-negative |
| Vertical gap | Vertical spacing between adjacent rows | Finite, non-negative |
| DPI | Pixels per inch for raster output | Positive bounded integer |

Reject NaN, infinity, negative values, malformed numeric strings, values outside configured limits, and dimensions that round to zero pixels. Apply checked arithmetic before allocation or native execution.

MVP 1.0 does not expose arbitrary pixel dimensions, user-controlled output paths, custom scripts, or user-supplied server resource budgets.

## 5. Measurement and raster mathematics

The UI and API speak mm. The renderer works in integer pixels.

For each physical length d_mm and selected DPI:

    d_px = floor((d_mm * DPI / 25.4) + 0.5)

This is deterministic half-up rounding. Apply it consistently to photo dimensions, sheet dimensions, margins, and gaps. All later geometry uses checked integer arithmetic. Never use unchecked floating-point arithmetic for allocation sizes or buffer offsets.

Example at 300 DPI:

- 35 mm becomes approximately 413 px.
- 45 mm becomes approximately 531 px.

The exact rounded pixel dimensions are authoritative for raster placement. The UI should show the chosen mm dimensions and DPI so the customer understands the print-size assumption.

## 6. Single-sheet grid and capacity contract

The first release uses a regular, non-rotated grid. The customer chooses photo width/height and sheet width/height; the system calculates the grid. The customer does not manually position individual photos.

Let:

- S_w, S_h: sheet width and height in pixels
- P_w, P_h: photo cell width and height in pixels
- M: outer margin in pixels
- G_x, G_y: horizontal and vertical gaps in pixels

The maximum number of columns and rows that physically fit are:

    C_max = floor((S_w - 2*M + G_x) / (P_w + G_x))
    R_max = floor((S_h - 2*M + G_y) / (P_h + G_y))
    Capacity = C_max * R_max

All numerator, denominator, multiplication, and conversion operations must be checked. If either maximum is zero, the layout cannot fit even one photo.

MVP 1.0 selects the maximum feasible column count, then computes:

    Rows_used = ceil(Copies / C_max)

The grid is row-major: left-to-right, then top-to-bottom. The final row may be incomplete; unused cells are left blank and are not rendered. The occupied grid is centred inside the printable area while respecting the minimum outer margin. Photo cells are not rotated automatically.

The output canvas is the complete customer-requested sheet size, not a smaller canvas cropped around the photos:

    CanvasWidth = S_w
    CanvasHeight = S_h

The occupied grid dimensions are:

    GridWidth = Columns_used*P_w + (Columns_used-1)*G_x
    GridHeight = Rows_used*P_h + (Rows_used-1)*G_y

The implementation must prove that the grid plus its centred offset remains within the sheet and preserves at least the requested outer margin.

### 6.1 Copy overflow behavior

If Copies is greater than Capacity, the server rejects the request before native rendering. It must not:

- truncate the requested copy count;
- render fewer copies and report success;
- create a second sheet;
- scale photos down automatically;
- reduce margins or gaps automatically.

The response must use a stable, documented layout-capacity error and include the calculated maximum copy count as safe customer-facing data. The UI should explain which dimensions or spacing values the customer can change. This physical-capacity error is a validation result, not an infrastructure 413 resource-limit error.

## 7. Crop-to-fill rendering contract

For each cell:

1. Decode the source and use actual decoded dimensions.
2. Compare source aspect ratio with target cell aspect ratio.
3. Select the largest source rectangle with the target aspect ratio.
4. Centre that crop deterministically in the source.
5. Resize the cropped rectangle to the exact target pixel dimensions.
6. Place the result into the planned cell.

The operation must preserve source proportions. It must never stretch one axis independently. It must never silently use FIT/contain behavior.

The preview must communicate that crop-to-fill removes some source area. MVP 1.0 does not expose crop anchors or manual crop handles. If source and target aspect ratios already match, no geometric crop is needed.

Crop calculations must use overflow-safe integer/rational comparisons where possible. Define odd-pixel rounding deterministically and cover portrait, landscape, square, extreme-aspect-ratio, and one-pixel boundary cases with fixtures.

## 8. Output contract

- Return one complete JPEG image representing the exact requested sheet dimensions in pixels.
- Use a documented JPEG quality setting controlled by the server; do not accept arbitrary codec flags from the client.
- Return the correct image/jpeg content type and a safe generated download filename.
- Never return partial output as a successful result.
- Do not expose native diagnostics, filesystem paths, source filenames, API keys, or internal plan structures.
- Do not advertise PDF, PNG, multi-page output, or multiple-sheet output in MVP 1.0.

A successful result must satisfy all of the following:

- decoded output width and height equal the resolved sheet pixel dimensions;
- exactly the requested number of photo placements executed;
- every placement is inside the sheet and non-overlapping;
- every photo is crop-to-fill with no distortion;
- all required gaps and margins are respected;
- the returned JPEG is independently decodable.

## 9. Security, privacy, and reliability contract

- Treat every upload and every client field as untrusted.
- Validate file content, actual decoded dimensions, pixel count, and all geometry at the server/native boundaries.
- The browser preview and browser-side calculations are never authoritative.
- Keep API credentials in server-only runtime configuration; never expose them to browser JavaScript.
- Use bounded request admission and server-owned resource budgets. Never let a client choose its own memory or output budget.
- Use a private, randomly named, per-request temporary workspace with restrictive permissions and server-generated filenames.
- Remove source, intermediate, and output files on success, failure, timeout, and abandonment; retain no permanent image artifact or durable output URL.
- Logs and metrics may contain redacted operational facts, but never image bytes, secrets, or arbitrary paths.
- A timeout or disconnected browser must not be described as proof that native work was cancelled. Admission and cleanup remain held until the native operation actually finishes.
- Resource limits must be based on measured whole-request behavior. A JPEG destination cap alone is not a complete memory bound.

## 10. MVP scope exclusions

The following are explicitly out of scope for MVP 1.0:

- multiple source images on one sheet;
- multiple sheets or automatic overflow pagination;
- A4/A5 presets or country-specific passport compliance claims;
- manual crop anchors, freeform crop handles, rotation, or per-cell editing;
- borders, bleed, crop marks, background selection, layers, text, shapes, and general design-editor features;
- PDF, PNG, WebP, or other output formats;
- user accounts, saved projects, cloud galleries, permanent image storage, and shareable output URLs;
- billing, subscriptions, credits, and public pricing promises;
- durable queues, Redis, PostgreSQL, object storage, or microservices without a separately approved requirement.

These exclusions protect the first release from becoming a broad editor before the core upload → layout → preview → download workflow is correct.

## 11. Existing technical limits versus product promises

P8 documents provisional integration controls including a 1 MiB request/input ceiling, one active worker plus a bounded queue, a 10-second execution deadline, a 4,000,000-pixel canvas ceiling, and a 64-copy/64-column limit. P9.3 PR #9 is merged and adds an additional provisional 16 MiB conservative FIT-layout JPEG destination allocation.

These are implementation guardrails, not yet validated customer capacity or production promises. The merged 16 MiB P9.3 cap applies to the existing FIT-only native layout path. It does not prove a whole-request memory bound and does not automatically establish safe limits for the future crop-to-fill path. Reassess limits with representative crop workloads, decoded-source memory, intermediate buffers, Rust copies, response buffering, concurrency, and actual target-VPS measurements before release.

Do not increase limits merely to make a demo pass.

## 12. Acceptance criteria

MVP 1.0 is not complete until the following are demonstrated end-to-end.

### Product behavior

- [ ] One JPEG upload succeeds.
- [ ] One PNG upload succeeds.
- [ ] Invalid, truncated, and unsupported input is rejected safely.
- [ ] Customer-entered photo and sheet dimensions are interpreted as mm.
- [ ] DPI conversion is deterministic and matches the documented rounding rule.
- [ ] Custom sheet width/height are honoured; no fixed paper size is substituted.
- [ ] The layout solver returns a deterministic regular grid and exact capacity.
- [ ] A request above capacity is rejected with the correct maximum-copy count.
- [ ] No request silently loses copies or creates another sheet.
- [ ] Crop-to-fill is centred, deterministic, and never distorts the source.
- [ ] Preview communicates crop and layout before final generation.
- [ ] Downloaded JPEG dimensions exactly match the custom sheet dimensions.
- [ ] Output contains exactly the requested copies.

### Engineering and security

- [ ] Unit/property tests cover mm-to-pixel rounding and integer boundaries.
- [ ] Geometry tests cover zero-fit, exact-fit, one-pixel-overflow, partial final row, and large-but-valid values.
- [ ] Crop tests cover portrait, landscape, square, extreme ratios, odd dimensions, and no-crop exact-ratio cases.
- [ ] Native tests independently validate the crop result and placement bounds.
- [ ] Rust validation rejects invalid requests before native execution.
- [ ] The native boundary independently enforces its own dimensions, counts, and memory limits.
- [ ] ABI ownership and failure-atomicity tests pass under ASAN/UBSAN/leak detection.
- [ ] Real JPEG and PNG requests pass through the actual HTTP endpoint.
- [ ] Failure, timeout, disconnect, and success all leave no request workspace behind after work has terminated.
- [ ] Frontend tests, typecheck, production build, and browser workflow smoke tests pass.
- [ ] The authoritative Ubuntu 24.04 verifier passes on the integrated revision.
- [ ] Resource and performance measurements are recorded for representative source sizes and crop ratios.
- [ ] Actual-VPS testing establishes safe concurrency, peak memory, latency, and cost per render before production claims.

## 13. Release gates

| Gate | Required outcome |
|---|---|
| P10 — Product contract | Scope, customer workflow, units, overflow, crop, and acceptance criteria reviewed and frozen |
| P11 — Geometry and crop correctness | Pure layout planner and crop math implemented with property/boundary tests |
| P12 — End-to-end workflow | Upload, server validation, preview, render, download, and cleanup work together |
| P13 — Security/resource boundary | Whole-request memory, concurrency, temporary storage, timeout, and abuse limits are measured and enforced |
| P14 — Production candidate | Reproducible Ubuntu verification, deployment/rollback/runbook, observability, and actual-VPS evidence |
| P15 — Controlled launch | Limited users, monitored cost/reliability, support process, and explicit go/no-go review |

A gate closes only on recorded evidence. A passing unit test alone does not close an end-to-end, security, deployment, or business gate.

## 14. Business measurement

Before pricing or scaling claims, measure the economics of a completed render:

- CPU time and wall-clock latency per render;
- peak memory and temporary-storage use per render;
- failure, timeout, overload, and cleanup-failure rates;
- safe concurrent renders on the target VPS;
- infrastructure cost per successful render;
- expected support and payment costs;
- customer willingness to pay and repeat usage.

Do not equate the number of copies on a sheet with the number of billable renders unless a later pricing decision explicitly says so. Do not promise a price or margin before these costs and customer demand are measured.

## 15. Change control

Any change to the frozen decisions in Section 2 requires an explicit product decision and an update to this contract, acceptance tests, API contract, UI copy, and release evidence. Implementation convenience is not sufficient reason to change customer-visible behavior.
