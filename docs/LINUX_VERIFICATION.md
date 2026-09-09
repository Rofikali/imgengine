# Linux Native Verification

**Status:** Required Priority 1 gate.

## Latest Gate Evidence

**Result:** Passed on 2026-09-09 using Docker Desktop 28.0.4 and
`imgengine-verify:ubuntu24` (`ubuntu:24.04`, image ID `bacdfe6f45f4`).

| Phase | Result |
| --- | --- |
| GCC 13.3.0 normal build and CTest | 4/4 passed |
| Normal security, geometry, progressive, and CMYK regressions | 3/3 passed |
| ASAN/UBSAN build and CTest | 4/4 passed |
| ASAN/UBSAN regression suite | 3/3 passed with leak detection enabled |
| Seccomp lifecycle test | 1/1 passed |
| Clang 18.1.3 libFuzzer targets | `fuzz_input_validator`: 1,000 runs passed; `fuzz_decoder`: 1,000 runs passed |

The verifier uses CMake 3.28.3, Ninja 1.11.1, GCC 13.3.0, Clang 18.1.3,
and includes `libclang-rt-18-dev` so the Clang ASAN interfaces are available.
The fuzz build reports three non-blocking pre-existing warnings: a nonliteral
logging format string, an unnamed C23-style parameter, and an unused JPEG
encoder helper. They are follow-up quality work, not sanitizer or fuzz failures.

IMGENGINE release verification runs in Docker Desktop using Ubuntu 24.04, not the Windows host toolchain. The reproducible image is `imgengine/Dockerfile.verify` and installs GCC, Clang/LLVM, CMake, Ninja, NASM, Python, ImageMagick, `libturbojpeg0-dev`, `libnuma-dev`, and `liburing-dev`.

Build and run the gate from the repository root:

```powershell
docker build -f imgengine/Dockerfile.verify -t imgengine-verify:ubuntu24 imgengine
docker run --rm -v "${PWD}/imgengine:/workspace" -w /workspace imgengine-verify:ubuntu24 `
  bash scripts/verify_linux.sh
```

The script creates independent normal, ASAN/UBSAN, sandbox, and Clang fuzz/coverage builds. It runs CTest, geometry/progressive/security regressions, leak detection, the seccomp engine lifecycle test, and bounded input-validator and decoder fuzz targets. Override `IMGENGINE_FUZZ_RUNS` only when recording an explicitly labelled shorter local run.

Record the Ubuntu image digest, `gcc --version`, `clang --version`, `cmake --version`, `ninja --version`, `libturbojpeg` package version, commands, test output, sanitizer options, fuzz run count, CPU flags, and Docker Desktop version with release evidence. Rust is intentionally not required for this Priority 1 native gate.
