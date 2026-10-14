# P7.3 Principal Review of the C-Core Audit

## Review result

The P7.3 inventory is directionally correct, but this review refines H-01 and
H-02 from actual source. No source behavior was changed.

## Active-path classification

| Class | Evidence-backed members |
| --- | --- |
| A. Active verified path | ABI v1 `imgengine_process_encoded_image_to_jpeg` -> `img_api_process_raw` -> `img_api_process_raw_decode` -> `g_io_vtable.decode` -> encoder/release; worker queues; slab-backed decoded buffers; ABI tests and Rust FFI smoke. |
| B. Active, not product-exposed | P7.2.8 private layout task/renderer/observer; it is CMake-tested and verified but absent from installed headers, ABI symbols, Rust, HTTP, and Nuxt. |
| C. Historical / legacy | file-path `img_api_begin_job`, CLI layout/PDF flow, mmap file wrappers, latency/decoder benchmarks, historical grid/FIT. |
| D. Quarantined | mmap-owning decode when caller bytes are borrowed; float FIT and partial grid for new plans; blit without prior bounds validation. |
| E. Remove candidate | commented worker alternatives only; not approved for deletion until build/reference proof. |
| F. Unknown | plugin/fused transform runtime reachability from present product flows; no removal or safety conclusion follows. |

## Finding-by-finding review

### H-01 — decoder ownership: contained for current paths, open as an invariant

- **Source:** `api_job_decode_input.c` calls the historical input-release path;
  `api_job_release_input.c` calls `munmap`. In contrast,
  `api_process_raw_decode.c` invokes `g_io_vtable.decode` with `const` input;
  P7.2.8 has a separately named non-owning adapter around the same vtable.
- **Reachability:** ABI v1 calls `img_api_process_raw`, not `img_api_begin_job`.
  P7.2.8 also avoids the mmap-owning route. Legacy file/CLI paths still use
  the mmap-oriented flow.
- **Impact:** no proven defect in the verified ABI/P7.2.8 paths. A future
  caller can nevertheless select the wrong legacy API and violate ownership.
- **Disposition:** downgrade from implementation HARDEN to **documented
  boundary invariant / ADAPT**. P7.4 should add focused regression/documentary
  evidence that borrowed input is never released, not redesign an ABI.

### H-02 / SIMD-001 — confirmed semantic divergence, not product-exposed

- **Source:** scalar grayscale (`arch/x86_64/scalar/color_scalar.c`) computes
  per RGB pixel `(R*77 + G*150 + B*29) >> 8` and writes all channels. AVX2
  grayscale (`arch/x86_64/avx2/color_avx2.c`) processes arbitrary 32-byte
  chunks with coefficient 77 for every byte, not RGB deinterleaving. It is not
  scalar-equivalent. Fused scalar and AVX2 paths similarly apply `77` to each
  channel when `has_grayscale` is set.
- **Dispatch:** `hardware_registry.c` selects AVX2 grayscale on AVX2/AVX512;
  plugin registration and fused dispatch provide potential callers.
- **Reachability:** current ABI v1 raw JPEG path only decode/encodes and does
  not construct the plugin/fused grayscale pipeline. Thus it is not a proven
  active SaaS output defect, but it is a real correctness defect in an active
  native capability family.
- **Disposition:** **MUST-FIX P7.4.** Smallest scope: independent scalar RGB
  oracle; deterministic RGB/tail/stride fixtures; direct scalar-versus-AVX2
  comparisons for standalone and fused grayscale; sanitizer/regression gate.
  Do not redesign dispatch or add product exposure.

### M-01 — scheduler evidence gap remains

`img_runtime_wait_task` spins then yields; `worker_loop` polls local, scheduler
global, API global, then steal queues. A submitted stack task is safe only
because callers wait to terminal `IMG_TASK_DONE`. Existing lifecycle evidence
is positive, but saturation, concurrent submit/wait, worker teardown, repeated
shutdown, ordering/starvation, and failure propagation require a bounded native
stress suite before scheduler modification is contemplated. **SHOULD-FIX.**

### M-02 — arena/slab evidence gap remains

Slab allocation/free uses a mutex and allocation bitmap; arena aligned
allocation checks alignment and remaining size. Global pool is configured at
512 MiB with 32 MiB blocks; a thread context path uses 128 MiB/32 MiB. These
are native implementation capacities, not approved product limits. Ownership
is clear on inspected raw/P7.2.8 paths, but allocation failure, reuse, and
concurrent teardown deserve a focused matrix. **SHOULD-FIX.**

### M-03 — observability

Current raw API logs result codes/names; CLI and benchmarks print supplied file
paths, which is appropriate only for local tools. `cold/debug.c` prints pointer
addresses. No API-key/image-byte logging was found in the inspected C path.
Native diagnostics must not be forwarded to HTTP clients. **DEFER / HARDEN**:
separate CLI/debug diagnostics from service-safe observability before service
exposure; no logging framework is justified now.

## Build, ABI, historical code, and dependencies

CMake installs only public `include/imgengine`; private P7.2.8 headers are not
installed. ABI v1 remains seven exports, with consumer/compat/export tests and
SOVERSION 1. TurboJPEG is the material native codec dependency; stb is bundled
decoder support. Fuzz targets cover input validation and decoder only. Existing
commented historical alternatives, `queue_mpmc.c` stub, benchmarks and mmap
wrappers are retained pending direct build/caller proof. No dependency change
is recommended.

## Management assessment

Engineering: P7.2.8 is a contained, evidence-backed private capability; SIMD
correctness and ownership boundaries are the cheapest high-value hardening.
Security: current decoder/sandbox/fuzz evidence is useful but bounded; product
resource policy is distinct. Operations: Docker Ubuntu verification is
reproducible; native diagnostics are not yet service-safe by contract. Business:
avoid ABI/UI expansion until grayscale correctness and ownership regression
evidence reduce future support and incident cost.

## P7.4 proposal — narrow hardening only

| Item | Files / change | Evidence / rollback |
| --- | --- | --- |
| P7.4-A SIMD-001 | grayscale scalar/AVX2/fused tests; corrective implementation only after oracle proves mismatch | deterministic oracle, tails/stride, AVX2-gated comparison, ASAN/UBSAN, full verifier; revert isolated kernel/test commit if needed |
| P7.4-B borrowed decode invariant | P7.2.8/private or API-focused regression/documentation only | prove decoder does not release borrowed input; ASAN/leak and full verifier; no ABI change |
| P7.4-C scheduler/allocator evidence | new native stress tests only, no topology/allocator rewrite | bounded queue, submit/wait, failure, shutdown and reuse tests; sanitizer; no change if evidence exposes no defect |

MUST-FIX: P7.4-A. SHOULD-FIX: P7.4-B/C. DEFER: service-safe logging policy,
product resource ceilings, public layout ABI/Rust/HTTP/UI. QUARANTINE and
remove-candidate decisions remain in the existing register.

## Gate

**READY FOR P7.4 IMPLEMENTATION**, limited to the three items above. P7.4 must
not become cleanup, ABI, Rust, HTTP, Nuxt, scheduler-redesign, or infrastructure
work. Production readiness remains not claimed.
