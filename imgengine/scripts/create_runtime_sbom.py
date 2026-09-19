#!/usr/bin/env python3
"""Create an SPDX 2.3 JSON SBOM for a staged IMGENGINE Linux runtime package."""

import argparse
import hashlib
import json
import os
import re
import subprocess
import sys
from datetime import UTC, datetime
from pathlib import Path


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as artifact:
        for chunk in iter(lambda: artifact.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def run(command: list[str], environment: dict[str, str] | None = None) -> str:
    return subprocess.check_output(command, text=True, stderr=subprocess.DEVNULL, env=environment).strip()


def is_elf(path: Path) -> bool:
    with path.open("rb") as artifact:
        return artifact.read(4) == b"\x7fELF"


def ldd_paths(path: Path, library_path: str) -> set[Path]:
    environment = os.environ.copy()
    environment["LD_LIBRARY_PATH"] = library_path
    paths: set[Path] = set()
    for line in run(["ldd", str(path)], environment).splitlines():
        match = re.search(r"=>\s+(/[^\s]+)", line)
        if not match and line.lstrip().startswith("/"):
            match = re.match(r"\s*(/[^\s]+)", line)
        if match:
            resolved = Path(match.group(1)).resolve()
            if resolved.exists():
                paths.add(resolved)
    return paths


def system_package(path: Path) -> tuple[str, str]:
    try:
        owner = run(["dpkg-query", "-S", str(path)]).split(":", 1)[0].split(",", 1)[0]
        version = run(["dpkg-query", "-W", "-f=${Version}", owner])
        return owner, version
    except (OSError, subprocess.CalledProcessError):
        return path.name, "NOASSERTION"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--stage", required=True, type=Path, help="CMake install staging root")
    parser.add_argument("--version", required=True, help="IMGENGINE release version")
    parser.add_argument("--git-revision", required=True, help="source Git revision")
    parser.add_argument("--output", type=Path, help="SBOM output (defaults inside stage)")
    args = parser.parse_args()

    if sys.platform != "linux":
        parser.error("runtime SBOM generation requires Linux")
    stage = args.stage.resolve()
    if not stage.is_dir():
        parser.error(f"stage directory does not exist: {stage}")

    packaged_files = sorted(path for path in stage.rglob("*") if path.is_file())
    elf_files = [path for path in packaged_files if is_elf(path)]
    if not elf_files:
        parser.error("stage directory contains no ELF artifacts")

    stage_library_path = str(stage / "lib")
    dependencies: set[Path] = set()
    for artifact in elf_files:
        dependencies.update(ldd_paths(artifact, stage_library_path))

    stage_files = {path.resolve() for path in packaged_files}
    external_dependencies = sorted(path for path in dependencies if path not in stage_files)
    packages = [
        {
            "SPDXID": "SPDXRef-Package-imgengine",
            "name": "imgengine",
            "versionInfo": args.version,
            "downloadLocation": "NOASSERTION",
            "filesAnalyzed": True,
            "licenseConcluded": "NOASSERTION",
            "licenseDeclared": "NOASSERTION",
            "supplier": "NOASSERTION",
        }
    ]
    relationships = [{"spdxElementId": "SPDXRef-DOCUMENT", "relationshipType": "DESCRIBES", "relatedSpdxElement": "SPDXRef-Package-imgengine"}]
    for dependency in external_dependencies:
        name, version = system_package(dependency)
        identifier = "SPDXRef-System-" + hashlib.sha256(str(dependency).encode()).hexdigest()[:16]
        packages.append(
            {
                "SPDXID": identifier,
                "name": name,
                "versionInfo": version,
                "downloadLocation": "NOASSERTION",
                "filesAnalyzed": False,
                "licenseConcluded": "NOASSERTION",
                "licenseDeclared": "NOASSERTION",
                "supplier": "NOASSERTION",
                "externalRefs": [{"referenceCategory": "OTHER", "referenceType": "runtime-library-path", "referenceLocator": str(dependency)}],
            }
        )
        relationships.append({"spdxElementId": "SPDXRef-Package-imgengine", "relationshipType": "DEPENDS_ON", "relatedSpdxElement": identifier})

    files = [
        {
            "SPDXID": "SPDXRef-File-" + hashlib.sha256(path.relative_to(stage).as_posix().encode()).hexdigest()[:16],
            "fileName": "./" + path.relative_to(stage).as_posix(),
            "checksums": [{"algorithm": "SHA256", "checksumValue": sha256(path)}],
            "licenseConcluded": "NOASSERTION",
            "copyrightText": "NOASSERTION",
        }
        for path in packaged_files
    ]
    document = {
        "spdxVersion": "SPDX-2.3",
        "dataLicense": "CC0-1.0",
        "SPDXID": "SPDXRef-DOCUMENT",
        "name": f"imgengine-{args.version}-linux-x86_64-runtime",
        "documentNamespace": f"https://spdx.org/spdxdocs/imgengine-{args.version}-{args.git_revision}",
        "creationInfo": {"created": datetime.now(UTC).replace(microsecond=0).isoformat().replace("+00:00", "Z"), "creators": ["Tool: imgengine/scripts/create_runtime_sbom.py"]},
        "packages": packages,
        "files": files,
        "relationships": relationships,
    }
    output = args.output or stage / "runtime.spdx.json"
    output.write_text(json.dumps(document, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"runtime SPDX SBOM written: {output}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
