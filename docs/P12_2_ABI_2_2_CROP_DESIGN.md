# P12.2 Native ABI 2.2 Design: Centered-Fill Sheet Rendering

## Status

**Design decision / implementation gate.** This document freezes the proposed public C boundary for P12's physical-sheet renderer. It does not claim that the ABI, native renderer, Rust wrapper, or HTTP route has been implemented.

This is an additive ABI 2.2 capability. Existing ABI v1, layout ABI 2.0, and bounded-layout ABI 2.1 remain unchanged.

## 1. Goals and non-goals

### Goals

- Add one native operation for rendering an exact physical-sheet plan with centered crop-to-fill.
- Keep the current single native decode. C derives crop geometry from the decoded source buffer's authoritative dimensions and stride.
- Keep the existing placement rectangle contract and output ownership contract where reuse is ABI-safe.
- Require a caller-supplied, server-owned encoded-output byte budget.
- Validate every untrusted field independently in C before allocation or scheduler submission.
- Preserve existing public symbols and their behavior byte-for-byte at the ABI contract level.

### Non-goals

- Do not change `imgengine_layout_request_v2_t`, `imgengine_layout_render_v2`, or `imgengine_layout_render_v2_bounded`.
- Do not change the P8 FIT renderer or the meaning of `POST /api/v1/layout-render`.
- Do not add a caller-supplied crop rectangle, source dimensions, stride, raw pixel pointer, filesystem path, output path, or output budget field.
- Do not add an unbounded sheet-render symbol.
- Do not add PDF/PNG output, pagination, auto-rotation, or multiple source images.

## 2. Public header and symbol

Add a self-contained public header:

```text
include/imgengine/api/v2/sheet.h
```

It includes the stable v2 layout header for the already-published rectangle and output types:

```c
#include "imgengine/api/v2/layout.h"
```

The new symbol is:

```c
IMGENGINE_API imgengine_status_t
imgengine_sheet_render_v1_bounded(
    imgengine_engine_t *engine,
    const imgengine_sheet_request_v1_t *request,
    size_t max_output_bytes,
    imgengine_layout_output_v2_t *output);
```

The symbol is assigned to the new linker version node `IMGENGINE_2.2`, which inherits `IMGENGINE_2.1`. Do not change the library SONAME or ABI major version. Do not export any helper or private renderer symbol.

The name deliberately distinguishes this new crop-to-fill sheet operation from the existing generic FIT layout operation. The `v1` suffix versions this new request contract; the ELF symbol version identifies its ABI introduction as 2.2.

## 3. Request data contract

The proposed public declaration is:

```c
#define IMGENGINE_SHEET_RENDER_V1_VERSION 1u

#define IMGENGINE_SHEET_CROP_CENTERED_FILL 1u

typedef struct imgengine_sheet_request_v1 {
    uint32_t struct_size;
    uint32_t version;

    const uint8_t *input;
    size_t input_size;

    uint32_t canvas_width;
    uint32_t canvas_height;

    uint8_t background_r;
    uint8_t background_g;
    uint8_t background_b;
    uint8_t reserved0;

    uint32_t crop_mode;
    uint32_t reserved1[4];

    const imgengine_layout_rect_v2_t *placements;
    uint32_t placement_count;
    uint32_t expected_count;
} imgengine_sheet_request_v1_t;
```

### Field meaning

| Field | Contract |
| --- | --- |
| `struct_size` | Caller-provided byte size of the request structure. Must be at least the v1 structure size. |
| `version` | Must equal `IMGENGINE_SHEET_RENDER_V1_VERSION`. |
| `input`, `input_size` | Borrowed encoded JPEG/PNG bytes, valid until the synchronous function returns. |
| `canvas_width`, `canvas_height` | Exact output canvas in pixels, already converted and checked by Rust; C independently enforces native limits. |
| `background_r/g/b` | Canvas background color. |
| `reserved0`, `reserved1` | Must all be zero. |
| `crop_mode` | Must equal `IMGENGINE_SHEET_CROP_CENTERED_FILL`. Zero and unknown values are rejected. |
| `placements` | Borrowed array of stable ABI-v2 rectangles. C copies the bounded array before scheduling work. |
| `placement_count` | Number of requested rectangles; must be 1 through 64. |
| `expected_count` | Must equal `placement_count`; no partial success or silent copy reduction. |

The rectangle type is reused because its four `uint32_t` fields and meaning (canvas-relative x/y/width/height) are already part of the public ABI 2.0 contract. The new request structure is separate; this does not mutate or extend the existing v2 layout request.

### Structure evolution

