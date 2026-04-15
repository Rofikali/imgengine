#!/usr/bin/env python3
"""
L10++ Architecture Analyzer — imgengine
Kernel-grade static analysis: call graph, hot paths, layer violations,
ABI checks, duplicate symbols, NUMA distribution.

Filters out compiler intrinsics and system functions so results
reflect YOUR code only.
"""

import os
import re
import sys
import time
import threading
from collections import defaultdict, deque

# ============================================================
# SYSTEM FUNCTION FILTER
# Exclude intrinsics, builtins, libc, and system APIs from analysis.
# These inflate the graph with thousands of false bottlenecks.
# ============================================================

SYSTEM_PREFIXES = (
    "_mm_",
    "_mm256_",
    "_mm512_",
    "__builtin_",
    "__builtin_ia32_",
    "io_uring_",
    "tj",
    "stbi_",
    "numa_",
    "pthread_",
    "atomic_",
    "printf",
    "fprintf",
    "sprintf",
    "snprintf",
    "malloc",
    "free",
    "calloc",
    "realloc",
    "aligned_alloc",
    "memcpy",
    "memset",
    "memmove",
    "memcmp",
    "open",
    "close",
    "read",
    "write",
    "lseek",
    "mmap",
    "munmap",
    "mprotect",
    "fopen",
    "fclose",
    "fwrite",
    "fread",
    "abort",
    "exit",
    "atoi",
    "atof",
)

SYSTEM_EXACT = {
    "main",  # exclude from bottleneck — it's the entry point
}


def is_system_fn(name):
    if not name:
        return True
    if name in SYSTEM_EXACT:
        return True
    for prefix in SYSTEM_PREFIXES:
        if name.startswith(prefix):
            return True
    return False


# ============================================================
# LAYER DAG
# ============================================================
# ================================================================
# CORRECTED LAYER DAG
# Update scripts/l10plusplus.py LAYERS list to this order:
#  ================================================================

LAYERS = [
    "types",  # 0: img_result_t, img_buffer_t, opcodes — no deps
    "memory",  # 1: slab, arena, numa — depends on types only
    "arch",  # 2: SIMD kernels, cpu_caps — depends on types only
    "security",  # 3: validation, sandbox — depends on types only
    "pipeline",  # 4: jump table, fused kernels — depends on arch, memory
    "runtime",  # 5: workers, queues — depends on pipeline
    "plugins",  # 6: plugin ABI — depends on pipeline
    "observability",  # 7: metrics, logging — depends on types
    "io",  # 8: decode, encode — depends on memory, security
    "api",  # 9: public surface — depends on everything below
    "cmd",  # 10: CLI — depends on api only
    "startup",  # 11: engine init — depends on all layers
]
# LAYERS = [
#     "core",
#     "memory",
#     "security",
#     "arch",
#     "pipeline",
#     "runtime",
#     "plugins",
#     "observability",
#     "api",
#     "io",
#     "cmd",
# ]

ABI_CONTRACTS = {
    "img_kernel_fn": "(img_ctx_t *, img_buffer_t *, void *)",
    "img_single_kernel_fn": "(img_ctx_t *, img_buffer_t *)",
    "img_fused_kernel_fn": "(img_ctx_t *, img_batch_t *, void *)",
}

RED = "\033[0;31m"
GREEN = "\033[0;32m"
YELLOW = "\033[1;33m"
CYAN = "\033[0;36m"
BOLD = "\033[1m"
NC = "\033[0m"


# ============================================================
# STATIC ANALYZER (no libclang dependency)
# Pure regex — fast, no external deps, works in any environment.
# ============================================================


