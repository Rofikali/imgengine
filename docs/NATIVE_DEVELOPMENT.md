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
IMGENGINE_BUILD_DIR="$PWD/imgengine/build/dev" python3 imgengine/scripts/check_exported_symbols.py
```

The baseline disables LTO and benchmarks to reduce CI variance. Enable them only in dedicated performance jobs with recorded hardware and input corpus.

## Windows

Windows is not a supported native-engine release target yet. The current POSIX memory-mapping implementation blocks compilation. Use Linux CI, WSL with a configured distribution, or a Linux container until the platform abstraction is complete.

## Loop A Evidence

- Linux CI succeeds without modifying tracked source files.
- CTest runs the generated-registration test.
- CLI launches and prints usage.
- Progressive-JPEG regression passes.
- ABI checker resolves `libimgengine.so` and all required symbols.
