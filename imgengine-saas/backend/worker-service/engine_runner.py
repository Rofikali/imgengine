# backend/service/engine_runner.py

import subprocess


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

    return {
        "returncode": result.returncode,
        "stdout": result.stdout,
        "stderr": result.stderr,
    }
