# IMGENGINE Performance Strategy

**Status:** Planned target strategy; native benchmark evidence remains authoritative.

## Performance Model

End-to-end latency is upload validation + queue wait + C render + output stream. The service protects latency and availability with a bounded semaphore, byte/pixel/output limits, and overload rejection (`429` or `503` with `Retry-After`) rather than unbounded queues.

## Native Work

Keep C for decode, resize, composition, encoding, and architecture-specific kernels. Preserve scalar output as the correctness oracle. Prioritize the audited correctness and safety issues before broad optimization: stable ABI/export policy, scheduler overflow handling, AVX OS-state checks, architecture-conditional sources, arena/slab overflow and synchronization, sandbox ordering, and sanitizer/fuzz instrumentation coverage.

## Measurement Gates

Every performance claim records hardware, OS/kernel, compiler, build flags, corpus version, command, sample size, median, p95, p99, throughput, RSS, and output checksum. Compare against a checked-in baseline with a documented threshold; do not silently overwrite a regression baseline.

The Rust migration adds request-level metrics: upload bytes, decoded pixels, render duration, queue wait, end-to-end latency, output bytes, timeout count, rejection count, child exit class, and current permits. Benchmarks include normal photos, large valid images, malformed input, and concurrent load within the VPS resource envelope.

