# IMGENGINE API Contract: Rust Target

**Status:** P5 is implemented and verified in the Ubuntu 24.04 Docker gate.
P8 adds a narrow, capability-backed FIT-only layout endpoint; its full
Ubuntu-authoritative release evidence is required before a production claim.
The broader `v1` target remains planned. The current `/api/generate`
asynchronous contract remains supported until its announced retirement.

## CURRENT P5/P6 IMPLEMENTATION

The only implemented P5 endpoint is `POST /api/v1/render`:

- `multipart/form-data` with exactly one `file` field;
- JPEG or PNG input and a completed JPEG response;
- synchronous, ephemeral processing with no job, polling, durable artifact, or
  legacy API-parity claim;
- `X-API-Key` authentication from `IMGENGINE_API_KEYS`;
- bounded HTTP/admission input handling, including `413` for oversized input
  and `429 overloaded` for bounded-admission rejection; and
- safe, redacted HTTP error mapping and verified real JPEG/PNG processing.

The provisional 1 MiB input limit, queue capacity 1, 10-second post-dequeue
execution deadline, and 16 MiB FIT-layout JPEG destination budget are
integration values, not production capacity commitments. Queue wait, upload,
and response delivery are outside that deadline; native work is never
cancelled by expiry. The layout output budget is server-owned (optionally
configured through `IMGENGINE_LAYOUT_MAX_OUTPUT_BYTES`) and is enforced before
the native JPEG destination allocation using a conservative TurboJPEG bound.
It does not bound decoded/canvas/intermediate memory, the Rust response copy,
or aggregate workspace bytes. See `API_CONTRACT_PARITY_GATE.md` for the
authoritative P5/P6 evidence and limit inventory.

### P7.1 frontend API boundary

**Implemented + frontend-unit-tested:** the Nuxt browser boundary submits only
the `file` multipart field to a same-origin `POST /api/render` proxy and
receives the completed JPEG response. The Nuxt server proxy forwards that
request to `POST /api/v1/render` and supplies `X-API-Key` from its private
`IMGENGINE_RENDER_API_KEY` runtime configuration. The browser receives neither
that credential nor `IMGENGINE_API_KEYS`.

The typed client classifies only the documented problem status/code pairs:
`400 invalid_request`, `401 authentication_failed`, `413 resource_limit`,
`415 unsupported_media_type`, `422 invalid_image`, `429 overloaded`,
`500 internal`, `503 unavailable`, and `504 deadline_exceeded`; a transport
failure is distinct. It does not model jobs, polling, output URLs, layouts,
presets, PDFs, idempotency, or client-side API keys.

This is a controlled same-origin frontend boundary, not a production deployment
or frontend migration-completion claim. The legacy API remains unchanged and
supported until a separate migration and retirement decision.

## CURRENT P8 FIT-ONLY LAYOUT CAPABILITY

`POST /api/v1/layout-render` is an additive synchronous endpoint. It uses the
same authentication, request lifecycle, bounded Rust admission, retained C
scheduler, workspace cleanup, and JPEG response model as `/api/v1/render`.
It accepts `multipart/form-data` with exactly these fields:

- `file`: JPEG or PNG;
- `width`, `height`, `gap`, and `margin`, in `unit` `px` or `mm`;
- `dpi` only when `unit=mm` (decimal millimetres are converted using checked
  half-up rounding);
- `copies`, optional `columns` (one when omitted); and
- `output_format=jpeg`.

The implemented operation creates a custom white-background canvas, places the
requested number of non-overlapping cells in deterministic row-major order,
and renders every cell with **FIT** semantics only. The completed response is
`image/jpeg`, has an attachment filename, and includes the native-confirmed
canvas width/height and expected/planned/executed placement counts in
`X-Imgengine-*` headers. The same-origin Nuxt route is
`POST /api/layout-render`; it keeps `X-API-Key` in private server runtime
configuration and forwards only the declared fields.

P8 uses the current provisional 1 MiB input body limit, queue capacity 1,
10-second execution deadline, and 16 MiB server-owned layout JPEG destination
budget. Its defensive integration limits are at most 64 copies, 64 columns, a
4096-pixel canvas edge, and 4,000,000 canvas pixels. A JPEG whose final byte
count would fit can still be rejected when TurboJPEG's conservative destination
bound exceeds the configured budget. These are not production-capacity claims:
aggregate output memory and workspace-byte limits remain unimplemented.

The following are expressly **not** implemented by this endpoint: Fill/crop,
presets, templates, paper sizes, PDF or PNG output, transparency/background
selection, persistent jobs/output URLs, rate limiting, idempotency, and native
mid-operation cancellation. Invalid fields map safely to `400`; unsupported
image input remains `415`; malformed image data is `422`; layout/body resource
limits are `413`; queue overload is `429`; and expiry is `504`.

## FUTURE / PLANNED API

The following is a broader target, not implemented P5 behavior.

| Endpoint | Behavior |
| --- | --- |
| `POST /api/v1/render` | Future capability-backed expansion may add PDF and producer-to-client streaming only after separate design and verification. |
| `GET /healthz` | Planned liveness endpoint. |
| `GET /readyz` | Planned readiness endpoint. |
| `GET /metrics` | Planned private/authenticated metrics endpoint. |

Active-window `Idempotency-Key`, `X-Trace-Id`, generated download filenames,
rate limiting, broader output capabilities, and production streaming semantics
are planned or deferred. They are not implemented by P5.

## Compatibility Rules

1. Freeze the current request, response, error, status, and event payloads before writing Rust handlers.
2. Keep legacy routes behind an adapter with contract tests; do not expose Rust internals through legacy responses.
3. Announce deprecation only after production parity evidence and a documented client migration window.
4. Never leak temporary paths, command lines, storage keys, engine diagnostics containing paths, or authentication data.

## Planned Validation Order

Future API work may authenticate, apply a separately designed rate policy,
enforce byte limits while streaming, validate capability-backed layout/preset
fields, and add bounded output delivery. P5 performs only its documented
file-only JPEG/PNG-to-JPEG path.

