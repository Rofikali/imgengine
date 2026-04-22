# #!/usr/bin/env python3
# import os
# import re

# # =========================================================
# # 1. THE KERNEL TRUTH: ALLOWED / FORBIDDEN MATRIX
# # =========================================================
# # Row = Source | Col = Destination
# LAYER_NAMES = [
#     "core",
#     "memory",
#     "arch",
#     "pipeline",
#     "runtime",
#     "api",
#     "cmd",
#     "io",
#     "plugins",
#     "observability",
#     "security",
# ]

# MATRIX_DISPLAY = """
# FROM \ TO     core  mem   arch  pipe  run   api   cmd   io    plug  obs   sec
# -----------------------------------------------------------------------------
# core           —     ❌    ❌    ❌    ❌    ❌    ❌    ❌    ❌    ❌    ❌
# memory         ✓     —     ❌    ❌    ❌    ❌    ❌    ❌    ❌    ⚠️    ❌
# arch           ✓     ✓     —     ❌    ❌    ❌    ❌    ❌    ❌    ❌    ❌
# pipeline       ✓     ✓     ✓     —     ❌    ❌    ❌    ❌    ❌    ✓     ❌
# runtime        ✓     ✓     ❌    ✓     —     ❌    ❌    ⚠️    ✓     ✓     ✓
# api            ✓     ❌    ❌    ❌    ✓     —     ❌    ✓     ✓     ✓     ✓
# cmd            ✓     ❌    ❌    ❌    ❌    ✓     —     ❌    ❌    ❌    ❌
# io             ✓     ✓     ❌    ❌    ❌    ❌    ❌    —     ❌    ❌    ⚠️
# plugins        ✓     ❌    ❌    ✓     ✓     ❌    ❌    ❌    —     ❌    ❌
# observability  ✓     ⚠️    ❌    ❌    ❌    ❌    ❌    ❌    ❌    —     ❌
# security       ✓     ✓     ❌    ❌    ❌    ❌    ❌    ⚠️    ❌    ❌    —
# """

# MATRIX_LOGIC = {
#     "core": {"core": "OK"},
#     "memory": {"core": "OK", "memory": "OK", "observability": "WARN"},
#     "arch": {"core": "OK", "memory": "OK", "arch": "OK"},
#     "pipeline": {
#         "core": "OK",
#         "memory": "OK",
#         "arch": "OK",
#         "pipeline": "OK",
#         "observability": "OK",
#     },
#     "runtime": {
#         "core": "OK",
#         "memory": "OK",
#         "pipeline": "OK",
#         "runtime": "OK",
#         "io": "WARN",
#         "plugins": "OK",
#         "observability": "OK",
#         "security": "OK",
#     },
#     "api": {
#         "core": "OK",
#         "runtime": "OK",
#         "api": "OK",
#         "io": "OK",
#         "plugins": "OK",
#         "observability": "OK",
#         "security": "OK",
#     },
#     "cmd": {"core": "OK", "api": "OK", "cmd": "OK"},
#     "io": {"core": "OK", "memory": "OK", "io": "OK", "security": "WARN"},
#     "plugins": {"core": "OK", "pipeline": "OK", "runtime": "OK", "plugins": "OK"},
#     "observability": {"core": "OK", "memory": "WARN", "observability": "OK"},
#     "security": {"core": "OK", "memory": "OK", "io": "WARN", "security": "OK"},
# }

# # =========================================================
# # 2. SCANNER UTILS
# # =========================================================


# def detect_layer(path):
#     parts = path.replace("\\", "/").split("/")
#     for i, p in enumerate(parts):
#         if p in ["src", "include", "api"]:
#             if i + 1 < len(parts):
#                 layer = parts[i + 1]
#                 # Map 'mem' shorthand to 'memory' if necessary
#                 if layer == "mem":
#                     layer = "memory"
#                 return layer if layer in LAYER_NAMES else None
#     return None


# def is_hot(path):
#     return any(k in path for k in ["/hot/", "/arch/", "pipeline_exec", "avx", "fused"])


# # =========================================================
# # 3. THE ANALYZER
# # =========================================================


# class KernelAnalyzer:
#     def __init__(self):
#         self.files = []
#         self.errors = []  # Forbidden (❌)
#         self.warns = []  # Dangerous (⚠️)
#         self.ok_count = 0

#     def run(self):
#         print(f"\n{'=' * 80}\nFINAL ALLOWED / FORBIDDEN MATRIX\n{'=' * 80}")
#         print(MATRIX_DISPLAY)

#         for base, _, fs in os.walk("."):
#             for f in fs:
#                 if f.endswith((".c", ".h")):
#                     path = os.path.join(base, f).replace("\\", "/")
#                     self.process_file(path)

#     def process_file(self, path):
#         src = detect_layer(path)
#         if not src:
#             return

#         try:
#             with open(path, "r", errors="ignore") as f:
#                 for line in f:
#                     m = re.search(r'#include\s+["<]([^">]+)[">]', line)
#                     if m:
#                         inc = m.group(1)
#                         dst = None
#                         for l in LAYER_NAMES:
#                             if f"{l}/" in inc or inc.startswith(f"img_{l}"):
#                                 dst = l
#                                 break

#                         if dst:
#                             status = MATRIX_LOGIC.get(src, {}).get(dst, "ERR")
#                             res = {
#                                 "file": path,
#                                 "src": src,
#                                 "dst": dst,
#                                 "inc": inc,
#                                 "hot": is_hot(path),
#                             }

#                             if status == "ERR":
#                                 self.errors.append(res)
#                             elif status == "WARN":
#                                 self.warns.append(res)
#                             else:
#                                 self.ok_count += 1
#         except:
#             pass

#     def report(self):
#         print(f"\n{'=' * 80}\nKERNEL SCAN RESULTS\n{'=' * 80}")
#         print(f"ACCEPTED EDGES: {self.ok_count}")
#         print(f"REJECTED (ERRORS): {len(self.errors)}")
#         print(f"WARNINGS: {len(self.warns)}")

#         if self.errors:
#             print("\n❌ FORBIDDEN DEPENDENCIES (REJECTED):")
#             for e in self.errors:
#                 prio = " [CRITICAL-HOT]" if e["hot"] else ""
#                 print(
#                     f" - {e['file']}{prio}\n   REJECTED: [{e['src']}] cannot include [{e['dst']}] (via {e['inc']})"
#                 )

#         if self.warns:
#             print("\n⚠️ WARNINGS (NON-OPTIMAL):")
#             for w in self.warns:
#                 print(
#                     f" - {w['file']}\n   CAUTION: [{w['src']}] -> [{w['dst']}] (Verify for side-effects)"
#                 )

#         final_status = (
#             "🎯 STATUS: CLEAN (KERNEL-GRADE)"
#             if not self.errors
#             else "🚨 STATUS: VIOLATIONS PRESENT"
#         )
#         print(f"\n{'=' * 80}\n{final_status}\n{'=' * 80}\n")


# if __name__ == "__main__":
#     analyzer = KernelAnalyzer()
#     analyzer.run()
#     analyzer.report()


#!/usr/bin/env python3
import collections
import os
import re

# =========================================================
# 🔥 KERNEL FLOW (STRICT — DO NOT BREAK)
# =========================================================

FLOW = """
KERNEL FLOW:
cmd → api → runtime → pipeline → arch → memory → core

RULES:
- Only downward dependencies
- pipeline = PURE (NO malloc / IO / plugins / syscalls)
- runtime = controls execution ONLY
- api = contract boundary
"""

SIDE_SYSTEMS = """
SIDE SYSTEM RULES (KERNEL-GRADE):

security      → BEFORE api/runtime ONLY
io            → ONLY api layer (no runtime direct usage)
plugins       → runtime-controlled ONLY (never pipeline)
observability → passive recorder (never influences flow)
startup       → ignored (allowed to touch everything)
"""

FLOW_DISPLAY = r"""
          USER
           │
           ▼
         cmd
           │
           ▼
         api
           │
           ▼
       runtime
           │
           ▼
       pipeline
           │
           ▼
         arch
           │
           ▼
        memory
           │
           ▼
          core
"""

# =========================================================
# LAYERS
# =========================================================

LAYERS = [
    "core",
    "memory",
    "arch",
    "pipeline",
    "runtime",
    "api",
    "cmd",
    "io",
    "plugins",
    "observability",
    "security",
]


def detect_layer(path):
    if "startup" in path:
        return "startup"
    if "/hot/" in path.replace("\\", "/"):
        return "pipeline"
    if "/cold/" in path.replace("\\", "/"):
        return "pipeline"
    parts = path.replace("\\", "/").split("/")
    for p in parts:
        if p in LAYERS:
            return p
    return None


# =========================================================
# MATRIX (STRICT)
# =========================================================


def build_matrix():
    m = {l: {x: "ERR" for x in LAYERS} for l in LAYERS}

    for l in LAYERS:
        m[l][l] = "OK"

    # core
    # nothing

    # memory
    m["memory"]["core"] = "OK"

    # arch
    m["arch"]["core"] = "OK"
    m["arch"]["memory"] = "OK"

    # pipeline
    m["pipeline"]["core"] = "OK"
    m["pipeline"]["memory"] = "OK"
    m["pipeline"]["arch"] = "OK"
    m["pipeline"]["observability"] = "OK"

    # runtime
    m["runtime"]["core"] = "OK"
    m["runtime"]["memory"] = "OK"
    m["runtime"]["pipeline"] = "OK"
    m["runtime"]["plugins"] = "OK"
    m["runtime"]["observability"] = "OK"
    m["runtime"]["security"] = "OK"
    m["runtime"]["io"] = "WARN"  # will be further restricted below

    # api
    m["api"]["core"] = "OK"
    m["api"]["runtime"] = "OK"
    m["api"]["io"] = "OK"
    m["api"]["plugins"] = "OK"
    m["api"]["observability"] = "OK"
    m["api"]["security"] = "OK"

    # cmd
    m["cmd"]["core"] = "OK"
    m["cmd"]["api"] = "OK"

    # io
    m["io"]["core"] = "OK"
    m["io"]["memory"] = "OK"

    # plugins
    m["plugins"]["core"] = "OK"
    m["plugins"]["pipeline"] = "OK"
    m["plugins"]["runtime"] = "OK"

    # observability
    m["observability"]["core"] = "OK"
    m["observability"]["memory"] = "WARN"

    # security
    m["security"]["core"] = "OK"
    m["security"]["memory"] = "OK"

    return m


