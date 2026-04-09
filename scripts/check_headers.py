#!/usr/bin/env python3
import os
import sys

ROOT = "../"
EXTENSIONS = (".c", ".h")
IGNORED_DIRS = {"build", ".git", ".github"}

errors = []


def expected_header(path):
    rel = os.path.relpath(path, ROOT)
    return f"// ./{rel}"


def check_file(path):
    try:
        with open(path, "r", encoding="utf-8", errors="ignore") as f:
            first = f.readline().strip()
    except:
        return

    exp = expected_header(path)

    if first != exp:
        errors.append((path, exp))


def walk():
    for root, dirs, files in os.walk(ROOT):
        dirs[:] = [d for d in dirs if d not in IGNORED_DIRS]

        for file in files:
            if file.endswith(EXTENSIONS):
                check_file(os.path.join(root, file))


if __name__ == "__main__":
    print("🔍 Running header check...\n")

    walk()

    if errors:
        print("❌ Missing/incorrect headers:\n")
        for path, exp in errors:
            print(f"{path}")
            print(f"  expected: {exp}\n")

        sys.exit(1)  # 🔥 FAIL CI
    else:
        print("✅ All headers correct")