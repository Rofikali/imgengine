#!/usr/bin/env python3
import os
import sys


class HeaderChecker:
    def __init__(self, root="../"):
        self.root = os.path.abspath(root)
        self.extensions = (".c", ".h")
        self.ignored_dirs = {"build", ".git"}

        self.errors = []

    # ============================================================
    # EXPECTED HEADER
    # ============================================================

    def expected_header(self, file_path):
        rel = os.path.relpath(file_path, self.root)
        return f"// ./{rel}"

    # ============================================================
    # VALIDATE FILE
    # ============================================================

    def check_file(self, file_path):
        try:
            with open(file_path, "r", encoding="utf-8", errors="ignore") as f:
                lines = f.readlines()
        except:
            return

        if not lines:
            self.errors.append(f"{file_path}: EMPTY FILE")
            return

        expected = self.expected_header(file_path)

        # Count header occurrences at top
        header_count = 0
        for line in lines:
            if line.strip().startswith("// ./"):
                header_count += 1
            else:
                break

        # --------------------------------------------------------
        # RULE 1: Must have exactly ONE header at top
        # --------------------------------------------------------
        if header_count == 0:
            self.errors.append(f"{file_path}: MISSING HEADER")
            return

        if header_count > 1:
            self.errors.append(f"{file_path}: DUPLICATE HEADERS ({header_count})")
            return

        # --------------------------------------------------------
        # RULE 2: Header must match exact path
        # --------------------------------------------------------
        if lines[0].strip() != expected:
            self.errors.append(
                f"{file_path}: WRONG HEADER\n"
                f"  Expected: {expected}\n"
                f"  Found:    {lines[0].strip()}"
            )

    # ============================================================
    # WALK PROJECT
    # ============================================================

    def run(self):
        for root, dirs, files in os.walk(self.root):
            dirs[:] = [d for d in dirs if d not in self.ignored_dirs]

            for file in files:
                if file.endswith(self.extensions):
                    full_path = os.path.join(root, file)
                    self.check_file(full_path)

        # --------------------------------------------------------
        # RESULT
        # --------------------------------------------------------
        if self.errors:
            print("❌ HEADER CHECK FAILED:\n")
            for err in self.errors:
                print(" -", err)

            print("\n🚨 Fix headers before merging.")
            sys.exit(1)
        else:
            print("✅ Header check passed.")


if __name__ == "__main__":
    HeaderChecker().run()