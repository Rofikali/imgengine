# IMGENGINE API Contract: Rust Target

**Status:** Planned broader `v1` target. The current `/api/generate` asynchronous contract remains supported until its announced retirement.

## P5 First HTTP Slice

The implemented P5 adapter scope is narrower than this future target:
`POST /api/v1/render` accepts exactly one multipart `file` field with JPEG or
PNG input and returns a completed JPEG response. It has no layout fields,
presets, PDF output, job identifier, polling, durable artifact, or legacy API
parity claim. See `API_CONTRACT_PARITY_GATE.md` for the authoritative P5
authentication, multipart, limit, and verification contract.

## Public Endpoints

| Endpoint | Behavior |
| --- | --- |
| `POST /api/v1/render` | P5: accepts exactly one JPEG/PNG `file` and returns completed JPEG bytes. Layout fields, PDF, and streaming are future-target capabilities. |
| `GET /healthz` | Liveness only; no dependency checks. |
| `GET /readyz` | Readiness: config loaded, writable temporary root, engine executable/version available, concurrency capacity configured. |
| `GET /metrics` | Prometheus metrics on a private network or authenticated scrape path. |

`POST /api/v1/render` accepts an `Idempotency-Key` for safe retry only during the active request window. It is not a durable job identifier. The response contains `X-Request-Id`, `X-Trace-Id`, content type, and a safe generated download filename. Errors use RFC 9457-style JSON with stable `type`, `title`, `status`, `code`, and `request_id` fields.

## Compatibility Rules

1. Freeze the current request, response, error, status, and event payloads before writing Rust handlers.
2. Keep legacy routes behind an adapter with contract tests; do not expose Rust internals through legacy responses.
3. Announce deprecation only after production parity evidence and a documented client migration window.
4. Never leak temporary paths, command lines, storage keys, engine diagnostics containing paths, or authentication data.

## Validation Order

Authenticate and apply rate limits; enforce byte limits while streaming; validate content signature; decode bounded metadata; validate layout/preset; reserve concurrency; render in isolation; stream only a completed bounded output; remove all temporary state in `finally`/drop handling.