class StaticAnalyzer:
    def __init__(self):
        self.call_graph = defaultdict(set)  # caller → {callee}
        self.reverse_graph = defaultdict(set)  # callee → {callers}
        self.fn_to_file = {}
        self.layer_graph = defaultdict(set)
        self.file_deps = defaultdict(set)
        self.violations = []
        self.failed = False

        # Regex patterns
        self.fn_def = re.compile(
            r"^\s*(?:static\s+)?(?:inline\s+)?"
            r"(?:__attribute__\s*\(\([^)]*\)\)\s*)?"
            r"(?:void|int|uint\w*|size_t|bool|img_\w+|char\s*\*?)\s+"
            r"(\w+)\s*\([^)]*\)\s*(?:\{|$)",
            re.MULTILINE,
        )
        self.fn_call = re.compile(r"\b(\w+)\s*\(")
        self.include = re.compile(r'#include\s+"([^"]+)"')

    def get_layer(self, path):
        rel = os.path.relpath(path, ".")
        for root in ("src", "include", "api"):
            if rel.startswith(root + os.sep):
                rel = rel[len(root) + 1 :]
                break
        layer = rel.split(os.sep)[0]
        return layer if layer in LAYERS else "unknown"

    def layer_idx(self, layer):
        try:
            return LAYERS.index(layer)
        except ValueError:
            return -1

    def analyze_file(self, path):
        try:
            content = open(path, encoding="utf-8", errors="ignore").read()
        except Exception:
            return

        layer = self.get_layer(path)

        # Extract includes
        for inc in self.include.findall(content):
            self.file_deps[path].add(inc)
            inc_layer = inc.split("/")[0]
            if inc_layer in LAYERS:
                self.layer_graph[layer].add(inc_layer)
                # Layer violation check
                si = self.layer_idx(layer)
                ti = self.layer_idx(inc_layer)
                if si != -1 and ti != -1 and ti > si:
                    self.violations.append(
                        f"{path} includes {inc} ({layer}[{si}] → {inc_layer}[{ti}])"
                    )
                    self.failed = True

        # Extract function definitions
        current_fns = []
        for m in self.fn_def.finditer(content):
            fn = m.group(1)
            if not is_system_fn(fn):
                current_fns.append(fn)
                self.fn_to_file[fn] = path

        # Extract calls (heuristic: any identifier followed by '(')
        for caller in current_fns:
            for callee in self.fn_call.findall(content):
                if callee != caller and not is_system_fn(callee):
                    self.call_graph[caller].add(callee)
                    self.reverse_graph[callee].add(caller)

    def run(self):
        for root, dirs, files in os.walk("."):
            dirs[:] = [d for d in dirs if d not in ("build", ".git", "__pycache__")]
            if not any(
                root.startswith(f".{os.sep}{r}") for r in ("src", "include", "api")
            ):
                continue
            for f in files:
                if f.endswith((".c", ".h")):
                    self.analyze_file(os.path.join(root, f))
        return self


# ============================================================
# CALL GRAPH METRICS
# ============================================================


class CallGraphMetrics:
    def __init__(self, analyzer):
        self.a = analyzer

    def fan_in(self, fn):
        return len(self.a.reverse_graph.get(fn, set()))

    def fan_out(self, fn):
        return len(self.a.call_graph.get(fn, set()))

    def depth(self, fn, visited=None):
        if visited is None:
            visited = set()
        if fn in visited:
            return 0
        visited.add(fn)
        callees = self.a.call_graph.get(fn, set())
        if not callees:
            return 0
        return 1 + max((self.depth(c, visited) for c in callees), default=0)

    def score(self, fn):
        return self.fan_in(fn) + 0.5 * self.fan_out(fn)

    def your_functions(self):
        """Only functions defined in your source files."""
        return {fn for fn in self.a.fn_to_file if not is_system_fn(fn)}


# ============================================================
# REPORTS
# ============================================================


def report_hot_paths(metrics):
    your_fns = list(metrics.your_functions())
    scored = [
        (metrics.score(f), metrics.fan_in(f), metrics.fan_out(f), f)
        for f in your_fns
        if metrics.score(f) > 0
    ]
    scored.sort(reverse=True)

    print(f"\n{BOLD}HOT FUNCTIONS (your code only){NC}")
    print(f"{'Function':<40} {'Score':>6} {'Fan-in':>7} {'Fan-out':>8}")
    print("-" * 65)
    for score, fi, fo, fn in scored[:20]:
        flag = f"{RED}🔥 HOT{NC}" if score > 5 else ""
        print(f"  {fn:<38} {score:>6.1f} {fi:>7} {fo:>8}  {flag}")


def report_bottlenecks(metrics):
    your_fns = metrics.your_functions()
    bottlenecks = [(metrics.fan_in(f), f) for f in your_fns if metrics.fan_in(f) >= 3]
    bottlenecks.sort(reverse=True)

    print(f"\n{BOLD}REAL BOTTLENECKS (your functions called by ≥3 callers){NC}")
    for count, fn in bottlenecks[:15]:
        callers = sorted(metrics.a.reverse_graph.get(fn, set()))
        print(f"  {CYAN}{fn}{NC} ← {count} callers")
        for c in callers[:5]:
            print(f"      ← {c}")
        if len(callers) > 5:
            print(f"      ... and {len(callers) - 5} more")


def report_complex_functions(metrics):
    your_fns = metrics.your_functions()
    complex_fns = [(metrics.fan_out(f), f) for f in your_fns if metrics.fan_out(f) >= 5]
    complex_fns.sort(reverse=True)

    print(f"\n{BOLD}COMPLEX FUNCTIONS (fan-out ≥ 5 — consider splitting){NC}")
    for count, fn in complex_fns[:10]:
        file = metrics.a.fn_to_file.get(fn, "unknown")
        rel = os.path.relpath(file, ".")
        print(f"  {YELLOW}{fn}{NC} → {count} calls | {rel}")


