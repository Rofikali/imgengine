# `libimgengine` Public ABI

**Status:** Implemented and Linux-verified on 2026-09-11.

## Scope

IMGENGINE is a document-processing platform. ABI v1 establishes the common
runtime contract and implements one capability only:

```text
image.jpeg_encode: encoded JPEG or PNG input -> encoded JPEG output
```

Photo-sheet layout, RGB24 rendering, PDF/document processing, OCR, batch
processing, and third-party plugins are not ABI v1 capabilities.

## Public Boundary

The installed header root is:

```text
include/imgengine/api/v1/imgengine.h
```

It is self-contained and uses only standard C headers. No `core`,
`pipeline`, `memory`, `runtime`, or plugin header is part of this contract.

The ABI v1 export set is deliberately small:

```text
imgengine_abi_version
imgengine_engine_create
imgengine_engine_destroy
imgengine_capability_supported
imgengine_process_encoded_image_to_jpeg
imgengine_output_release
imgengine_status_message
```

All other current exports are implementation details, not v1 ABI promises.

## Lifecycle and Ownership

- `imgengine_engine_create` creates one opaque engine handle from a
  versioned options structure.
- v1 supports one active engine per process because the current C core owns
  process-global workers and execution state.
- `imgengine_engine_destroy` releases that engine. Callers must not use a
  handle after destruction or race destruction with any other operation.
- `imgengine_process_encoded_image_to_jpeg` borrows its input only for the
  duration of the call.
- On success, the caller owns the output and must release it exactly through
  `imgengine_output_release`. The release operation resets the output and is
  idempotent for a zeroed/released value.
- On any error, the output is reset to `{ NULL, 0 }`.

## Status Model

The public status enum distinguishes: success, invalid argument,
invalid encoded image, resource limit, out of memory, unsupported capability,
unavailable runtime, and internal error. `imgengine_status_message` returns a
static diagnostic-safe name for logging; it does not expose decoder internals
or filesystem paths.

## Capabilities and Versioning

`imgengine_abi_version()` returns ABI major version `1`. `SOVERSION 1` remains
the Linux SONAME for this major ABI.

`imgengine_capability_supported("image.jpeg_encode", &supported)` is the v1
capability-discovery mechanism. Unknown identifiers return success with
`supported == 0`; adding a new identifier is ABI-compatible. Changing a
published operation's meaning, ownership, status semantics, or structure
layout is not compatible and requires a new ABI version.

Public option structures include `struct_size` and reserved fields. Callers
must zero-initialize them. The library accepts a structure at least as large as
the v1 required prefix and ignores recognized reserved fields only when zero.

## Threading and Cancellation

The current underlying engine is process-global, so v1 does not promise an
arbitrary number of independent engines. The v1 wrapper serializes work on
one handle until the core's concurrency contract is independently proven.
There is no mid-operation cancellation in v1; callers must enforce a
request-level deadline outside this ABI. A future cancellation API will be a
new capability/compatible addition only if its resource and lifetime semantics
are fully specified.

## Export and Compatibility Policy

Linux builds use hidden-by-default visibility and a linker version script
that exports only the v1 list above. The symbol checker will fail on both
missing and unexpected dynamic exports and compare the list with an ABI
baseline. Installed-header, external C consumer, ownership/error, sanitizer,
and Rust FFI tests are part of the verified ABI v1 gate.

## Verification

The Docker Ubuntu 24.04 verifier validates ABI v1 with an external C consumer
compiled only against the staged installed header and library, an exact dynamic
export check, layout compatibility assertions, ownership/error/lifecycle tests,
process-scope and serialized-thread tests, ASAN/UBSAN, and a minimal Rust FFI
smoke executable. The Rust wrapper keeps every FFI call in its local boundary
and deliberately does not implement `Send` or `Sync` for the opaque engine.

`IMG_ERR_SECURITY` from the existing engine is reported as
`IMGENGINE_STATUS_RESOURCE_LIMIT`; callers receive no parser detail and cannot
distinguish individual security policy violations. Invalid decodes reported as
`IMG_ERR_FORMAT` map to `IMGENGINE_STATUS_INVALID_IMAGE`.

The ABI wrapper forwards the existing in-memory processing path without input
or output duplication; it adds process-scope and operation mutex checks only.
There is no equivalent historical `image.jpeg_encode` baseline in the
repository, so this change makes no numeric no-regression claim. A fixture-based
throughput baseline must be recorded before publishing a performance claim.
