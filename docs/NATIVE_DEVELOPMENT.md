# Native Engine Development

## Supported Baseline

Ubuntu Linux is the authoritative native-engine development and CI target. The GitHub Actions workflow installs dependencies, configures CMake with Ninja, builds, runs CTest, executes CLI help, runs the progressive-JPEG regression, and checks the public ABI.

Install prerequisites:

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake ninja-build python3 clang-format \
  imagemagick libturbojpeg-dev libnuma-dev liburing-dev nasm
```

## Reproducible Commands

Run from repository root:

```bash
cmake -S imgengine -B imgengine/build/dev -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DIMGENGINE_LTO=OFF \
  -DIMGENGINE_BENCH=OFF -DIMGENGINE_ENABLE_DSL_CODEGEN=OFF
cmake --build imgengine/build/dev --parallel
ctest --test-dir imgengine/build/dev --output-on-failure
imgengine/build/dev/imgengine_cli --help
cmake --build imgengine/build/dev --target regression_progressive
cmake --build imgengine/build/dev --target regression_geometry
cmake --build imgengine/build/dev --target regression_security
IMGENGINE_BUILD_DIR="$PWD/imgengine/build/dev" python3 imgengine/scripts/check_exported_symbols.py
```

Build the production portable baseline (no optional SIMD objects) and compare it with optimized output:

```bash
cmake -S imgengine -B imgengine/build/portable -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DIMGENGINE_LTO=OFF \
  -DIMGENGINE_BENCH=OFF -DIMGENGINE_PORTABLE_BASELINE=ON \
  -DIMGENGINE_ENABLE_DSL_CODEGEN=OFF
cmake --build imgengine/build/portable --parallel
ctest --test-dir imgengine/build/portable --output-on-failure
bash imgengine/tests/regression/scalar_equivalence.sh \
  imgengine/build/dev/imgengine_cli imgengine/build/portable/imgengine_cli
```


Run the sanitizer gate separately; it is a correctness check, not a performance measurement:

```bash
cmake -S imgengine -B imgengine/build/sanitize -G Ninja \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo -DIMGENGINE_SANITIZE=ON \
  -DIMGENGINE_LTO=OFF -DIMGENGINE_BENCH=OFF \
  -DIMGENGINE_ENABLE_DSL_CODEGEN=OFF
cmake --build imgengine/build/sanitize --parallel
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
  ctest --test-dir imgengine/build/sanitize --output-on-failure
cmake --build imgengine/build/sanitize --target regression_progressive
```

Run the bounded libFuzzer dimension/file-size security target with Clang:

```bash
CC=clang cmake -S imgengine -B imgengine/build/fuzz -G Ninja \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo -DIMGENGINE_FUZZ=ON \
  -DIMGENGINE_LTO=OFF -DIMGENGINE_BENCH=OFF \
  -DIMGENGINE_ENABLE_DSL_CODEGEN=OFF
cmake --build imgengine/build/fuzz --target fuzz_input_validator --parallel
imgengine/build/fuzz/fuzz_input_validator -max_total_time=30 -max_len=4096 -runs=0
convert imgengine/photo.jpg -strip /tmp/imgengine-fuzz-seed.png
imgengine/build/fuzz/fuzz_decoder imgengine/photo.jpg /tmp/imgengine-fuzz-seed.png \
  -max_total_time=60 -max_len=8388608 -runs=0
```

The manual `imgengine-native-coverage` workflow uses a real Clang-instrumented build, runs CTest, and retains LLVM profile, text, and LCOV artifacts for 90 days. It is evidence collection, not a percentage gate; define a threshold only after stable measurements and review of error-only paths.

The manual `imgengine-native-release-candidate` workflow stages a Linux x86_64 package containing the CLI, versioned shared library, plugin, public v1 headers, README, SPDX runtime SBOM, and SHA-256 manifest. It verifies CTest, regressions, ABI exports, and staged CLI loading before retaining a non-published archive for approval.

The baseline disables LTO and benchmarks to reduce CI variance. Enable them only in dedicated performance jobs with recorded hardware and input corpus.

For the benchmark contract and canonical evidence command, see [Native Benchmarking](NATIVE_BENCHMARKING.md).

## Windows

Windows is not a supported native-engine release target yet. The current POSIX memory-mapping implementation blocks compilation. Use Linux CI, WSL with a configured distribution, or a Linux container until the platform abstraction is complete.

## Loop A Evidence

- Linux CI succeeds without modifying tracked source files.
- CTest runs the generated-registration test.
- CTest runs deterministic layout property invariants.
- CLI launches and prints usage.
- Progressive-JPEG regression passes.
- FIT/FILL, border, bleed, and crop-mark geometry regression passes.
- Malformed and oversized image fixtures fail safely without an output artifact.
- Optimized and portable baseline binaries produce pixel-equivalent output. A direct AVX2/scalar CTest validates the resize-kernel arithmetic before AVX2 dispatch is released.
- ABI checker resolves `libimgengine.so` and all required symbols.
- ASan/UBSan CTest and progressive-JPEG regression are clean on Linux.
- Bounded libFuzzer input-validation run completes without a sanitizer finding.
- Bounded JPEG/PNG decoder fuzzing completes using a one-block 8 MiB test context and releases successful decode buffers.
- Manual LLVM source-coverage artifacts are retained for review before introducing a coverage threshold.