MATRIX = build_matrix()

# =========================================================
# INCLUDE PARSER
# =========================================================

INCLUDE_RE = re.compile(r'#include\s+"([^"]+)"')
CALL_RE = re.compile(r"\b([A-Za-z_]\w*)\s*\(")
FUNC_DEF_RE = re.compile(
    r"(?m)^[ \t]*(?P<prefix>(?:static\s+)?(?:inline\s+)?(?:extern\s+)?"
    r"(?:const\s+)?(?:[A-Za-z_][\w\s\*\(\),]*?\s+))"
    r"(?:\*+\s*)?(?P<name>[A-Za-z_]\w*)\s*\((?P<args>[^;{}]*)\)\s*\{"
)
COMMENT_STRING_RE = re.compile(
    r'//.*?$|/\*.*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'',
    re.MULTILINE | re.DOTALL,
)

KEYWORDS = {
    "if",
    "for",
    "while",
    "switch",
    "return",
    "sizeof",
    "defined",
    "do",
    "else",
    "case",
}

EXPECTED_KERNEL_FLOW = ["cmd", "api", "runtime", "pipeline", "arch", "memory", "core"]
ENTRYPOINTS = [
    ("imgengine_cli", "src/cmd/imgengine/main.c", "main"),
    ("bench_lat", "src/cmd/bench/lat_bench.c", "main"),
]
KNOWN_INDIRECT_CALLS = {
    "g_io_vtable.decode": "img_decode_to_buffer",
    "g_io_vtable.encode": "img_encode_from_buffer",
    "g_io_vtable.encode_pdf": "img_encode_pdf",
    "resize_fn": "img_arch_avx2_resize",
}

# =========================================================
# HOT PATH DETECTION
# =========================================================


def is_hot(path):
    path = path.replace("\\", "/")

    # TRUE HOT PATH ONLY
    if "src/pipeline/" in path:
        return True

    if "src/arch/" in path:
        return True

    if "worker_loop.c" in path:
        return True

    if "pipeline_exec" in path or "batch_exec" in path:
        return True

    return False


# def is_hot(path):
# return any(k in path for k in ["pipeline", "exec", "batch", "fused", "avx"])
# return any(k in path for k in ["pipeline", "io", "cmd", "cold", "core",'hot','plugins','startup','runtime','api','arch'])


def detect_hot(content):
    issues = []
    if "malloc" in content:
        issues.append("malloc")
    if "free(" in content:
        issues.append("free")
    if "printf" in content:
        issues.append("printf")
    if "fopen" in content:
        issues.append("file_io")
    if "read(" in content or "write(" in content:
        issues.append("syscall")
    return issues


# =========================================================
# SIDE SYSTEM ENFORCEMENT
# =========================================================


def check_side_systems(src, dst):
    key = f"{src}->{dst}"

    # ❌ pipeline must NEVER touch plugins
    if key == "pipeline->plugins":
        return "ERR", "Pipeline must remain deterministic (no plugins)."

    # ❌ runtime should not directly use io
    if key == "runtime->io":
        return "ERR", "Runtime must not perform IO. Route via API."

    # ❌ anything except api using io
    if dst == "io" and src != "api":
        return "ERR", "IO is only allowed in API layer."

    # ❌ plugins should not call api/io
    if src == "plugins" and dst in ["api", "io"]:
        return "ERR", "Plugins must be sandboxed. No direct IO/API."

    # ❌ observability must not influence system
    if src == "observability" and dst != "core":
        return "ERR", "Observability must be passive."

    # ❌ security must not act like runtime
    if src == "security" and dst in ["runtime", "pipeline"]:
        return "ERR", "Security is pre-check only."

    return None, None


# =========================================================
# FIX ENGINE
# =========================================================


def fix_dag(src, dst):
    fixes = {
        "pipeline->io": "Move IO to API. Pass buffers.",
        "pipeline->plugins": "Move plugin execution to runtime.",
        "runtime->io": "Move IO to API layer.",
        "api->pipeline": "Route via runtime.",
    }
    return fixes.get(f"{src}->{dst}", "Follow strict downward DAG.")


def fix_hot(issue):
    fixes = {
        "malloc": "Use slab/arena allocator.",
        "free": "Avoid free in hot path.",
        "printf": "Move to observability.",
        "file_io": "Move IO to API.",
        "syscall": "Remove syscalls from hot path.",
    }
    return fixes.get(issue, "Remove side-effects.")


# =========================================================
# SCAN
# =========================================================


def scan(root):
    files = []
    for base, _, fs in os.walk(root):
        for f in fs:
            if f.endswith((".c", ".h")):
                files.append(os.path.join(base, f))
    return files


def strip_comments_and_strings(text):
    def repl(match):
        s = match.group(0)
        return "".join("\n" if ch == "\n" else " " for ch in s)

    return COMMENT_STRING_RE.sub(repl, text)


def find_matching_brace(text, open_pos):
    depth = 0
    for idx in range(open_pos, len(text)):
        ch = text[idx]
        if ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                return idx
    return -1


def build_callgraph(files, layers):
    defs = {}
    defs_by_file = collections.defaultdict(dict)
    global_defs = collections.defaultdict(list)

    for path in files:
        if not path.endswith(".c"):
            continue

        try:
            original = open(path, "r", errors="ignore").read()
        except Exception:
            continue

        cleaned = strip_comments_and_strings(original)

        for match in FUNC_DEF_RE.finditer(cleaned):
            name = match.group("name")
            prefix = match.group("prefix") or ""
            if name in KEYWORDS:
                continue

            brace_pos = cleaned.find("{", match.start())
            end_brace = find_matching_brace(cleaned, brace_pos)
            if end_brace < 0:
                continue

            key = (path, name, match.start())
            info = {
                "key": key,
                "name": name,
                "file": path,
                "layer": layers.get(path),
                "static": "static" in prefix.split(),
                "body": cleaned[brace_pos + 1 : end_brace],
                "calls": set(),
                "indirect": set(),
                "resolved": set(),
                "unresolved": set(),
            }
            defs[key] = info
            defs_by_file[path].setdefault(name, []).append(key)
            if not info["static"]:
                global_defs[name].append(key)

    for key, info in defs.items():
        body = info["body"]
        for expr, target in KNOWN_INDIRECT_CALLS.items():
            if expr + "(" in body:
                info["indirect"].add(expr)
                info["calls"].add(target)

        for call_name in CALL_RE.findall(body):
            if call_name in KEYWORDS:
                continue
            info["calls"].add(call_name)

        local_candidates = defs_by_file[info["file"]]
        for call_name in sorted(info["calls"]):
            candidates = []

            if call_name in local_candidates:
                candidates = local_candidates[call_name]
            elif len(global_defs.get(call_name, [])) == 1:
                candidates = global_defs[call_name]

            if len(candidates) == 1:
                info["resolved"].add(candidates[0])
            else:
                info["unresolved"].add(call_name)

    return defs


def trace_entrypoints(defs, layers):
    defs_by_suffix = collections.defaultdict(list)
    for key, info in defs.items():
        defs_by_suffix[(info["file"].replace("\\", "/"), info["name"])].append(key)

    traces = []
    for label, suffix, func_name in ENTRYPOINTS:
        suffix = suffix.replace("\\", "/")
        roots = []
        for (path, name), keys in defs_by_suffix.items():
            if path.endswith(suffix) and name == func_name:
                roots.extend(keys)

        if not roots:
            traces.append(
                {
                    "label": label,
                    "roots": [],
                    "reachable": set(),
                    "cross_edges": collections.Counter(),
                    "unresolved": collections.Counter(),
                    "layers": [],
                }
            )
            continue

        seen = set()
        queue = collections.deque(roots)
        cross_edges = collections.Counter()
        unresolved = collections.Counter()

        while queue:
            node = queue.popleft()
            if node in seen:
                continue
            seen.add(node)

            info = defs[node]
            for unresolved_name in info["unresolved"]:
                unresolved[unresolved_name] += 1

            for callee in info["resolved"]:
                src_layer = info["layer"]
                dst_layer = defs[callee]["layer"]
                if src_layer and dst_layer and src_layer != dst_layer:
                    cross_edges[(src_layer, dst_layer)] += 1
                if callee not in seen:
                    queue.append(callee)

        layer_set = {defs[node]["layer"] for node in seen if defs[node]["layer"]}
        layer_order = [layer for layer in EXPECTED_KERNEL_FLOW if layer in layer_set]
        for layer in LAYERS:
            if layer in layer_set and layer not in layer_order:
                layer_order.append(layer)

        traces.append(
            {
                "label": label,
                "roots": roots,
                "reachable": seen,
                "cross_edges": cross_edges,
                "unresolved": unresolved,
                "layers": layer_order,
            }
        )

    return traces


def report_flow_traces(defs, traces):
    print("\nREAL FLOW TRACE")
    print("=" * 60)

    all_reachable = set()
    flow_violations = []
    flow_index = {layer: idx for idx, layer in enumerate(EXPECTED_KERNEL_FLOW)}

    for trace in traces:
        print(f"\n[{trace['label']}]")
        if not trace["roots"]:
            print("  missing entrypoint definition")
            continue

        all_reachable |= trace["reachable"]
        reached_count = len(trace["reachable"])
        print(f"  reachable functions: {reached_count}")

        layers_hit = trace["layers"]
        if layers_hit:
            print(f"  reached layers: {' -> '.join(layers_hit)}")
        else:
            print("  reached layers: none")

        missing = [layer for layer in EXPECTED_KERNEL_FLOW if layer not in layers_hit]
        if missing:
            print(f"  missing expected layers: {', '.join(missing)}")
        else:
            print("  missing expected layers: none")

        if trace["cross_edges"]:
            print("  exercised cross-layer edges:")
            for (src, dst), count in sorted(trace["cross_edges"].items()):
                print(f"    {src} -> {dst} [{count}]")
                if src in flow_index and dst in flow_index:
                    if src in {"cmd", "api"} and dst not in {"memory", "core"} and \
                       flow_index[dst] - flow_index[src] > 1:
                        flow_violations.append((trace["label"], src, dst, count))
        else:
            print("  exercised cross-layer edges: none")

        if trace["unresolved"]:
            top = trace["unresolved"].most_common(8)
            formatted = ", ".join(f"{name}({count})" for name, count in top)
            print(f"  unresolved/external calls: {formatted}")
        else:
            print("  unresolved/external calls: none")

    dead = [info for key, info in defs.items() if key not in all_reachable]
    print("\nUNUSED / NOT REACHED FROM ENTRYPOINTS")
    print("=" * 60)
    print(f"unreached function definitions: {len(dead)}")
    for info in sorted(dead, key=lambda x: (x["file"], x["name"]))[:20]:
        print(f"  {info['file']} :: {info['name']}")

    print("\nENTRYPOINT FLOW VIOLATIONS")
    print("=" * 60)
    if not flow_violations:
        print("none")
    else:
        for label, src, dst, count in flow_violations:
            print(f"  {label}: {src} -> {dst} [{count}] skips kernel stages")


