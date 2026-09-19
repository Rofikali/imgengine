#!/usr/bin/env python3
"""Create immutable evidence for locally built Compose service images."""

from __future__ import annotations

import argparse
import datetime as dt
import hashlib
import json
import subprocess
import sys
from pathlib import Path


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def inspect_image(reference: str) -> dict[str, object]:
    result = subprocess.run(
        ["docker", "image", "inspect", reference],
        check=False,
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        raise ValueError(f"could not inspect image {reference!r}: {result.stderr.strip()}")

    records = json.loads(result.stdout)
    if len(records) != 1:
        raise ValueError(f"expected one inspection record for image {reference!r}")
    record = records[0]
    image_id = record.get("Id")
    if not isinstance(image_id, str) or not image_id.startswith("sha256:"):
        raise ValueError(f"image {reference!r} has no immutable image ID")

    return {
        "reference": reference,
        "local_image_id": image_id,
        "deployment_identity": image_id,
        "deployment_identity_source": "local_image_id",
        "created": record.get("Created"),
        "os": record.get("Os"),
        "architecture": record.get("Architecture"),
    }


def parse_service_image(value: str) -> tuple[str, str]:
    service, separator, image = value.partition("=")
    if not separator or not service or not image:
        raise argparse.ArgumentTypeError("service images must use SERVICE=IMAGE")
    return service, image


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compose-config", required=True, type=Path)
    parser.add_argument("--candidate-version", required=True)
    parser.add_argument("--git-revision", required=True)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--service-image", action="append", default=[], type=parse_service_image)
    args = parser.parse_args()

    if len(args.git_revision) < 7:
        parser.error("--git-revision must be a Git revision")
    if not args.candidate_version.strip():
        parser.error("--candidate-version must not be empty")
    if not args.compose_config.is_file():
        parser.error(f"compose config does not exist: {args.compose_config}")
    if not args.service_image:
        parser.error("at least one --service-image is required")

    service_images = dict(args.service_image)
    if len(service_images) != len(args.service_image):
        parser.error("each service can be specified only once")

    try:
        images = {service: inspect_image(reference) for service, reference in sorted(service_images.items())}
    except (ValueError, json.JSONDecodeError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1

    manifest = {
        "schema_version": 1,
        "artifact_type": "imgengine_saas_container_candidate",
        "candidate_version": args.candidate_version,
        "git_revision": args.git_revision,
        "created_at": dt.datetime.now(dt.timezone.utc).replace(microsecond=0).isoformat(),
        "compose_config": {
            "path": args.compose_config.name,
            "sha256": sha256_file(args.compose_config),
        },
        "service_images": images,
        "identity_policy": {
            "local_build": "A Docker image ID identifies an unpushed candidate image.",
            "registry_publish": "Registry digests are deliberately absent: an approved push must resolve and record them before deployment.",
        },
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
