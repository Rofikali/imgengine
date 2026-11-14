# M0 - C17 portability baseline and WebAssembly decision record

## Status

**Implemented and validated in Ubuntu 24.04 Docker on 2026-11-14;
WebAssembly implementation deferred.** This record defines the native language
policy and a decision framework. It does not add a browser image-processing
runtime, modify the C ABI, or claim a new product capability.

## Decision

The shipping portable C core is compiled as strict ISO C17; public C ABI
headers remain C11-consumable:

- CMake sets `CMAKE_C_STANDARD=17`, requires that standard, and disables
  compiler extensions.
- The public ABI remains source-compatible with a C11 consumer. The existing
  installed-consumer test continues to compile with `-std=c11`.
- The authoritative Ubuntu 24.04 verifier also performs a whole-core C11
  compatibility regression using `-DIMGENGINE_C11_COMPATIBILITY=ON`.
- C11 compatibility is regression evidence only. New production-core code
  must not rely on compiler extensions or silently lower the C17 baseline.

The following are deliberately unchanged by M0: public ABI v1 and additive
v2.x symbol contracts, object layouts, scheduler ownership, Rust FFI/API,
image algorithms, and frontend behavior.

## Compatibility matrix

| Profile | Intended use | Language mode | Required evidence |
| --- | --- | --- | --- |
| C17 strict | Production core, public ABI, optimized and portable builds | ISO C17, extensions off | CTest, regressions, ABI/export and installed-consumer checks, Rust integration, ASan/UBSan, fuzz, Ubuntu verifier |
| C11 compatibility | Regression only; not a shipping language target | ISO C11, extensions off | Whole-core CTest/regressions plus existing C11 installed-consumer test |
| C23 experiment | Future isolated experiment only | Explicit compiler- and feature-gated profile | Independent design review, C17 isolation proof, portability/security/performance evidence; no public header exposure |

The C11 profile demonstrates that today's core remains portable to that prior
baseline. It does not authorize new C11-only policy, and it does not make C11
the advertised production language level.

## C23 containment policy

C23 may be considered only when all of these are true:

1. The experiment is in an isolated target or translation unit with no C23
   tokens, types, macros, or ABI-visible layout in public headers.
2. C17 builds continue to compile and run without the experiment.
3. Configuration uses explicit compiler feature detection and an opt-in
   capability flag; there is a C17 fallback with equivalent observable
   behavior.
4. A design review records compiler support, security and portability impact,
   benchmark evidence, test strategy, and removal/fallback path.
5. The authoritative Ubuntu verifier, ABI checks, sanitizers, and fuzz gates
   remain unchanged and pass.

No C23 feature is adopted by this record.

## WebAssembly feasibility options

| Option | Benefits | Principal risks/costs | Assessment |
| --- | --- | --- | --- |
| Browser-native codecs plus a small Wasm C17 geometry core | Lowest download and attack surface; browser codecs manage decode/encode; geometry can be deterministic and Worker-hosted | Browser codec behavior, metadata handling, and output bytes can vary; requires a precise browser/server geometry contract | **Recommended first feasibility path** |
| Full C17/Wasm decode, render, and encode engine | Strong native algorithm reuse and output-control potential | TurboJPEG/Wasm build and licensing analysis, large bundles, hostile decoder attack surface in the browser, memory pressure, browser feature variance, substantially higher maintenance | Not justified without measured product need |
| Hybrid browser preview with Rust/C server fallback | Progressive capability and one authoritative server path for expensive work | Two execution paths can diverge; privacy and fallback disclosure must be explicit; requires parity fixtures | Follow only after the small geometry feasibility path proves value |

## Recommendation and constraints

Start with an **experimental Web Worker plus limited C17/Wasm geometry core**
only after a separately approved design. It must accept validated scalar
geometry, have a bounded memory contract, use no API secrets, and make no
compliance or browser-local-processing claim until browser evidence exists.
Use browser-native image decode/preview initially; retain the server path as
the authoritative renderer and fallback. Do not compile the current full
native decoder/encoder stack into Wasm until its codec licensing, sandboxing,
bundle-size, memory, compatibility, and parity trade-offs are independently
reviewed.

## Evidence gaps before a Wasm implementation decision

- Supported browser/version matrix, mobile memory ceiling, and Worker startup
  measurements.
- Bundle-size budget and cold/warm latency measurements with representative
  JPEG/PNG fixtures.
- Codec orientation, color, metadata, crop, and pixel-parity contract.
- Licensing and security review for any codec/toolchain included in a browser
  binary.
- CSP, COOP/COEP, Worker isolation, error telemetry, accessibility, and
  privacy disclosure design.
- Product evidence that local preview/geometry materially improves conversion
  or operating cost.

## M0 acceptance evidence

M0 is complete only after the strict C17 build, whole-core C11 compatibility
regression, ABI/public-layout checks, portable and optimized tests,
ASan/UBSan, fuzz regression corpus, Rust integration, and the authoritative
Ubuntu 24.04 Docker verifier pass on the same revision. Passing M0 is not a
claim of WebAssembly support, production readiness, or actual-VPS capacity.

## Validation record

The M0 implementation revision was verified in the Ubuntu 24.04
`imgengine/Dockerfile.verify` image using GCC 13.3 and Clang 18 tooling.

- Focused strict-C17 portable build: CTest (16/16) and security, geometry,
  progressive-JPEG, and output-format regressions passed.
- Focused C11 compatibility build: CTest (16/16) and the same regression
  targets passed.
- The authoritative `scripts/verify_linux.sh` run passed with the new C11
  profile: strict-C17 normal native and ABI tests, C11 compatibility tests,
  Rust FFI/supervisor/API tests, real-image validation, ASan/UBSan with leak
  detection, sandbox lifecycle, and both 5,000-run fuzz targets.

The verification source was mounted read-only and `CARGO_TARGET_DIR` was
redirected outside the source mount. That is a harness isolation detail only;
it does not alter compiler flags, source, test selection, or product behavior.
