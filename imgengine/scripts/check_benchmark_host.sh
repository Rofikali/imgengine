#!/usr/bin/env bash
set -euo pipefail

strict=0
if [[ "${1:-}" == "--strict" ]]; then
    strict=1
    shift
fi
if [[ $# -ne 0 ]]; then
    echo "Usage: $(basename "$0") [--strict]" >&2
    exit 64
fi

if [[ "$(uname -s)" != "Linux" ]]; then
    echo "Benchmark host must be Linux." >&2
    exit 1
fi

for command in cmake ninja cc lscpu sha256sum; do
    command -v "$command" >/dev/null || {
        echo "Missing required command: $command" >&2
        exit 1
    }
done

shopt -s nullglob
governor_files=(/sys/devices/system/cpu/cpu*/cpufreq/scaling_governor)
issues=0

echo "kernel=$(uname -r)"
echo "cpu_count=$(getconf _NPROCESSORS_ONLN)"
if (( ${#governor_files[@]} == 0 )); then
    echo "cpu_governor=unavailable"
    issues=1
else
    declare -A governors=()
    for governor_file in "${governor_files[@]}"; do
        governor="$(<"$governor_file")"
        governors["$governor"]=1
    done
    echo "cpu_governor=$(printf '%s,' "${!governors[@]}" | sed 's/,$//')"
    if [[ ${#governors[@]} -ne 1 || -z "${governors[performance]:-}" ]]; then
        issues=1
    fi
fi

if [[ -r /sys/devices/system/cpu/intel_pstate/no_turbo ]]; then
    echo "intel_turbo_disabled=$(</sys/devices/system/cpu/intel_pstate/no_turbo)"
fi
if [[ -r /sys/devices/system/cpu/cpufreq/boost ]]; then
    echo "cpu_boost_disabled=$(</sys/devices/system/cpu/cpufreq/boost)"
fi

if (( strict && issues )); then
    echo "Benchmark runner is not pinned to the performance governor." >&2
    exit 1
fi
