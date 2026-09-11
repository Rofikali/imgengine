# `libimgengine` ABI Audit

**Status:** Historical pre-stabilization audit. ABI v1 is implemented and
Linux-verified; `docs/ABI.md` is the authoritative public contract.

## Evidence

The audit used the Priority 1 Ubuntu 24.04 verifier build on 2026-09-09:

- `nm -D --defined-only libimgengine.so` reported **338** dynamic symbols.
- `docs/abi/exported_symbols.json` lists **39** symbols, including scheduler,
  allocator, SIMD, IO, observability, plugin, and global-state internals.
- A staged install containing the current `include/api/v1` headers failed an
  external C consumer compile. With only `/usr/include`, `img_api.h` cannot
  find `api/v1/img_error.h`; with both include roots, `img_error.h` cannot find
  uninstalled `core/result.h`.

## Findings

| Area | Finding | Impact |
| --- | --- | --- |
| Header isolation | `api/v1/img_error.h`, `img_job.h`, `img_buffer_utils.h`, and `img_plugin_api.h` re-export private `core`, `pipeline`, and `memory` headers. | Installed headers are not independently consumable. |
| Include layout | Installed header location and quoted include paths require incompatible include roots. | A normal external include of `<imgengine/api/v1/img_api.h>` fails. |
| Export surface | The shared library exports 338 symbols, including `g_engine`, scheduler/slab/arena internals, SIMD kernels, IO hooks, and `stbi_*` third-party symbols. | Private implementation is an accidental ABI. |
| Symbol checker | `check_exported_symbols.py` detects missing manifest entries only; its `extra` set is the intersection, so unexpected exports never fail. | ABI expansion and private-symbol leakage are undetected. |
| Manifest scope | The existing 39-entry manifest calls private scheduler, allocator, plugin, logging, and SIMD entry points public. | It cannot be the stable Rust-facing v1 contract. |
| Lifecycle | `img_api_init()` returns process-global `g_engine`; workers and task state also use fixed global storage. The header says call once per process but has no explicit create/configuration/error contract. | Multiple-engine, reinitialization, and Rust thread-safety semantics are not established. |
| Job layout | Public `img_job_t` is re-exported from `pipeline/job.h`; it has an ABI version field but exposes a mutable struct layout and `float` fields. | Layout changes can silently break foreign consumers. |
| Buffers | Public declarations forward-declare `img_buffer_t`, while `img_buffer_utils.h` exposes slab-backed internals. `img_api_process_fast()` and release APIs require that private type. | Buffer ownership and construction are unsuitable for a stable external ABI. |
| Output ownership | Raw APIs return `uint8_t **`/`size_t *`; comments alternately require `img_encoded_free()` and `free()`. Error-path output initialization and lifetime are not a single documented contract. | A Rust binding cannot safely infer allocator and release rules. |
| Error model | `img_result_t` is re-exported from `core/result.h`; invalid arguments, image validation, resource limits, and internal failures are often all returned as `IMG_ERR_SECURITY` or `IMG_ERR_INTERNAL`. | External error handling is ambiguous and unstable. |
| File-system coupling | Main public job functions take input/output paths. In-memory functions exist but are low-level or expose private pipeline/buffer concepts. | The current surface does not match the planned ephemeral Rust request path. |
| Thread safety | Header claims concurrent jobs, but the process-global lifecycle, global queues, and shutdown synchronization do not establish a complete external concurrency contract. | Rust `Send`/`Sync` claims would be unsafe today. |
| Versioning/install | `VERSION 1.0.0` and `SOVERSION 1` exist, but there is no public ABI policy, linker version script, CMake package export, or compatibility baseline. | SONAME alone does not protect compatibility. |

## Non-Goals for v1

The stable Rust-facing ABI must not expose scheduler, arena, slab, SIMD,
plugin, decoder, IO-vtable, observability, or internal buffer structures. The
existing plugin ABI is a separate compatibility concern and must not be folded
into the application-facing ABI without an explicit later decision.

## Recommended Minimal Direction

The implementation phase should add a clean installed header tree rooted at
`include/imgengine/api/v1/` with no dependencies outside that tree. It should
provide only:

1. opaque engine and output handles or an output value with one designated
   release function;
2. a versioned, fixed-layout request/options structure with `struct_size` and
   reserved fields, or builder functions that avoid exposing mutable layout;
3. an in-memory request-to-output operation for the future Rust boundary;
4. a small explicit status enum and status-to-static-message function;
5. lifecycle, ownership, and single-engine/threading rules documented in the
   public header and `docs/ABI.md`.

Linux export control should use hidden-by-default visibility plus a linker
version script containing only the new intentional v1 symbols. The existing
entry points can remain implementation/private symbols during the transition;
they must not be silently promised as the future stable ABI.

## Required Proof Before Declaring v1 Stable

- Clean installed-header C consumer compile and link test.
- Symbol manifest test that fails on both missing and unexpected exports.
- C ABI lifecycle, success, ownership, invalid-argument, and error tests.
- ASAN/UBSAN execution of ABI tests.
- ABI baseline comparison for compatible releases.
- A minimal Rust wrapper proof, with `unsafe` contained inside the FFI crate
  and no unproven `Send` or `Sync` implementation.
- Linux Docker verification and a representative benchmark comparison.

## Resolution

The audit findings above are resolved for ABI v1 by the sealed
`include/imgengine/api/v1/imgengine.h` surface, opaque engine ownership,
`imgengine_output_release`, and `cmake/libimgengine.map`. The canonical export
manifest is now `imgengine/abi/exported_symbols.json`; the checker rejects both
missing and unexpected dynamic symbols. Ubuntu 24.04 verification on
2026-09-11 passed normal and ASAN/UBSAN ABI consumers, installed-header
consumers, exact exports, security regressions, sandbox lifecycle, bounded
fuzzing, and the Rust FFI smoke test.