# =========================================================
# MAIN
# =========================================================


def analyze(root="src"):
    files = scan(root)

    layers = {f: detect_layer(f) for f in files}
    defs = build_callgraph(files, layers)
    traces = trace_entrypoints(defs, layers)

    dag_errors = []
    hot_errors = []

    for f in files:
        sl = layers[f]
        if sl == "startup":
            continue  # 🔥 IGNORE STARTUP COMPLETELY

        if not sl:
            continue

        try:
            content = open(f, "r", errors="ignore").read()
        except:
            content = ""

        includes = INCLUDE_RE.findall(content)

        for inc in includes:
            target = next((x for x in files if x.endswith(inc)), None)
            if not target:
                continue

            dl = layers.get(target)
            if not dl or dl == "startup":
                continue

            # MATRIX CHECK
            if MATRIX[sl][dl] == "ERR":
                dag_errors.append((f, sl, target, dl, "MATRIX"))

            # SIDE SYSTEM CHECK
            rule, msg = check_side_systems(sl, dl)
            if rule == "ERR":
                dag_errors.append((f, sl, target, dl, msg))

        # HOT PATH CHECK
        if is_hot(f):
            for i in detect_hot(content):
                hot_errors.append((f, sl, i))

    # =========================================================
    # REPORT
    # =========================================================

    print(FLOW)
    print(SIDE_SYSTEMS)
    print(FLOW_DISPLAY)

    print("\nL11 KERNEL ARCH ANALYZER")
    print("=" * 60)

    print(f"\n❌ DAG Violations: {len(dag_errors)}")
    print(f"🔥 Hot Violations: {len(hot_errors)}")

    for f, sl, t, dl, reason in dag_errors:
        print(f"\n[DAG] {f}")
        print(f"  ❌ {sl} → {dl}")
        print(f"  ⚠️ {reason}")
        print(f"  🛠 FIX: {fix_dag(sl, dl)}")

    for f, sl, issue in hot_errors:
        print(f"\n[HOT:{issue}] {f}")
        print(f"  🔥 Layer: {sl}")
        print(f"  🛠 FIX: {fix_hot(issue)}")

    if not dag_errors and not hot_errors:
        print("\n🎯 CLEAN: TRUE KERNEL-GRADE (L11)")
    else:
        print("\n🚨 NOT KERNEL-GRADE — FIX REQUIRED")

    report_flow_traces(defs, traces)


# =========================================================
# ENTRY
# =========================================================

if __name__ == "__main__":
    analyze("src")


# #!/usr/bin/env python3
# """
# l10plusplus.py — imgengine Architecture Analyzer
# =================================================
# Kernel-grade static analysis tool for C codebases.

# What this script does (in order):
#   1. Walks all .c and .h files under src/, include/, api/
#   2. Extracts function definitions with accurate body boundaries
#      using brace-depth tracking (this is the key algorithm)
#   3. Attributes every function call to the correct caller
#      (not "all calls in the file" — the actual owning function body)
#   4. Builds a call graph: caller -> set of callees
#   5. Builds a reverse graph: callee -> set of callers
#   6. Scans #include directives and checks them against the layer DAG
#   7. Reports: hot functions, bottlenecks, complex functions,
#      layer violations, module heat, unused functions, next actions

# Usage:
#   cd /workspaces/imgengine/imgengine
#   python3 scripts/l10plusplus.py

# Exit code:
#   0 = clean (no violations)
#   1 = violations found (CI will fail the build)
# """

# import os
# import re
# import sys
# import time
# from collections import defaultdict


# # ============================================================
# # SECTION 1: CONFIGURATION
# # ============================================================

# # Where to look for source files.
# # We only scan these directories — not build/, .git/, third_party/.
# SRC_ROOTS = ("./src", "./include", "./api")

# # Layer DAG — ordered from lowest (most fundamental) to highest.
# #
# # A layer may only include headers from layers with a LOWER index.
# # Including a header from a HIGHER index = layer violation.
# #
# # Example:
# #   memory[1] including security[3] = VIOLATION (1 -> 3, upward)
# #   security[3] including memory[1] = OK        (3 -> 1, downward)
# #
# # This order was derived from imgengine's actual dependency graph.
# # Key decisions:
# #   observability[4] is BELOW pipeline[5] so pipeline can record metrics
# #   startup[11] is EXCLUDED from checking — it wires everything together
# LAYERS = [
#     "core",  # 0: img_result_t, img_buffer_t, opcodes, img_batch_t
#     "memory",  # 1: slab, arena, numa, poison
#     "arch",  # 2: SIMD kernels, cpu_caps, resize_params
#     "security",  # 3: input_validator, sandbox
#     "observability",  # 4: metrics, logging, binlog, tracing
#     "pipeline",  # 5: jump table, fused kernels, dispatch, job, layout
#     "runtime",  # 6: workers, queues, scheduler, exec_router
#     "plugins",  # 7: plugin ABI, plugin registry
#     "io",  # 8: decode, encode, vfs, pdf
#     "api",  # 9: public surface (re-exports from lower layers)
#     "cmd",  # 10: CLI (main.c, args.c, job_builder.c)
#     "startup",  # 11: engine_init.c — wires all layers, excluded from checks
# ]

# # These layers are excluded from violation checking.
# # startup/ must include everything — that is its job.
# # unknown/ means a file outside our standard structure — skip it.
# LAYERS_EXCLUDE = {"startup", "unknown"}

# # Functions to always exclude from the call graph.
# # These are compiler intrinsics, libc, and POSIX API functions.
# # Without this filter, _mm_setzero_si128 appears as a bottleneck
# # with 290 callers — completely drowning out your actual code.
# SYSTEM_PREFIXES = (
#     "_mm",  # Intel SSE/AVX/AVX-512 intrinsics
#     "__builtin",  # GCC builtins (__builtin_expect, __builtin_memset etc.)
#     "io_uring_",  # liburing API
#     "__io_uring",  # liburing internals
#     "tj",  # libjpeg-turbo (tjCompress2 etc.)
#     "stbi_",  # stb_image
#     "numa_",  # libnuma
#     "pthread_",  # pthreads
#     "atomic_",  # C11 atomics
#     "copy_bitmask",  # libnuma bitmask ops
# )

# # Exact function names to exclude (no prefix match needed).
# # These are libc functions without a recognizable prefix.
# SYSTEM_EXACT = {
#     "printf",
#     "fprintf",
#     "sprintf",
#     "snprintf",
#     "vsnprintf",
#     "malloc",
#     "calloc",
#     "realloc",
#     "free",
#     "aligned_alloc",
#     "memcpy",
#     "memset",
#     "memmove",
#     "memcmp",
#     "memchr",
#     "strlen",
#     "strcmp",
#     "strncmp",
#     "strcpy",
#     "strncpy",
#     "strstr",
#     "open",
#     "close",
#     "read",
#     "write",
#     "lseek",
#     "stat",
#     "fstat",
#     "mmap",
#     "munmap",
#     "mprotect",
#     "madvise",
#     "fopen",
#     "fclose",
#     "fwrite",
#     "fread",
#     "fseek",
#     "ftell",
#     "abort",
#     "exit",
#     "assert",
#     "atoi",
#     "atof",
#     "atol",
#     "strtol",
#     "strtof",
#     "dlopen",
#     "dlsym",
#     "dlclose",
#     "sched_getcpu",
#     "sched_setaffinity",
#     "clock_gettime",
# }

# # C keywords that look like function calls but are not.
# # "if(", "for(", "while(" all match identifier+( but are control flow.
# C_KEYWORDS = {
#     "if",
#     "else",
#     "for",
#     "while",
#     "do",
#     "switch",
#     "case",
#     "default",
#     "return",
#     "break",
#     "continue",
#     "goto",
#     "sizeof",
#     "typeof",
#     "alignof",
#     "offsetof",
#     "typedef",
#     "struct",
#     "union",
#     "enum",
#     "extern",
#     "static",
#     "inline",
#     "const",
#     "volatile",
#     "restrict",
#     "void",
#     "int",
#     "char",
#     "float",
#     "double",
#     "unsigned",
#     "signed",
#     "long",
#     "short",
# }

# # Terminal color codes — work on Linux terminal and VS Code
# RED = "\033[0;31m"
# GREEN = "\033[0;32m"
# YELLOW = "\033[1;33m"
# CYAN = "\033[0;36m"
# BOLD = "\033[1m"
# DIM = "\033[2m"
# NC = "\033[0m"  # reset all formatting


# # ============================================================
# # SECTION 2: HELPER FUNCTIONS
# # ============================================================


# def is_system_fn(name):
#     """
#     Returns True if 'name' is a system/compiler function.

#     Call this before adding any function to the call graph.
#     Prevents intrinsics like _mm_setzero_si128 (290 fake callers)
#     from polluting the analysis results.
#     """
#     if not name:
#         return True
#     if name in SYSTEM_EXACT:
#         return True
#     for prefix in SYSTEM_PREFIXES:
#         if name.startswith(prefix):
#             return True
#     return False


# def get_layer(path):
#     """
#     Given a file path, returns which layer it belongs to.