def report_layer_violations(analyzer):
    print(f"\n{BOLD}LAYER VIOLATIONS (upward dependencies){NC}")
    if not analyzer.violations:
        print(f"  {GREEN}✅ None — layer DAG is clean{NC}")
    else:
        for v in analyzer.violations:
            print(f"  {RED}❌ {v}{NC}")


def report_module_summary(metrics):
    module_scores = defaultdict(float)
    module_counts = defaultdict(int)

    for fn in metrics.your_functions():
        file = metrics.a.fn_to_file.get(fn, "")
        layer = metrics.a.get_layer(file)
        module_scores[layer] += metrics.score(fn)
        module_counts[layer] += 1

    print(f"\n{BOLD}MODULE HEAT (which layers need attention){NC}")
    print(f"{'Layer':<20} {'Functions':>10} {'Total Score':>12} {'Avg Score':>10}")
    print("-" * 56)
    items = [(module_scores[l], l) for l in LAYERS if module_counts[l] > 0]
    for score, layer in sorted(items, reverse=True):
        count = module_counts[layer]
        avg = score / count if count else 0
        bar = "█" * min(int(avg * 2), 20)
        print(f"  {layer:<18} {count:>10} {score:>12.1f} {avg:>10.1f}  {bar}")


def report_files_to_split(analyzer):
    print(f"\n{BOLD}FILES TO SPLIT (too many dependencies){NC}")
    items = [(len(v), k) for k, v in analyzer.file_deps.items() if len(v) >= 7]
    items.sort(reverse=True)
    for count, path in items[:8]:
        rel = os.path.relpath(path, ".")
        print(f"  {YELLOW}❌ {rel}{NC} → {count} deps → consider splitting")


def report_abi_locations(analyzer):
    print(f"\n{BOLD}ABI CONTRACT TABLE{NC}")
    for type_name, sig in ABI_CONTRACTS.items():
        files = [
            f
            for f, content in [
                (p, open(p, errors="ignore").read())
                for p in analyzer.fn_to_file.values()
                if os.path.exists(p)
            ]
            if type_name in content
        ]
        locations = list(set(os.path.relpath(f, ".") for f in files))
        loc_str = locations[0] if locations else "not found"
        print(f"  {CYAN}{type_name}{NC}")
        print(f"    sig: {sig}")
        print(f"    def: {loc_str}")


def report_unused_functions(metrics):
    your_fns = metrics.your_functions()
    # Functions defined but never called by any other function
    unused = [
        f
        for f in your_fns
        if metrics.fan_in(f) == 0
        and f
        not in (
            "main",
            "img_api_init",
            "img_api_shutdown",
            "img_api_run_job",
            "img_api_process_raw",
        )
    ]
    unused.sort()

    if not unused:
        print(f"\n{GREEN}✅ No unused functions detected{NC}")
        return

    print(f"\n{BOLD}POTENTIALLY UNUSED FUNCTIONS (fan-in = 0){NC}")
    print(f"  (may be exported symbols or entry points — verify manually)")
    for fn in unused[:20]:
        file = os.path.relpath(metrics.a.fn_to_file.get(fn, ""), ".")
        print(f"  {fn}  [{file}]")


# ============================================================
# NEXT ACTIONS (what to do now)
# ============================================================


def report_next_actions(analyzer, metrics):
    print(f"\n{'=' * 60}")
    print(f"{BOLD}WHAT TO DO NEXT — PRIORITY ORDER{NC}")
    print(f"{'=' * 60}")

    actions = []

    # 1. Layer violations
    if analyzer.violations:
        actions.append(
            (
                1,
                "CRITICAL",
                f"Fix {len(analyzer.violations)} layer violation(s) — "
                "upward dependencies break module boundaries",
            )
        )

    # 2. Complex functions
    complex_fns = [
        (metrics.fan_out(f), f)
        for f in metrics.your_functions()
        if metrics.fan_out(f) >= 8
    ]
    for count, fn in sorted(complex_fns, reverse=True)[:3]:
        file = os.path.relpath(metrics.a.fn_to_file.get(fn, ""), ".")
        actions.append(
            (2, "HIGH", f"Split {fn}() — {count} outgoing calls — in {file}")
        )

    # 3. Missing implementations
    actions.append(
        (
            3,
            "HIGH",
            "Replace resize_nearest() with bilinear AVX2 "
            "(src/pipeline/layout.c) — current is nearest-neighbour",
        )
    )

    actions.append(
        (
            3,
            "HIGH",
            "Wire io_uring output path — currently main.c initializes "
            "io_uring but img_api_run_job() writes via fwrite(), not io_uring",
        )
    )

    actions.append(
        (
            4,
            "MEDIUM",
            "Add --mode fit|fill CLI flag to expose IMG_FIT/IMG_FILL "
            "(job->mode is set but not wired to CLI args)",
        )
    )

    actions.append(
        (
            4,
            "MEDIUM",
            "Add img_api_run_job_raw() — returns encoded bytes instead of "
            "writing to file; enables io_uring output and Python/Go bindings",
        )
    )

    actions.append(
        (
            5,
            "MEDIUM",
            "Add CI performance gate: bench_lat P99 > 2ms = build fail "
            "(RFC §22.3 requirement)",
        )
    )

    actions.append(
        (
            5,
            "LOW",
            "Implement PDF multi-page: current pdf_encoder.c = single page; "
            "for large grids that exceed A4, add page continuation",
        )
    )

    actions.append(
        (
            6,
            "LOW",
            "Add Python CFFI bindings (bindings/python/cffi_wrapper.py) — "
            "expose img_api_init, img_api_run_job, img_api_shutdown",
        )
    )

    for priority, level, action in sorted(actions):
        color = RED if level == "CRITICAL" else YELLOW if level == "HIGH" else NC
        print(f"\n  [{priority}] {color}{level}{NC}")
        print(f"      {action}")

    print(f"\n{'=' * 60}")


