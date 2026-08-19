#!/usr/bin/env python3
"""Validate comparable benchmark evidence and summarize a proposed baseline."""

import argparse
import json
import re
import statistics
import sys
from pathlib import Path


REQUIRED_METADATA = (
    "git_revision",
    "fixture_sha256",
    "fixture_bytes",
    "preset",
    "iterations",
    "warmup",
    "git_worktree",
    "compiler",
    "cmake_version",
)
P95_PATTERN = re.compile(r"^\s*p95:\s+([0-9]+(?:\.[0-9]+)?) ms", re.MULTILINE)
P99_PATTERN = re.compile(r"^\s*p99:\s+([0-9]+(?:\.[0-9]+)?) ms", re.MULTILINE)


def parse_environment(path: Path) -> dict[str, str]:
    values: dict[str, str] = {}
    for line in path.read_text(encoding="utf-8").splitlines():
        if "=" in line:
            key, value = line.split("=", 1)
            values[key] = value
        elif line.startswith("Model name:"):
            values["cpu_model"] = line.split(":", 1)[1].strip()
    return values


def stage_metric(text: str, label: str, pattern: re.Pattern[str]) -> float:
    start = text.find(label)
    if start < 0:
        raise ValueError(f"missing benchmark stage: {label}")
    next_stage = text.find("\n---", start + len(label))
    section = text[start:] if next_stage < 0 else text[start:next_stage]
    match = pattern.search(section)
    if not match:
        raise ValueError(f"missing metric for benchmark stage: {label}")
    return float(match.group(1))


def load_run(directory: Path) -> dict[str, object]:
    environment_file = directory / "environment.txt"
    latency_file = directory / "latency.txt"
    preflight_file = directory / "preflight.txt"
    for required_file in (environment_file, latency_file, preflight_file):
        if not required_file.is_file():
            raise ValueError(f"missing required evidence file: {required_file}")

    metadata = parse_environment(environment_file)
    missing = [key for key in REQUIRED_METADATA if not metadata.get(key)]
    if missing:
        raise ValueError(f"{directory}: missing metadata: {', '.join(missing)}")
    if metadata["git_worktree"] != "clean":
        raise ValueError(f"{directory}: benchmark was taken from a dirty worktree")

    preflight = parse_environment(preflight_file)
    if preflight.get("cpu_governor") != "performance":
        raise ValueError(f"{directory}: strict performance governor evidence is missing")

    latency = latency_file.read_text(encoding="utf-8")
    return {
        "directory": str(directory),
        "metadata": metadata,
        "prepared_render_p95_ms": stage_metric(
            latency, "Prepared render stage", P95_PATTERN
        ),
        "prepared_render_p99_ms": stage_metric(
            latency, "Prepared render stage", P99_PATTERN
        ),
    }


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Validate comparable native benchmark runs and write a baseline summary."
    )
    parser.add_argument("results", nargs="+", type=Path, help="benchmark evidence directories")
    parser.add_argument("--output", required=True, type=Path, help="summary JSON path")
    parser.add_argument("--minimum-runs", type=int, default=5)
    args = parser.parse_args()

    if args.minimum_runs < 1:
        parser.error("--minimum-runs must be positive")
    if len(args.results) < args.minimum_runs:
        parser.error(f"at least {args.minimum_runs} result directories are required")

    try:
        runs = [load_run(directory) for directory in args.results]
    except ValueError as error:
        print(f"baseline analysis failed: {error}", file=sys.stderr)
        return 2

    reference = runs[0]["metadata"]
    comparison_keys = REQUIRED_METADATA + ("cpu_model",)
    for run in runs[1:]:
        metadata = run["metadata"]
        incompatible = [
            key
            for key in comparison_keys
            if metadata.get(key) != reference.get(key)
        ]
        if incompatible:
            print(
                f"baseline analysis failed: incompatible evidence in {run['directory']}: "
                + ", ".join(incompatible),
                file=sys.stderr,
            )
            return 2

    p95_values = [run["prepared_render_p95_ms"] for run in runs]
    p99_values = [run["prepared_render_p99_ms"] for run in runs]
    summary = {
        "schema_version": 1,
        "metric": "prepared_render",
        "unit": "milliseconds",
        "run_count": len(runs),
        "metadata": {key: reference.get(key) for key in comparison_keys},
        "median_run_p95_ms": statistics.median(p95_values),
        "worst_run_p99_ms": max(p99_values),
        "runs": runs,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(summary, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(json.dumps(summary, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    sys.exit(main())
