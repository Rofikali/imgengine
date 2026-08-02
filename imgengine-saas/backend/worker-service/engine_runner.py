# backend/service/engine_runner.py

import subprocess
from pathlib import Path


def run_engine(job: dict):
    cmd = [
        "imgengine_cli",
        "--input",
        job["input"],
        "--output",
        job["output"],
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

    output_path = Path(job["output"])
    output_valid = result.returncode == 0 and output_path.is_file() and output_path.stat().st_size > 0
    stderr = result.stderr
    if result.returncode == 0 and not output_valid:
        stderr = f"Engine completed without creating a non-empty output file: {output_path}"

    return {
        "returncode": 0 if output_valid else (result.returncode or 1),
        "stdout": result.stdout,
        "stderr": stderr,
    }