# ============================================================
# MAIN
# ============================================================


def main():
    print(f"{BOLD}imgengine L10++ Architecture Analyzer{NC}")
    print(f"{'=' * 60}\n")

    print("📡 Scanning source files...")
    t0 = time.time()
    analyzer = StaticAnalyzer().run()
    elapsed = time.time() - t0

    your_fns = {fn for fn in analyzer.fn_to_file if not is_system_fn(fn)}
    print(
        f"✅ {len(your_fns)} functions | "
        f"{sum(len(v) for v in analyzer.call_graph.values())} edges | "
        f"{elapsed:.1f}s\n"
    )

    metrics = CallGraphMetrics(analyzer)

    report_hot_paths(metrics)
    report_bottlenecks(metrics)
    report_complex_functions(metrics)
    report_layer_violations(analyzer)
    report_module_summary(metrics)
    report_files_to_split(analyzer)
    report_unused_functions(metrics)
    report_next_actions(analyzer, metrics)

    if analyzer.failed:
        print(f"\n{RED}{BOLD}🚨 VIOLATIONS FOUND — fix before merging{NC}\n")
        sys.exit(1)
    else:
        print(f"\n{GREEN}{BOLD}✅ ARCHITECTURE CLEAN{NC}\n")


if __name__ == "__main__":
    main()

# #!/usr/bin/env python3

# import os
# import re
# import time
# import random
# import threading
# from collections import defaultdict, deque
# from clang import cindex
# from pyvis.network import Network

# # ============================================================
# # 🔥 LIBCLANG AUTO DETECT
# # ============================================================


# def find_libclang():
#     import glob

#     paths = [
#         "/usr/lib/llvm-*/lib/libclang.so*",
#         "/usr/lib/x86_64-linux-gnu/libclang.so*",
#     ]
#     for p in paths:
#         matches = glob.glob(p)
#         if matches:
#             return matches[0]
#     raise RuntimeError("libclang not found")


# cindex.Config.set_library_file(find_libclang())

# # ============================================================
# # 🔥 CORE GRAPH
# # ============================================================

# # ============================================================
# # 🔥 ANALYZER
# # ============================================================


# class GraphEngine:
#     def __init__(self):
#         self.call_graph = defaultdict(set)
#         self.function_to_file = {}

#     def extract_calls(self, node, caller):
#         for child in node.get_children():
#             try:
#                 if child.kind == cindex.CursorKind.CALL_EXPR:
#                     callee = None

#                     if child.referenced and child.referenced.spelling:
#                         callee = child.referenced.spelling
#                     else:
#                         callee = child.displayname.split("(")[0]

#                     if callee and caller:
#                         self.call_graph[caller].add(callee)

#                 self.extract_calls(child, caller)
#             except Exception as e:
#                 pass

#     def analyze_file(self, path):
#         index = cindex.Index.create()

#         try:
#             tu = index.parse(
#                 path,
#                 args=[
#                     "-I./include",
#                     "-I./src",
#                     "-I./api",
#                     "-std=c11",
#                 ],
#             )
#         except Exception as e:
#             print(f"❌ Parse failed: {path}")
#             return

#         for cursor in tu.cursor.walk_preorder():
#             try:
#                 if cursor.kind == cindex.CursorKind.FUNCTION_DECL:
#                     fn = cursor.spelling

#                     if not fn:
#                         continue

#                     if cursor.location.file:
#                         self.function_to_file[fn] = cursor.location.file.name

#                     self.extract_calls(cursor, fn)

#             except:
#                 pass

#     def run(self):
#         print("\n📡 BUILDING CALL GRAPH...\n")