- Reject `struct_size < sizeof(imgengine_sheet_request_v1_t)`.
- Accept a larger structure size and read only the v1 prefix. Future fields require a new request version and explicit compatibility rules; an older library must never guess their meaning.
- Require the v1 `version` value exactly.
- Require every reserved field defined in the v1 prefix to be zero.
- The caller must pass a valid, readable structure and valid readable input/placement memory for the duration of the call. C can validate nullness, lengths, counts, arithmetic, and semantic bounds; it cannot make an arbitrary invalid process pointer safe to dereference.

## 4. Output and ownership

Reuse `imgengine_layout_output_v2_t` and `imgengine_layout_output_release`. The output has the same meaning for both layout and sheet rendering: an owned complete JPEG plus canvas dimensions and expected/planned/executed placement counts.

Rules:

1. `output` must be non-null. If non-null, zero it before validating the engine or request.
2. The only successful result owns one complete JPEG buffer.
3. Caller releases success exactly once with `imgengine_layout_output_release`.
4. On every failure, output remains all-zero and owns no memory.
5. If encoding or any postcondition fails, C releases internal output before returning.
6. `expected_count`, `planned_count`, and `executed_count` must all agree with the requested placement count on success.
7. No partial JPEG is returned.

The output release function remains the existing ABI 2.0 symbol because it already owns and releases the exact output representation reused here. Do not add a duplicate release function.

## 5. Bounded-output policy

The output cap is a required argument to `imgengine_sheet_render_v1_bounded`, not a request field:

- Reject zero.
- Reject values greater than `INT_MAX`, matching the existing bounded encoder boundary.
- Enforce the cap during JPEG encoding, not only after an unbounded allocation.
- On cap exhaustion, return `IMGENGINE_STATUS_RESOURCE_LIMIT` with an all-zero output.
- The cap is server-owned. It must never be populated from multipart input or browser state.

The current provisional 16 MiB cap is an encoded destination limit only. It does not prove an aggregate request-memory, decoder-memory, or process-RSS bound.

## 6. Native validation order

The public wrapper should follow this order. No caller-controlled allocation or scheduler submission may occur before the corresponding checks pass.

1. If output is null, return invalid argument. Otherwise clear output immediately.
2. Validate engine and request pointers.
3. Validate request `struct_size` and `version`.
4. Validate all defined reserved fields are zero.
5. Validate non-null input, non-zero input size, non-null placements, and non-zero counts.
6. Validate `crop_mode == CENTERED_FILL`; reject zero and all unknown enum values.
7. Validate `expected_count == placement_count`.
8. Validate `placement_count <= 64` before copying the array.
9. Validate non-zero canvas dimensions, each dimension <= 4096, and checked canvas area <= 10,000,000 pixels.
10. Validate output cap is in the supported range.
11. Copy the now-bounded placement array into native-owned metadata.
12. Validate every rectangle has non-zero width/height, checked right/bottom bounds within the canvas, and no overlap. Keep the existing placement validation policy; do not rely only on Rust.
13. Enter the existing operation lock and synchronous native render path.
14. Native decode validates the encoded source once. C computes centered crop from that decoded buffer's dimensions and stride for each target cell.
15. Validate final counts and output postconditions before transferring ownership to the caller.

Status mapping should match the established ABI status model: invalid arguments/enums/structure contract -> `INVALID_ARGUMENT`; hostile or malformed encoded image -> `INVALID_IMAGE`; dimensions/counts/geometry/output cap -> `RESOURCE_LIMIT`; allocation failure -> `OUT_OF_MEMORY`; unexpected native failure -> `INTERNAL_ERROR`.

### P12 sheet resource envelope

The P12 sheet operation uses a separate 10,000,000-pixel canvas ceiling, with each dimension still limited to 4096 px and placements still limited to 64. This is the smallest round-number policy that admits A4 at 300 DPI: 210 mm × 297 mm resolves by half-up rounding to 2480 × 3508 = 8,699,840 pixels. The existing P8 FIT ABI and `imgengine_layout_render_v2[_bounded]` keep their 4,000,000-pixel limit unchanged. The 10 MP ceiling is a geometry admission limit, not proof of peak RSS; the native A4 integration test must measure/verify bounded execution memory before release.

## 7. Crop semantics at the C boundary

C must not trust a crop rectangle from Rust or the browser. The request contains only the crop mode.

For each target placement of width `tw` and height `th`, C uses the already-decoded source dimensions `sw` and `sh`:

- Compare `sw * th` and `sh * tw` using checked-width integer arithmetic (or a proven wider intermediate).
- If source is wider than the target aspect ratio, retain full source height and compute the centered crop width.
- If source is taller than the target aspect ratio, retain full source width and compute the centered crop height.
- If aspect ratios match, use the full source.
- Apply the same integer floor and centered odd-excess convention as P11's `centered_crop`.
- Validate crop width/height are positive and crop right/bottom remain within the decoded source.
- Construct a borrowed source view (or equivalent bounded crop operation) that preserves the original stride and pixel lifetime. Never free the borrowed view as an owned image.
- Resize the cropped view to the exact target cell size without stretching.

The C implementation must have its own tests. Rust's P11 helper is a deterministic reference oracle for parity, not a security boundary.

## 8. ELF symbol versioning and export manifest

Append a version node to `imgengine/cmake/libimgengine.map`:

```text
IMGENGINE_2.2 {
    global:
        imgengine_sheet_render_v1_bounded;
} IMGENGINE_2.1;
```

Add exactly this symbol to `imgengine/abi/exported_symbols.json` with ABI label `IMGENGINE_2.2`.

Do not alter the existing 1.0, 2.0, or 2.1 symbol entries. The export checker must continue to enforce an exact set: no missing symbols and no accidental exports.

## 9. Rust FFI boundary

Add `#[repr(C)]` Rust representations for the new request and crop constant in the existing native sys boundary. Reuse the existing `LayoutRectV2` and `LayoutOutputV2` types.

Requirements:

- Verify Rust field order, integer widths, pointer constness, and alignment against the C header.
- Keep the raw extern declaration private to the FFI module where practical.
- Expose a safe wrapper that builds the request from validated Rust-owned values and keeps input bytes and placement storage alive through the synchronous call.
- Initialize output to the all-zero default.
- Copy the JPEG bytes only after status is success and pointer/size/count postconditions are valid.
- Release native output exactly once on every success path, including copy/allocation errors after native success. Never release an all-zero failure output as if it owned memory unless the existing release contract explicitly guarantees it.
- Do not implement `Send` or `Sync` for engine handles or bypass the existing engine operation lock.
- Do not pass browser-derived source dimensions or crop coordinates.

## 10. Required ABI tests

### Compile-time layout compatibility

Add static assertions for:
- request field offsets and total size on the supported LP64 ABI target;
- rectangle and output types reused from ABI 2.0;
- crop enum numeric value and request version constant;
- existing ABI-v1 and ABI-v2 structure assertions unchanged.

### Public consumer tests

Compile an external C consumer using only installed public headers and the shared library. Test:
- valid centered-fill request returns a decodable JPEG and exact canvas/count metadata;
- zero crop mode and unknown crop mode are rejected;
- too-small `struct_size`, wrong `version`, and non-zero reserved fields are rejected;
- null input/placements, zero counts, count mismatch, and >64 placements are rejected;
- zero or oversized canvas, area overflow/budget, zero-sized/out-of-bounds/overlapping rectangles are rejected;
- zero and too-large output caps are rejected;
- bounded output exhaustion returns resource-limit with an all-zero output;
- success output releases exactly once and is zeroed;
- failure owns no output;
- existing ABI v1 JPEG consumer and ABI v2 FIT consumer still pass unchanged.

### Native crop parity and safety

Test square/equal-aspect, wide-to-tall, tall-to-wide, portrait, landscape, odd excess, 1-pixel edge cases, and large dimensions. Compare C crop coordinates with the P11 Rust reference vectors. Include stride-sensitive borrowed-view cases and sanitizer coverage.

### Export and installed consumer

- Add the symbol to the exact export manifest under 2.2.
- Verify ELF reports the new symbol under `IMGENGINE_2.2`.
- Ensure no old symbol is removed or version-moved.
- Run `abi_public_consumer`, `abi_compatibility`, `abi_installed_consumer`, and `abi_exports`.
- Run the Docker Ubuntu 24.04 normal, ASAN/UBSAN, fuzz, and SaaS integration gates for the exact commit.

## 11. Implementation sequence

1. Review this ABI design against the public header, wrapper, map, manifest, and current native renderer.
2. Add header, C ABI wrapper, map node, export manifest entry, Rust FFI representation, and layout assertions without changing the renderer.
3. Add negative public-consumer tests for all request validation and output ownership rules.
4. Add an isolated C crop-geometry helper with parity tests against the P11 reference vectors.
5. Integrate crop-to-fill into the native layout renderer while retaining the single decode and borrowed-source lifetime.
6. Add safe Rust wrapper and API-level integration.
7. Run all Docker verification gates; inspect exact revision CI and review the ABI diff before removing Draft status.

No implementation step should begin by changing the existing ABI-v2 request or the existing FIT renderer.
