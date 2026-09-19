# Linux Native Verification

**Status:** Required native gate; also carries the ABI v1 verification.

## Latest Gate Evidence

**Result:** Passed on 2026-09-11 using Docker Desktop 28.0.4 and
`imgengine-verify:ubuntu24` (`ubuntu:24.04`, image ID
`34ed6584dffd26480a56959cce98b517505d07e1633f6d4e6a518d14c5571c5a`).

| Phase | Result |
| --- | --- |
| GCC 13.3.0 normal build and CTest | 8/8 passed, including ABI consumer, installed-header, compatibility, and export checks |
| Normal security, geometry, progressive, and CMYK regressions | 3/3 passed |
| Rust 1.75.0 FFI smoke | passed against sealed `libimgengine` ABI v1 |
| ASAN/UBSAN build and CTest | 8/8 passed with leak detection enabled |
| ASAN/UBSAN regression suite | 3/3 passed with leak detection enabled |
| Seccomp lifecycle test | 1/1 passed |
| Clang 18.1.3 libFuzzer targets | `fuzz_input_validator`: 1,000 runs passed; `fuzz_decoder`: 1,000 runs passed |

The verifier uses CMake 3.28.3, Ninja 1.11.1, GCC 13.3.0, Clang 18.1.3,
Rust/Cargo 1.75.0, and TurboJPEG `1:2.1.5-2ubuntu2`; it includes
`libclang-rt-18-dev` so the Clang ASAN interfaces are available.
The fuzz build reports three non-blocking pre-existing warnings: a nonliteral
logging format string, an unnamed C23-style parameter, and an unused JPEG
encoder helper. They are follow-up quality work, not sanitizer or fuzz failures.

IMGENGINE release verification runs in Docker Desktop using Ubuntu 24.04, not the Windows host toolchain. The reproducible image is `imgengine/Dockerfile.verify` and installs GCC, Clang/LLVM, CMake, Ninja, NASM, Python, Rust/Cargo, ImageMagick, `libturbojpeg0-dev`, `libnuma-dev`, and `liburing-dev`.

Build and run the gate from the repository root:

```powershell
docker build -f imgengine/Dockerfile.verify -t imgengine-verify:ubuntu24 imgengine
docker run --rm -v "${PWD}/imgengine:/workspace" -w /workspace imgengine-verify:ubuntu24 `
  bash scripts/verify_linux.sh
```

The script creates independent normal, ASAN/UBSAN, sandbox, and Clang fuzz/coverage builds. It runs CTest, geometry/progressive/security regressions, leak detection, the seccomp engine lifecycle test, and bounded input-validator and decoder fuzz targets. Override `IMGENGINE_FUZZ_RUNS` only when recording an explicitly labelled shorter local run.

Record the Ubuntu image digest, `gcc --version`, `clang --version`, `cmake --version`, `ninja --version`, `rustc --version`, `cargo --version`, `libturbojpeg` package version, commands, test output, sanitizer options, fuzz run count, CPU flags, and Docker Desktop version with release evidence. Rust is required only for the ABI v1 FFI smoke check, not for the Priority 1 native gate.
