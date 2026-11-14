# Historical original root next.md — 2026-08-17

**Source:** [`next.md` at `a7be44310311859f764cd4170728e0465d02a446`](https://github.com/Rofikali/imgengine/blob/a7be44310311859f764cd4170728e0465d02a446/next.md). **Historical only:** captured for provenance, not a current task mandate. It describes the earlier async FastAPI/Celery/Redis/PostgreSQL service, with some native tasks later completed. Current status and priorities live in [`../next.md`](../next.md); current source/API contracts supersede dated plans.

## Verbatim archived content

```text
replace removed prints or prints Debug with observability logger
add sanitizer ci job for native-engine

Next Principal-Level Moves
Stabilize local platform
Add a make verify / PowerShell script: build, migrations, health checks, JPEG/PNG/CMYK smoke jobs, output validation.
Make this the required gate before every change.

Production observability
Keep JSON stdout as primary; use Fluent Bit/OpenTelemetry → Loki or ELK.
Alert on API 5xx, worker failures, queue backlog, job latency, and storage errors.

Reliability contract
Define SLOs: e.g. 99.9% API availability, p95 submit latency, p95 completion time.
Add dashboards and runbooks for Redis, Postgres, worker, storage, and native-engine failures.

Native-engine quality
Add regression fixtures for JPEG baseline/progressive/CMYK/YCCK and PNG.
Run them in Docker CI with sanitizers and fuzzing for decoder boundaries.

Security and tenancy
Keep key ownership/idempotency; add API-key rotation, per-tenant quotas, upload content sniffing, malware scanning policy, and audit retention.

Delivery discipline
Use PRD → contract → tests → implementation → deployment evidence for every loop.
Avoid new features until the verification pipeline and incident runbooks are repeatable.
```
