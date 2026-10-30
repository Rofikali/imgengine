# P9.3 Bounded Native Output API — Gate A Design

## Status and selected design

**Gate A is closed. Gate B implementation and Ubuntu 24.04 verification are
complete.** The approved design is the conservative TurboJPEG
destination-capacity preflight below. It intentionally does not promise
exact-final-byte acceptance.

## Existing behavior

`img_layout_render_v1_execute()` calls the registered encoder, which ultimately
uses `tjCompress2()` with a null destination pointer. TurboJPEG allocates (and
may reallocate) the entire JPEG. `imgengine_layout_render_v2()` transfers that
C-owned pointer to the caller; the Rust wrapper copies it into `Vec<u8>` then
calls the paired release function. Thus a Rust output-length check happens
after both native allocation and Rust duplication.

The installed TurboJPEG header documents `TJFLAG_NOREALLOC`: compression
returns an error if a caller-provided destination is invalid or too small,
rather than allocating or reallocating it. This is the viable enforcement seam.

## Proposed additive ABI

Keep all `IMGENGINE_1.0` and `IMGENGINE_2.0` symbols unchanged. Add an
`IMGENGINE_2.1` version node and this exported symbol in the existing V2 public
header:

```c
IMGENGINE_API imgengine_status_t
imgengine_layout_render_v2_bounded(imgengine_engine_t *engine,
                                   const imgengine_layout_request_v2_t *request,
                                   size_t max_output_bytes,
                                   imgengine_layout_output_v2_t *output);
```

It preserves the existing `imgengine_layout_output_release()` ownership rule.
On every non-OK return `output` is all-zero and must not be released. Existing
`imgengine_layout_render_v2()` remains semantically identical and unbounded.

`max_output_bytes == 0` or a value not representable by the underlying
TurboJPEG API is `IMGENGINE_STATUS_INVALID_ARGUMENT`; callers must make an
explicit, valid bounded/unbounded choice. A valid budget below the conservative
TurboJPEG destination bound is `IMGENGINE_STATUS_RESOURCE_LIMIT`. The bounded
contract is therefore a worst-case destination-allocation limit, not a promise
that a JPEG whose final byte count is below the budget will be accepted.

## Native implementation shape

1. Add a bounded private layout execution path that passes an explicit output
   budget through `layout_render_private.c` into a new encoder entry point.
2. The JPEG encoder calculates `tjBufSize(width, height, TJSAMP_444)` for the
   already validated RGB canvas. It rejects the operation before allocation
   when that conservative maximum exceeds `max_output_bytes`. Otherwise it
   allocates exactly that conservative bound with `malloc()`, initializes
   `jpegSize` to it, and calls
   `tjCompress2(..., TJFLAG_FASTDCT | TJFLAG_NOREALLOC)`.
3. It does not call an allocating/reallocating codec path. `malloc()` is
   deliberately paired with the existing `img_encoded_free()`/`free()` layout
   result release contract. A `tjCompress2()` failure remains `IMG_ERR_IO`;
   it is not guessed to be a resource limit from an unstable diagnostic
   string. On any failure the destination is freed and the output remains
   empty.
4. The private state cleanup remains the sole owner before result transfer;
   after success it transfers one C-owned JPEG pointer to the public output.
   Existing release frees it exactly once.
5. Validate all size conversions and `malloc()` allocation inputs; no multiplication or
   capacity arithmetic is required beyond checked representability.

TurboJPEG can still allocate codec-internal state and the image canvas exists
before encoding. The design bounds the encoded destination allocation; it does
not bound decoded/canvas/intermediate memory.

## Result and HTTP mapping

The preflight output-budget rejection uses the existing internal
`IMG_ERR_SECURITY` resource-limit class. Its public ABI mapping is the stable
`IMGENGINE_STATUS_RESOURCE_LIMIT`; Rust maps it to `Error::ResourceLimit`,
then the existing application/API mapper returns `413 resource_limit`. Encoder
failure remains `IMG_ERR_IO` and maps through the existing safe internal-error
path. No TurboJPEG diagnostic text participates in public error
classification.

