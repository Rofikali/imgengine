# P13 Interactive Crop Editor: Contract and Implementation Plan

## Status

**Design proposal; no implementation or release claim.** P13 adds user-controlled crop positioning to the existing physical-sheet workflow. It is intentionally separate from P12's centered-fill behavior.

## 1. Product outcome

A user selects a source photo, sees a crop frame matching the selected photo-cell aspect ratio, and can:

- drag the image to choose the subject position;
- zoom within a bounded range;
- reset to the default centered crop;
- preview the selected crop before rendering;
- generate the physical sheet using that crop consistently for every copy.

The editor is an image-selection aid, not a trust boundary. The server and native C core validate all crop inputs independently.

## 2. Existing behavior that must remain stable

- P8 FIT layout remains unchanged.
- P12 ABI 2.2 sheet rendering remains centered-fill and unchanged.
- Existing `POST /api/v2/sheet-render` behavior remains backward-compatible when crop fields are absent.
- Existing C ABI symbols, request structures, output ownership, and crop geometry must not be modified in place.
- Legacy FastAPI/worker flows remain in place until a separately approved migration milestone.

## 3. Proposed API shape

Add an explicitly versioned crop-aware sheet operation rather than extending the ABI 2.2 request in place.

Proposed native capability: **ABI 2.3**, with a new request version and a new bounded symbol. Names are provisional until header/export review.

The crop-aware request carries a normalized crop rectangle in a fixed-point coordinate space:

| Field | Meaning |
|---|---|
| `x`, `y` | Crop origin, normalized to source width/height |
| `width`, `height` | Crop extent, normalized to source width/height |
| `coordinate_scale` | Fixed constant, proposed 1,000,000 units per full source dimension |
| `crop_mode` | Explicit user crop mode; never silently interpreted as centered-fill |

The request must not carry browser-provided source pixel dimensions, stride, pointers, or file paths. C derives actual source dimensions from its single decode and converts normalized values using checked integer arithmetic.

### Geometry rules

- Require all extents to be non-zero.
- Require `x + width <= coordinate_scale` and `y + height <= coordinate_scale`, using checked arithmetic.
- Require crop aspect ratio to match the target placement aspect ratio within a documented integer tolerance. The preferred implementation is to make the browser crop frame exact-ratio and have C reject mismatched geometry rather than stretch.
- Convert normalized crop bounds to source pixels with deterministic rounding rules documented in the ABI.
- Clamp only rounding artifacts at the final source edge; never silently clamp malicious/out-of-range user input.
- Require at least one source pixel in each dimension.
- Preserve source stride in a borrowed crop view; never treat the view as owned memory.
- Resize the validated crop to the exact target dimensions without distortion.
- Use the same crop for each placement in one sheet request; all placements must have compatible aspect ratios.

### Normative geometry contract (selected)

**Representation: normalized crop rectangle in source-image coordinates**, using unsigned integer fixed-point values with a constant scale of 1,000,000. This is selected over focal-point + zoom because it directly represents the visible crop, is easy to serialize, and avoids divergent browser/C interpretations of zoom semantics.

Fields are `x`, `y`, `width`, and `height`; each is an integer in [0, 1,000,000], and extents must be non-zero. The rectangle is half-open: [x, x + width) × [y, y + height). Require x + width <= scale and y + height <= scale with checked addition. The scale is a protocol constant, not a caller-controlled field.

**Pixel conversion is deterministic:**
1. Decode the source once; use the decoded, orientation-corrected width and height as authoritative.
2. Convert left/top edges with floor: `floor(coord * source_dimension / scale)`.
3. Convert right/bottom edges with ceil: `ceil((coord + extent) * source_dimension / scale)`, using checked 64-bit integer arithmetic.
4. Clamp only right/bottom rounding to the source dimension. Reject if the resulting pixel rectangle is empty or outside the source.
5. Validate the resulting pixel rectangle against the target aspect ratio using an integer cross-product. Permit only the documented one-source-pixel rounding envelope; reject larger mismatch. The implementation must publish the exact inequality and reference vectors in the native header/tests before ABI freeze.
6. Resize the selected pixels to the exact target dimensions with no stretching.

This edge-based conversion preserves the user's selected region instead of shrinking it through truncation. The browser crop frame must use the exact target aspect ratio; the server remains authoritative. No floating-point value crosses the HTTP/FFI boundary.

The same rectangle applies to every placement in a sheet request; placements with incompatible aspect ratios are rejected. Existing centered-fill remains the default when no explicit crop is supplied.


## 4. HTTP contract

