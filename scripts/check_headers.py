#!/usr/bin/env python3
import os
import sys

# 🔥 STRICT ENGINE ROOT
ENGINE_ROOT = "../imgengine"

EXTENSIONS = (".c", ".h")

IGNORED_DIRS = {
    "build",
    ".git",
    ".github",
    "__pycache__"
}

errors = []


# ============================================================
# EXPECTED HEADER
# ============================================================

def expected_header(path):
    rel = os.path.relpath(path, ENGINE_ROOT)
    return f"// ./imgengine/{rel}"


# ============================================================
# CHECK FILE
# ============================================================

def check_file(path):
    try:
        with open(path, "r", encoding="utf-8", errors="ignore") as f:
            first = f.readline().strip()
    except:
        return

    exp = expected_header(path)

    if first != exp:
        errors.append((path, exp))


# ============================================================
# WALK ONLY ENGINE
# ============================================================

def walk():
    for root, dirs, files in os.walk(ENGINE_ROOT):
        dirs[:] = [d for d in dirs if d not in IGNORED_DIRS]

        for file in files:
            if file.endswith(EXTENSIONS):
                check_file(os.path.join(root, file))


# ============================================================

if __name__ == "__main__":
    print(f"🔍 Checking headers inside '{ENGINE_ROOT}/' only...\n")

    if not os.path.exists(ENGINE_ROOT):
        print(f"❌ Missing required folder: {ENGINE_ROOT}/")
        sys.exit(1)

    walk()

    if errors:
        print("❌ Header violations:\n")

        for path, exp in errors:
            print(f"{path}")
            print(f"  expected: {exp}\n")

        sys.exit(1)

    print("✅ All headers correct (ENGINE STRICT MODE)")