# IMGENGINE API Contract: Rust Target

**Status:** P5 is implemented and verified in the Ubuntu 24.04 Docker gate.
The broader `v1` target remains planned. The current `/api/generate`
asynchronous contract remains supported until its announced retirement.

## CURRENT P5 IMPLEMENTATION

The only implemented P5 endpoint is `POST /api/v1/render`:

- `multipart/form-data` with exactly one `file` field;
- JPEG or PNG input and a completed JPEG response;
- synchronous, ephemeral processing with no job, polling, durable artifact, or
  legacy API-parity claim;
- `X-API-Key` authentication from `IMGENGINE_API_KEYS`;
- bounded HTTP/admission input handling, including `413` for oversized input
  and `429 overloaded` for bounded-admission rejection; and
- safe, redacted HTTP error mapping and verified real JPEG/PNG processing.

The provisional 1 MiB input limit, queue capacity 1, and 10-second
post-dequeue execution deadline are integration values, not production capacity
commitments. Queue wait, upload, and response delivery are outside that
deadline; native work is never cancelled by expiry. Output bytes are currently
held in memory without a configured output-size ceiling, and workspace byte
accounting is not implemented. See `API_CONTRACT_PARITY_GATE.md` for the
authoritative P5/P6 evidence and limit inventory.

## FUTURE / PLANNED API

The following is a broader target, not implemented P5 behavior.

| Endpoint | Behavior |
| --- | --- |
| `POST /api/v1/render` | Future capability-backed expansion may add layout fields, PDF, and producer-to-client streaming only after separate design and verification. |
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

