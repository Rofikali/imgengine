# P8 First Online Product Contract

## Status and scope

This document defines the first capability-backed layout product. It is the
contract to implement and verify on branch `codex/p8-first-online-layout`; it
does not claim that the capability is already implemented, validated, or ready
for production release.

The existing `POST /api/v1/render` endpoint remains unchanged: one JPEG or PNG
multipart `file` produces one completed JPEG. P8 adds a separate,
backwards-compatible endpoint rather than changing that P5/P6 contract.

## First supported product operation

```text
POST /api/v1/layout-render
multipart/form-data
required field: file
required fields: width, height, unit, copies, gap, margin, output_format
required when unit=mm: dpi
optional field: columns
```

Input is exactly one JPEG or PNG. `output_format` has one currently supported
value, `jpeg`; the UI may present it as a one-option selector but must not
advertise PNG, PDF, or any unimplemented format. A success is a complete,
decodable JPEG sheet with `Content-Type: image/jpeg`, `Content-Length`,
`X-Request-Id`, and a safe `Content-Disposition` download filename.

The request is synchronous and ephemeral. It has no job ID, durable output,
stored upload, client-selected server path, database record, or browser-visible
API credential. A response abandonment or post-operation deadline does not
cancel ABI-native work; its established lifecycle cleanup semantics continue.

## Geometry model

`width` and `height` describe each individual photo cell, not the whole sheet.
`gap` and `margin` use the same unit as the photo dimensions. The resolved
canvas is the smallest complete row-major grid that holds `copies`; a final row
may be incomplete and contains no implicit placement. With resolved pixel cell
dimensions `W × H`, columns `C`, rows `R`, gap `G`, and outer margin `M`:

```text
canvas_width  = 2*M + C*W + (C-1)*G
canvas_height = 2*M + R*H + (R-1)*G
R = ceil(copies / C)
```

An omitted `columns` resolves deterministically to one column. This first
release deliberately favours predictable custom dimensions over an unreviewed
auto-layout heuristic. When present, `columns` is an explicit bounded positive
integer. Placements are generated row-major, left-to-right then top-to-bottom.

For `unit=pixel`, all dimension fields must be base-10 positive integer pixel
counts. For `unit=mm`, photo dimensions, gap, and margin are finite positive
or non-negative decimal millimetres; `dpi` is a positive integer. Conversion
uses half-up deterministic rounding:

```text
pixels = floor(mm * dpi / 25.4 + 0.5)
```

All conversion, grid, coordinate, count, byte-estimate, and canvas arithmetic
uses checked integer arithmetic before native work or allocation. A conversion
that rounds a required positive photo dimension to zero is invalid.

The same maximum canvas edge, canvas-pixel budget, and placement-count ceiling
are also enforced by the public native ABI before placement allocation and
again inside the private renderer. Rust-side limits remain defense in depth,
not the sole protection for native callers.

## Rendering semantics

P8 implements only the frozen private `layout.render.v1` FIT behavior:

- each source copy preserves aspect ratio, never crops, and never stretches;
- each cell and the outside canvas use explicit opaque white RGB `(255,255,255)`;
- FIT uses the P7.2.6 integer rule and centres odd residual pixels to the
  right/bottom;
- rectangles are half-open `(x, y, width, height)`, non-overlapping, inside
  the resolved canvas, and must execute exactly once;
- success requires `requested == planned == executed == copies`.

FILL, crop anchors, borders, bleed, crop marks, presets, paper sizes,
orientation policy, multi-image sheets, PDF, PNG output, streaming output,
idempotency, and durable artifacts are not part of P8.

## Initial bounded policy

P8 retains the existing provisional 1 MiB multipart/input ceiling. The layout
operation further rejects plans before native execution when any of these
checked facts would exceed the first-release envelope:

| Policy | P8 value | Reason |
| --- | ---: | --- |
| copies | 64 | Bounded explicit metadata and deterministic preview |
| explicit columns | 64 | Prevent grid/count multiplication growth |
| individual resolved dimension | 4,096 px | Below native dimension guard; practical custom-photo bound |
| canvas pixels | 4,000,000 | RGB canvas estimate is at most 12,000,000 bytes, below the existing 64 MiB development tmpfs while leaving headroom for decode/intermediates |
| canvas RGB byte estimate | 12,000,000 bytes | Checked preflight fact, not a claim about total process RSS |
| execution deadline | existing provisional 10 seconds | Reuses P6 semantics; native work is not cancelled |

These are first-release safety bounds, not production capacity claims. Native
decoder dimension/pixel guards remain authoritative for hostile inputs. P9.3
adds a provisional 16 MiB server-owned FIT-layout JPEG destination budget
(`IMGENGINE_LAYOUT_MAX_OUTPUT_BYTES` may provide a validated deployment
override). Native code compares TurboJPEG's conservative `tjBufSize()` bound
before allocating its destination; insufficient budget maps to `413
resource_limit` and returns no partial JPEG. This is not a complete memory
policy: decoded/canvas/intermediate allocations, the Rust output copy, and
aggregate workspace bytes still require measured production limits.

## Error mapping

Existing safe problem-document shape and authentication apply. Invalid or
infeasible layout fields map to `400 invalid_request`; unsupported media maps
to `415 unsupported_media_type`; body/input or plan resource bounds map to
`413 resource_limit`; malformed decoder input maps to `422 invalid_image`;
full admission maps to `429 overloaded`; drain/unavailable maps to `503
unavailable`; late execution maps to `504 deadline_exceeded`. No response may
contain a path, source name, image bytes, native diagnostic, API key, or
internal plan data.

## Native and FFI boundary

ABI v1 remains frozen at its seven exported symbols and its current public
header. P8 requires a separately versioned `IMGENGINE_2.0` layout operation
that accepts the existing opaque `imgengine_engine_t` handle, concrete resolved
pixels, an explicit placement slice, and a borrowed encoded source for a
synchronous terminal call. It returns C-owned JPEG output released by the
paired layout-result release operation. The V2 operation is additive and must
not alter V1 symbols, their ownership rules, or the P5 engine lifecycle.

The Rust safe wrapper constructs the FFI view from an immutable transport-
neutral plan, validates returned count/dimensions, copies output into a Rust
`Vec<u8>`, and releases native output exactly once. The existing supervisor,
admission controller, workspace, deadline, abandonment, and C scheduler remain
the sole lifecycle/execution path; P8 adds neither another queue nor native
cancellation.

## Verification definition

P8 cannot be accepted on mocked UI evidence. Required evidence includes exact
layout-plan boundary tests; native rectangle/FIT/ownership tests; Rust
validation and FFI release tests; real JPEG and PNG inputs through HTTP; a
decoded JPEG output whose dimensions, copy placement count, and white
background match the resolved plan; Nuxt upload/preview/download smoke tests;
success/failure cleanup; frontend test/typecheck/build; and the unchanged
Ubuntu 24.04 authoritative verifier with sanitizers, sandbox, and fuzzing.

Production readiness remains **not claimed**.
