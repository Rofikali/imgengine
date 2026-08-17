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

**Current implemented behavior:** accepts JPEG (including progressive CMYK/YCCK JPEGs normalized to RGB) and PNG uploads only when their magic bytes match the declared MIME type, validates bounded layout fields against `GenerateJob`, creates a `queued` job, and publishes to Celery. Storage keys use the detected type rather than the user filename. The SaaS worker produces JPEG output. Files over `MAX_UPLOAD_BYTES` return `413`; unsupported or spoofed content types return `415`; missing/invalid API key returns `401`; unavailable Redis returns `503` within approximately three seconds.

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
  "output_key": "outputs/uuid.jpg",
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

The worker accepts only payload `version: 1`. It rejects unknown versions, invalid layout, or malformed payloads before invoking the CLI, marking a known job as `failed` without retrying invalid work.

## 4. Nuxt UX Requirements

The UI exposes inputs for source image, width, height, DPI, rows, columns, gap, padding, border, bleed, and crop-mark length/thickness/offset. It also exposes the native `passport-45x35`, `passport-38x35`, and `printready-6x6` catalogs through `GET /api/presets`. Selecting a preset runs the native preset without mixing in custom layout flags.

The UI polls job status, shows a human-readable error returned by the server, disables download until `completed`, and never displays raw storage paths. Layout controls must map exactly to the API contract; no hidden client-only defaults.

## 5. Security and Operations

- Configure `API_KEYS`, `INTERNAL_API_TOKEN`, database URL, broker URL, storage paths, and CORS origins through environment variables.
- `DATABASE_URL` must be supplied with non-default credentials for production. The API rejects the local `imgengine:imgengine` database password whenever `DEPLOYMENT_ENV=production`.
- Docker images pin the `uv` installer version and retry dependency downloads. API, worker, cleanup, and web services restart unless stopped; the API health check uses `/healthz`.
- Every job is owned by the SHA-256 fingerprint of the API key that created it. Status, output, and job-log endpoints return `404` unless the same key is supplied; this prevents job-ID enumeration and cross-tenant reads. Raw API keys are never persisted.
- `API_KEYS` is the comma-separated API allow-list. `WEB_API_KEY` is one member of that list and is the only key injected into the Nuxt server-side proxy; never pass the entire allow-list as `NUXT_API_KEY`.
- `POST /api/generate` accepts an optional `Idempotency-Key` header (maximum 128 characters). Repeating a key with the same API-key owner returns the original job without creating or queuing a second job. Concurrent duplicate requests are resolved by a database unique index and the losing upload is deleted.
- Jobs created before the ownership migration have no owner fingerprint and are intentionally inaccessible through public endpoints. Let normal retention remove them, or migrate them with a controlled, one-time administrative process.
- `DEPLOYMENT_ENV=production` rejects default API keys, internal tokens shorter than 32 characters, wildcard CORS, and localhost CORS origins at startup.
- Production rejects default development secrets at startup.
- Upload filenames are sanitized and storage keys are server-generated.
- Upload MIME declarations are verified against JPEG/PNG signatures before storage. This is an ingress control, not a malware-scanning substitute.
- `GENERATE_RATE_LIMIT` defaults to `30/minute` per hashed API key; unauthenticated attempts are limited by source address and accepted requests use the authenticated key identity.
- Worker subprocess calls have execution timeout, output-size limit, resource constraints, and captured logs.
- Celery accepts JSON payloads only. Jobs are acknowledged after execution, use one-message worker prefetch, and are re-delivered if a worker process is lost. Delivery is therefore at-least-once; status transitions are idempotent so a redelivery cannot overwrite a terminal job.
- Queue publication has bounded retries. `CELERY_TASK_SOFT_TIME_LIMIT_SECONDS` and `CELERY_TASK_TIME_LIMIT_SECONDS` must remain greater than `ENGINE_TIMEOUT_SECONDS`; Redis visibility timeout must exceed the task time limit.
- Local worker limits default to 30 seconds wall time, 25 seconds CPU time, 1 GiB address space, and 100 MiB output. Configure `ENGINE_*` and `MAX_OUTPUT_BYTES` for deployment capacity.
- `JOB_RETENTION_HOURS` defines artifact expiry. The cleanup service marks terminal jobs `expired` and deletes their private upload/output keys.
- `GET /api/jobs/{job_id}/logs` returns authenticated, path-sanitized job logs capped by `MAX_JOB_LOG_CHARS` (default `16000`). Logs are cleared by the same retention cleanup that expires artifacts.
- Each job also has a bounded, ordered audit timeline (`queued`, worker start, engine completion/failure, and retries) linked by `job_id` and `trace_id`. `GET /api/jobs/{job_id}/logs` returns this timeline plus safe native diagnostics; both are deleted when the job expires.
- API and worker operational logs are newline-delimited JSON written to separate rotating files under `LOG_DIR` (`api.log`, `worker.log` by default). Each event includes UTC timestamp, level, service, component, event, and available request/job/trace identifiers. Configure `LOG_LEVEL`, `LOG_FILE_MAX_BYTES` (default 20 MiB), and `LOG_FILE_BACKUP_COUNT` (default 5); ship stdout or these files to the central log platform in production.
- `STORAGE_BACKEND=s3` enables an S3-compatible backend; configure endpoint, bucket, and credentials through `S3_*` variables. The API issues a short-lived signed redirect only after caller authorization.
- `/healthz` reports process liveness; `/readyz` reports database, broker, and active storage readiness and returns `503` when any dependency is unavailable.
- Prometheus metrics include request result, upload size, queue publication failures, job transitions, worker duration, engine exit code, and output size.
- Alert on sustained queue publication failures, increases in `failed`/`retrying` transitions, engine failures, and readiness returning `503`.

## 6. SaaS Acceptance Matrix

| Scenario | Expected behavior |
| --- | --- |
| Browser submits JPEG | Baseline, progressive, CMYK, and YCCK JPEGs are normalized to RGB; the job becomes terminal without exposing a secret. |
| Redis unavailable | API responds `503` quickly; job records queue failure; UI shows retryable message. |
| Engine fails | Worker records `failed` with safe error code/message; no partial download. |
| Valid engine run | Job becomes `completed`; authorized output endpoint returns the expected content type. |
| Invalid layout | API returns `422` with field-level errors before persistence. |
| Unauthorized request | API returns `401`; no job or upload is created. |
| Output expired | Download returns `410` and UI explains expiry. |
