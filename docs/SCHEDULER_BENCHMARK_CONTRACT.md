# Scheduler Characterization and Benchmark Contract

**Status:** Decision gate. This document defines evidence required before any
C scheduler responsibility may move to Rust.

## Scope and Boundaries

The comparison has two actual paths, both using `libimgengine` ABI v1 and the
same native C execution engine:

1. **C direct ABI:** a C benchmark calls
   `imgengine_process_encoded_image_to_jpeg` on one engine handle.
2. **Rust admission:** callers submit owned bytes to the bounded Rust channel;
   its dedicated worker owns the same safe ABI wrapper and native engine.

Neither path is a new Rust scheduler. The ABI serializes operations on the
single engine handle. Therefore concurrent measurements characterize caller
contention, queueing, overload, and lifecycle behavior—not parallel native
render throughput.

The direct C ABI borrows fixture bytes, while Rust admission owns queued bytes.
The reported wall-time difference intentionally includes Rust's required copy
and channel handoff. Rust additionally reports worker processing time; ABI v1
does not expose an equivalent internal processing timestamp for direct C calls,
so that field is `null` for the C path.

## Workloads

`scripts/run_scheduler_characterization.sh` deterministically creates these
CI-safe project-owned fixtures using ImageMagick:

- 320×240 JPEG and PNG;
- 1600×1200 representative textured JPEG;
- 2048×1536 large JPEG;
- 640×427 progressive JPEG;
- a 2048×1365 project-owned CC0 camera-landscape JPEG, with an enforced
  SHA-256 recorded in `tests/fixtures/README.md`;
- truncated JPEG for a failed-completion path.

Sequential, concurrent, and queue-saturation runs are included. The script can
also measure a local `--real-image` without copying it to results or source
control. Local images are supplemental only; the tracked CC0 fixture provides
the reproducible real-camera workload.

## Metrics and Method

Each JSONL record includes fixture metadata, request count, warm-up,
concurrency, queue capacity, current/max queue depth, completion/failure/
overload counts, cleanup result, wall P50/P95, Rust worker P50/P95,
throughput, process CPU time, and peak RSS. `environment.txt` captures the
commit, Ubuntu container, compiler, CMake, Rust, CPU, memory, fixture hashes,
dimensions, and command parameters.

Warm-up requests are excluded from measured latency and completion counts.
P50/P95 use nearest-rank values over successful measured requests. Throughput
uses all measured attempts divided by total wall time; saturation results must
be interpreted with their rejection count.

Run in the verification image:

```bash
docker run --rm \
  -e IMGENGINE_GIT_REVISION="$(git rev-parse HEAD)" \
  -v "$PWD:/workspace" -w /workspace/imgengine \
  imgengine-verify:ubuntu24-format-check \
  bash scripts/run_scheduler_characterization.sh \
    --iterations 30 --warmup 5 --concurrency 4 --queue-capacity 4
```

## Provisional Acceptance Thresholds

There is no historical ABI-v1 scheduler baseline. The first successful run is
the baseline and must be retained with its environment metadata. Until three
stable runs on a pinned host exist, thresholds are provisional:

- for successful sequential fixtures, Rust admission P50 and P95 wall latency
  may be no more than **15%** above direct C ABI;
- for successful concurrent fixtures with queue capacity at least concurrency,
  Rust admission P95 may be no more than **25%** above direct C ABI;
- both paths must preserve output success/failure classification; Rust must
  leave no workspace entry after completion, failure, or orderly shutdown;
- saturation must reject excess work rather than growing an unbounded queue;
- new submissions after shutdown must return `unavailable`; accepted work must
  drain because ABI v1 has no mid-operation cancellation.

These percentages are guardrails for review, not performance promises. Replace
them with host-calibrated limits after three reproducible measurements. A
failed threshold blocks migration; it does not justify changing the C scheduler.

## Lifecycle Evidence

The Rust supervisor test suite covers queue saturation, overload response,
input limits, malformed input, cleanup after success/failure, dropped response
handles, submissions after shutdown, orderly drain, and repeated start/stop
cycles. Full Linux verification continues to cover C regressions, ASAN/UBSAN,
sandbox lifecycle, ABI consumers, real-image validation, and fuzz smoke runs.

## Decision Rule

Choose one explicitly after recorded results:

- **A. Keep scheduler in C:** Rust overhead or lifecycle evidence is not
  acceptable, or C's execution-plane behavior is the better fit.
- **B. Move selected orchestration only:** retain C scheduler and move bounded
  admission, deadlines, temporary-workspace policy, observability, and HTTP
  lifecycle to Rust.
- **C. Incremental scheduler migration:** only after separate design review,
  repeatable parity evidence, failure/teardown semantics, and rollback design.

No result from this contract authorizes a full Rust scheduler rewrite by itself.
