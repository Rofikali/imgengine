#!/usr/bin/env python3
"""
scripts/clang-format-check.py

Verify that source files conform to `clang-format -style=file`.
Exits with status 0 if all files match, 1 otherwise.

Accepts an optional list of positional file paths (CMake passes
${CLANG_FORMAT_FILES} this way). If none are given, discovers
files under src/, include/, and tests/.
"""
import argparse
import subprocess
import sys
from pathlib import Path
import difflib


def find_files(root: Path):
    patterns = ("src", "include", "tests")
    exts = {".c", ".h", ".cc", ".cpp", ".hpp", ".cxx", ".inc"}
    files = []
    for p in patterns:
        base = root / p
        if not base.exists():
            continue
        for f in base.rglob("*"):
            if f.is_file() and f.suffix.lower() in exts:
                if "build" in f.parts:
                    continue
                files.append(f)
    for f in root.glob("*.c"):
        files.append(f)
    for f in root.glob("*.h"):
        files.append(f)
    return sorted(set(files))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--clang",
        default="clang-format",
        help="clang-format executable",
    )
    # CMake expands ${CLANG_FORMAT_FILES} as positional args.
    # nargs="*" collects them into a list; zero args is also fine.
    parser.add_argument(
        "files",
        nargs="*",
        help="Source files to check (optional; discovered automatically if omitted)",
    )
    args = parser.parse_args()

    repo = Path(__file__).resolve().parent.parent

    if args.files:
        # CMake passed the file list — use it, filtering to real files only.
        files = sorted({Path(f) for f in args.files if Path(f).is_file()})
    else:
        files = find_files(repo)

    if not files:
        print("[fmt-check] No source files found")
        return 0

    try:
        subprocess.run(
            [args.clang, "--version"],
            check=True,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
    except Exception:
        print(f"[fmt-check] clang-format not found at '{args.clang}'", file=sys.stderr)
        return 2

    bad = []
    for f in files:
        src = f.read_text(encoding="utf-8", errors="replace")
        try:
            proc = subprocess.run(
                [args.clang, "--style=file", str(f)],
                check=False,
                capture_output=True,
                text=True,
            )
        except FileNotFoundError:
            print(f"[fmt-check] clang-format not found: {args.clang}", file=sys.stderr)
            return 2

        formatted = proc.stdout
        if formatted != src:
            bad.append(f)
            print(f"---- {f} ----")
            diff = difflib.unified_diff(
                src.splitlines(),
                formatted.splitlines(),
                fromfile=str(f),
                tofile=str(f) + " (formatted)",
                lineterm="",
            )
            for line in diff:
                print(line)

    if bad:
        print(f"\n[fmt-check] {len(bad)} file(s) need formatting.")
        print("Run:  make format   (or clang-format -style=file -i <files>)")
        return 1

    print(f"[fmt-check] {len(files)} file(s) checked — all clean.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
    
# #!/usr/bin/env python3
# """
# scripts/clang-format-check.py

# Verify that source files conform to `clang-format -style=file`.
# Exits with status 0 if all files match, 1 otherwise.
# """
# import argparse
# import subprocess
# import sys
# from pathlib import Path
# import difflib


# def find_files(root: Path):
#     patterns = ("src", "include", "tests")
#     exts = {".c", ".h", ".cc", ".cpp", ".hpp", ".cxx", ".inc"}
#     files = []
#     for p in patterns:
#         base = root / p
#         if not base.exists():
#             continue
#         for f in base.rglob("*"):
#             if f.is_file() and f.suffix.lower() in exts:
#                 # skip build artifacts
#                 if "build" in f.parts:
#                     continue
#                 files.append(f)
#     # also include top-level .c/.h
#     for f in root.glob("*.c"):
#         files.append(f)
#     for f in root.glob("*.h"):
#         files.append(f)
#     return sorted(set(files))


# def main():
#     parser = argparse.ArgumentParser()
#     parser.add_argument("--clang", default="clang-format", help="clang-format executable")
#     args = parser.parse_args()

#     repo = Path(__file__).resolve().parent.parent
#     files = find_files(repo)
#     if not files:
#         print("[fmt-check] No source files found")
#         return 0

#     try:
#         subprocess.run([args.clang, "--version"], check=True, stdout=subprocess.DEVNULL)
#     except Exception:
#         print(f"[fmt-check] clang-format not found at '{args.clang}'", file=sys.stderr)
#         return 2

#     bad = []
#     for f in files:
#         src = f.read_text(encoding="utf-8", errors="replace")
#         try:
#             proc = subprocess.run([args.clang, "--style=file", str(f)], check=False, capture_output=True, text=True)
#         except FileNotFoundError:
#             print(f"[fmt-check] clang-format not found: {args.clang}", file=sys.stderr)
#             return 2
#         formatted = proc.stdout
#         if formatted != src:
#             bad.append(f)
#             print(f"---- {f} ----")
#             diff = difflib.unified_diff(src.splitlines(), formatted.splitlines(), fromfile=str(f), tofile=str(f) + " (formatted)", lineterm="")
#             for line in diff:
#                 print(line)

#     if bad:
#         print(f"[fmt-check] {len(bad)} files need formatting")
#         return 1

#     print("[fmt-check] All files are formatted")
#     return 0


# if __name__ == "__main__":
#     sys.exit(main())
