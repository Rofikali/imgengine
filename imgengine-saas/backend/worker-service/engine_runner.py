# backend/service/engine_runner.py

import subprocess

from app.core.storage import artifact_store


def run_engine(job: dict):
    input_path = artifact_store.download(job["input"])
    output_path = artifact_store.path_for(job["output"])
    cmd = [
        "imgengine_cli",
        "--input",
        str(input_path),
        "--output",
        str(output_path),
        "--cols",
        str(job["cols"]),
        "--rows",
        str(job["rows"]),
        "--gap",
        str(job["gap"]),
        "--dpi",
        str(job["dpi"]),
        "--border",
        str(job["border"]),
        "--padding",
        str(job["padding"]),
        "--crop-mark",
        str(job["crop_mark"]),
        "--crop-thickness",
        str(job["crop_thickness"]),
        "--bleed",
        str(job["bleed"]),
        "--crop-offset",
        str(job["crop_offset"]),
        "--width",
        str(job["width"]),
        "--height",
        str(job["height"]),
    ]

    result = subprocess.run(cmd, capture_output=True, text=True)

    output_valid = result.returncode == 0 and output_path.is_file() and output_path.stat().st_size > 0
    stderr = result.stderr
    if result.returncode == 0 and not output_valid:
        stderr = f"Engine completed without creating a non-empty output file: {output_path}"
    if output_valid:
        artifact_store.upload(job["output"])

    return {
        "returncode": 0 if output_valid else (result.returncode or 1),
        "stdout": result.stdout,
        "stderr": stderr,
    }
