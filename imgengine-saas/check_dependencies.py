# # check_dependencies.py

import os
import sys

# Constants for project layout
ROOT = os.path.dirname(os.path.abspath(__file__))
API_DIR = "api-service"
WORKER_DIR = "worker-service"

# Define strict isolation rules
# Format: { "service_folder": ["list", "of", "forbidden", "imports"] }
POLICIES = {
    WORKER_DIR: [
        "app",
        "api_service",
        "fastapi",
        "uvicorn",
    ],  # Worker shouldn't know about API/Web
    API_DIR: [
        "worker_service",
        "engine_runner",
        "tasks",
    ],  # API shouldn't touch the engine
}

# Directories to ignore entirely
IGNORE_DIRS = {".venv", "__pycache__", ".git", "build", "dist"}


def get_imports_from_file(filepath):
    """Extracts all top-level imports from a file."""
    imports = []
    try:
        with open(filepath, "r", encoding="utf-8") as f:
            for line in f:
                line = line.strip()
                if line.startswith("import ") or line.startswith("from "):
                    imports.append(line)
    except Exception as e:
        print(f"⚠️ Could not read {filepath}: {e}")
    return imports


def check_service(service_name, forbidden_list):
    violations = 0
    service_path = os.path.join(ROOT, service_name)

    print(f"🔍 Auditing Boundary: {service_name}")

    for root, dirs, files in os.walk(service_path):
        # Google/Amazon Grade: Prune ignored directories to save time and prevent false positives
        dirs[:] = [d for d in dirs if d not in IGNORE_DIRS]

        for file in files:
            if not file.endswith(".py"):
                continue

            full_path = os.path.join(root, file)
            relative_path = os.path.relpath(full_path, ROOT)
            file_imports = get_imports_from_file(full_path)

            for imp in file_imports:
                for forbidden in forbidden_list:
                    # Match exact module names to avoid partial string hits
                    if f" {forbidden}" in imp or f".{forbidden}" in imp:
                        print(f"  ❌ BOUNDARY VIOLATION: {relative_path}")
                        print(f"     -> Found: '{imp}' (Policy: No '{forbidden}')")
                        violations += 1
    return violations


def main():
    print("🛡️  ImgEngine Microservice Boundary Guard\n" + "=" * 40)
    total_violations = 0

    for service, forbidden in POLICIES.items():
        total_violations += check_service(service, forbidden)
        print("-" * 40)

    if total_violations > 0:
        print(
            f"\n🚨 DEPLOYMENT BLOCKED: {total_violations} architectural violations found."
        )
        sys.exit(1)  # Fail the CI/CD pipeline
    else:
        print("\n✅ ARCHITECTURE CLEAN: All boundaries respected.")
        sys.exit(0)


if __name__ == "__main__":
    main()