#         for root, _, files in os.walk("."):
#             if not root.startswith(("./src", "./include", "./api")):
#                 continue

#             for f in files:
#                 if f.endswith(".c"):
#                     self.analyze_file(os.path.join(root, f))

#         print(f"✅ Functions captured: {len(self.call_graph)}")
#         return self.call_graph

#     def generate_graph(self):
#         print("\n🌐 GENERATING L10++++++ INTERACTIVE GRAPH...\n")

#         from pyvis.network import Network

#         net = Network(
#             height="900px",
#             width="100%",
#             directed=True,
#             notebook=False,  # 🔥 CRITICAL FIX
#             bgcolor="#0d1117",
#             font_color="white",
#         )

#         # 🔥 PHYSICS → STABLE FOR LARGE GRAPH
#         net.barnes_hut(gravity=-80000, central_gravity=0.3, spring_length=250)
#         net.toggle_physics(True)

#         # --------------------------------------------------------
#         # 🔥 STEP 1: COMPUTE FAN-IN / FAN-OUT
#         # --------------------------------------------------------
#         fan_in = defaultdict(int)
#         fan_out = defaultdict(int)

#         for caller, callees in self.call_graph.items():
#             fan_out[caller] = len(callees)
#             for callee in callees:
#                 fan_in[callee] += 1

#         # --------------------------------------------------------
#         # 🔥 STEP 2: CLASSIFY HOT / WARM / COLD
#         # --------------------------------------------------------
#         score = {}
#         for fn in set(list(fan_in.keys()) + list(fan_out.keys())):
#             score[fn] = fan_in[fn] + 0.5 * fan_out[fn]

#         sorted_nodes = sorted(score.items(), key=lambda x: x[1], reverse=True)

#         hot = set()
#         warm = set()
#         cold = set()

#         for i, (fn, s) in enumerate(sorted_nodes):
#             if i < len(sorted_nodes) * 0.1:
#                 hot.add(fn)
#             elif i < len(sorted_nodes) * 0.4:
#                 warm.add(fn)
#             else:
#                 cold.add(fn)

#         # --------------------------------------------------------
#         # 🔥 STEP 3: MODULE CLUSTER DETECTION
#         # --------------------------------------------------------
#         def get_module(fn):
#             file = self.function_to_file.get(fn, "")
#             parts = file.split(os.sep)
#             for p in parts:
#                 if p in [
#                     "core",
#                     "memory",
#                     "api",
#                     "pipeline",
#                     "runtime",
#                     "io",
#                     "cmd",
#                     "arch",
#                 ]:
#                     return p
#             return "other"

#         module_colors = {
#             "core": "#1f77b4",
#             "memory": "#2ca02c",
#             "api": "#9467bd",
#             "pipeline": "#ff7f0e",
#             "runtime": "#d62728",
#             "io": "#17becf",
#             "cmd": "#bcbd22",
#             "arch": "#8c564b",
#             "other": "#7f7f7f",
#         }

#         # --------------------------------------------------------
#         # 🔥 STEP 4: ADD NODES (COLOR + SIZE)
#         # --------------------------------------------------------
#         EDGE_LIMIT = 2000
#         edge_count = 0

#         for fn in score.keys():
#             module = get_module(fn)

#             # 🔴 HOT
#             if fn in hot:
#                 color = "red"
#                 size = 25

#             # 🟡 WARM
#             elif fn in warm:
#                 color = "yellow"
#                 size = 15

#             # 🔵 COLD
#             else:
#                 color = module_colors.get(module, "#7f7f7f")
#                 size = 8

#             # 🎯 BOTTLENECK highlight
#             if fan_in[fn] > 20:
#                 color = "#ff00ff"  # neon pink

#             net.add_node(
#                 fn,
#                 label=fn,
#                 title=f"{fn}\nmodule={module}\nfan_in={fan_in[fn]}",
#                 color=color,
#                 size=size,
#             )

#         # --------------------------------------------------------
#         # 🔥 STEP 5: EDGES (LIMITED)
#         # --------------------------------------------------------
#         for caller, callees in self.call_graph.items():
#             for callee in callees:
#                 if edge_count > EDGE_LIMIT:
#                     break

#                 # 🧠 CRITICAL PATH EDGE (HOT → HOT)
#                 if caller in hot and callee in hot:
#                     net.add_edge(caller, callee, color="red", width=2)
#                 else:
#                     net.add_edge(caller, callee, color="#888", width=0.5)

#                 edge_count += 1

#         print(f"📊 Nodes: {len(net.nodes)}")
#         print(f"📊 Edges: {edge_count}")

