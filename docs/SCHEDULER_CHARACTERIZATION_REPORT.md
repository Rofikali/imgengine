# Scheduler Characterization Decision — 2026-09-14

## Architectural Decision

**Measured decision:** retain the boundary below. No C scheduler migration,
rewrite, or Rust scheduler is authorized by this evidence.

```text
Request -> Rust admission/control -> C scheduler -> C processing engine -> Output
```

Rust owns bounded admission, overload mapping, lifecycle policy, and safe FFI
ownership. C owns engine serialization, scheduling, native resources, sandbox
lifecycle, decoding, rendering, and output ownership.

## Controlled Repetitions

Three complete runs were executed in order; none was discarded. Each ran the
same committed contract at `248f19f4a08362480758aa635539e254772ff941` in
`imgengine-verify:ubuntu24-format-check` pinned with Docker `--cpuset-cpus=0-3`.

| Run | UTC start | Result directory | Records | Result |
|---|---|---|---:|---|
| 1 | 2026-09-14 11:53:07 | `pinned-20260914-run1` | 24 | complete |
| 2 | 2026-09-14 11:54:30 | `pinned-20260914-run2` | 24 | complete |
| 3 | 2026-09-14 11:55:57 | `pinned-20260914-run3` | 24 | complete |

All raw JSONL, environment, and per-run Markdown output are retained locally
under `imgengine/build/scheduler-characterization-results/`. They are ignored
because they are host-specific evidence, not source artifacts.

### Environment and Workload

- Ubuntu 24.04.4, Linux 5.15.167.4-microsoft-standard-WSL2, Docker Desktop.
- Intel Core i5-1235U (12 logical CPUs; runs constrained to CPUs 0–3),
  3,930,036 KiB visible memory.
- GCC 13.3.0, CMake 3.28.3, Rust/Cargo 1.75.0; Release build; LTO disabled.
- Every workload: 30 measured attempts, 5 sequential warm-ups, concurrency 4;
  Rust queue capacity 4 except the saturation case (capacity 1, concurrency 8).
- Public `image.jpeg_encode` path: JPEG output bytes are counted per request;
  valid inputs retain their decoded dimensions under the existing ABI behavior.
  The exact input dimensions, bytes, hashes, and output-byte totals are in each
  run's `environment.txt` and `results.jsonl`.

### Licensed Camera Fixture

`tests/fixtures/cc0_camera_landscape.jpg` is a 690,175-byte, 2048×1365 sRGB
JPEG camera photograph. Its source, CC0 1.0 redistribution right, author
attribution, retrieval date, and enforced SHA-256 are documented in
`imgengine/tests/fixtures/README.md`. The runner refuses a checksum mismatch.
The pre-existing undocumented `imgengine/photo.jpg` was not used as this
project-owned fixture.

### Input and Output Characteristics

The sequential C and Rust paths emitted identical output byte counts within
each run. The generated fixture bytes varied only where ImageMagick produced a
fresh deterministic-workload file per run; the fixed CC0 fixture did not vary.
All successful outputs are JPEG through `image.jpeg_encode`.

| Fixture | Input dimensions | Input bytes | Output bytes per success across runs |
|---|---:|---:|---:|
| small JPEG | 320×240 | 3,857 | 5,766 |
| small PNG | 320×240 | 1,415 | 5,395 |
| representative JPEG | 1600×1200 | 347,072 | 428,919–429,324 |
| large JPEG | 2048×1536 | 537,850 | 667,350–667,855 |
| progressive JPEG | 640×427 | 57,032 | 72,340–73,036 |
| CC0 camera JPEG | 2048×1365 | 690,175 | 566,531 |

## Individual Results

Each cell is `C direct -> Rust admission` and contains P50/P95 wall latency in
milliseconds followed by throughput in operations/second. These are measured
facts, not scalability claims.

