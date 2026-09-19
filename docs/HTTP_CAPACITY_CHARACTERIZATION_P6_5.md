# P6.5 HTTP Capacity Characterization

**Status:** CHARACTERIZATION evidence only. This is not a production-capacity
claim, a user-capacity claim, or dedicated-host evidence.

## Scope and configuration

This report measures the existing loopback HTTP path without altering its
configuration:

```text
Axum -> RequestLifecycleService -> bounded Rust admission -> C scheduler
     -> ABI v1 -> C native engine
```

| Setting | Value | Status |
| --- | --- | --- |
| HTTP/lifecycle input limit | 1 MiB | provisional |
| Rust admission queue | 1 waiting request | provisional |
| Rust admission worker | 1 | current tested configuration |
| Execution deadline | 10 seconds post-dequeue | provisional |
| Native cancellation | unavailable in ABI v1 | unchanged |

The harness uses real loopback TCP requests to `POST /api/v1/render`, real
multipart JPEG/PNG bodies, `X-API-Key`, and the production adapter defaults.
It does not create a second engine, change admission, or bypass the lifecycle.

## Environment and reproducibility

The following single run was made with `imgengine-verify:ubuntu24` under Docker
Desktop/WSL2, on 2026-09-18. It is Docker/WSL2 characterization, not dedicated
Linux host evidence.

| Field | Observed value |
| --- | --- |
| Git state | uncommitted P5/P6 worktree |
| OS | Ubuntu 24.04.5 LTS |
| Kernel | 5.15.167.4-microsoft-standard-WSL2 |
| Rust/Cargo | 1.75.0 / 1.75.0 |
| C compiler | GCC 13.3.0 |
| CMake / Ninja | 3.28.3 / 1.11.1 |
| CPU exposed to container | 12th Gen Intel Core i5-1235U; 12 logical CPUs |
| Memory exposed to container | 3,930,036 KiB |
| Native build | Debug, no LTO, no sanitizers |
| Harness build | Rust release |

Run from the Ubuntu verifier container with the repository mounted at the same
path, for example:

```bash
docker run --rm -v "$PWD:/workspace/repo" -w /workspace/repo/imgengine \
  imgengine-verify:ubuntu24 bash scripts/run_http_capacity_characterization.sh
```

The command records the exact environment, fixture SHA-256 hashes, JSON raw
measurements, and Bash `time -p` process CPU data beneath the ignored
`imgengine/build-http-characterization/results/` directory. Each run creates
deterministic ImageMagick fixtures: small JPEG (736 B), small PNG (419 B), and
uses the project-owned `photo.jpg` as the large JPEG (814,549 B). The recorded
fixture SHA-256 values were respectively `bfca7742…e55d803`,
`cb6adb35…ab068e86`, and `dfe5cf3d…96134c41`.

## Method

Two warm-up requests precede each sequential workload. Each sequential fixture
uses 8 measured requests. Concurrent levels 1, 2, 3, and 4 use 12 batches each
(12, 24, 36, and 48 attempts). Latency is client-observed socket-write through
complete HTTP-response-read time; it therefore includes loopback HTTP,
multipart parsing, admission waiting, native execution, and response delivery.
It is not a native-processing-only metric. Percentiles are nearest-rank values
from these small samples and must not be over-interpreted.

## Recorded results

All figures below are from the one described run. Latencies are microseconds.

| Sequential fixture | 200 responses | P50 | P95 | P99 | Mean | Output total |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| small JPEG | 8/8 | 4,620 | 6,224 | 6,224 | 5,042 | 16,624 B |
| small PNG | 8/8 | 4,922 | 5,601 | 5,601 | 4,916 | 15,600 B |
| large JPEG | 8/8 | 200,408 | 215,189 | 215,189 | 200,530 | 7,464,000 B |

| Concurrent level | Attempts | 200 | 429 | P50 | P95 | P99 | Attempt throughput |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | 12 | 12 | 0 | 201,874 | 219,374 | 219,374 | 4.89/s |
| 2 | 24 | 21 | 3 | 210,701 | 425,474 | 443,623 | 5.59/s attempted |
| 3 | 36 | 24 | 12 | 201,881 | 454,392 | 541,950 | 7.10/s attempted |
| 4 | 48 | 24 | 24 | 8,158 | 415,340 | 511,737 | 9.65/s attempted |

At levels 3 and 4, 429 is the expected bounded-admission response, rather than
a capacity-test error. There were no 503 responses. The recorded admission
snapshot was 122 accepted/completed, 112 succeeded, 10 failed (the intentional
malformed inputs), 39 rejected for overload, zero resource-limit rejections,
zero deadline expirations, and maximum queue depth 1. This confirms the current
one-active plus one-waiting bounded behavior without changing the queue.

Malformed claimed JPEGs returned 422 in all 8/8 samples (P50 4,132 us); an
unsupported media type returned 415 in all 8/8 samples (P50 174 us). A healthy
JPEG immediately afterward returned 200. Workspace inspection after shutdown
was clean. The harness observed no response abandonment. Its in-process RSS
snapshot was 51,048 KiB current / 100,208 KiB high-water; whole-harness process
accounting was 20.15 user CPU seconds and 1.85 system CPU seconds across 72.79
real seconds (including build/tooling). Container and host memory behaviour
need dedicated-host repetition before comparison.

## Deadline, disconnect, and shutdown

This load run did not induce a 10-second expiry: all valid requests completed
well within it. Existing deterministic evidence remains authoritative for those
semantics:

- P6.4 proves expiry returns 504, discards late output, cleans the workspace,
  and does not cancel ABI-v1 native work; it also proves an accepted deadline
  result is drained during graceful shutdown.
- P6.2 proves real loopback TCP upload disconnect has no admission/native work,
  while post-admission disconnect produces exactly one abandonment event without
  cancelling native work and leaves admission healthy.
- P6.3 proves active and queued accepted work drains, a live pre-admission
  request receives 503 while draining, listener closure follows, and the worker
  joins. A new connection after listener closure is intentionally a
  connection-level result, not a required 503.

## Interpretation and limitations

The observed saturation boundary matches the intentionally small configuration:
two concurrent large requests can occupy active plus queue capacity; above that,
the adapter promptly returns 429. This is a behavioural characterization, not a
recommendation to raise queue size or worker count. The substantial latency
spread at levels 2–4 is expected when accepted work waits behind the single
worker, and cannot diagnose a C-scheduler bottleneck from this workload alone.

Known limits remain unchanged: no output-byte ceiling, no per-workspace or
aggregate disk quota, no selected product decoded-pixel policy, no queue-wait
budget, and no response-write timeout. This run provides no external-network,
TLS/proxy, multi-process, long-duration, CPU-throttling, or deployment
termination evidence.

Dedicated Linux follow-up requires at least repeated runs on a pinned host with
CPU governor and memory-pressure controls, a fixed container/runtime version,
collected CPU utilization and cgroup RSS, longer steady-state windows, output
validation, and separate network/client placement. Only that work can support
operational capacity planning; it must still preserve the C scheduler unless
new measured evidence justifies an explicit architectural decision.