#     Algorithm:
#       1. Strip the leading root directory (src/, include/, api/)
#       2. Take the first directory component of what remains
#       3. Check if that component is in the LAYERS list

#     Examples:
#       src/pipeline/canvas.c     -> "pipeline"
#       include/memory/slab.h     -> "memory"
#       api/v1/img_error.h        -> "api"
#       src/startup/engine_init.c -> "startup"
#       src/third_party/stb/...   -> "unknown"
#     """
#     rel = os.path.relpath(path, ".")

#     for root in ("src", "include", "api"):
#         prefix = root + os.sep
#         if rel.startswith(prefix):
#             rel = rel[len(prefix) :]
#             break

#     layer = rel.split(os.sep)[0]
#     return layer if layer in LAYERS else "unknown"


# def layer_index(layer):
#     """
#     Returns the numeric position of a layer in the LAYERS list.
#     Lower number = more fundamental = fewer allowed dependencies.
#     Returns -1 if the layer is not in our DAG.
#     """
#     try:
#         return LAYERS.index(layer)
#     except ValueError:
#         return -1


# # ============================================================
# # SECTION 3: THE KEY ALGORITHM — BRACE-DEPTH FUNCTION EXTRACTION
# # ============================================================


# def extract_function_bodies(content):
#     """
#     THE CORE ALGORITHM. This is what separates a kernel-grade analyzer
#     from a naive regex script.

#     PROBLEM WITH NAIVE REGEX:
#       A simple approach scans all `identifier(` patterns in a file
#       and assigns every call to every function defined in that file.
#       Result: every function in api.c gets fan-out=57 (all 57 call
#       sites in the entire file). This is completely wrong.

#     THE CORRECT APPROACH — BRACE DEPTH TRACKING:
#       A function body starts at the `{` following the function signature
#       and ends at the `}` that brings brace depth back to 0.
#       Any call found BETWEEN those two braces belongs to THAT function only.

#       This is the same approach used by:
#         - sparse (Linux kernel's static analysis tool)
#         - cflow (GNU call graph generator)
#         - clang's AST (which tracks source ranges per function)

#     ALGORITHM STEPS:
#       1. Find function signatures using regex
#          (line with return type + name + params, no semicolon)
#       2. From the signature, scan forward char by char to find `{`
#       3. If we find `;` before `{` — it was a declaration, skip it
#       4. Once inside the body (depth=1), track depth:
#            `{` increases depth
#            `}` decreases depth
#            depth returns to 0 = body complete
#       5. Skip braces inside string literals and comments
#          (they are not real braces and would corrupt depth tracking)
#       6. Extract the body text and return (fn_name, body_text) pairs

#     COMPLEXITY: O(n) where n = file size. Single pass.

#     Returns: list of (fn_name, body_text) tuples
#     """

#     # Regex to recognize a function definition line.
#     #
#     # Matches (examples):
#     #   img_result_t img_canvas_init(img_canvas_t *c, img_slab_pool_t *p)
#     #   static void fused_exec_scalar(img_ctx_t *ctx, img_buffer_t *buf)
#     #   int img_api_run_job(img_engine_t *engine, const char *in, ...)
#     #   void img_slab_free(img_slab_pool_t *pool, void *ptr)
#     #
#     # Does NOT match (correctly skipped):
#     #   extern img_result_t img_api_run_job(...);   <- has semicolon
#     #   img_result_t (*fn_ptr)(...)                 <- function pointer
#     #   typedef void (*img_op_fn)(...)              <- typedef
#     fn_sig_re = re.compile(
#         r"^[ \t]*"  # optional leading whitespace
#         r"(?:static\s+)?"  # optional 'static'
#         r"(?:inline\s+)?"  # optional 'inline'
#         r"(?:__attribute__\s*\(\([^)]*\)\)\s*)?"  # optional __attribute__
#         r"(?:"  # return type — one of:
#         r"void|int|unsigned|uint8_t|uint16_t|uint32_t|uint64_t"
#         r"|int8_t|int16_t|int32_t|int64_t"
#         r"|size_t|ssize_t|ptrdiff_t|bool|char|float|double"
#         r"|img_\w+"  # any img_* type from imgengine
#         r")\s*\*?\s*"  # optional pointer (*)
#         r"([a-zA-Z_]\w*)"  # CAPTURE group 1: function name
#         r"\s*\([^;]*$",  # params followed by end of line (no semicolon)
#         re.MULTILINE,
#     )

#     results = []

#     for m in fn_sig_re.finditer(content):
#         fn_name = m.group(1)

#         # Skip system functions and keywords
#         if not fn_name or is_system_fn(fn_name) or fn_name in C_KEYWORDS:
#             continue

#         # ── PHASE 1: Find the opening brace of the function body ──
#         #
#         # Scan forward from end of the signature match.
#         # The opening `{` may be on the same line as the params,
#         # or on the very next line (K&R and Allman styles both work).
#         pos = m.end()
#         body_start = -1
#         depth = 0
#         scan_limit = min(pos + 300, len(content))  # 300 chars is enough

#         while pos < scan_limit:
#             ch = content[pos]

#             if ch == ";":
#                 # Semicolon before brace = this was a declaration, not a definition
#                 # Example: img_result_t img_canvas_init(...);\n
#                 break

#             if ch == "{":
#                 body_start = pos
#                 depth = 1
#                 pos += 1
#                 break

#             pos += 1

#         if body_start == -1:
#             # No opening brace found — skip this match
#             continue

#         # ── PHASE 2: Scan the body tracking brace depth ──
#         #
#         # We must correctly skip braces that appear inside:
#         #   - String literals: "...{...}" should not change depth
#         #   - Character literals: '{' should not change depth
#         #   - Single-line comments: // { should not change depth
#         #   - Multi-line comments: /* { */ should not change depth
#         #
#         # Without this, a string like:
#         #   fprintf(stderr, "open { failed\n");
#         # would increment depth by 1, causing us to miss the real closing brace.
#         body_end = -1

#         while pos < len(content):
#             ch = content[pos]

#             # Skip single-line comment: // ...
#             if ch == "/" and pos + 1 < len(content) and content[pos + 1] == "/":
#                 while pos < len(content) and content[pos] != "\n":
#                     pos += 1
#                 continue

#             # Skip multi-line comment: /* ... */
#             if ch == "/" and pos + 1 < len(content) and content[pos + 1] == "*":
#                 pos += 2
#                 while pos + 1 < len(content):
#                     if content[pos] == "*" and content[pos + 1] == "/":
#                         pos += 2
#                         break
#                     pos += 1
#                 continue

#             # Skip string literal: "..."
#             if ch == '"':
#                 pos += 1
#                 while pos < len(content):
#                     if content[pos] == "\\":
#                         pos += 2  # skip escaped char (e.g., \", \\)
#                         continue
#                     if content[pos] == '"':
#                         pos += 1
#                         break
#                     pos += 1
#                 continue

#             # Skip character literal: '.'
#             if ch == "'":
#                 pos += 1
#                 while pos < len(content):
#                     if content[pos] == "\\":
#                         pos += 2
#                         continue
#                     if content[pos] == "'":
#                         pos += 1
#                         break
#                     pos += 1
#                 continue

#             # Track real brace depth
#             if ch == "{":
#                 depth += 1
#             elif ch == "}":
#                 depth -= 1
#                 if depth == 0:
#                     # Found the matching closing brace
#                     body_end = pos
#                     break

#             pos += 1

#         if body_end == -1:
#             # Unmatched braces — likely a macro or complex preprocessor use
#             # Skip this function rather than producing wrong results
#             continue

#         # Extract the complete function body text
#         body_text = content[body_start : body_end + 1]
#         results.append((fn_name, body_text))

#     return results


# def extract_calls_from_body(body_text):
#     """
#     Given the TEXT OF ONE FUNCTION BODY (from extract_function_bodies),
#     returns the set of all function names called within it.

#     This is called AFTER brace-depth extraction, so the body_text
#     contains ONLY the code that belongs to this specific function.
#     No cross-contamination from other functions in the same file.

#     Algorithm:
#       Find all `identifier(` patterns.
#       Filter out C keywords, system functions, and empty strings.
#     """
#     call_re = re.compile(r"\b([a-zA-Z_]\w*)\s*\(")

#     callees = set()
#     for m in call_re.finditer(body_text):
#         name = m.group(1)
#         if name and name not in C_KEYWORDS and not is_system_fn(name):
#             callees.add(name)

#     return callees


# # ============================================================
# # SECTION 4: INCLUDE SCANNER AND VIOLATION DETECTOR
# # ============================================================


# def scan_includes(path):
#     """
#     Scans a file for #include "..." directives (LOCAL includes only).

#     We only care about quoted includes like #include "memory/slab.h"
#     because angle-bracket includes (#include <stdio.h>) are system
#     headers and not part of our layer DAG.

#     Returns: list of included path strings (the part inside quotes).
#     """
#     include_re = re.compile(r'#\s*include\s+"([^"]+)"')
#     try:
#         content = open(path, encoding="utf-8", errors="ignore").read()
#         return include_re.findall(content)
#     except Exception:
#         return []


# def check_layer_violation(src_path, included_path):
#     """
#     Checks whether including 'included_path' from 'src_path' violates
#     the layer DAG.

#     RULE: file in layer[X] may include a header from layer[Y] only if Y <= X.
#     Including a header from a layer with a HIGHER index = VIOLATION.

#     Returns:
#       (is_violation, src_layer, dest_layer, src_idx, dest_idx)

#     Examples:
#       src/memory/slab.c includes security/poision.h
#         memory[1] -> security[3] : 3 > 1 = VIOLATION

#       src/pipeline/canvas.c includes core/result.h
#         pipeline[5] -> core[0] : 0 < 5 = OK

#       src/startup/engine_init.c includes runtime/worker.h
#         startup[11] -> runtime[6] : excluded from checking = OK
#     """
#     src_layer = get_layer(src_path)
#     dest_layer = included_path.split("/")[0]

#     # Exclude layers from violation checking
#     if src_layer in LAYERS_EXCLUDE:
#         return False, src_layer, dest_layer, -1, -1

#     # If the destination layer is not in our DAG, it's third-party — skip
#     if dest_layer not in LAYERS:
#         return False, src_layer, dest_layer, -1, -1

