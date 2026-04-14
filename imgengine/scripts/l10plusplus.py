#!/usr/bin/env python3
import os
import re
import sys
from collections import defaultdict


class L10PlusPlus:
    def __init__(self):
        self.failed = False

        # 🔥 STRICT LAYER DAG (LOW → HIGH)
        self.layers = [
            "core",
            "memory",
            "security",
            "arch",
            "pipeline",
            "runtime",
            "plugins",
            "observability",
            "api",
            "io",
            "cmd",
        ]

        self.roots = ["include", "src", "api"]

        # Graphs
        self.file_graph = defaultdict(set)
        self.layer_graph = defaultdict(set)

        # Colors
        self.RED = "\033[0;31m"
        self.GREEN = "\033[0;32m"
        self.YELLOW = "\033[1;33m"
        self.NC = "\033[0m"

    # ============================================================
    # LOGGING
    # ============================================================

    def error(self, msg):
        print(f"{self.RED}❌ ERROR:{self.NC} {msg}")
        self.failed = True

    def warn(self, msg):
        print(f"{self.YELLOW}⚠️ WARN:{self.NC} {msg}")

    def suggest(self, msg):
        print(f"{self.YELLOW}💡 FIX:{self.NC} {msg}")

    # ============================================================
    # PATH → LAYER
    # ============================================================

    def get_layer(self, path):
        rel = os.path.relpath(path, ".")

        for root in self.roots:
            if rel.startswith(root + os.sep):
                rel = rel[len(root) + 1 :]
                break

        layer = rel.split(os.sep)[0]
        return layer if layer in self.layers else "unknown"

    def layer_index(self, layer):
        return self.layers.index(layer) if layer in self.layers else -1

    def is_api_file(self, file_path):
        return file_path.startswith("api/") or "/api/" in file_path

    # ============================================================
    # CORE RULES
    # ============================================================

    def check_layer_violation(self, file_path, layer, layer_idx, inc):
        inc_layer = inc.split("/")[0]
        inc_idx = self.layer_index(inc_layer)

        if inc_idx == -1:
            return

        self.layer_graph[layer].add(inc_layer)

        if inc_idx > layer_idx:
            self.error(f"{file_path} → {inc} (layer violation)")
            self.suggest("Move dependency DOWN or introduce interface abstraction")

    def check_hot_path(self, file_path, content):
        if "hot" in file_path:
            if re.search(r"\bmalloc|calloc|realloc\b", content):
                self.error(f"{file_path}: malloc in HOT path")
                self.suggest("Use slab/arena allocator")

            if "printf" in content:
                self.error(f"{file_path}: printf in HOT path")
                self.suggest("Use observability logger")

            if "pthread_mutex" in content:
                self.error(f"{file_path}: mutex in HOT path")
                self.suggest("Use lock-free atomics")

    def check_security(self, file_path, content):
        if "api" in file_path:
            if "img_security_validate_request" not in content:
                self.error(f"{file_path}: missing input validation")
                self.suggest("Call img_security_validate_request()")

    def check_zero_copy(self, file_path, content):
        if "plugins" in file_path:
            if "memcpy" in content:
                self.warn(f"{file_path}: memcpy detected")
                self.suggest("Ensure zero-copy or document explicitly")

    def check_lock_free(self, file_path, content):
        if "queue" in file_path:
            if "pthread_mutex" in content:
                self.error(f"{file_path}: mutex in lock-free queue")
                self.suggest("Use atomic operations only")

    def check_type_ownership(self, file_path, content):
        if "struct img_buffer" in content and "buffer.h" not in file_path:
            self.error(f"{file_path}: illegal struct img_buffer definition")
            self.suggest("Move to include/core/buffer/buffer.h")

        if "img_pipeline_desc_t" in content:
            if '#include "core/pipeline/pipeline_types.h"' not in content:
                self.error(f"{file_path}: missing pipeline_types.h include")

    def check_slab_usage(self, file_path, content):
        if "img_slab_free(" in content:
            if not re.search(r"img_slab_free\s*\(\s*[^,]+,\s*[^)]+\)", content):
                self.error(f"{file_path}: invalid img_slab_free usage")
                self.suggest("Use img_slab_free(pool, ptr)")

    # ============================================================
    # 🔥 HARD L10 API RULES (KERNEL-GRADE)
    # ============================================================

    def check_api_no_internal_includes(self, file_path, includes):
        if not self.is_api_file(file_path):
            return

        forbidden = ("core/", "memory/", "runtime/")

        for inc in includes:
            if inc.startswith(forbidden):
                self.error(f"{file_path}: API includes internal header → {inc}")
                self.suggest("Expose via api/types or interface only")

    def check_api_no_alloc(self, file_path, content):
        if not self.is_api_file(file_path):
            return

        if re.search(r"\b(malloc|calloc|realloc|free)\b", content):
            self.error(f"{file_path}: API must NOT allocate memory")
            self.suggest("Move allocation to core/memory layer")

        if "img_slab_alloc" in content or "img_slab_free" in content:
            self.error(f"{file_path}: API using slab allocator")
            self.suggest("Only lower layers manage memory")

    def check_api_no_logic(self, file_path, content):
        if not self.is_api_file(file_path):
            return

        if re.search(r"\b(if|for|while|switch)\b", content):
            self.error(f"{file_path}: API contains logic")
            self.suggest("Move logic to pipeline/runtime")

        if re.search(r"\)\s*\{", content):
            self.error(f"{file_path}: API contains function definitions")
            self.suggest("Keep API as declarations only")

        if re.search(r"\b(img_pipeline_|img_runtime_)\w+", content):
            self.error(f"{file_path}: API calling execution layer")
            self.suggest("Use abstraction layer")

    def enforce_api_rules(self, file_path, content, includes):
        self.check_api_no_internal_includes(file_path, includes)
        self.check_api_no_alloc(file_path, content)
        self.check_api_no_logic(file_path, content)

    # ============================================================
    # SCAN FILE
    # ============================================================

    def scan_file(self, file_path):
        try:
            with open(file_path, "r", encoding="utf-8", errors="ignore") as f:
                content = f.read()
        except:
            return

        layer = self.get_layer(file_path)
        layer_idx = self.layer_index(layer)

        includes = re.findall(r'#include\s+"(.+?)"', content)

        for inc in includes:
            self.file_graph[file_path].add(inc)
            self.check_layer_violation(file_path, layer, layer_idx, inc)

        # Standard rules
        self.check_hot_path(file_path, content)
        self.check_security(file_path, content)
        self.check_zero_copy(file_path, content)
        self.check_lock_free(file_path, content)
        self.check_type_ownership(file_path, content)
        self.check_slab_usage(file_path, content)

        # 🔥 HARD API RULES
        self.enforce_api_rules(file_path, content, includes)

    # ============================================================
    # DUPLICATE SYMBOL DETECTOR
    # ============================================================

    def detect_duplicate_symbols(self):
        symbols = defaultdict(list)

        for file in list(self.file_graph.keys()):
            try:
                with open(file, "r", encoding="utf-8", errors="ignore") as f:
                    content = f.read()
            except:
                continue

            funcs = re.findall(r"\bvoid\s+(\w+)\s*\(", content)

            for fn in funcs:
                symbols[fn].append(file)

        for fn, files in symbols.items():
            if len(files) > 1:
                self.error(f"Duplicate function: {fn} in {files}")
                self.suggest("Make static OR move to shared module")

    # ============================================================
    # CYCLE DETECTION
    # ============================================================

    def detect_file_cycles(self):
        visited = set()
        stack = set()

        def dfs(node, path):
            if node in stack:
                self.error(f"FILE CYCLE: {' -> '.join(path + [node])}")
                return

            if node in visited:
                return

            visited.add(node)
            stack.add(node)

            for neighbor in self.file_graph.get(node, []):
                dfs(neighbor, path + [node])

            stack.remove(node)

        for node in list(self.file_graph.keys()):
            dfs(node, [])

    def detect_layer_cycles(self):
        visited = set()
        stack = set()

        def dfs(node):
            if node in stack:
                self.error(f"LAYER CYCLE DETECTED at {node}")
                return

            if node in visited:
                return

            visited.add(node)
            stack.add(node)

            for neighbor in self.layer_graph.get(node, []):
                dfs(neighbor)

            stack.remove(node)

        for node in list(self.layer_graph.keys()):
            dfs(node)

    # ============================================================
    # BUILD ORDER
    # ============================================================

    def enforce_build_order(self):
        for src, targets in self.layer_graph.items():
            for tgt in targets:
                if self.layer_index(src) > self.layer_index(tgt):
                    self.error(f"Build order violation: {src} depends on {tgt}")

    # ============================================================
    # EXPORT GRAPH
    # ============================================================

    def export_graph(self):
        with open("dependency_graph.dot", "w") as f:
            f.write("digraph G {\n")
            for src, targets in self.file_graph.items():
                for tgt in targets:
                    f.write(f'"{src}" -> "{tgt}";\n')
            f.write("}\n")

    def export_layer_graph(self):
        with open("layer_graph.dot", "w") as f:
            f.write("digraph L {\n")
            for src, targets in self.layer_graph.items():
                for tgt in targets:
                    f.write(f'"{src}" -> "{tgt}";\n')
            f.write("}\n")




    # ============================================================
    # RUN
    # ============================================================

    def run(self):
        print("🔍 L10++ ANALYZER STARTING...\n")

        for root, dirs, files in os.walk("."):
            dirs[:] = [d for d in dirs if d not in ("build", ".git")]

            for file in files:
                if file.endswith((".c", ".h")):
                    self.scan_file(os.path.join(root, file))

        print("\n🔍 Checking cycles...")
        self.detect_file_cycles()
        self.detect_layer_cycles()

        print("\n🔍 Checking duplicate symbols...")
        self.detect_duplicate_symbols()

        print("\n🔍 Enforcing build order...")
        self.enforce_build_order()

        print("\n" + "=" * 50)

        if self.failed:
            print(f"{self.RED}🚨 BUILD BLOCKED (L10++ VIOLATIONS){self.NC}")
            sys.exit(1)
        else:
            print(f"{self.GREEN}✅ ARCHITECTURE CLEAN (L10++){self.NC}")

    def export_png(self):
        # import subprocess

        print("Running Graphviz...")
        
        r1 = subprocess.run(
            ["dot", "-Tpng", "dependency_graph.dot", "-o", "dependency_graph.png"],
            capture_output=True, text=True
        )
        print("dependency_graph:", r1.stderr)

        r2 = subprocess.run(
            ["dot", "-Tpng", "layer_graph.dot", "-o", "layer_graph.png"],
            capture_output=True, text=True
        )
        print("layer_graph:", r2.stderr)

