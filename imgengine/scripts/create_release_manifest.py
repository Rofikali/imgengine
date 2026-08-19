#!/usr/bin/env python3
"""Create a deterministic manifest for a staged IMGENGINE Linux package."""

import argparse
import hashlib
import json
import os
import re
import subprocess
import sys
from pathlib import Path


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as artifact:
        for chunk in iter(lambda: artifact.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def git_revision(source_root: Path) -> str:
    try:
        return subprocess.check_output(
            ["git", "-C", str(source_root), "rev-parse", "HEAD"], text=True
        ).strip()
    except (OSError, subprocess.CalledProcessError):
        return "unknown"


def project_version(source_root: Path) -> str:
    cmake = (source_root / "CMakeLists.txt").read_text(encoding="utf-8")
    match = re.search(r"project\(imgengine\s+VERSION\s+([0-9]+\.[0-9]+\.[0-9]+)", cmake)
    if not match:
        raise ValueError("could not determine IMGENGINE project version")
    return match.group(1)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--stage", required=True, type=Path, help="CMake install staging directory")
    parser.add_argument("--source-root", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--output", type=Path, help="Manifest path (defaults inside stage)")
    parser.add_argument("--release-version", help="Expected release version")
    args = parser.parse_args()

    stage = args.stage.resolve()
    source_root = args.source_root.resolve()
    if not stage.is_dir():
        parser.error(f"stage directory does not exist: {stage}")

    version = project_version(source_root)
    if args.release_version and args.release_version != version:
        parser.error(
            f"release version {args.release_version} does not match CMake project version {version}"
        )

    required_paths = (
        Path("bin/imgengine_cli"),
        Path("include/imgengine/api/v1/img_api.h"),
        Path("lib/libimgengine.so"),
        Path("lib/imgengine/plugins/libplugin_resize.so"),
        Path("runtime.spdx.json"),
    )
    missing = [str(path) for path in required_paths if not (stage / path).exists()]
    if missing:
        parser.error("staged package missing required files: " + ", ".join(missing))

    files = []
    for artifact in sorted(path for path in stage.rglob("*") if path.is_file()):
        relative = artifact.relative_to(stage).as_posix()
        if relative == "manifest.json":
            continue
        files.append({"path": relative, "sha256": sha256(artifact), "size": artifact.stat().st_size})

    manifest = {
        "schema_version": 1,
        "name": "imgengine",
        "version": version,
        "git_revision": git_revision(source_root),
        "platform": "linux-x86_64",
        "runtime_requirements": ["libnuma", "libturbojpeg", "liburing"],
        "files": files,
    }
    output = args.output or stage / "manifest.json"
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"release manifest written: {output}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