Extend the same-origin Nuxt proxy and Rust API with optional crop fields only after the ABI contract is approved.

- Omitted crop fields: preserve current centered-fill behavior.
- Complete crop fields: use the explicit user crop.
- Partial, duplicate, malformed, non-finite, negative, or out-of-range fields: reject with `invalid_request`.
- Browser sends normalized values only; server does not trust them.
- Crop parameters are bounded small integers, not arbitrary floating-point strings.
- API key remains server-side; the browser continues using the same-origin proxy.
- No crop metadata is persisted by the server after the request.

## 5. Frontend behavior

The P13 UI adds a crop step before sheet rendering:

1. Load the selected image into a browser-local preview.
2. Show a fixed-aspect crop frame based on photo width/height.
3. Support pointer and keyboard crop positioning.
4. Support bounded zoom with visible reset.
5. Keep preview state local; do not upload on every drag/zoom event.
6. Submit the final crop parameters only when the user renders.
7. On reset, file replacement, unmount, or failed render, revoke preview object URLs and clear transient state.
8. Respect reduced-motion and keyboard accessibility; provide a non-drag alternative for position controls.

The preview is an approximation for interaction. The server-rendered JPEG remains authoritative and is shown after success.

## 6. Security and resource rules

- Enforce existing upload byte limits before parsing/forwarding.
- C validates request version, reserved fields, normalized bounds, target geometry, output budget, and placement count before allocation/scheduling.
- Rust validates the HTTP representation and constructs a safe FFI request.
- Do not trust client MIME type, filename, EXIF dimensions, or crop coordinates.
- Preserve single-decode behavior.
- Do not store source images, crop previews, or crop coordinates persistently.
- Bound CPU, memory, dimensions, request time, and concurrency using existing admission policy.
- Cancellation of browser fetch is not a guarantee that native processing stops; document this until native cancellation exists.

## 7. Acceptance criteria

### Native / ABI
- New ABI is additive; symbol version and exact export manifest updated.
- Public consumer tests cover valid crop, edge crop, invalid ranges, overflow, aspect mismatch, zero extents, wrong version, reserved fields, output-cap exhaustion, and output ownership.
- C crop geometry is tested against deterministic integer reference vectors.
- Tests cover odd dimensions, 1-pixel boundaries, portrait/landscape, EXIF-oriented inputs, and stride-sensitive borrowed views.
- Existing ABI v1, v2.0, v2.1, and P12 v2.2 consumers remain unchanged and pass.
- ASan/UBSan and bounded fuzz tests cover crop parsing and geometry.

### Rust / HTTP
- Missing fields preserve centered-fill behavior.
- Valid explicit crop returns exact output dimensions and expected/executed counts.
- Partial, malformed, out-of-range and aspect-mismatched crop requests fail closed.
- Tests prove browser-supplied dimensions are not used as native source truth.
- API error responses remain problem-details compliant.

### Frontend
- Drag, keyboard positioning, zoom, reset, and preview work.
- Crop frame aspect ratio matches the selected photo size.
- Server output corresponds to the submitted crop within documented rounding tolerance.
- Object URLs and transient crop state are cleaned up on reset/unmount.
- Accessibility checks cover keyboard-only use and screen-reader labels.

### Release
- Exact-head GitHub Actions pass for native, ABI, Rust, Nuxt, sanitizer, fuzz, and Docker verification.
- Measure representative peak RSS and concurrency in the final container.
- Local Ubuntu 24.04 Docker characterization remains a release gate when the developer environment is available.
- Do not call this production-ready until TLS, secrets, firewall, resource capacity, rollback and canary gates are separately satisfied.

## 8. Implementation sequence

1. Approve the crop representation and rounding contract.
2. Add native geometry helper + unit/property tests, without touching P12 centered-fill.
3. Add additive ABI 2.3 request/symbol + export/version audit.
4. Add safe Rust FFI wrapper and API validation/integration tests.
5. Add Nuxt crop editor with local preview and accessible controls.
6. Add end-to-end tests proving UI crop parameters reach the native renderer correctly.
7. Run the full CI matrix and review the ABI/security diff.
8. Merge in small PRs: native contract, API integration, frontend editor, release verification.

## 9. Business acceptance

P13 should reduce photo-agent rework: users can position the face/subject themselves instead of repeatedly uploading different source images. Track:

- successful render rate;
- crop-adjustment-to-render conversion;
- average retries per completed sheet;
- render latency and resource cost;
- support complaints about face/head positioning or print framing.

Do not add paid crop features or pricing complexity until the workflow is validated with real users.