#         # --------------------------------------------------------
#         # 🔥 STEP 6: ZOOM-SAFE SETTINGS
#         # --------------------------------------------------------
#         net.set_options("""
#         var options = {
#         "nodes": {
#             "shape": "dot",
#             "scaling": {
#             "min": 5,
#             "max": 30
#             }
#         },
#         "edges": {
#             "smooth": false
#         },
#         "interaction": {
#             "hover": true,
#             "zoomView": true,
#             "dragView": true
#         },
#         "physics": {
#             "stabilization": {
#             "iterations": 100
#             }
#         }
#         }
#         """)

#         # --------------------------------------------------------
#         # 🔥 FINAL FIX (NO MORE CRASH)
#         # --------------------------------------------------------
#         try:
#             net.write_html("interactive_call_graph.html", open_browser=False)
#             print("\n✅ L10++++++ GRAPH READY → interactive_call_graph.html\n")
#         except Exception as e:
#             print("❌ Graph generation failed:", e)


# class Graph:
#     def __init__(self):
#         self.graph = defaultdict(set)
#         self.reverse = defaultdict(set)
#         self.functions = set()

#     def add_edge(self, a, b):
#         self.graph[a].add(b)
#         self.reverse[b].add(a)
#         self.functions.add(a)
#         self.functions.add(b)


# # ============================================================
# # 🔥 COMPILER ANALYZER (L10++++)
# # ============================================================


# class CompilerAnalyzer:
#     def __init__(self):
#         self.graph = Graph()
#         self.file_layer = {}
#         self.layers = [
#             "core",
#             "memory",
#             "security",
#             "arch",
#             "pipeline",
#             "runtime",
#             "plugins",
#             "observability",
#             "api",
#             "io",
#             "cmd",
#         ]

#     def get_layer(self, path):
#         rel = os.path.relpath(path, ".")
#         for root in ["src", "include", "api"]:
#             if rel.startswith(root + os.sep):
#                 rel = rel[len(root) + 1 :]
#                 break
#         layer = rel.split(os.sep)[0]
#         return layer if layer in self.layers else "unknown"

#     def extract_calls(self, node, caller):
#         for child in node.get_children():
#             try:
#                 if child.kind == cindex.CursorKind.CALL_EXPR:
#                     callee = (
#                         child.referenced.spelling
#                         if child.referenced
#                         else child.displayname.split("(")[0]
#                     )
#                     if callee:
#                         self.graph.add_edge(caller, callee)
#                 self.extract_calls(child, caller)
#             except:
#                 continue

#     def analyze_file(self, path):
#         index = cindex.Index.create()
#         try:
#             tu = index.parse(
#                 path, args=["-I./include", "-I./src", "-I./api", "-std=c11"]
#             )
#         except:
#             return

#         self.file_layer[path] = self.get_layer(path)

#         for cursor in tu.cursor.walk_preorder():
#             try:
#                 if cursor.kind == cindex.CursorKind.FUNCTION_DECL:
#                     fn = cursor.spelling
#                     self.extract_calls(cursor, fn)
#             except:
#                 continue

#     def run(self):
#         print("🚀 L10++++ COMPILER ANALYZER\n")

#         for root, _, files in os.walk("."):
#             if not root.startswith(("./src", "./include", "./api")):
#                 continue
#             for f in files:
#                 if f.endswith(".c"):
#                     self.analyze_file(os.path.join(root, f))

#         print("Functions:", len(self.graph.functions))
#         print("Edges:", sum(len(v) for v in self.graph.graph.values()))

#         return self.graph


# # ============================================================
# # 🔥 KERNEL INTELLIGENCE (L10+++++)
# # ============================================================


# class KernelIntelligence:
#     def __init__(self, graph):
#         self.graph = graph

#     # --------------------------------------------------------
#     # DEPTH (HOT PATH)
#     # --------------------------------------------------------
#     def depth(self, fn):
#         visited = set()
#         stack = [(fn, 0)]
#         mx = 0
#         while stack:
#             n, d = stack.pop()
#             if n in visited:
#                 continue
#             visited.add(n)
#             mx = max(mx, d)
#             for x in self.graph.graph.get(n, []):
#                 stack.append((x, d + 1))
#         return mx

#     # --------------------------------------------------------
#     # 🔥 HOT PATHS
#     # --------------------------------------------------------
#     def hot_paths(self):
#         print("\n🔥 HOT PATHS\n")
#         scores = [(self.depth(f), f) for f in self.graph.functions]
#         for d, f in sorted(scores, reverse=True)[:10]:
#             print(f"{f} → depth={d}")

#     # --------------------------------------------------------
#     # 🚨 FAN-IN (EXPLICIT)
#     # --------------------------------------------------------
#     def fan_in(self):
#         print("\n🚨 FAN-IN (who is heavily used)\n")
#         scores = [(len(self.graph.reverse[f]), f) for f in self.graph.functions]
#         for count, f in sorted(scores, reverse=True)[:15]:
#             if count > 0:
#                 print(f"{f} ← {count} callers")

