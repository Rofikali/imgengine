# P7.5 Native ABI Diagnostic Observability Policy

## Scope

P75-002 applies only to the ABI-v1 encoded-image request path:

```text
imgengine_process_encoded_image_to_jpeg
  -> img_api_process_raw
  -> decoder / processing / encoder
```

It does not change ABI-v1 status values, scheduler behavior, Rust request
lifecycle behavior, HTTP behavior, or a legacy job/CLI interface.

## Finding and decision

Before this change, `img_api_process_raw()` wrote a fixed label plus an
internal result code/name to process-global `stderr` when preparation or
execution failed. The inspected calls did not interpolate image bytes, paths,
filenames, headers, credentials, pointers, or other request-controlled values.
Accordingly, no demonstrated sensitive-data disclosure was found.

Nevertheless, a per-request native `stderr` side channel has no request
correlation, central redaction, retention, or rate policy. It duplicates the
stable result category returned through ABI v1 and could amplify logs under
repeated invalid input. The policy decision is therefore:

> The ABI-v1 raw processing path returns stable status only. It emits no
> per-request stdout or stderr diagnostic.

The public caller still receives its stable category (`INVALID_IMAGE`,
`UNSUPPORTED_FORMAT`, `RESOURCE_LIMIT`, `OUT_OF_MEMORY`, `UNAVAILABLE`, or
`INTERNAL`) through the existing ABI mapping. Rust and HTTP continue to map
those categories at their own boundary; this change does not claim that a new
structured Rust logging facility was introduced.

## Boundary review

| Boundary | Result | Policy |
| --- | --- | --- |
| ABI-v1 raw request processing | Previously fixed-code `stderr`; now silent | Return stable status only |
| Rust FFI / lifecycle / Axum | Receives category, not native diagnostic text | Existing safe classification remains the outward contract |
| Decoder path used by ABI raw processing | Returns result codes | No request diagnostics emitted by the inspected path |
| Legacy native job helpers | Separate legacy file-job interface | Out of scope; historical stderr remains quarantined, not reachable through the inspected ABI request path |
| Sandbox startup | System-call failure diagnostics | Infrastructure/startup domain, not request content |
| Plugin loader | Descriptor-name output | Separate plugin-loading domain; not an ABI image request diagnostic |

The ABI-v1 result categories preserve operational classification without
exposing native implementation details to the client. Request-level structured
observability and any application-level rate policy remain future work; their
absence must not be interpreted as permission to restore native request
diagnostics.

## Regression evidence

`tests/abi/test_public_consumer.c` redirects only its own process stderr to an
unlinked temporary stream, submits a deterministic malformed byte buffer
thirty-two times through the installed public ABI, then restores stderr before
reporting any test failure. It requires every call to return
`IMGENGINE_STATUS_INVALID_IMAGE`, with an empty output, and requires that the
former ABI diagnostic marker (`[API]`) is absent from the capture. The test
does not assert that an entire multithreaded process is globally silent:
unrelated startup/runtime diagnostics, such as a container's NUMA `mbind`
warning, are outside this raw-request boundary. The probe string contains
path- and credential-shaped text solely to make accidental diagnostic
interpolation detectable; it is never logged or persisted by the test.

This is a negative safety test for the exact former native diagnostic path. It
does not prove that unrelated legacy interfaces or external process logging are
silent.

## Verification status

| Evidence | Command | Result |
| --- | --- | --- |
| Focused public-ABI consumer | `ctest --test-dir /tmp/p75-observability-normal --output-on-failure -R '^abi_public_consumer$'` | PASS |
| Focused ASAN/UBSAN/leak public-ABI consumer | `ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 ctest --test-dir /tmp/p75-observability-asan --output-on-failure -R '^abi_public_consumer$'` | PASS |
| Authoritative Ubuntu 24.04 verifier | `docker build --progress=plain -f imgengine/Dockerfile.p75-authoritative.tmp -t imgengine-p75-authoritative-current imgengine` | PASS (BuildKit `5dhkrehl8awi3qrgkqf5rshiu`, 5m34s) |

Production readiness is not claimed.