#     src_idx = layer_index(src_layer)
#     dest_idx = layer_index(dest_layer)

#     if src_idx == -1 or dest_idx == -1:
#         return False, src_layer, dest_layer, -1, -1

#     # Violation: source layer index < destination layer index
#     # (source is more fundamental but depends on something higher up)
#     is_violation = dest_idx > src_idx

#     return is_violation, src_layer, dest_layer, src_idx, dest_idx


# # ============================================================
# # SECTION 5: MAIN ANALYZER
# # ============================================================


# class Analyzer:
#     """
#     Main analysis engine. Orchestrates file scanning and graph building.

#     After calling .run(), these data structures are populated:

#       call_graph[fn]    = set of functions that fn calls
#                           (only your functions, system functions excluded)

#       reverse_graph[fn] = set of functions that call fn
#                           (used for fan-in calculation)

#       fn_to_file[fn]    = path of the file that defines fn
#                           (for reporting which file a function lives in)

#       file_deps[path]   = set of local headers that path includes
#                           (used for "files to split" report)

#       violations        = list of (src_path, included, sl, dl, si, di) tuples
#                           (all layer violations found)
#     """

#     def __init__(self):
#         self.call_graph = defaultdict(set)
#         self.reverse_graph = defaultdict(set)
#         self.fn_to_file = {}
#         self.file_deps = defaultdict(set)
#         self.violations = []

#     def _process_content(self, path, content):
#         """
#         Core processing logic shared by .c and .h file handlers.

#         Steps:
#           1. Extract function bodies using brace-depth algorithm
#           2. For each body, extract calls and add to call graph
#           3. Scan #include directives and check for violations
#         """

#         # Step 1: Extract all function bodies with accurate boundaries
#         fn_bodies = extract_function_bodies(content)

#         for fn_name, body_text in fn_bodies:
#             # Record where this function is defined
#             # First definition wins (handles duplicate symbols gracefully)
#             if fn_name not in self.fn_to_file:
#                 self.fn_to_file[fn_name] = path

#             # Step 2: Extract calls from this body only
#             callees = extract_calls_from_body(body_text)

#             # Remove self-calls (recursion doesn't inflate fan-out)
#             callees.discard(fn_name)

#             # Add to call graph
#             for callee in callees:
#                 self.call_graph[fn_name].add(callee)
#                 self.reverse_graph[callee].add(fn_name)

#         # Step 3: Check includes for layer violations
#         includes = scan_includes(path)
#         self.file_deps[path].update(includes)

#         for inc in includes:
#             is_viol, sl, dl, si, di = check_layer_violation(path, inc)
#             if is_viol:
#                 self.violations.append((path, inc, sl, dl, si, di))

#     def analyze_c_file(self, path):
#         """Process a .c source file."""
#         try:
#             content = open(path, encoding="utf-8", errors="ignore").read()
#         except Exception:
#             return
#         self._process_content(path, content)

#     def analyze_header_file(self, path):
#         """
#         Process a .h header file.

#         Headers may contain:
#           - static inline function definitions (have bodies — process them)
#           - extern declarations (no body — the regex skips these correctly
#             because they have a semicolon before any opening brace)
#           - #include directives (checked for violations)

#         The brace-depth algorithm handles this correctly:
#           extern img_result_t img_canvas_init(...);  <- has ';', skipped
#           static inline void foo(...) { ... }        <- has '{', processed
#         """
#         try:
#             content = open(path, encoding="utf-8", errors="ignore").read()
#         except Exception:
#             return
#         self._process_content(path, content)

#     def run(self):
#         """
#         Walk all source directories and analyze every .c and .h file.

#         Traversal:
#           - Only enters directories listed in SRC_ROOTS
#           - Skips build/, .git/, __pycache__/ automatically
#             (they are not under SRC_ROOTS)
#           - .c files -> analyze_c_file()
#           - .h files -> analyze_header_file()
#         """
#         SKIP_DIRS = {"build", ".git", "__pycache__", "third_party", ".cache"}

#         for root_dir in SRC_ROOTS:
#             for dirpath, dirnames, filenames in os.walk(root_dir):
#                 # Prune directories we never want to enter
#                 dirnames[:] = [d for d in dirnames if d not in SKIP_DIRS]

#                 for fname in filenames:
#                     fpath = os.path.join(dirpath, fname)
#                     if fname.endswith(".c"):
#                         self.analyze_c_file(fpath)
#                     elif fname.endswith(".h"):
#                         self.analyze_header_file(fpath)

#         return self


# # ============================================================
# # SECTION 6: METRICS
# # ============================================================


# class Metrics:
#     """
#     Computes call graph metrics for each function.

#     fan_in(fn)  = number of YOUR functions that call fn
#                   High fan_in = widely used = potential bottleneck
#                   If this function is slow or buggy, many callers suffer

#     fan_out(fn) = number of distinct functions that fn calls
#                   High fan_out = complex = candidate for splitting
#                   A function calling 10+ others is doing too much

#     score(fn)   = fan_in + 0.5 * fan_out
#                   Composite heat score.
#                   The 0.5 weight on fan_out reflects that being called
#                   (fan_in) is more architecturally significant than
#                   calling others (fan_out).
#                   Same formula used in Linux perf call graph analysis.
#     """

#     def __init__(self, analyzer):
#         self.a = analyzer

#     def fan_in(self, fn):
#         return len(self.a.reverse_graph.get(fn, set()))

#     def fan_out(self, fn):
#         return len(self.a.call_graph.get(fn, set()))

#     def score(self, fn):
#         return self.fan_in(fn) + 0.5 * self.fan_out(fn)

#     def your_functions(self):
#         """
#         Returns only functions that are defined in your source files.
#         Excludes system functions that were called but not defined.
#         """
#         return {fn for fn in self.a.fn_to_file if not is_system_fn(fn)}


# # ============================================================
# # SECTION 7: REPORT GENERATORS
# # ============================================================


# def report_hot_functions(metrics):
#     """
#     Prints the 20 hottest functions by composite score.

#     HOT (score > 5) means the function is both widely used AND complex.
#     These are the functions that:
#       - Must be correct (bugs affect many callers)
#       - Must be fast (slowness affects the entire pipeline)
#       - Must be well documented

#     With brace-depth tracking, fan_out is now accurate per function.
#     A function in api.c will show its ACTUAL calls, not all 57 calls
#     that happen to appear anywhere in the file.
#     """
#     your_fns = list(metrics.your_functions())

#     scored = [
#         (metrics.score(f), metrics.fan_in(f), metrics.fan_out(f), f)
#         for f in your_fns
#         if metrics.score(f) > 0
#     ]
#     scored.sort(reverse=True)

#     print(f"{BOLD}HOT FUNCTIONS (your code only){NC}")
#     print(f"  {'Function':<42} {'Score':>6}  {'Fan-in':>7}  {'Fan-out':>8}")
#     print(f"  {'-' * 70}")

#     for score, fi, fo, fn in scored[:20]:
#         hot = f"  {RED}HOT{NC}" if score > 5 else ""
#         print(f"  {fn:<42} {score:>6.1f}  {fi:>7}  {fo:>8}{hot}")
#     print()


# def report_bottlenecks(metrics):
#     """
#     Prints functions called by 3 or more of YOUR functions.

#     A bottleneck function is critical infrastructure:
#       - If img_slab_alloc is slow, every allocation is slow
#       - If img_slab_free has a bug, every free is buggy
#       - These deserve the most testing, documentation, and profiling

#     We list the actual callers so you can verify the data is correct.
#     If align64 appears as a caller of img_slab_free, that means
#     align64's body contains a call to img_slab_free() — verify in code.
#     """
#     your_fns = metrics.your_functions()

#     bottlenecks = sorted(
#         [(metrics.fan_in(f), f) for f in your_fns if metrics.fan_in(f) >= 3],
#         reverse=True,
#     )

#     print(f"{BOLD}REAL BOTTLENECKS (called by 3+ of your functions){NC}")

#     for count, fn in bottlenecks[:15]:
#         callers = sorted(
#             c for c in metrics.a.reverse_graph.get(fn, set()) if not is_system_fn(c)
#         )
#         file_rel = os.path.relpath(metrics.a.fn_to_file.get(fn, ""), ".")
#         print(f"\n  {CYAN}{fn}{NC} <- {count} callers  [{DIM}{file_rel}{NC}]")
#         for c in callers[:5]:
#             print(f"      <- {c}")
#         if len(callers) > 5:
#             print(f"      {DIM}... and {len(callers) - 5} more{NC}")
#     print()


# def report_complex_functions(metrics):
#     """
#     Prints functions with fan_out >= 5.

#     A function calling 5+ distinct functions is a complexity signal.
#     It may be violating single-responsibility principle.

#     Exceptions (expected to have high fan_out):
#       - img_api_init: orchestrates engine startup — many calls expected
#       - img_engine_init: same reason
#       - bench_lat main: calls many things to set up benchmarks
#     """
#     your_fns = metrics.your_functions()

#     complex_fns = sorted(
#         [(metrics.fan_out(f), f) for f in your_fns if metrics.fan_out(f) >= 5],
#         reverse=True,
#     )

#     print(f"{BOLD}COMPLEX FUNCTIONS (fan-out >= 5 — consider splitting){NC}")
#     for count, fn in complex_fns[:10]:
#         file_rel = os.path.relpath(metrics.a.fn_to_file.get(fn, ""), ".")
#         print(f"  {YELLOW}{fn}{NC} -> {count} calls | {DIM}{file_rel}{NC}")
#     print()


# def report_layer_violations(analyzer):
#     """
#     Prints all layer violations (upward dependencies in the DAG).

#     WHY VIOLATIONS MATTER:
#       A layer violation means lower-level code knows about higher-level
#       concepts. For example:
#         memory/slab.c including api/v1/img_error.h
#         This means the memory allocator depends on the public API.
#         If the API changes, the allocator must recompile.
#         If you want to test the allocator in isolation, you must
#         also compile the entire API layer.

#     This is exactly the problem that Linux kernel developers reject
#     in code review. The fix is always the same:
#       Move the type/function to the LOWER layer.