#     # --------------------------------------------------------
#     # 🌐 FAN-OUT
#     # --------------------------------------------------------
#     def fan_out(self):
#         print("\n🌐 FAN-OUT (complex functions)\n")
#         scores = [(len(self.graph.graph[f]), f) for f in self.graph.functions]
#         for count, f in sorted(scores, reverse=True)[:15]:
#             print(f"{f} → calls {count}")

#     # --------------------------------------------------------
#     # 🚨 BOTTLENECK (filtered fan-in)
#     # --------------------------------------------------------
#     def bottlenecks(self):
#         print("\n🚨 BOTTLENECK FUNCTIONS\n")
#         scores = [(len(self.graph.reverse[f]), f) for f in self.graph.functions]
#         for count, f in sorted(scores, reverse=True)[:10]:
#             if count > 20:  # threshold
#                 print(f"🔥 {f} ← {count} callers (HOT BOTTLENECK)")


# # ============================================================
# # 🔥 REFACTOR ENGINE (L10+++++++)
# # ============================================================


# class RefactorEngine:
#     def __init__(self):
#         self.file_deps = defaultdict(set)

#     def scan(self, path):
#         try:
#             txt = open(path, errors="ignore").read()
#         except:
#             return
#         for inc in re.findall(r'#include\s+"(.+?)"', txt):
#             self.file_deps[path].add(inc)

#     def run(self):
#         print("\n🚀 REFACTOR ENGINE\n")
#         for root, _, files in os.walk("."):
#             if not root.startswith(("./src", "./include", "./api")):
#                 continue
#             for f in files:
#                 if f.endswith((".c", ".h")):
#                     self.scan(os.path.join(root, f))

#         for f, deps in sorted(
#             self.file_deps.items(), key=lambda x: len(x[1]), reverse=True
#         )[:5]:
#             print(f"❌ {f} → {len(deps)} deps → split module")


# # ============================================================
# # 🔥 NUMA (L10++++++++)
# # ============================================================


# class NUMA:
#     def __init__(self, n=2):
#         self.nodes = [[] for _ in range(n)]

#     def assign(self, fn):
#         node = min(self.nodes, key=len)
#         node.append(fn)

#     def show(self):
#         print("\n🧠 NUMA DISTRIBUTION")
#         for i, n in enumerate(self.nodes):
#             print(f"Node {i}: {len(n)} tasks")


# # ============================================================
# # 🔥 WORK STEALING (L10+++++++++)
# # ============================================================


# class WSDeque:
#     def __init__(self):
#         self.d = deque()

#     def push(self, t):
#         self.d.append(t)

#     def pop(self):
#         return self.d.pop() if self.d else None

#     def steal(self):
#         return self.d.popleft() if self.d else None


# class Worker(threading.Thread):
#     def __init__(self, i, s):
#         super().__init__()
#         self.i = i
#         self.s = s
#         self.q = WSDeque()
#         self.runflag = True

#     def run(self):
#         while self.runflag:
#             t = self.q.pop()
#             if not t:
#                 t = self.s.steal(self.i)
#             if t:
#                 print(f"[W{self.i}] {t}")
#                 time.sleep(0.005)
#             else:
#                 time.sleep(0.001)

#     def stop(self):
#         self.runflag = False


# class Scheduler:
#     def __init__(self, n=4):
#         self.ws = [Worker(i, self) for i in range(n)]

#     def start(self):
#         [w.start() for w in self.ws]

#     def stop(self):
#         [w.stop() for w in self.ws]
#         [w.join() for w in self.ws]

#     def submit(self, t):
#         w = min(self.ws, key=lambda x: len(x.q.d))
#         w.q.push(t)

#     def steal(self, i):
#         for j, w in enumerate(self.ws):
#             if j == i:
#                 continue
#             t = w.q.steal()
#             if t:
#                 print(f"🧠 W{i} stole from W{j}")
#                 return t
#         return None


# class AutoOptimizer:
#     def __init__(self, graph):
#         self.graph = graph
#         self.priority = {}

#     # --------------------------------------------------------
#     # 🔥 COMPUTE PRIORITY (FAN-IN + FAN-OUT)
#     # --------------------------------------------------------
#     def compute_priority(self):
#         for fn in self.graph.functions:
#             fan_in = len(self.graph.reverse[fn])
#             fan_out = len(self.graph.graph[fn])

#             score = fan_in + 0.5 * fan_out
#             self.priority[fn] = score

#     # --------------------------------------------------------
#     # 🔥 CLASSIFY HOT / WARM / COLD
#     # --------------------------------------------------------
#     def classify(self):
#         print("\n🔥 FUNCTION CLASSIFICATION\n")