if __name__ == "__main__":
    import subprocess

    analyzer = L10PlusPlus()

    print("🔍 L10++ ANALYZER STARTING...\n")

    # Scan
    VALID_ROOTS = ("./src", "./include", "./api")

    for root, dirs, files in os.walk("."):
        if not root.startswith(VALID_ROOTS):
            continue

        dirs[:] = [d for d in dirs if d not in ("build", ".git")]

        for file in files:
            if file.endswith((".c", ".h")):
                analyzer.scan_file(os.path.join(root, file))

    for root, dirs, files in os.walk("."):
        dirs[:] = [d for d in dirs if d not in ("build", ".git")]

        for file in files:
            if file.endswith((".c", ".h")):
                analyzer.scan_file(os.path.join(root, file))

    # Phases
    print("\n🔍 Checking cycles...")
    analyzer.detect_file_cycles()
    analyzer.detect_layer_cycles()

    print("\n🔍 Checking duplicate symbols...")
    analyzer.detect_duplicate_symbols()

    print("\n🔍 Enforcing build order...")
    analyzer.enforce_build_order()

    print("\n🔍 Exporting graphs...")
    analyzer.export_graph()
    analyzer.export_layer_graph()

    print("\n🔍 Generating PNG graphs...")
    analyzer.export_png()   # ✅ FIXED

    print("\n" + "=" * 50)

    if analyzer.failed:
        print(f"{analyzer.RED}🚨 BUILD BLOCKED (L10++ VIOLATIONS){analyzer.NC}")
        sys.exit(1)
    else:
        print(f"{analyzer.GREEN}✅ ARCHITECTURE CLEAN (L10++){analyzer.NC}")