#     HOW TO READ THE OUTPUT:
#       memory[1] -> api[9] means memory (index 1) includes something
#       from api (index 9). Since 9 > 1, this is an upward dependency.
#       Fix: move the needed type from api/ to core/ (index 0).
#     """
#     print(f"{BOLD}LAYER VIOLATIONS (upward dependencies){NC}")

#     if not analyzer.violations:
#         print(f"  {GREEN}CLEAN — zero layer violations{NC}")
#         print()
#         return

#     # Deduplicate: same file+include pair may appear in both
#     # the .c scan and the .h scan of the same header
#     seen = set()
#     unique = []
#     for v in analyzer.violations:
#         key = (v[0], v[1])
#         if key not in seen:
#             seen.add(key)
#             unique.append(v)

#     for path, inc, sl, dl, si, di in unique:
#         rel = os.path.relpath(path, ".")
#         print(f"  {RED}X{NC} {rel}")
#         print(f"     includes {YELLOW}{inc}{NC}")
#         print(f"     {sl}[{si}] -> {dl}[{di}]  {DIM}({di} > {si} = violation){NC}")
#     print()


# def report_module_heat(metrics, analyzer):
#     """
#     Prints per-layer heat scores.

#     For each layer: function count, total score, average score.
#     The average score tells you which layer needs the most attention.

#     HIGH average score = functions in this layer are critical:
#       - Widely called by other layers (high fan_in in those functions)
#       - Bugs here have wide blast radius

#     The bar chart is proportional to average score (max 20 chars).
#     """
#     module_total = defaultdict(float)
#     module_count = defaultdict(int)

#     for fn in metrics.your_functions():
#         layer = get_layer(metrics.a.fn_to_file.get(fn, ""))
#         module_total[layer] += metrics.score(fn)
#         module_count[layer] += 1

#     items = [(module_total[l], l) for l in LAYERS if module_count[l] > 0]

#     print(f"{BOLD}MODULE HEAT (which layers need most attention){NC}")
#     print(f"  {'Layer':<20} {'Fns':>5}  {'Total':>8}  {'Avg':>6}  Chart")
#     print(f"  {'-' * 58}")

#     for total, layer in sorted(items, reverse=True):
#         count = module_count[layer]
#         avg = total / count if count else 0
#         bar = "█" * min(int(avg * 2), 20)
#         print(f"  {layer:<20} {count:>5}  {total:>8.1f}  {avg:>6.1f}  {bar}")
#     print()


# def report_files_to_split(analyzer):
#     """
#     Prints files with 7+ local #include directives.

#     A file with many includes is pulling in many dependencies.
#     This is a signal that the file has too many responsibilities.

#     Linux kernel guideline: files should focus on one thing.
#     Multiple responsibilities = multiple reasons to change = fragile.
#     """
#     items = sorted(
#         [(len(v), k) for k, v in analyzer.file_deps.items() if len(v) >= 7],
#         reverse=True,
#     )

#     print(f"{BOLD}FILES TO SPLIT (7+ local dependencies){NC}")

#     if not items:
#         print(f"  {GREEN}All files have manageable dependency counts{NC}")
#     else:
#         for count, path in items[:8]:
#             rel = os.path.relpath(path, ".")
#             print(f"  {YELLOW}!{NC} {rel} -> {count} deps")
#     print()


# def report_unused_functions(metrics):
#     """
#     Prints functions with fan_in = 0 (never called by scanned code).

#     POSSIBLE REASONS a function has fan_in = 0:

#     1. Dead code — was written but never wired up
#        ACTION: delete it or wire it in

#     2. Entry point — called from outside (CLI, tests, external lib)
#        ACTION: add to KNOWN_ENTRY_POINTS below, or add a test that calls it

#     3. Function pointer target — called via fn_ptr, not direct call
#        ACTION: document this in a comment: "called via g_jump_table"

#     4. Static analyzer limitation — called from a macro that expands
#        to a function call (we can't see inside macros)
#        ACTION: add a comment or explicit call to make it visible

#     Notable unused functions in imgengine:
#       img_engine_init          <- should be called from img_api_init()
#       img_fused_init           <- should be called after img_jump_table_init()
#       img_batch_execute_fused  <- should be called from worker_loop()
#       img_arch_resize_h/v_avx2 <- should be registered in hardware_registry.c
#     """
#     KNOWN_ENTRY_POINTS = {
#         "main",
#         "img_api_init",
#         "img_api_shutdown",
#         "img_api_run_job",
#         "img_api_process_raw",
#         "img_api_process_fast",
#         "img_engine_init",
#         "LLVMFuzzerTestOneInput",
#     }

#     your_fns = metrics.your_functions()

#     unused = sorted(
#         [
#             fn
#             for fn in your_fns
#             if metrics.fan_in(fn) == 0 and fn not in KNOWN_ENTRY_POINTS
#         ]
#     )

#     if not unused:
#         print(f"  {GREEN}No unused functions{NC}")
#         return

#     print(f"{BOLD}POTENTIALLY UNUSED FUNCTIONS (fan-in = 0){NC}")
#     print(f"  {DIM}May be entry points, function-pointer targets, or dead code.{NC}\n")

#     for fn in unused[:25]:
#         file_rel = os.path.relpath(metrics.a.fn_to_file.get(fn, ""), ".")
#         print(f"  {fn:<48} {DIM}[{file_rel}]{NC}")

#     if len(unused) > 25:
#         print(f"  {DIM}... and {len(unused) - 25} more{NC}")
#     print()


# def report_next_actions(analyzer, metrics):
#     """
#     Prints a prioritized action list based on all analysis results.

#     Priority 1 = CRITICAL: fix before any merge, blocks CI
#     Priority 2 = HIGH: fix this sprint
#     Priority 3 = HIGH: important for correctness and performance
#     Priority 4 = MEDIUM: important for completeness
#     Priority 5 = MEDIUM: required by RFC but not blocking
#     Priority 6 = LOW: nice to have

#     This is the most actionable section of the analyzer output.
#     """
#     # Count unique violations
#     seen = set()
#     unique_viol = sum(
#         1
#         for v in analyzer.violations
#         if (v[0], v[1]) not in seen and not seen.add((v[0], v[1]))
#     )

#     actions = []

#     if unique_viol > 0:
#         actions.append(
#             (
#                 1,
#                 "CRITICAL",
#                 RED,
#                 f"Fix {unique_viol} layer violation(s). "
#                 "Upward dependencies break module boundaries and "
#                 "prevent independent testing of each layer.",
#             )
#         )

#     complex_fns = sorted(
#         [
#             (metrics.fan_out(f), f)
#             for f in metrics.your_functions()
#             if metrics.fan_out(f) >= 8
#         ],
#         reverse=True,
#     )
#     for count, fn in complex_fns[:2]:
#         file = os.path.relpath(metrics.a.fn_to_file.get(fn, ""), ".")
#         actions.append(
#             (
#                 2,
#                 "HIGH",
#                 YELLOW,
#                 f"Split {fn}() in {file}: "
#                 f"{count} outgoing calls — too many responsibilities.",
#             )
#         )

#     actions.append(
#         (
#             3,
#             "HIGH",
#             YELLOW,
#             "Replace resize_nearest() in src/pipeline/layout.c with "
#             "bilinear interpolation AVX2. Current nearest-neighbour "
#             "produces aliasing at print DPI (300dpi visible artifacts).",
#         )
#     )

#     actions.append(
#         (
#             3,
#             "HIGH",
#             YELLOW,
#             "Wire io_uring output path. img_api_run_job() currently "
#             "uses fwrite() internally. Add img_api_run_job_raw() that "
#             "returns encoded bytes, then call io_uring_write_file().",
#         )
#     )

#     actions.append(
#         (
#             4,
#             "MEDIUM",
#             NC,
#             "Add --mode fit|fill to CLI (src/cmd/imgengine/args.c). "
#             "job->mode is set in img_job_defaults() to IMG_FILL "
#             "but img_parse_args() never reads a --mode argument.",
#         )
#     )

#     actions.append(
#         (
#             4,
#             "MEDIUM",
#             NC,
#             "Wire dead functions: img_engine_init(), img_fused_init(), "
#             "img_batch_execute_fused(), img_arch_resize_h_avx2(), "
#             "img_arch_resize_v_avx2() all have fan_in=0. "
#             "They are built but nothing calls them.",
#         )
#     )

#     actions.append(
#         (
#             5,
#             "MEDIUM",
#             NC,
#             "CI performance gate: add to CMakeLists.txt a post-build "
#             "step that runs bench_lat and fails if P99 > 2ms. "
#             "RFC section 22.3 requires this — currently not enforced.",
#         )
#     )

#     actions.append(
#         (
#             6,
#             "LOW",
#             DIM,
#             "PDF multi-page support: pdf_encoder.c is single-page only. "
#             "Large grids (6x10 at 4.5x3.5cm) exceed A4 height. "
#             "Add page continuation logic.",
#         )
#     )

#     actions.append(
#         (
#             6,
#             "LOW",
#             DIM,
#             "Python CFFI bindings: create bindings/python/cffi_wrapper.py "
#             "exposing img_api_init, img_api_run_job, img_api_shutdown. "
#             "Required for scripting and batch processing workflows.",
#         )
#     )

#     print(f"{'=' * 60}")
#     print(f"{BOLD}WHAT TO DO NEXT — PRIORITY ORDER{NC}")
#     print(f"{'=' * 60}")

#     for priority, level, color, action in sorted(actions):
#         print(f"\n  [{priority}] {color}{BOLD}{level}{NC}")
#         # Word-wrap at 68 characters for readable terminal output
#         words = action.split()
#         line = "      "
#         for word in words:
#             if len(line) + len(word) + 1 > 72:
#                 print(line.rstrip())
#                 line = "      " + word + " "
#             else:
#                 line += word + " "
#         if line.strip():
#             print(line.rstrip())

#     print(f"\n{'=' * 60}\n")


# # ============================================================
# # SECTION 8: ENTRY POINT
# # ============================================================


# def main():
#     """
#     Entry point. Run the full analysis pipeline and print all reports.

#     Exit codes (for CI integration):
#       0 = architecture clean, no violations
#       1 = violations found, do not merge

