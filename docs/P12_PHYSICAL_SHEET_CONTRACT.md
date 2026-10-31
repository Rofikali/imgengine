# P12 Physical Sheet Rendering Contract

## Status

**Decision record / implementation gate.** This document defines the next product capability on top of P11. It does not claim that the endpoint, crop renderer, frontend, or native ABI extension is implemented.

P12 is intentionally separate from P11. P11 remains a pure, deterministic Rust geometry planner. This contract is based on the existing P8 product contract, P9.3 bounded-output design, the current HTTP adapter, and the public C ABI version map.

## Compatibility decision

The existing `POST /api/v1/layout-render` endpoint is frozen to its P8 meaning:

- `width` and `height` are the dimensions of each individual photo cell;
- its existing grid/canvas calculation remains unchanged;
- its rendering mode remains FIT: preserve the full source image, preserve aspect ratio, do not crop, and do not stretch;
- its existing multipart field allowlist, response, error mapping, and bounded-output policy remain unchanged.

P12 must not silently reinterpret these fields as paper/sheet dimensions and must not change the old endpoint's output.

## New operation

P12 proposes a separately versioned operation:

```text
POST /api/v2/sheet-render
Content-Type: multipart/form-data
```

This route name is a P12 proposal, not an existing route.

### Request fields

| Field | Type | Meaning |
| --- | --- | --- |
| `file` | JPEG or PNG bytes | One source photograph |
| `sheet_width_mm` | finite positive decimal | Physical width of the complete output sheet |
| `sheet_height_mm` | finite positive decimal | Physical height of the complete output sheet |
| `photo_width_mm` | finite positive decimal | Physical width of each photo cell |
| `photo_height_mm` | finite positive decimal | Physical height of each photo cell |
| `dpi` | bounded positive integer | Output raster resolution; default 300 for this MVP |
| `copies` | integer 1..=64 | Exact number of requested photo placements |
| `margin_mm` | finite non-negative decimal | Equal outer margin on all four sides |
| `gap_x_mm` | finite non-negative decimal | Horizontal gap between columns |
| `gap_y_mm` | finite non-negative decimal | Vertical gap between rows |
| `output_format` | literal `jpeg` | The only P12 output format |

Unknown and duplicate multipart fields are rejected. No client field may select a filesystem path, output budget, native resource limit, crop rectangle, or server configuration.

The request is one source image and one output sheet. There is no multi-image input, PDF, PNG output, pagination, background job, durable artifact, or public output URL in this contract.

## Geometry and placement rules

1. Convert each physical dimension to integer pixels using the P11 deterministic half-up rule: `floor(mm * dpi / 25.4 + 0.5)`.
2. Reject a required positive dimension that resolves to zero pixels.
3. Preserve the exact requested sheet canvas after conversion. Do not derive sheet size from the grid.
4. Use P11's maximum-feasible-column policy, row-major ordering, and centered occupied grid within the printable area.
5. Permit a partial final row. Do not add phantom placements.
6. Keep each photo cell at its exact resolved pixel dimensions.
7. The planner must reject if the exact requested copy count cannot fit. It must never reduce copies, shrink cells, change sheet dimensions, or create another page.
8. Use checked arithmetic before allocation or native work.
9. Apply the P12 sheet envelope: at most 64 placements, maximum canvas dimension 4096 px, and maximum canvas area 10,000,000 pixels. This permits A4 at 300 DPI (2480 × 3508 = 8,699,840 pixels). The existing P8 FIT ABI retains its independent 4,000,000-pixel ceiling and must not change. Each cell and every placement must also be within the canvas and non-overlapping.

The P11 planner currently accepts pixel dimensions and returns placements under the limits implemented in P11. The 10,000,000-pixel P12 sheet envelope is a product/ABI design requirement, not yet implemented in the planner or renderer. When implemented, it must be scoped to P12 and must not change the existing P8 FIT native path's 4,000,000-pixel ceiling. P12's API adapter is responsible for parsing and converting physical units before calling the planner.

## Image fitting decision: centered crop-to-fill

The new P12 operation uses centered crop-to-fill:

- preserve the source aspect ratio;
- crop only the excess dimension needed to fill the target photo cell;
- centre the crop deterministically;
- resize the cropped region to the exact cell dimensions;
- never stretch or distort the source;
- never crop the existing P8 FIT endpoint.

P11's `centered_crop` function defines the integer crop geometry and its odd-excess-pixel convention. The native implementation must independently validate all crop coordinates and bounds. A Rust-generated crop rectangle is untrusted input at the C ABI boundary.

### Source dimensions and crop ownership: selected design

The current native render path already decodes the encoded source once into a validated `img_buffer_t` before scheduling the layout task. That decoded buffer carries authoritative width, height, stride, and pixel data into the renderer.

P12 will keep that single-decode path:

- Rust sends the explicit crop mode `CENTERED_FILL` in a new additive native request contract; it does not send source dimensions or a client-derived crop rectangle.
- C computes the crop rectangle from the already-decoded source dimensions and the target cell dimensions, using checked integer cross-products and the same floor/centering convention as P11's `centered_crop`.
- C applies the crop as a validated source view or equivalent bounded operation before resizing to the exact photo-cell dimensions. The implementation must prove that any borrowed view respects source stride and lifetime and is never released as an owned buffer.
- The C tests compare native crop geometry against P11's reference cases, including odd excess pixels, portrait/landscape transitions, and extreme dimensions.
- The Rust helper remains the pure deterministic reference used for planner-level tests and cross-layer parity; it is not treated as a security boundary.

