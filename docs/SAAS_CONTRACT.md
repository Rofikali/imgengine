# IMGENGINE SaaS Contract

**Status:** Living contract  
**Applies to:** Nuxt UI, Nuxt server routes, FastAPI, Redis/Celery, worker, PostgreSQL, and storage.

## 1. Service Boundaries

| Component | Responsibility | Must not do |
| --- | --- | --- |
| Nuxt | Upload/status/download UX; server-side proxy for secrets. | Expose upstream API key or run native processing. |
| FastAPI | Validate requests, persist jobs, authorize, publish queue work. | Render images on request thread. |
| Redis/Celery | Durable asynchronous delivery for initial release. | Be the source of truth for job state. |
| Worker | Execute native CLI, update state, persist output metadata. | Accept public browser traffic. |
| PostgreSQL | Durable job state and audit metadata. | Store unbounded image blobs in the first release. |
| Storage | Private uploads/outputs with retention policy. | Expose or persist raw filesystem paths as public API values. |

## 2. Public API v1

### `POST /api/generate`

**Authentication:** `X-API-Key` on the FastAPI boundary. Nuxt adds it only on the server.

**Input:** `multipart/form-data` with `file`, `width`, `height`, `dpi`, `cols`, `rows`, `gap`, `padding`, `border`, `bleed`, `crop_mark`, `crop_thickness`, and `crop_offset`.

**Current implemented behavior:** accepts JPEG/PNG uploads, validates bounded layout fields against `GenerateJob`, creates a `queued` job, and publishes to Celery. Files over `MAX_UPLOAD_BYTES` return `413`; unsupported content types return `415`; missing/invalid API key returns `401`; unavailable Redis returns `503` within approximately three seconds.

**Target response:**

```json
{
  "job_id": "uuid",
  "trace_id": "uuid",
  "status": "queued",
  "output_url": null,
  "error": null
}
```

The public response never reveals a storage path. Storage is represented internally by server-generated keys such as `uploads/{job_id}.jpg` and `outputs/{job_id}.png`.

### `GET /api/status/{job_id}`

Returns the caller-authorized job status. Unknown jobs return `404`.

Allowed lifecycle:

```text
queued → processing → completed
queued → failed
processing → retrying → processing
processing → failed
```

Terminal states (`completed`, `failed`) never transition again. Future retry requires a new job or an explicitly versioned retry attempt.

### `PATCH /internal/jobs/{job_id}`

Authenticated worker-only endpoint. Status updates are constrained to this lifecycle:

`queued → processing → completed | failed`, with `processing → retrying → processing | failed` for retriable failures. Terminal states cannot transition; repeating the current status is idempotent.

Worker-only endpoint protected by `X-Internal-Token`. It must validate payload schema and transition legality; raw dictionaries are temporary implementation debt.

## 3. API-to-Worker Payload

All new payloads must be versioned:

```json
{
  "version": 1,
  "job_id": "uuid",
  "trace_id": "uuid",
  "input_key": "uploads/uuid.jpg",
  "output_key": "outputs/uuid.png",
  "layout": {
    "width_cm": 4.5,
    "height_cm": 3.5,
    "dpi": 300,
    "cols": 6,
    "rows": 6,
    "gap_px": 15,
    "padding_px": 20,
    "border_px": 2,
    "bleed_px": 0,
    "crop_mark_px": 15,
    "crop_thickness_px": 2,
    "crop_offset_px": 8,
    "scale_mode": "fill"
  }
}
```

The worker rejects an unknown version, nonexistent input, invalid layout, or illegal job state before invoking the CLI.

## 4. Nuxt UX Requirements

The initial UI exposes inputs for source image, width, height, DPI, rows, columns, gap, padding, border, bleed, and crop-mark length/thickness/offset. Scale mode, output format, safe presets, and preset selector remain planned.

The UI polls job status, shows a human-readable error returned by the server, disables download until `completed`, and never displays raw storage paths. Layout controls must map exactly to the API contract; no hidden client-only defaults.

## 5. Security and Operations

- Configure `API_KEYS`, `INTERNAL_API_TOKEN`, database URL, broker URL, storage paths, and CORS origins through environment variables.
- Production rejects default development secrets at startup.
- Upload filenames are sanitized and storage keys are server-generated.
- Worker subprocess calls have execution timeout, output-size limit, resource constraints, and captured logs.
- Local worker limits default to 30 seconds wall time, 25 seconds CPU time, 1 GiB address space, and 100 MiB output. Configure `ENGINE_*` and `MAX_OUTPUT_BYTES` for deployment capacity.
- `JOB_RETENTION_HOURS` defines artifact expiry. The cleanup service marks terminal jobs `expired` and deletes their private upload/output keys.
- `STORAGE_BACKEND=s3` enables an S3-compatible backend; configure endpoint, bucket, and credentials through `S3_*` variables. The API issues a short-lived signed redirect only after caller authorization.
- `/healthz` reports process liveness; `/readyz` reports database, broker, and active storage readiness and returns `503` when any dependency is unavailable.
- Prometheus metrics include request result, upload size, queue publication failures, job transitions, worker duration, engine exit code, and output size.

## 6. SaaS Acceptance Matrix

| Scenario | Expected behavior |
| --- | --- |
| Browser submits JPEG | Job is queued and status becomes terminal without exposing a secret. |
| Redis unavailable | API responds `503` quickly; job records queue failure; UI shows retryable message. |
| Engine fails | Worker records `failed` with safe error code/message; no partial download. |
| Valid engine run | Job becomes `completed`; authorized output endpoint returns the expected content type. |
| Invalid layout | API returns `422` with field-level errors before persistence. |
| Unauthorized request | API returns `401`; no job or upload is created. |
| Output expired | Download returns `410` and UI explains expiry. |
