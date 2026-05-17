# ============================================================
# Makefile — thin wrapper around CMake (imgengine)
# Usage:
#   make            → Debug build
#   make release    → Release build
#   make profile    → Profile build (perf/gprof ready)
#   make run        → Run CLI
#   make perf       → perf stat
#   make clean      → Clean build dirs
# ============================================================

BUILD_DIR_DEBUG   := build/debug
BUILD_DIR_RELEASE := build/release
BUILD_DIR_PROFILE := build/profile

# Your actual binary (from CMake)
TARGET := build/debug/imgengine_cli

.PHONY: all debug release profile run perf perf-record gprof clean help

all: debug

# =========================
# DEBUG BUILD
# =========================
debug:
	@mkdir -p $(BUILD_DIR_DEBUG)
	@cmake -S . -B $(BUILD_DIR_DEBUG) \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		-G Ninja
	@cmake --build $(BUILD_DIR_DEBUG) --parallel $$(nproc)
	@echo "✅ Debug build → $(BUILD_DIR_DEBUG)/imgengine_cli"

# =========================
# RELEASE BUILD
# =========================
release:
	@mkdir -p $(BUILD_DIR_RELEASE)
	@cmake -S . -B $(BUILD_DIR_RELEASE) \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		-G Ninja
	@cmake --build $(BUILD_DIR_RELEASE) --parallel $$(nproc)
	@echo "🚀 Release build → $(BUILD_DIR_RELEASE)/imgengine_cli"

# =========================
# PROFILE BUILD
# =========================
profile:
	@mkdir -p $(BUILD_DIR_PROFILE)
	@cmake -S . -B $(BUILD_DIR_PROFILE) \
		-DCMAKE_BUILD_TYPE=RelWithDebInfo \
		-DCMAKE_C_FLAGS="-pg" \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		-G Ninja
	@cmake --build $(BUILD_DIR_PROFILE) --parallel $$(nproc)
	@echo "🔍 Profile build → $(BUILD_DIR_PROFILE)/imgengine_cli"

# =========================
# RUN
# =========================
run: debug
	@$(BUILD_DIR_DEBUG)/imgengine_cli

# =========================
# PERF (Linux)
# =========================
perf: profile
	@echo "🔍 Running perf stat..."
	@sudo perf stat -e cycles,instructions,cache-misses,branch-misses \
		$(BUILD_DIR_PROFILE)/imgengine_cli

perf-record: profile
	@echo "🔍 Recording perf..."
	@sudo perf record -g $(BUILD_DIR_PROFILE)/imgengine_cli
	@sudo perf report

# =========================
# GPROF
# =========================
gprof: profile
	@$(BUILD_DIR_PROFILE)/imgengine_cli
	@gprof $(BUILD_DIR_PROFILE)/imgengine_cli gmon.out | less

# =========================
# CLEAN
# =========================
clean:
	@rm -rf build/
	@echo "🧹 Cleaned build directory"

# =========================
# HELP
# =========================
help:
	@echo ""
	@echo "  make            → Debug build"
	@echo "  make release    → Optimized build"
	@echo "  make profile    → Build with gprof support"
	@echo "  make run        → Run CLI"
	@echo "  make perf       → Run perf stat"
	@echo "  make perf-record→ perf record + report"
	@echo "  make gprof      → gprof analysis"
	@echo "  make clean      → Clean build"
	@echo ""