| Workload | Run 1 | Run 2 | Run 3 |
|---|---|---|---|
| small JPEG, sequential | `0.729/1.121, 692.25 -> 1.351/9.685, 464.65` | `0.670/0.864, 731.67 -> 1.319/11.744, 472.82` | `0.679/0.862, 962.84 -> 1.387/13.543, 333.81` |
| small PNG, sequential | `1.555/21.630, 305.32 -> 1.711/10.933, 318.63` | `1.791/22.023, 278.31 -> 1.959/19.479, 205.26` | `1.485/21.602, 344.41 -> 1.883/20.778, 215.39` |
| representative JPEG, sequential | `45.536/74.555, 19.24 -> 50.029/80.593, 18.21` | `44.485/69.215, 20.32 -> 49.211/71.318, 19.21` | `44.264/71.305, 19.36 -> 48.111/74.562, 19.30` |
| large JPEG, sequential | `79.680/104.216, 12.24 -> 79.432/106.492, 12.04` | `79.481/103.707, 12.49 -> 80.377/112.440, 11.60` | `77.867/86.626, 12.82 -> 79.267/102.455, 12.36` |
| progressive JPEG, sequential | `8.293/29.257, 64.63 -> 19.355/30.013, 58.72` | `8.087/29.172, 66.05 -> 19.354/29.938, 58.86` | `7.492/28.635, 68.80 -> 18.792/29.027, 58.71` |
| CC0 camera JPEG, sequential | `174.599/197.687, 5.59 -> 186.366/203.996, 5.41` | `162.889/191.820, 5.95 -> 168.572/205.641, 5.72` | `172.668/200.654, 5.56 -> 185.261/201.112, 5.48` |
| truncated JPEG, sequential | `0/0, 1280573.70 -> 0/0, 2632.86` | `0/0, 1268606.22 -> 0/0, 561.13` | `0/0, 1220603.79 -> 0/0, 1340.72` |
| small JPEG, concurrent | `1.581/2.327, 211.49 -> 3.206/18.290, 455.74` | `1.906/3.268, 193.54 -> 3.768/18.448, 359.63` | `2.120/3.561, 210.80 -> 2.610/12.232, 547.68` |
| representative JPEG, concurrent | `99.984/144.476, 24.89 -> 111.640/225.942, 19.56` | `100.108/148.252, 25.36 -> 109.769/203.294, 20.38` | `109.991/170.166, 24.25 -> 113.900/220.999, 19.19` |
| large JPEG, concurrent | `170.000/282.009, 14.08 -> 173.579/326.321, 12.68` | `179.927/279.956, 14.13 -> 185.765/356.363, 12.26` | `180.015/300.624, 12.84 -> 187.282/343.202, 12.06` |
| CC0 camera JPEG, concurrent | `376.278/685.191, 5.96 -> 417.957/790.814, 5.25` | `360.026/680.493, 5.94 -> 393.844/779.322, 5.43` | `359.966/666.458, 6.02 -> 413.024/764.680, 5.32` |
| small JPEG, saturation | `1.264/2.207, 360.47 -> 1.892/2.858, 2225.52` | `1.420/2.491, 352.10 -> 1.696/2.908, 2256.65` | `1.580/2.268, 336.72 -> 2.400/12.786, 1087.76` |

The malformed row has no successful completions, so P50/P95 are zero by the
benchmark schema and throughput means attempts/second, not successful output.

## Aggregate Results

The following are arithmetic means across the three whole-run records. CPU is
process user + system milliseconds for each 30-attempt workload. RSS is peak
resident KiB. Output classification was identical between paths: every valid
row completed 90/90; malformed input failed 90/90; no output was produced for
malformed input.