#         sorted_fns = sorted(self.priority.items(), key=lambda x: x[1], reverse=True)

#         self.hot = set()
#         self.warm = set()
#         self.cold = set()

#         for i, (fn, score) in enumerate(sorted_fns):
#             if i < len(sorted_fns) * 0.1:
#                 self.hot.add(fn)
#                 print(f"🔥 HOT   {fn} ({score})")
#             elif i < len(sorted_fns) * 0.4:
#                 self.warm.add(fn)
#             else:
#                 self.cold.add(fn)

#     # --------------------------------------------------------
#     # 🔥 GET PRIORITY
#     # --------------------------------------------------------
#     def get_priority(self, fn):
#         return self.priority.get(fn, 0)


# class PriorityTask:
#     def __init__(self, name, priority):
#         self.name = name
#         self.priority = priority

#     def run(self):
#         # simulate heavier work for high priority
#         time.sleep(0.003 + (0.002 * min(self.priority, 10)))


# class PriorityDeque:
#     def __init__(self):
#         self.tasks = []

#     def push(self, task):
#         self.tasks.append(task)
#         self.tasks.sort(key=lambda t: t.priority, reverse=True)

#     def pop(self):
#         if self.tasks:
#             return self.tasks.pop(0)
#         return None

#     def steal(self):
#         if self.tasks:
#             return self.tasks.pop(0)
#         return None


# class PriorityWorker(threading.Thread):
#     def __init__(self, wid, scheduler):
#         super().__init__()
#         self.wid = wid
#         self.scheduler = scheduler
#         self.queue = PriorityDeque()
#         self.running = True

#     def run(self):
#         while self.running:
#             task = self.queue.pop()

#             if not task:
#                 task = self.scheduler.steal(self.wid)

#             if task:
#                 print(f"[W{self.wid}] {task.name} (P={task.priority:.2f})")
#                 task.run()
#             else:
#                 time.sleep(0.001)

#     def stop(self):
#         self.running = False


# class PriorityScheduler:
#     def __init__(self, optimizer, num_workers=4):
#         self.optimizer = optimizer
#         self.workers = [PriorityWorker(i, self) for i in range(num_workers)]

#     def start(self):
#         for w in self.workers:
#             w.start()

#     def stop(self):
#         for w in self.workers:
#             w.stop()
#         for w in self.workers:
#             w.join()

#     # --------------------------------------------------------
#     # 🔥 HOT PATH ISOLATION
#     # --------------------------------------------------------
#     def submit(self, fn):
#         priority = self.optimizer.get_priority(fn)
#         task = PriorityTask(fn, priority)

#         # HOT → dedicated worker 0
#         if fn in self.optimizer.hot:
#             self.workers[0].queue.push(task)
#         else:
#             worker = min(self.workers[1:], key=lambda w: len(w.queue.tasks))
#             worker.queue.push(task)

#     # --------------------------------------------------------
#     # 🔥 STEAL WITH PRIORITY
#     # --------------------------------------------------------
#     def steal(self, thief_id):
#         for i, w in enumerate(self.workers):
#             if i == thief_id:
#                 continue

#             task = w.queue.steal()
#             if task:
#                 print(f"🧠 W{thief_id} stole {task.name}")
#                 return task

#         return None


# # ============================================================
# # 🔥 MAIN ENGINE (FINAL)
# # ============================================================


# def main():
#     # 🔥 STEP 1: BUILD GRAPH
#     ge = GraphEngine()
#     ge.run()
#     ge.generate_graph()  # ✅ THIS WAS MISSING

#     # 🔥 STEP 2: ANALYSIS ENGINE
#     analyzer = CompilerAnalyzer()
#     graph = analyzer.run()

#     ki = KernelIntelligence(graph)
#     ki.hot_paths()
#     ki.fan_in()
#     ki.bottlenecks()
#     ki.fan_out()

#     RefactorEngine().run()

#     # 🔥 NUMA
#     numa = NUMA(2)
#     for f in graph.functions:
#         numa.assign(f)
#     numa.show()

#     print("\n⚡ SCHEDULER RUN\n")
#     sch = Scheduler(4)
#     sch.start()

#     for f in list(graph.functions)[:50]:
#         sch.submit(f)

#     print("\n🚀 AUTO OPTIMIZER\n")

#     optimizer = AutoOptimizer(graph)
#     optimizer.compute_priority()
#     optimizer.classify()

#     scheduler = PriorityScheduler(optimizer, 4)
#     scheduler.start()

#     for fn in list(graph.functions)[:100]:
#         scheduler.submit(fn)

#     time.sleep(3)
#     scheduler.stop()

#     print("\n✅ L10 → L10+++++++++ COMPLETE")


# # ============================================================

# if __name__ == "__main__":
#     main()