#     To integrate with GitHub Actions:
#       - jobs:
#           analyze:
#             steps:
#               - run: python3 scripts/l10plusplus.py
#     """
#     print(f"\n{BOLD}imgengine L10++ Architecture Analyzer{NC}")
#     print(f"{'=' * 60}\n")
#     print("Scanning source files...")

#     t0 = time.time()
#     analyzer = Analyzer().run()
#     elapsed = time.time() - t0

#     metrics = Metrics(analyzer)
#     your_fn_count = len(metrics.your_functions())
#     edge_count = sum(len(v) for v in analyzer.call_graph.values())

#     print(f"{your_fn_count} functions | {edge_count} edges | {elapsed:.1f}s\n")

#     report_hot_functions(metrics)
#     report_bottlenecks(metrics)
#     report_complex_functions(metrics)
#     report_layer_violations(analyzer)
#     report_module_heat(metrics, analyzer)
#     report_files_to_split(analyzer)
#     report_unused_functions(metrics)
#     report_next_actions(analyzer, metrics)

#     # Final verdict and exit code
#     seen = set()
#     unique_viol = sum(
#         1
#         for v in analyzer.violations
#         if (v[0], v[1]) not in seen and not seen.add((v[0], v[1]))
#     )

#     if unique_viol > 0:
#         print(f"{RED}{BOLD}VIOLATIONS FOUND: {unique_viol} — fix before merging{NC}\n")
#         sys.exit(1)
#     else:
#         print(f"{GREEN}{BOLD}ARCHITECTURE CLEAN — zero layer violations{NC}\n")
#         sys.exit(0)


# if __name__ == "__main__":
#     main()
# ******************************************************************************************************************************
# ******************************************************************************************************************************

# #!/usr/bin/env python3
# """
# L10++ Architecture Analyzer — imgengine
# Kernel-grade static analysis: call graph, hot paths, layer violations,
# ABI checks, duplicate symbols, NUMA distribution.

# Filters out compiler intrinsics and system functions so results
# reflect YOUR code only.
# """

# import os
# import re
# import sys
# import time
# import threading
# from collections import defaultdict, deque

# # ============================================================
# # SYSTEM FUNCTION FILTER
# # Exclude intrinsics, builtins, libc, and system APIs from analysis.
# # These inflate the graph with thousands of false bottlenecks.
# # ============================================================

# SYSTEM_PREFIXES = (
#     "_mm_",
#     "_mm256_",
#     "_mm512_",
#     "__builtin_",
#     "__builtin_ia32_",
#     "io_uring_",
#     "tj",
#     "stbi_",
#     "numa_",
#     "pthread_",
#     "atomic_",
#     "printf",
#     "fprintf",
#     "sprintf",
#     "snprintf",
#     "malloc",
#     "free",
#     "calloc",
#     "realloc",
#     "aligned_alloc",
#     "memcpy",
#     "memset",
#     "memmove",
#     "memcmp",
#     "open",
#     "close",
#     "read",
#     "write",
#     "lseek",
#     "mmap",
#     "munmap",
#     "mprotect",
#     "fopen",
#     "fclose",
#     "fwrite",
#     "fread",
#     "abort",
#     "exit",
#     "atoi",
#     "atof",
# )

# SYSTEM_EXACT = {
#     "main",  # exclude from bottleneck — it's the entry point
# }


# def is_system_fn(name):
#     if not name:
#         return True
#     if name in SYSTEM_EXACT:
#         return True
#     for prefix in SYSTEM_PREFIXES:
#         if name.startswith(prefix):
#             return True
#     return False


# # ============================================================
# # LAYER DAG
# # ============================================================
# # ================================================================
# # CORRECTED LAYER DAG
# # Update scripts/l10plusplus.py LAYERS list to this order:
# #  ================================================================

# # LAYERS = [
# #     "types",  # 0: img_result_t, img_buffer_t, opcodes — no deps
# #     "memory",  # 1: slab, arena, numa — depends on types only
# #     "arch",  # 2: SIMD kernels, cpu_caps — depends on types only
# #     "security",  # 3: validation, sandbox — depends on types only
# #     "pipeline",  # 4: jump table, fused kernels — depends on arch, memory
# #     "runtime",  # 5: workers, queues — depends on pipeline
# #     "plugins",  # 6: plugin ABI — depends on pipeline
# #     "observability",  # 7: metrics, logging — depends on types
# #     "io",  # 8: decode, encode — depends on memory, security
# #     "api",  # 9: public surface — depends on everything below
# #     "cmd",  # 10: CLI — depends on api only
# #     "startup",  # 11: engine init — depends on all layers
# # ]

# LAYERS = [
#     "memory",         # 0 — slab, arena, numa
#     "core",           # 1 — types, buffer, opcodes, batch
#     "arch",           # 2 — SIMD kernels, cpu_caps, resize_params
#     "security",       # 3 — validation, sandbox
#     "observability",  # 4 — metrics, logging, tracing
#     "pipeline",       # 5 — jump table, fused kernels, dispatch, job
#     "runtime",        # 6 — workers, queues, scheduler
#     "plugins",        # 7 — plugin ABI (pipeline/plugin_abi.h)
#     "io",             # 8 — decode, encode, vfs
#     "api",            # 9 — public surface (re-exports from lower layers)
#     "cmd",            # 10 — CLI
#     "startup",        # 11 — engine init, wires everything
# ]

# # LAYERS = [
# #     "core",
# #     "memory",
# #     "security",
# #     "arch",
# #     "pipeline",
# #     "runtime",
# #     "plugins",
# #     "observability",
# #     "api",
# #     "io",
# #     "cmd",
# # ]

# ABI_CONTRACTS = {
#     "img_kernel_fn": "(img_ctx_t *, img_buffer_t *, void *)",
#     "img_single_kernel_fn": "(img_ctx_t *, img_buffer_t *)",
#     "img_fused_kernel_fn": "(img_ctx_t *, img_batch_t *, void *)",
# }

# RED = "\033[0;31m"
# GREEN = "\033[0;32m"
# YELLOW = "\033[1;33m"
# CYAN = "\033[0;36m"
# BOLD = "\033[1m"
# NC = "\033[0m"


# # ============================================================
# # STATIC ANALYZER (no libclang dependency)
# # Pure regex — fast, no external deps, works in any environment.
# # ============================================================


# class StaticAnalyzer:
#     def __init__(self):
#         self.call_graph = defaultdict(set)  # caller → {callee}
#         self.reverse_graph = defaultdict(set)  # callee → {callers}
#         self.fn_to_file = {}
#         self.layer_graph = defaultdict(set)
#         self.file_deps = defaultdict(set)
#         self.violations = []
#         self.failed = False

#         # Regex patterns
#         self.fn_def = re.compile(
#             r"^\s*(?:static\s+)?(?:inline\s+)?"
#             r"(?:__attribute__\s*\(\([^)]*\)\)\s*)?"
#             r"(?:void|int|uint\w*|size_t|bool|img_\w+|char\s*\*?)\s+"
#             r"(\w+)\s*\([^)]*\)\s*(?:\{|$)",
#             re.MULTILINE,
#         )
#         self.fn_call = re.compile(r"\b(\w+)\s*\(")
#         self.include = re.compile(r'#include\s+"([^"]+)"')

#     @staticmethod
#     def strip_comments(content):
#         # Keep the analyzer focused on real dependencies, not commented examples.
#         content = re.sub(r"/\*.*?\*/", "", content, flags=re.S)
#         content = re.sub(r"//.*", "", content)
#         return content

#     def get_layer(self, path):
#         rel = os.path.relpath(path, ".")
#         for root in ("src", "include", "api"):
#             if rel.startswith(root + os.sep):
#                 rel = rel[len(root) + 1 :]
#                 break
#         layer = rel.split(os.sep)[0]
#         return layer if layer in LAYERS else "unknown"

#     def layer_idx(self, layer):
#         try:
#             return LAYERS.index(layer)
#         except ValueError:
#             return -1

#     def analyze_file(self, path):
#         try:
#             content = open(path, encoding="utf-8", errors="ignore").read()
#         except Exception:
#             return

#         content = self.strip_comments(content)
#         layer = self.get_layer(path)

#         # Extract includes
#         for inc in self.include.findall(content):
#             self.file_deps[path].add(inc)
#             inc_layer = inc.split("/")[0]
#             if inc_layer in LAYERS:
#                 self.layer_graph[layer].add(inc_layer)
#                 # Layer violation check
#                 si = self.layer_idx(layer)
#                 ti = self.layer_idx(inc_layer)
#                 if si != -1 and ti != -1 and ti > si:
#                     self.violations.append(
#                         f"{path} includes {inc} ({layer}[{si}] → {inc_layer}[{ti}])"
#                     )
#                     self.failed = True

#         # Extract function definitions
#         current_fns = []
#         for m in self.fn_def.finditer(content):
#             fn = m.group(1)
#             if not is_system_fn(fn):
#                 current_fns.append(fn)
#                 self.fn_to_file[fn] = path

#         # Extract calls (heuristic: any identifier followed by '(')
#         for caller in current_fns:
#             for callee in self.fn_call.findall(content):
#                 if callee != caller and not is_system_fn(callee):
#                     self.call_graph[caller].add(callee)
#                     self.reverse_graph[callee].add(caller)

#     def run(self):
#         for root, dirs, files in os.walk("."):
#             dirs[:] = [d for d in dirs if d not in ("build", ".git", "__pycache__")]
#             if not any(
#                 root.startswith(f".{os.sep}{r}") for r in ("src", "include", "api")
#             ):
#                 continue
#             for f in files:
#                 if f.endswith((".c", ".h")):
#                     self.analyze_file(os.path.join(root, f))
#         return self


# # ============================================================
# # CALL GRAPH METRICS
# # ============================================================


# class CallGraphMetrics:
#     def __init__(self, analyzer):
#         self.a = analyzer

#     def fan_in(self, fn):
#         return len(self.a.reverse_graph.get(fn, set()))

#     def fan_out(self, fn):
#         return len(self.a.call_graph.get(fn, set()))

#     def depth(self, fn, visited=None):
#         if visited is None:
#             visited = set()
#         if fn in visited:
#             return 0
#         visited.add(fn)
#         callees = self.a.call_graph.get(fn, set())
#         if not callees:
#             return 0
#         return 1 + max((self.depth(c, visited) for c in callees), default=0)