This avoids trusting browser metadata, avoids a second full decode, and keeps hostile-image dimension/pixel validation in the existing native decode path. The ABI design must still specify how the crop mode is represented and reject unknown values.

## Native ABI compatibility

The current symbol-version map already contains `IMGENGINE_1.0`, `IMGENGINE_2.0`, and `IMGENGINE_2.1`. ABI 2.1 adds the bounded JPEG output symbol. The current public ABI-v2 layout request is FIT-oriented and carries canvas dimensions plus placement rectangles; it has no crop rectangle or crop mode.

Therefore:

- do not change the layout request v2 struct in place;
- do not change `imgengine_layout_render_v2` or `imgengine_layout_render_v2_bounded` semantics;
- do not assume that P11's Rust helper is connected to C rendering;
- any native crop capability must be additive, with a new size/versioned request contract and a new symbol-version node after inspection of the ABI compatibility tests and export checker;
- keep existing ABI v1, 2.0, and 2.1 consumers working unchanged.

The exact ABI 2.2 struct and symbol names are deliberately not frozen by this product contract. They require a separate ABI design review covering struct-size evolution, reserved fields, crop rectangle validation, output ownership, error mapping, and symbol-export tests.

## Resource, lifecycle, and security rules

- Keep the existing request byte ceiling, authentication, admission control, deadline, response-abandonment behavior, private per-request workspace, and cleanup lifecycle unless a separately reviewed change proves necessary.
- The JPEG destination budget remains server-owned and bounded. It is not a multipart field and is not the same thing as the canvas-pixel budget.
- A successful response contains a complete decodable JPEG only. No partial output is returned on any failure.
- Rust validates request syntax, physical conversions, placement count, geometry, and policy before native execution.
- C independently validates struct size/version, pointers, source image, crop rectangle, placement count, every rectangle, canvas limits, arithmetic, and output budget before unsafe allocation or processing.
- No user filename, path, image bytes, credential, or raw codec diagnostic is written to logs.
- Temporary image data is removed through the existing lifecycle on success, error, timeout, or response abandonment.

The existing P8 FIT 16 MiB bounded-output default remains unchanged. P12 uses a separate provisional 64 MiB encoded-destination allocation budget so A4 at 300 DPI can pass TurboJPEG's conservative destination-bound check, which exceeds the raw RGB canvas size. Neither value is a complete per-request RSS or aggregate concurrency limit.

## Required test gates

### Contract and API tests

- exact field allowlist; duplicate and unknown fields rejected;
- invalid, missing, non-finite, negative, zero, and out-of-range dimensions rejected;
- mm-to-pixel half-up boundaries and zero-after-rounding cases;
- default DPI behavior and explicit DPI behavior;
- exact sheet canvas retained;
- max-column selection, partial final row, centering, and row-major ordering;
- exact capacity succeeds; one copy beyond capacity fails without truncation;
- old `/api/v1/layout-render` contract regression tests remain unchanged.

### Crop and native-boundary tests

- landscape-to-portrait, portrait-to-landscape, square, equal-aspect, and odd-excess crop cases;
- invalid and out-of-bounds crop rectangles rejected by C;
- no crop or stretching on the old FIT endpoint;
- crop-to-fill output has the exact requested cell dimensions;
- native placement and crop validation is tested independently of Rust;
- output ownership/release occurs exactly once on success and no output is owned on failure;
- bounded-output failure returns no partial JPEG;
- sanitizer, fuzz, ABI export, and installed public-consumer gates pass.

### End-to-end and product tests

- one valid upload returns a complete JPEG at the requested sheet pixel dimensions;
- exact copy count is visible in output and response accounting;
- frontend preview and server geometry agree on placements and crop;
- request cancellation/response abandonment still triggers established cleanup;
- old endpoint behavior is byte/behavior regression-tested where deterministic fixtures permit.

## Delivery sequence

1. Review and approve this contract without changing the old route.
2. Audit the frontend's current request schema and preview implementation.
3. Source-dimension handoff is decided: C reuses the single decoded source and computes crop geometry internally.
4. Freeze the additive ABI 2.2 request, symbol, output ownership, and validation contract in [P12.2 ABI design](P12_2_ABI_2_2_CROP_DESIGN.md).
5. Implement the pure Rust API adapter and tests.
6. Implement the additive C crop capability and independent boundary tests.
7. Integrate the new route through the existing supervisor and lifecycle.
8. Integrate the frontend only after request and preview contracts are stable.
9. Run the existing Docker Ubuntu 24.04 verification gate plus new integration, sanitizer, fuzz, and ABI tests.
10. Review the resulting PR and CI before merge. A green CI run is evidence for the tested revision, not a production-readiness claim.

## Explicit non-goals

P12 does not add PDF/PNG output, multi-page sheets, auto-rotation, multi-image composition, crop anchors, borders, bleed, crop marks, paper presets, persistent storage, database/queue infrastructure, or a new service. These require separate product and resource decisions.