| Workload/path | P50 ms | P95 ms | ops/s | CPU ms | Peak RSS KiB | Success/fail/reject | Max queue |
|---|---:|---:|---:|---:|---:|---:|---:|
| small JPEG seq / C | 0.693 | 0.949 | 795.58 | 38.5 | 39,199 | 90/0/0 | 0 |
| small JPEG seq / Rust | 1.353 | 11.657 | 423.76 | 70.0 | 41,363 | 90/0/0 | 1 |
| small PNG seq / C | 1.610 | 21.752 | 309.34 | 97.3 | 40,536 | 90/0/0 | 0 |
| small PNG seq / Rust | 1.851 | 17.063 | 246.42 | 130.0 | 42,245 | 90/0/0 | 1 |
| representative JPEG seq / C | 44.762 | 71.692 | 19.64 | 1,527.8 | 46,505 | 90/0/0 | 0 |
| representative JPEG seq / Rust | 49.117 | 75.491 | 18.90 | 1,596.7 | 48,625 | 90/0/0 | 1 |
| large JPEG seq / C | 79.009 | 98.183 | 12.52 | 2,397.3 | 50,911 | 90/0/0 | 0 |
| large JPEG seq / Rust | 79.692 | 107.129 | 12.00 | 2,500.0 | 53,749 | 90/0/0 | 1 |
| progressive JPEG seq / C | 7.957 | 29.021 | 66.49 | 451.4 | 42,012 | 90/0/0 | 0 |
| progressive JPEG seq / Rust | 19.167 | 29.659 | 58.76 | 513.3 | 43,643 | 90/0/0 | 1 |
| CC0 camera seq / C | 170.052 | 196.721 | 5.70 | 5,264.7 | 66,407 | 90/0/0 | 0 |
| CC0 camera seq / Rust | 180.066 | 203.583 | 5.53 | 5,423.3 | 66,917 | 90/0/0 | 1 |
| small JPEG concurrent / C | 1.869 | 3.052 | 205.28 | 98.8 | 47,824 | 90/0/0 | 0 |
| small JPEG concurrent / Rust | 3.195 | 16.323 | 454.35 | 70.0 | 41,412 | 90/0/0 | 3–4 |
| representative JPEG concurrent / C | 103.361 | 154.298 | 24.83 | 1,204.6 | 88,067 | 90/0/0 | 0 |
| representative JPEG concurrent / Rust | 111.770 | 216.745 | 19.71 | 1,520.0 | 49,892 | 90/0/0 | 4 |
| large JPEG concurrent / C | 176.647 | 287.530 | 13.69 | 2,188.2 | 89,628 | 90/0/0 | 0 |
| large JPEG concurrent / Rust | 182.209 | 341.962 | 12.34 | 2,433.3 | 56,016 | 90/0/0 | 4 |
| CC0 camera concurrent / C | 365.423 | 677.381 | 5.97 | 5,013.9 | 120,152 | 90/0/0 | 0 |
| CC0 camera concurrent / Rust | 408.275 | 778.272 | 5.33 | 5,630.0 | 69,195 | 90/0/0 | 4 |
| small JPEG saturation / Rust | 1.996 | 6.184 | 1,856.64 | 23.3 | 41,451 | 23/67/67 | 1 |

The direct C saturation row is intentionally omitted from the aggregate table:
it has no bounded admission queue or overload counter and therefore is not a
comparable overload-policy measurement. Its three runs completed all 90
attempts, with mean P50/P95 1.421/2.322 ms and 349.77 attempts/s.

## Threshold Assessment

**Provisional thresholds** in `SCHEDULER_BENCHMARK_CONTRACT.md` permit at most
15% sequential Rust P50/P95 overhead and at most 25% concurrent Rust P95
overhead when capacity is at least concurrency. These are guardrails, not
product promises.

**Measured facts:** the three-run mean exceeds the sequential limit for small
JPEG (P50 +95%, P95 +1128%) and progressive JPEG (P50 +141%). It meets the
limit for representative JPEG, large JPEG, and the CC0 camera fixture. It
exceeds the concurrent P95 limit for small JPEG (+435%) and representative
JPEG (+40%), while large JPEG (+19%) and CC0 camera JPEG (+15%) meet it.
Individual runs vary under WSL2, but the small and representative concurrent
failures are sufficiently consistent that no threshold is relaxed here.

**Architectural decision:** scheduler migration is not justified. Rust
admission is valuable because it provides a bounded queue, explicit overload
response, and cleanup telemetry; it is not evidence that Rust should own native
scheduling. All Rust workspaces were empty after orderly shutdown. Accepted
work drained, requests after shutdown were rejected by tests, and no
mid-operation cancellation was introduced.

## Next Architectural Question

The next smallest high-value Rust responsibility to evaluate is the
**HTTP/application request lifecycle boundary**: request-size and request-shape
validation, admission/error mapping, deadline policy, request IDs, redacted
lifecycle metrics, and temporary-workspace ownership. This is control-plane
work that can be measured for error-contract parity and cleanup correctness
without moving C scheduling, memory management, codecs, SIMD, or rendering.

This is a future hypothesis only. It requires a separate design and evidence
gate before implementation.