#     def score(self, fn):
#         return self.fan_in(fn) + 0.5 * self.fan_out(fn)

#     def your_functions(self):
#         """Only functions defined in your source files."""
#         return {fn for fn in self.a.fn_to_file if not is_system_fn(fn)}


# # ============================================================
# # REPORTS
# # ============================================================


# def report_hot_paths(metrics):
#     your_fns = list(metrics.your_functions())
#     scored = [
#         (metrics.score(f), metrics.fan_in(f), metrics.fan_out(f), f)
#         for f in your_fns
#         if metrics.score(f) > 0
#     ]
#     scored.sort(reverse=True)

#     print(f"\n{BOLD}HOT FUNCTIONS (your code only){NC}")
#     print(f"{'Function':<40} {'Score':>6} {'Fan-in':>7} {'Fan-out':>8}")
#     print("-" * 65)
#     for score, fi, fo, fn in scored[:20]:
#         flag = f"{RED}🔥 HOT{NC}" if score > 5 else ""
#         print(f"  {fn:<38} {score:>6.1f} {fi:>7} {fo:>8}  {flag}")


# def report_bottlenecks(metrics):
#     your_fns = metrics.your_functions()
#     bottlenecks = [(metrics.fan_in(f), f) for f in your_fns if metrics.fan_in(f) >= 3]
#     bottlenecks.sort(reverse=True)

#     print(f"\n{BOLD}REAL BOTTLENECKS (your functions called by ≥3 callers){NC}")
#     for count, fn in bottlenecks[:15]:
#         callers = sorted(metrics.a.reverse_graph.get(fn, set()))
#         print(f"  {CYAN}{fn}{NC} ← {count} callers")
#         for c in callers[:5]:
#             print(f"      ← {c}")
#         if len(callers) > 5:
#             print(f"      ... and {len(callers) - 5} more")


# def report_complex_functions(metrics):
#     your_fns = metrics.your_functions()
#     complex_fns = [(metrics.fan_out(f), f) for f in your_fns if metrics.fan_out(f) >= 5]
#     complex_fns.sort(reverse=True)

#     print(f"\n{BOLD}COMPLEX FUNCTIONS (fan-out ≥ 5 — consider splitting){NC}")
#     for count, fn in complex_fns[:10]:
#         file = metrics.a.fn_to_file.get(fn, "unknown")
#         rel = os.path.relpath(file, ".")
#         print(f"  {YELLOW}{fn}{NC} → {count} calls | {rel}")


# def report_layer_violations(analyzer):
#     print(f"\n{BOLD}LAYER VIOLATIONS (upward dependencies){NC}")
#     if not analyzer.violations:
#         print(f"  {GREEN}✅ None — layer DAG is clean{NC}")
#     else:
#         for v in analyzer.violations:
#             print(f"  {RED}❌ {v}{NC}")


# def report_module_summary(metrics):
#     module_scores = defaultdict(float)
#     module_counts = defaultdict(int)

#     for fn in metrics.your_functions():
#         file = metrics.a.fn_to_file.get(fn, "")
#         layer = metrics.a.get_layer(file)
#         module_scores[layer] += metrics.score(fn)
#         module_counts[layer] += 1

#     print(f"\n{BOLD}MODULE HEAT (which layers need attention){NC}")
#     print(f"{'Layer':<20} {'Functions':>10} {'Total Score':>12} {'Avg Score':>10}")
#     print("-" * 56)
#     items = [(module_scores[l], l) for l in LAYERS if module_counts[l] > 0]
#     for score, layer in sorted(items, reverse=True):
#         count = module_counts[layer]
#         avg = score / count if count else 0
#         bar = "█" * min(int(avg * 2), 20)
#         print(f"  {layer:<18} {count:>10} {score:>12.1f} {avg:>10.1f}  {bar}")


# def report_files_to_split(analyzer):
#     print(f"\n{BOLD}FILES TO SPLIT (too many dependencies){NC}")
#     items = [(len(v), k) for k, v in analyzer.file_deps.items() if len(v) >= 7]
#     items.sort(reverse=True)
#     for count, path in items[:8]:
#         rel = os.path.relpath(path, ".")
#         print(f"  {YELLOW}❌ {rel}{NC} → {count} deps → consider splitting")


# def report_abi_locations(analyzer):
#     print(f"\n{BOLD}ABI CONTRACT TABLE{NC}")
#     for type_name, sig in ABI_CONTRACTS.items():
#         files = [
#             f
#             for f, content in [
#                 (p, open(p, errors="ignore").read())
#                 for p in analyzer.fn_to_file.values()
#                 if os.path.exists(p)
#             ]
#             if type_name in content
#         ]
#         locations = list(set(os.path.relpath(f, ".") for f in files))
#         loc_str = locations[0] if locations else "not found"
#         print(f"  {CYAN}{type_name}{NC}")
#         print(f"    sig: {sig}")
#         print(f"    def: {loc_str}")


# def report_unused_functions(metrics):
#     your_fns = metrics.your_functions()
#     # Functions defined but never called by any other function
#     unused = [
#         f
#         for f in your_fns
#         if metrics.fan_in(f) == 0
#         and f
#         not in (
#             "main",
#             "img_api_init",
#             "img_api_shutdown",
#             "img_api_run_job",
#             "img_api_process_raw",
#         )
#     ]
#     unused.sort()

#     if not unused:
#         print(f"\n{GREEN}✅ No unused functions detected{NC}")
#         return

#     print(f"\n{BOLD}POTENTIALLY UNUSED FUNCTIONS (fan-in = 0){NC}")
#     print(f"  (may be exported symbols or entry points — verify manually)")
#     for fn in unused[:20]:
#         file = os.path.relpath(metrics.a.fn_to_file.get(fn, ""), ".")
#         print(f"  {fn}  [{file}]")


# # ============================================================
# # NEXT ACTIONS (what to do now)
# # ============================================================


# def report_next_actions(analyzer, metrics):
#     print(f"\n{'=' * 60}")
#     print(f"{BOLD}WHAT TO DO NEXT — PRIORITY ORDER{NC}")
#     print(f"{'=' * 60}")

#     actions = []

#     # 1. Layer violations
#     if analyzer.violations:
#         actions.append(
#             (
#                 1,
#                 "CRITICAL",
#                 f"Fix {len(analyzer.violations)} layer violation(s) — "
#                 "upward dependencies break module boundaries",
#             )
#         )

#     # 2. Complex functions
#     complex_fns = [
#         (metrics.fan_out(f), f)
#         for f in metrics.your_functions()
#         if metrics.fan_out(f) >= 8
#     ]
#     for count, fn in sorted(complex_fns, reverse=True)[:3]:
#         file = os.path.relpath(metrics.a.fn_to_file.get(fn, ""), ".")
#         actions.append(
#             (2, "HIGH", f"Split {fn}() — {count} outgoing calls — in {file}")
#         )

#     # 3. Missing implementations
#     actions.append(
#         (
#             3,
#             "HIGH",
#             "Replace resize_nearest() with bilinear AVX2 "
#             "(src/pipeline/layout.c) — current is nearest-neighbour",
#         )
#     )

#     actions.append(
#         (
#             3,
#             "HIGH",
#             "Wire io_uring output path — currently main.c initializes "
#             "io_uring but img_api_run_job() writes via fwrite(), not io_uring",
#         )
#     )

#     actions.append(
#         (
#             4,
#             "MEDIUM",
#             "Add --mode fit|fill CLI flag to expose IMG_FIT/IMG_FILL "
#             "(job->mode is set but not wired to CLI args)",
#         )
#     )

#     actions.append(
#         (
#             4,
#             "MEDIUM",
#             "Add img_api_run_job_raw() — returns encoded bytes instead of "
#             "writing to file; enables io_uring output and Python/Go bindings",
#         )
#     )

#     actions.append(
#         (
#             5,
#             "MEDIUM",
#             "Add CI performance gate: bench_lat P99 > 2ms = build fail "
#             "(RFC §22.3 requirement)",
#         )
#     )

#     actions.append(
#         (
#             5,
#             "LOW",
#             "Implement PDF multi-page: current pdf_encoder.c = single page; "
#             "for large grids that exceed A4, add page continuation",
#         )
#     )

#     actions.append(
#         (
#             6,
#             "LOW",
#             "Add Python CFFI bindings (bindings/python/cffi_wrapper.py) — "
#             "expose img_api_init, img_api_run_job, img_api_shutdown",
#         )
#     )

#     for priority, level, action in sorted(actions):
#         color = RED if level == "CRITICAL" else YELLOW if level == "HIGH" else NC
#         print(f"\n  [{priority}] {color}{level}{NC}")
#         print(f"      {action}")

#     print(f"\n{'=' * 60}")


# # ============================================================
# # MAIN
# # ============================================================


# def main():
#     print(f"{BOLD}imgengine L10++ Architecture Analyzer{NC}")
#     print(f"{'=' * 60}\n")

#     print("📡 Scanning source files...")
#     t0 = time.time()
#     analyzer = StaticAnalyzer().run()
#     elapsed = time.time() - t0

#     your_fns = {fn for fn in analyzer.fn_to_file if not is_system_fn(fn)}
#     print(
#         f"✅ {len(your_fns)} functions | "
#         f"{sum(len(v) for v in analyzer.call_graph.values())} edges | "
#         f"{elapsed:.1f}s\n"
#     )

#     metrics = CallGraphMetrics(analyzer)

#     report_hot_paths(metrics)
#     report_bottlenecks(metrics)
#     report_complex_functions(metrics)
#     report_layer_violations(analyzer)
#     report_module_summary(metrics)
#     report_files_to_split(analyzer)
#     report_unused_functions(metrics)
#     report_next_actions(analyzer, metrics)

#     if analyzer.failed:
#         print(f"\n{RED}{BOLD}🚨 VIOLATIONS FOUND — fix before merging{NC}\n")
#         sys.exit(1)
#     else:
#         print(f"\n{GREEN}{BOLD}✅ ARCHITECTURE CLEAN{NC}\n")


# if __name__ == "__main__":
#     main()


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
