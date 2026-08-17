# backend/service/engine_runner.py

import subprocess
import resource
import time
import os

from app.core.config import ENGINE_CPU_TIME_SECONDS, ENGINE_MEMORY_LIMIT_BYTES, ENGINE_TIMEOUT_SECONDS, MAX_OUTPUT_BYTES
from app.core.storage import artifact_store


def apply_resource_limits() -> None:
    resource.setrlimit(resource.RLIMIT_AS, (ENGINE_MEMORY_LIMIT_BYTES, ENGINE_MEMORY_LIMIT_BYTES))
    resource.setrlimit(resource.RLIMIT_CPU, (ENGINE_CPU_TIME_SECONDS, ENGINE_CPU_TIME_SECONDS))


def run_engine(job: dict):
    artifact_store.ensure_directories()
    input_path = artifact_store.download(job["input"])
    output_path = artifact_store.path_for(job["output"])
    cmd = [
        "imgengine_cli",
        "--input",
        str(input_path),
        "--output",
        str(output_path),
    ]
    if job.get("preset"):
        cmd.extend(["--preset", job["preset"]])
    else:
        cmd.extend([
            "--cols", str(job["cols"]),
            "--rows", str(job["rows"]),
            "--gap", str(job["gap"]),
            "--dpi", str(job["dpi"]),
            "--border", str(job["border"]),
            "--padding", str(job["padding"]),
            "--crop-mark", str(job["crop_mark"]),
            "--crop-thickness", str(job["crop_thickness"]),
            "--bleed", str(job["bleed"]),
            "--crop-offset", str(job["crop_offset"]),
            "--width", str(job["width"]),
            "--height", str(job["height"]),
        ])

    try:
        started_at = time.perf_counter()
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            timeout=ENGINE_TIMEOUT_SECONDS,
            preexec_fn=apply_resource_limits,
            env={**os.environ, "IMGENGINE_TRACE_ID": job["trace_id"]},
        )
    except subprocess.TimeoutExpired:
        output_path.unlink(missing_ok=True)
        return {
            "returncode": 1,
            "stdout": "",
            "stderr": "Engine execution exceeded the configured time limit.",
            "output_bytes": 0,
            "duration_ms": round((time.perf_counter() - started_at) * 1000, 2),
        }

    output_valid = (
        result.returncode == 0
        and output_path.is_file()
        and 0 < output_path.stat().st_size <= MAX_OUTPUT_BYTES
    )
    stderr = result.stderr
    if result.returncode == 0 and not output_valid:
        output_path.unlink(missing_ok=True)
        stderr = "Engine did not create an output within the configured size limit."
    if output_valid:
        artifact_store.upload(job["output"])

    return {
        "returncode": 0 if output_valid else (result.returncode or 1),
        "stdout": result.stdout,
        "stderr": stderr,
        "output_bytes": output_path.stat().st_size if output_valid else 0,
        "duration_ms": round((time.perf_counter() - started_at) * 1000, 2),
    }