The safe Rust `Engine::render_layout_jpeg_bounded()` takes a nonzero output
budget, invokes only the new symbol, and uses the existing RAII native-output
release path. `Supervisor::process_layout()` receives the configured output
limit and remains responsible for workspace cleanup before response mapping.
`ApiConfig` adds a validated, explicitly provisional output-limit field; it is
not inferred from the 4 MP canvas policy. The API does not report partial
bytes, internal codec reasons, paths, or budgets beyond the stable problem
class.

## Candidate configuration policy

The provisional server-owned default is 16 MiB, configured through
`ApiConfig` and propagated to `SupervisorOptions`; it is never derived from a
multipart field. Deployments may set the validated server environment variable
`IMGENGINE_LAYOUT_MAX_OUTPUT_BYTES`; invalid, zero, or greater-than-`INT_MAX`
values fail configuration. It is an integration default, not a production
capacity claim. P9.1 observed a 1,140,027-byte exact-4MP JPEG, which is not a
safe universal maximum. Selecting a deployed value still requires the measured
encoded-output distribution plus memory and delivery headroom.

## Required regression matrix

| Case | Expected result |
| --- | --- |
| zero budget | invalid argument; no native output |
| conservative bound at or below budget | success, complete decodable JPEG, one release |
| one byte below conservative bound | resource limit, null/zero output, no partial result |
| clearly over budget | resource limit before JPEG destination allocation |
| malformed input | existing invalid-image behavior, no output |
| codec/encoder failure | existing safe failure, no output |
| repeated limit failures | subsequent bounded success and clean workspace |
| abandonment/deadline | native work still completes; admission remains held until completion |
| full admission queue | `429`, no retained excess body or encoder invocation |

Native tests must prove allocator pairing and no partial transfer; Rust FFI,
supervisor, API/HTTP, real-image, ASAN/UBSAN/leak, sandbox, fuzz, frontend
integration, and the unchanged authoritative Ubuntu 24.04 verifier must pass.

## Compatibility and remaining risks

This is additive: ABI v1, existing V2, scheduler ownership, admission
linearization, P5/P8 routes, input limits, deadline semantics, and disconnect
semantics remain unchanged. It does not cancel native work and it does not
create an output streaming protocol.

Residual risks remain decoded source/canvas/intermediate allocations,
simultaneous active/queued input, Rust's copy of accepted output, response
buffering, and host/container overhead. A bounded output destination is an
important per-request control, not complete memory accounting.

## Final Gate A condition review

### A. Authoritative output budget — resolved design

**Source evidence.** `tjCompress2()` accepts a caller-owned destination and
capacity in `jpegSize`; installed `/usr/include/turbojpeg.h` documents
`TJFLAG_NOREALLOC` as an error-on-invalid-or-too-small destination guarantee.
`tjBufSize()` supplies the documented conservative destination capacity for a
validated image geometry.

**Decision.** The server owns a nonzero configured budget. The new bounded
entry point takes that budget explicitly, rejects zero or values larger than
`INT_MAX` as invalid argument before allocation, and never treats a client
field as a budget. It checks the codec's documented conservative `tjBufSize()`
result against the budget before allocating that bound. Allocation failure maps
to existing `OUT_OF_MEMORY`, not resource limit. No multiplication is used for
the output capacity; conversion and all request/canvas arithmetic remain
checked.

**Evidence.** The native bounded-encoder test executes zero-budget rejection,
one-byte-below-bound rejection, deterministic destination-`malloc()` failure,
exact-conservative-bound success, and recovery after the injected failure.
The installed ABI consumer checks the public zero/over-budget/malformed/success
contract; Rust/API tests prove the server-owned cap maps to HTTP `413
resource_limit`; ABI export checks verify the additive `IMGENGINE_2.1` symbol.

**Uncertainty.** The deployed default number remains unvalidated; it requires
fixture/VPS measurement. This is noncritical because configuration can require
an explicit test value initially.

### B. Error classification — resolved conservative alternative

