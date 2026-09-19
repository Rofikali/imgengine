# Real-Image Verification

**Status:** Passed on 2026-09-12 for the Priority 1 and Priority 2 acceptance
gate. This is an additional evidence layer; it does not replace CTest,
sanitizers, sandbox tests, or fuzzing.

## Method

`imgengine/tests/abi/real_image_verification.sh` generates deterministic
fixtures inside the Ubuntu 24.04 Docker verifier using ImageMagick. The script
uses `test_abi_real_image_consumer`, which includes only
`<imgengine/api/v1/imgengine.h>`, and writes JPEG output through the sealed
ABI. ImageMagick `identify` independently validates the output format and
dimensions. The same inputs are passed to the minimal safe Rust FFI wrapper;
the wrapper writes its result and `identify` validates it independently.

`imgengine/photo.jpg` is a pre-existing tracked repository asset, while
`imgengine/photo.png` is a locally added file ignored by Git. When present,
both are used only as opt-in local evidence and never as reproducible project
fixtures. The verification does not add, modify, or stage either file.

## Inputs and Results

The first successful run used three repetitions per valid input. Output size
and elapsed time vary with the image content and environment; the values are a
baseline record, not a production performance claim.

A ten-repetition normal-build comparison showed no material wrapper divergence
for large inputs: the local `2400x1601` JPEG took 1.963 s through C and 1.936 s
through Rust; the local `1536x1024` PNG took 1.996 s through C and 2.003 s
through Rust. These are wall-clock measurements, include engine startup and
the per-call work, and are not a claim about a production workload.

| Fixture | Input | C ABI and Rust FFI result | Independent output validation |
|---|---|---|---|
| small RGB JPEG | `320x240`, 2,359 B | success | JPEG `320x240` |
| large RGB JPEG | `2048x1536`, 942,502 B | success | JPEG `2048x1536` |
| RGBA PNG | `333x127`, 766 B | success | JPEG `333x127` |
| grayscale JPEG | `513x257`, 3,325 B | success | JPEG `513x257` |
| progressive JPEG | `640x427`, 122,401 B | success | JPEG `640x427` |
| CMYK JPEG | `640x427`, 8,919 B | success | JPEG `640x427` |
| unusual aspect PNG | `17x701`, 1,246 B | success | JPEG `17x701` |
| pre-existing tracked JPEG (not a fixture) | `2400x1601`, 814,549 B | success | JPEG `2400x1601` |
| local PNG (not committed) | `1536x1024`, 2,610,667 B | success | JPEG `1536x1024` |

Invalid-input results were: truncated JPEG -> `RESOURCE_LIMIT` (3), truncated
PNG -> `INVALID_IMAGE` (2), empty input -> `INVALID_ARGUMENT` (1), unsupported
bytes -> `INVALID_IMAGE` (2), and a `16385x1` JPEG -> `RESOURCE_LIMIT` (3).
Every failed operation returned `{ NULL, 0 }` output.

## Safety and Environment

- Ubuntu 24.04 Docker image `imgengine-verify:ubuntu24`.
- GCC 13.3.0 normal C ABI and Rust FFI run passed.
- GCC 13.3.0 ASAN/UBSAN with `detect_leaks=1:halt_on_error=1` passed for all
  real inputs through the C ABI, including the local images.
- Rust/Cargo 1.75.0 FFI output was validated by ImageMagick and uses no
  `Send`/`Sync` assertion for the opaque native engine.
- The normal full verifier retains the sandbox lifecycle and libFuzzer gates;
  the real-image fixture script is also invoked by that verifier.

Run the reproducible gate from the repository root:

```powershell
docker build -f imgengine/Dockerfile.verify -t imgengine-verify:ubuntu24 imgengine
docker run --rm -e IMGENGINE_FUZZ_RUNS=1000 -v "${PWD}/imgengine:/workspace" -w /workspace `
  imgengine-verify:ubuntu24 bash scripts/verify_linux.sh
```
