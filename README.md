# imgengine

> A native, high-performance image layout engine for print-ready photo sheets — built in C with a stable public API and a path toward a Rust-backed SaaS platform.

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Language](https://img.shields.io/badge/core-C11-00599C.svg)](https://en.wikipedia.org/wiki/C11)
[![Build](https://img.shields.io/badge/build-CMake-064F8C.svg)](https://cmake.org/)
[![Focus](https://img.shields.io/badge/focus-systems%20%2B%20image%20processing-111827.svg)](#)

## Why imgengine?

`imgengine` is designed around a simple engineering boundary:

**the native engine owns image processing; the application layer owns product concerns.**

The native core handles decoding, geometry, layout, rendering and encoding. Authentication, tenant policy, uploads, persistence, retries and billing belong outside the engine.

That separation makes the project useful both as a native library/CLI and as the processing core of a future SaaS product.

## What it does

- Decode supported source images
- Convert physical print dimensions (cm + DPI) into pixel dimensions
- `FIT` or `FILL` image placement
- Grid-based sheet composition
- Borders, bleed and crop marks
- PNG/JPEG/PDF output paths
- CLI and public C API
- RGB24/raw ingress for already-decoded frames
- SIMD-accelerated paths with portable fallbacks
- Bounded resource validation for untrusted input

## Architecture

```text
                         Product / SaaS Layer
                                  │
                         ┌────────▼────────┐
                         │ Rust Backend     │
                         │ API / policy /   │
                         │ lifecycle /      │
                         │ observability    │
                         └────────┬────────┘
                                  │ stable FFI
                         ┌────────▼────────┐
                         │ imgengine Core   │
                         │ C11             │
                         ├─────────────────┤
                         │ decode          │
                         │ resize / crop   │
                         │ layout          │
                         │ render          │
                         │ encode          │
                         │ SIMD dispatch   │
                         └─────────────────┘
                                  │
                    ┌─────────────┴─────────────┐
                    ▼                           ▼
                  PNG/JPEG                    PDF
```

The native specification defines the processing contract and acceptance matrix in [`docs/ENGINE_SPEC.md`](docs/ENGINE_SPEC.md).

## Processing pipeline

```text
Input
  │
  ▼
Decode / validate
  │
  ▼
FIT or FILL
  │
  ▼
Grid placement
  │
  ▼
Border
  │
  ▼
Bleed + crop marks
  │
  ▼
Encode
  │
  ▼
Print-ready output
```

## Native API

The public C API exposes an explicit engine lifecycle and job execution boundary, including file-to-file, raw encoded output and RGB24 ingress paths.

The project treats ABI stability as an engineering contract: public symbols, ownership, lifecycle, threading and release semantics are tested rather than left implicit.

## Build

### Requirements

- CMake 3.16+
- GCC or Clang
- Linux (primary deployment target)
- NASM for JPEG SIMD dependency builds where required

```bash
git clone https://github.com/Rofikali/imgengine.git
cd imgengine

cmake -S . -B build
cmake --build build -j
```

Run the CLI:

```bash
./build/imgengine_cli --help
```

## Example

```bash
./build/imgengine_cli \
  --input photo.jpg \
  --output sheet.png \
  --cols 6 \
  --rows 6 \
  --width 4.5 \
  --height 3.5 \
  --dpi 300 \
  --bleed 10 \
  --crop-mark 20
```

## Engineering quality

Correctness is treated as a first-class feature. The native acceptance matrix covers:

- deterministic geometry and layout fixtures
- FIT vs FILL behavior
- border, bleed and crop-mark behavior
- layout-boundary/property testing
- malformed-input and unsafe-dimension handling
- SIMD vs scalar equivalence
- ASan/UBSan validation
- bounded fuzzing of untrusted parsing/decoder paths
- memory and arithmetic boundary tests

See [`docs/ENGINE_SPEC.md`](docs/ENGINE_SPEC.md) for the detailed contract.

## Project direction

The project is evolving from a native image-processing engine toward a low-cost, secure SaaS architecture:

```text
C native engine
      ↓
stable C ABI
      ↓
Rust FFI boundary
      ↓
Rust service / lifecycle orchestration
      ↓
Nuxt + TypeScript frontend
```

The design deliberately avoids making the native engine responsible for authentication, billing, persistence or product policy.

## Repository structure

```text
imgengine/
├── include/          # public/internal C interfaces
├── src/              # native implementation
├── tests/             # native correctness and security tests
├── docs/              # specifications and verification evidence
├── imgengine-saas/    # application/SaaS layer
├── CMakeLists.txt
└── README.md
```

## Status

This is an actively engineered project. Native correctness and ABI hardening are being completed before retiring the legacy application stack and moving the production orchestration layer to Rust.

## License

MIT
