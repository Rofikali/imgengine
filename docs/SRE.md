# IMGENGINE SRE Contract

## Service Objectives

| Indicator | Objective | Measurement |
| --- | --- | --- |
| API availability | 99.9% monthly | Successful `/readyz` probes |
| Job acceptance | 99.5% monthly | `POST /api/generate` non-5xx responses excluding client errors |
| Job completion | 99% daily | `completed / (completed + failed)` for accepted jobs |
| Queue publication | 99.9% daily | No sustained increase in `imgengine_queue_publication_failures_total` |
| Processing latency | p95 under 30 seconds | `worker_task_duration_seconds` over a 15-minute window |

## Alert Response

1. Confirm API, worker, Redis, PostgreSQL, and storage readiness.
2. Search JSON logs by `trace_id`; retrieve the authorized job timeline by `job_id`.
3. For queue failures, stop accepting risky deployment changes, inspect Redis connectivity, and retry only idempotent submissions.
4. For engine failures, preserve the safe error code and fixture metadata, then add a regression before changing native code.
5. Record impact, timeline, cause, mitigation, and follow-up owner in the incident review.

## Logging Policy

JSON stdout is the primary production signal. Rotating files under `/data/logs` are a node-local recovery aid, not the durable source of truth. Ship stdout to a managed collector before multi-node deployment. Never place customer image bytes, API keys, credentials, or unredacted filenames in logs, metrics, traces, or alerts.
