# P7.3 C-Core Inventory

## Baseline and method

- Baseline commit: `90cedafb20fc628ae3478e41ad66ecf5f5f9c5f6` (`90cedaf`).
- Audit tree: the uncommitted, verified P7.2.8 private-layout changes are
  intentionally retained and are not part of ABI v1.
- Method: source, CMake target, public-install, test, verifier, and direct
  caller searches. A component with incomplete reachability proof is marked
  **RESEARCH**, never dead or removable.

## Architecture / reachability map

```text
ABI v1 / Rust safe wrapper / CLI
  -> img_api_init / img_api_process_raw / legacy job path
  -> global MPMC submit queue or scheduler queues
  -> worker loop -> decode / pipeline / layout-render task
  -> slab/arena-backed RGB buffers -> TurboJPEG encoder -> native output release
```

The published ABI is exactly seven documented symbols in
`include/imgengine/api/v1/imgengine.h`; CMake installs only `include/imgengine`,
sets `SOVERSION 1`, and ABI consumer/export/compatibility tests are present.
P7.2.8 uses private headers only and does not alter the installed tree.

## Component inventory

| Area | Principal source groups | Current reachability / role | Classification |
| --- | --- | --- | --- |
| ABI v1 | `src/api/imgengine_v1.c`, `include/imgengine/api/v1` | Rust/consumer boundary; serialized operation lock and single active engine | KEEP |
| legacy/internal API | `src/api` (56 C files) | CLI and internal image processing; contains historical mmap-owning route | ADAPT |
| runtime | `src/runtime` (45) | queues, workers, submit/wait, scheduler, task lifecycle | KEEP / HARDEN |
| memory | `src/memory` (14) | global/thread slab and arena allocations | KEEP / HARDEN |
| codecs/IO | `src/io` (14), TurboJPEG/stb bridges | decoder vtable and JPEG output | KEEP / HARDEN |
| pipeline/layout | `src/pipeline` (47) | canvas, resize, crop, historical grid/FIT, private P7.2.8 renderer | KEEP / ADAPT / QUARANTINE |
| SIMD | `src/arch/x86_64` (12) and dispatch | AVX2/scalar kernels and capability selection | HARDEN |
| plugins/hot | `src/plugins`, `src/hot` | legacy transform/pipeline dispatch | RESEARCH |
| security | `src/security` (5) | input validation and seccomp sandbox | KEEP / HARDEN |
| startup/config | `src/startup`, `src/cmd`, `src/cold` | engine/CLI configuration and startup | RESEARCH |
| observability | `src/observability` (16) | native logging/binlog | HARDEN |
| build/tests | CMake, `tests`, `src/tests`, fuzz targets | normal/ASAN/sandbox/fuzz/ABI evidence | KEEP |

## Ownership map (high-value paths)

| Object | Allocated / owned | transfer | release / failure cleanup |
| --- | --- | --- | --- |
| ABI engine | `calloc` in `imgengine_engine_create` | ABI handle | destroy mutex, `img_api_shutdown`, `free` |
| ABI JPEG output | encoder allocation | caller only on success | `imgengine_output_release` -> `img_encoded_free` |
| raw decoded buffer | decoder + slab `owner_pool` | worker/private state | `img_runtime_release_raw_buffer` |
| P7.2.8 placements/state | private `calloc` | submitter owns through task wait | private destroy after terminal task |
| P7.2.8 canvas/scaled buffers | worker-local slab | no external transfer | canvas/raw-buffer release on every path |
| arena temporary state | `img_arena_create` / thread cache | local render pipeline | reset or destroy in finish/context cleanup |
| MPMC task pointer | caller-owned task/state | queue to worker | submitter waits before stack task/state destruction |

## Test / evidence matrix

| Component | Unit/regression | ASAN/UBSAN | Fuzz | integration/reference | Evidence gap |
| --- | --- | --- | --- | --- | --- |
| ABI v1 | consumer, compatibility, exports | verifier | indirect | FFI smoke/real images | ABI surface expansion deferred |
| decode/encode | malformed/real-image regressions | verifier | decoder fuzz | Rust/CLI real images | corpus depth / resource policy |
| resize SIMD | kernel equivalence | verifier | no | scalar equivalence | grayscale SIMD independent oracle |
| P7.2.8 renderer | deterministic/private test | focused + verifier | indirect decoder fuzz | pre-JPEG oracle | public exposure deferred |
| scheduler | lifecycle tests / P7.2.8 | verifier | no | supervisor evidence | stress/starvation proof |
| arena/slab | safety boundaries | verifier | no | indirect | allocator concurrency/limit matrix |
| sandbox | sandbox lifecycle | sandbox build | no | verifier | policy drift review |

## Historical candidates requiring a decision record

`img_api_decode_input` plus `img_api_release_input` assumes mmap input;
float `img_layout_prepare_fit`; `img_layout_grid_place_cells`; commented-out
worker/scheduler alternatives; `api_file_mmap`; and legacy benchmark/CLI
paths are recorded in the quarantine register. Their presence alone is not
proof of deadness.