**Source evidence.** The installed TurboJPEG header states that
`TJFLAG_NOREALLOC` returns an error for an invalid or too-small destination.
Its public `tjGetErrorCode(tjhandle)` returns only severity (`TJERR_WARNING` or
`TJERR_FATAL`), not a typed destination-capacity cause; `tjGetErrorStr2()` is a
diagnostic string, not a stable structured error contract. The current encoder
returns every `tjCompress2()` failure as `IMG_ERR_IO`.

**Decision.** The approved implementation is conservative preflight:
calculate `tjBufSize()` for the validated canvas and reject when that
documented worst-case destination capacity exceeds the server budget. This is
reliably classifiable as `RESOURCE_LIMIT`; malformed input is still decoded and
classified through its existing path, allocation failure stays
`OUT_OF_MEMORY`, and `tjCompress2()` failure stays `IMG_ERR_IO`. No vendor
error text is parsed. The trade-off is explicit: a JPEG whose final length
would fit can still be rejected when its conservative destination bound does
not.

### C. Ownership and failure atomicity — resolved design

**Source evidence.** `imgengine_layout_render_v2()` zeroes output before
validation, transfers `result.jpeg_data` only after all result accounting
checks, and `imgengine_layout_output_release()` calls the matching internal
release then zeros the public structure. Rust RAII drops the native output
after copying.

**Decision.** The bounded function follows this exact sequence. It has no
engine/request retained output state. On success, returned actual encoded
length is nonzero and at most the requested budget; ownership transfers once to
the caller and uses the existing release function. On every failure, C frees
any partial buffer, clears private state, returns an all-zero public output,
and transfers no ownership.

**Evidence.** The public ABI consumer verifies zeroed output on zero, limit,
and malformed-input failures plus idempotent release after success. The native
bounded-encoder test verifies zeroed output on deterministic allocation failure
and a subsequent successful request. The full Ubuntu verifier executed those
tests under ASAN/UBSAN with leak detection.

### D. Aggregate memory accounting — resolved limitation

**Source evidence.** The output is first native C allocation then a Rust
`Vec<u8>` copy. The layout canvas is RGB and P8 preflights it at 4,000,000
pixels (12,000,000-byte RGB estimate); decoded source and codec internal
allocations are native-owned. Admission has one active worker and one queued
owned input body, each input capped at 1 MiB. An abandoned response drops its
receiver but native work and its admission slot remain until completion.

**Decision.** A bounded native destination caps one allocation to the selected
budget but does not cap the Rust response copy, canvas, decode, codec-internal,
or host allocations. Maximum active output bytes are one native destination
plus potentially one Rust copy; queued work contains input, not output.
Workspace tmpfs is not an output-memory accounting control. These facts must
be documented with any implementation; no total-memory claim is permitted.

**Evidence.** Existing queue/abandonment tests and the P9 local constrained
gate remain applicable; the new output-limit tests prove only this destination
bound. Exact total peak remains unmeasured and actual-VPS evidence remains
required.

### E. ABI and symbol compatibility — resolved design

**Source evidence.** `cmake/libimgengine.map` currently contains
`IMGENGINE_1.0` and additive `IMGENGINE_2.0`; CMake runs
`scripts/check_exported_symbols.py` against the canonical
`abi/exported_symbols.json` manifest. The public consumer already compiles and
runs existing V2 layout symbols. `IMGENGINE_STATUS_RESOURCE_LIMIT` is the
existing public status code 3.

**Decision.** A selected bounded function is an `IMGENGINE_2.1` export derived
from `IMGENGINE_2.0`; no prior node/symbol/header structure changes. Existing
V1/V2 calls retain behavior. Manifest, exported-symbol documentation, installed
consumer, and ABI tests must be updated to assert old symbols remain present
with their old versions and the new symbol alone is additive.

**Evidence.** The authoritative Ubuntu verifier passed the export-manifest
checker, installed/public ABI consumer, ABI compatibility, and the normal and
sanitized native suites. The existing unbounded V2 entry point remains a
separate call to the same implementation with an explicit zero internal cap.
