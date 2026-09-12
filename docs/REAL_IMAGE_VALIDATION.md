# Real-Image Validation

Unit tests and synthetic fixtures prove deterministic rules; real-image validation checks that the complete decode → layout → render → encode path behaves correctly with representative image files.

## Purpose

Use this gate when changing decoding, image dimensions, FIT/FILL behavior, layout, rendering, encoding, SIMD dispatch, the public ABI, or the Rust FFI boundary.

The goal is not to publish attractive screenshots. The goal is to produce reproducible engineering evidence.

## Minimum validation matrix

| Case | Input | What to verify |
| --- | --- | --- |
| JPEG → PNG | representative camera/photo JPEG | decode succeeds, dimensions are sane, output is readable |
| PNG → PNG | RGB/RGBA PNG | alpha/channel handling and output validity |
| FIT | landscape + portrait source | source remains contained inside target cells |
| FILL | landscape + portrait source | crop is deterministic and fills target cells |
| Large image | high-resolution source | bounded dimensions/memory behavior |
| Malformed input | intentionally invalid file | clean rejection, no crash or unsafe allocation |
| ABI consumer | real output path through public C API | ownership/release contract remains valid |
| Rust FFI | real job through FFI smoke/integration path | ABI and lifecycle semantics remain intact |

## Evidence to record

For a release, milestone, or significant native change, record:

- commit SHA
- host/container image and digest
- compiler and CMake versions
- CPU architecture and relevant SIMD capabilities
- input format and dimensions
- output format and dimensions
- command or test target used
- exit status
- sanitizer configuration when applicable
- output validation method
- any performance measurement and its methodology

Do not commit private user photographs. Prefer small, redistributable fixtures with known provenance, or generate deterministic test images during the test run.

## Acceptance rule

A real-image check is evidence of end-to-end behavior, not a replacement for unit, regression, ABI, sanitizer, or fuzz tests. A change should not be described as fully verified when only a manual visual check was performed.

For the current Linux verification gate, see [`LINUX_VERIFICATION.md`](LINUX_VERIFICATION.md).
