# IMGENGINE root developer command menu (GNU Make, Ubuntu/Linux).
# Thin orchestration: CMake, Cargo, pnpm and the existing Linux verifier are authoritative.
# No default builds, deployments, destructive operations, permission changes or paid services.
.DEFAULT_GOAL := help
SHELL := /bin/sh
.SUFFIXES:

JOBS ?= 2
FUZZ_RUNS ?= 1000
RUST_CRATE ?= imgengine-api
NATIVE := imgengine
WEB := imgengine-saas/frontend
DBG := imgengine/build/make-debug
REL := imgengine/build/make-release
C11 := imgengine/build/make-c11
ASAN := imgengine/build/make-asan
FUZZ := imgengine/build/make-fuzz
LOGS := build/make-logs
VERIFY := $(CURDIR)/imgengine/build/make-linux-verify
NATIVE_LIB := $(CURDIR)/$(DBG)

# Reject bad parallelism before starting costly compilation.
ifneq ($(words $(JOBS)),1)
$(error JOBS must be a single integer from 1 through 16)
endif
ifeq ($(filter $(JOBS),1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16),)
$(error JOBS must be an integer from 1 through 16)
endif
ifneq ($(filter $(RUST_CRATE),imgengine imgengine-ffi-smoke imgengine-supervisor imgengine-api),$(RUST_CRATE))
$(error Unsupported RUST_CRATE)
endif

.PHONY: help status frontend-setup frontend-test frontend-check \
 frontend-browser frontend-browser-live native-debug native-test native-release \
 native-c11 native-sanitize native-fuzz rust-test rust-fmt-check \
 verify-linux verify-docker clean

help:
	@printf '%s\n' \
	 'IMGENGINE — focused local verification; existing GitHub CI runs seven PR jobs.' \
	 '' \
	 'make                         Safe help (no builds)' \
	 'make status                  Git status, branch and SHA' \
	 'make frontend-setup          Frozen pnpm dependency install' \
	 'make frontend-test           Nuxt unit tests' \
	 'make frontend-check          Nuxt unit + typecheck + build' \
	 'make frontend-browser        Browser tests (requires build + browsers)' \
	 'make frontend-browser-live   Live browser tests (requires local services)' \
	 'make native-debug            C17 Debug compilation only' \
	 'make native-test             C17 Debug + CTest' \
	 'make native-release          Optimized C17 build; not a release gate' \
	 'make native-c11              C11 compatibility + CTest' \
	 'make native-sanitize         ASan/UBSan + CTest' \
	 'make native-fuzz             Clang bounded fuzzing' \
	 'make rust-test               C prerequisite + focused Cargo test' \
	 'make rust-fmt-check          Focused Cargo formatting check' \
	 'make verify-linux CONFIRM_FULL=YES    Complete authoritative Linux verifier' \
	 'make verify-docker CONFIRM_FULL=YES   Complete Ubuntu 24.04 Docker verifier' \
	 'make clean CONFIRM_CLEAN=YES         Remove ONLY root-Makefile-owned outputs' \
	 '' \
	 'JOBS=2 (1..16); FUZZ_RUNS=1000; RUST_CRATE=imgengine-api (or other approved crate).' \
	 'Full verification logs: build/make-logs/. Only failed log excerpts enter chat.' \
	 'Never substitute a local make result for required GitHub CI or P14 deployment evidence.'

status:
	@git status --short
	@git branch --show-current
	@git rev-parse HEAD

# Dependency installation is explicit, not repeated for every focused test.
frontend-setup:
	@cd "$(WEB)" && corepack enable && pnpm install --frozen-lockfile

frontend-test:
	@cd "$(WEB)" && pnpm test

frontend-check:
	@cd "$(WEB)" && pnpm exec nuxi prepare && pnpm test && pnpm run typecheck && pnpm run build

# Browser tests require a built Nuxt output and installed Playwright Chromium.
frontend-browser:
	@cd "$(WEB)" && pnpm run test:browser

# Live browser tests require separately started Nuxt/Rust services and test credentials.
frontend-browser-live:
	@cd "$(WEB)" && pnpm run test:browser:live

native-debug:
	@cmake -S "$(NATIVE)" -B "$(DBG)" -G Ninja \
	  -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
	  -DIMGENGINE_LTO=OFF -DIMGENGINE_BENCH=OFF -DIMGENGINE_ENABLE_DSL_CODEGEN=OFF
	@cmake --build "$(DBG)" --parallel "$(JOBS)"

native-test: native-debug
	@ctest --test-dir "$(DBG)" --output-on-failure

native-release:
	@cmake -S "$(NATIVE)" -B "$(REL)" -G Ninja \
	  -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
	  -DIMGENGINE_LTO=OFF -DIMGENGINE_BENCH=OFF -DIMGENGINE_ENABLE_DSL_CODEGEN=OFF
	@cmake --build "$(REL)" --parallel "$(JOBS)"

native-c11:
	@cmake -S "$(NATIVE)" -B "$(C11)" -G Ninja \
	  -DCMAKE_BUILD_TYPE=Debug -DIMGENGINE_C11_COMPATIBILITY=ON \
	  -DIMGENGINE_LTO=OFF -DIMGENGINE_BENCH=OFF -DIMGENGINE_ENABLE_DSL_CODEGEN=OFF
	@cmake --build "$(C11)" --parallel "$(JOBS)"
	@ctest --test-dir "$(C11)" --output-on-failure

