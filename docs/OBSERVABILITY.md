# ImgEngine Observability

## Objectives

Every production request and asynchronous image job must be diagnosable using a request ID, `trace_id`, and `job_id` without logging uploaded image content, API keys, storage credentials, or absolute artifact paths.

## Signals

| Signal | Owner | Purpose |
| --- | --- | --- |
| JSON service logs | API and worker | Operational debugging and incident investigation |
| Job audit timeline | PostgreSQL | Authorized job-specific support and traceability |
| Prometheus metrics | API and worker | Alerting, capacity, and reliability trends |
| OpenTelemetry spans | API and worker | Cross-service latency analysis when an exporter is configured |

## Structured Logs

The API writes `LOG_DIR/api.log`; the worker writes `LOG_DIR/worker.log`. Every line is JSON with these standard fields:

```json
{
  "timestamp": "2026-08-03T12:00:00+00:00",
  "level": "info",
  "service": "worker",
  "component": "worker",
  "event": "engine_execution_finished",
  "trace_id": "...",
  "job_id": "...",
  "duration_ms": 32.4
}
```

Files rotate at `LOG_FILE_MAX_BYTES` (default 20 MiB), retaining `LOG_FILE_BACKUP_COUNT` backups (default 5). Containers also emit these records to stdout for production collection by the platform log agent. Do not use Redis as a durable log store.

## Job Audit Timeline

`GET /api/jobs/{job_id}/logs` requires an API key and returns safe native diagnostics plus ordered job events. Standard events are:

- `job_queued`
- `engine_execution_started`
- `engine_execution_completed` or `engine_execution_failed`
- `worker_retry_scheduled`
- `queue_publication_failed`

The worker creates these events through the authenticated internal API, so the database is the audit source of truth. Cleanup deletes events, diagnostics, and artifacts when `JOB_RETENTION_HOURS` expires.

## Operations

Search by `trace_id` across API and worker logs first; use `job_id` to retrieve the authorized audit timeline. Alert on readiness failures, queue-publication failures, retry growth, engine failures, and anomalous engine durations. Ship JSON logs to Elasticsearch, Loki, CloudWatch, or an equivalent centralized platform before multi-node deployment; the local Logstash stack is development-only.

### Local Stack

The API/worker stack does not start observability services by default. Start the local stack only when needed:

```powershell
$env:API_KEYS='local-test-key'
$env:INTERNAL_API_TOKEN='local-internal-token'
$env:OTEL_EXPORTER_OTLP_TRACES_ENDPOINT='http://jaeger:4318/v1/traces'
docker compose -f imgengine-saas/infra/docker-compose.yml --profile observability up -d
```

Kibana is available at `http://localhost:5601`, Grafana at `http://localhost:3001`, Prometheus at `http://localhost:9090`, Jaeger at `http://localhost:16686`, and Elasticsearch at `http://localhost:9200`. Logstash tails only new JSON lines and persists read offsets, preventing duplicate ingestion after restarts.

## Security Boundaries

- Never add API keys, bearer tokens, cookies, image bytes, or storage credentials to log fields.
- Do not log user-supplied filenames or absolute storage paths.
- Keep native hot paths free of per-pixel logging; the worker emits lifecycle and duration telemetry around the CLI invocation.
- The worker passes `trace_id` to the CLI as `IMGENGINE_TRACE_ID`; the CLI emits only safe process-boundary events (`engine_started`, `engine_completed`, `engine_failed`, initialization/build failures) and never logs artifact paths through this channel.
- Job-log access is restricted to the hashed API-key owner of the job. Replace this identity with a tenant/user principal when full account authentication is introduced.