native-sanitize:
	@cmake -S "$(NATIVE)" -B "$(ASAN)" -G Ninja \
	  -DCMAKE_BUILD_TYPE=RelWithDebInfo -DIMGENGINE_SANITIZE=ON \
	  -DIMGENGINE_LTO=OFF -DIMGENGINE_BENCH=OFF -DIMGENGINE_ENABLE_DSL_CODEGEN=OFF
	@cmake --build "$(ASAN)" --parallel "$(JOBS)"
	@ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
	  UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
	  ctest --test-dir "$(ASAN)" --output-on-failure

# Focused local fuzz; CI and release gates remain authoritative.
native-fuzz:
	@case "$(FUZZ_RUNS)" in *[!0-9]*|'') echo "FUZZ_RUNS must be a positive integer" >&2; exit 2;; esac; \
	  test "$(FUZZ_RUNS)" -ge 1 && test "$(FUZZ_RUNS)" -le 100000 || { echo "FUZZ_RUNS must be 1..100000" >&2; exit 2; }
	@CC=clang cmake -S "$(NATIVE)" -B "$(FUZZ)" -G Ninja \
	  -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_C_COMPILER=clang \
	  -DIMGENGINE_FUZZ=ON -DIMGENGINE_LTO=OFF \
	  -DIMGENGINE_BENCH=OFF -DIMGENGINE_ENABLE_DSL_CODEGEN=OFF
	@cmake --build "$(FUZZ)" --target fuzz_input_validator fuzz_decoder --parallel "$(JOBS)"
	@"$(FUZZ)/fuzz_input_validator" -runs="$(FUZZ_RUNS)" -max_len=4096
	@"$(FUZZ)/fuzz_decoder" -runs="$(FUZZ_RUNS)" -max_len=1048576

# Rust crates use a compiled local C shared library and native linker flags.
rust-test: native-debug
	@LD_LIBRARY_PATH="$(NATIVE_LIB)" \
	  RUSTFLAGS="-L native=$(NATIVE_LIB)" \
	  cargo test --manifest-path "backend/$(RUST_CRATE)/Cargo.toml"

rust-fmt-check:
	@cargo fmt --manifest-path "backend/$(RUST_CRATE)/Cargo.toml" -- --check

# Full verifier removes its dedicated build root. Explicit confirmation required.
# Redirect successful output to ignored local log files; retain real exit status.
verify-linux:
	@if [ "$(CONFIRM_FULL)" != YES ]; then echo "Set CONFIRM_FULL=YES to run full verifier" >&2; exit 2; fi
	@mkdir -p "$(LOGS)"
	@log="$(LOGS)/verify-linux.log"; \
	  if IMGENGINE_VERIFY_BUILD_ROOT="$(VERIFY)" \
	    IMGENGINE_VERIFY_JOBS="$(JOBS)" IMGENGINE_FUZZ_RUNS="$(FUZZ_RUNS)" \
	    bash "$(NATIVE)/scripts/verify_linux.sh" >"$$log" 2>&1; then \
	    printf 'PASS | full Linux verifier | %s\n' "$$log"; \
	  else code=$$?; printf 'FAIL (%s) | %s\n' "$$code" "$$log"; tail -n 50 "$$log"; exit "$$code"; fi

# Mirror the existing GitHub Ubuntu 24.04 container job; no ports or production secrets.
verify-docker:
	@if [ "$(CONFIRM_FULL)" != YES ]; then echo "Set CONFIRM_FULL=YES to run Docker verifier" >&2; exit 2; fi
	@mkdir -p "$(LOGS)"
	@log="$(LOGS)/verify-docker.log"; \
	  if docker build --file "$(NATIVE)/Dockerfile.verify" --tag imgengine-verify:make "$(NATIVE)" >"$$log" 2>&1 \
	    && docker run --rm --volume "$(CURDIR):/workspace" --workdir /workspace/imgengine \
	       --env IMGENGINE_VERIFY_BUILD_ROOT=/tmp/imgengine-verify-build \
	       --env CARGO_TARGET_DIR=/tmp/imgengine-cargo-target \
	       --env IMGENGINE_VERIFY_JOBS="$(JOBS)" \
	       --env IMGENGINE_FUZZ_RUNS="$(FUZZ_RUNS)" \
	       imgengine-verify:make bash scripts/verify_linux.sh >>"$$log" 2>&1; then \
	    printf 'PASS | Ubuntu 24.04 Docker verifier | %s\n' "$$log"; \
	  else code=$$?; printf 'FAIL (%s) | %s\n' "$$code" "$$log"; tail -n 50 "$$log"; exit "$$code"; fi

# Never clean entire build/, user data, Cargo target or unrelated native worktrees.
clean:
	@if [ "$(CONFIRM_CLEAN)" != YES ]; then echo "Set CONFIRM_CLEAN=YES to clean owned paths" >&2; exit 2; fi
	@rm -rf -- "$(DBG)" "$(REL)" "$(C11)" "$(ASAN)" "$(FUZZ)" "$(VERIFY)" "$(LOGS)"
	@printf '%s\n' 'Removed only root Makefile-owned build products.'
