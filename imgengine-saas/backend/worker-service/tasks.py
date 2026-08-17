# # # worker-service/tasks.py

# from celery import Celery
# from engine_runner import run_engine
# import requests
# import os
# from prometheus_client import Counter, Histogram
# import time
# from prometheus_client import start_http_server

# from opentelemetry import trace
# from opentelemetry.trace.propagation.tracecontext import TraceContextTextMapPropagator

# tracer = trace.get_tracer(__name__)

# # At the top with other metrics
# SUCCESSFUL_IMAGES = Counter(
#     "worker_images_succeeded_total", "Successfully processed images"
# )
# TASK_COUNT = Counter("worker_tasks_total", "Total tasks processed")
# TASK_LATENCY = Histogram("worker_task_duration_seconds", "Task duration")


# start_http_server(8000)

# API_URL = os.getenv("API_URL", "http://api:8000")

# celery = Celery(
#     "worker",
#     broker="redis://redis:6379/0",
#     backend="redis://redis:6379/0",
# )


# @celery.task(
#     name="tasks.process_image",
#     bind=True,
#     autoretry_for=(Exception,),
#     retry_backoff=5,
#     retry_kwargs={"max_retries": 3},
# )
# def process_image(self, job: dict, carrier: dict):
#     ctx = TraceContextTextMapPropagator().extract(carrier)

#     with tracer.start_as_current_span("worker-process", context=ctx):
#         start = time.time()
#         TASK_COUNT.inc()

#         job_id = job["job_id"]
#         trace_id = job.get("trace_id")

#         print(f"[TRACE {trace_id}] Processing job {job['job_id']}")

#         try:
#             # 🔥 mark processing
#             with tracer.start_as_current_span("update-job-status"):
#                 requests.patch(
#                     f"{API_URL}/internal/jobs/{job_id}",
#                     json={"status": "processing", "trace_id": trace_id},
#                 )
#                 print("CALLING API 1 :", f"{API_URL}/internal/jobs/{job_id}")

#                 # 🔥 run engine
#                 result = run_engine(job)

#                 # 🔥 success / failure
#                 if result["returncode"] != 0:
#                     # Inside the 'if result["returncode"] == 0' block
#                     SUCCESSFUL_IMAGES.inc()

#                     requests.patch(
#                         f"{API_URL}/internal/jobs/{job_id}",
#                         json={
#                             "status": "failed",
#                             "trace_id": trace_id,
#                             "error": result["stderr"],
#                         },
#                     )
#                     print("CALLING API 2 :", f"{API_URL}/internal/jobs/{job_id}")
#                     return

#                 # ✅ success
#                 requests.patch(
#                     f"{API_URL}/internal/jobs/{job_id}",
#                     json={
#                         "status": "completed",
#                         "trace_id": trace_id,
#                         "logs": result["stdout"],
#                     },
#                 )
#                 print("CALLING API 3 :", f"{API_URL}/internal/jobs/{job_id}")

#         except Exception as e:
#             with tracer.start_as_current_span("update-job-status"):
#                 # 🔥 retry + mark retrying
#                 requests.patch(
#                     f"{API_URL}/internal/jobs/{job_id}",
#                     json={
#                         "status": "retrying",
#                         "trace_id": trace_id,
#                         "error": str(e),
#                     },
#                 )
#                 print("CALLING API 4 :", f"{API_URL}/internal/jobs/{job_id}")
#                 raise self.retry(exc=e)

#             requests.patch(
#                 f"http://api:8000/jobs/{job_id}",
#                 json={"status": "completed"},
#             )

#         finally:
#             TASK_LATENCY.observe(time.time() - start)


from celery import Celery
from engine_runner import run_engine
import requests
import os
from prometheus_client import Counter, Histogram, start_http_server
import time
from opentelemetry import trace
from opentelemetry.trace.propagation.tracecontext import TraceContextTextMapPropagator
from datetime import datetime
from app.core.config import (
    CELERY_BROKER_URL,
    CELERY_RESULT_BACKEND,
    CELERY_RESULT_EXPIRES_SECONDS,
    CELERY_TASK_SOFT_TIME_LIMIT_SECONDS,
    CELERY_TASK_TIME_LIMIT_SECONDS,
    CELERY_VISIBILITY_TIMEOUT_SECONDS,
    RETENTION_CLEANUP_BATCH_SIZE,
    RETENTION_CLEANUP_INTERVAL_SECONDS,
)
from app.core.db import SessionLocal
from app.core.storage import artifact_store
from app.models.job import Job
from app.models.job_event import JobEvent
from app.core.job_logs import sanitize_job_log
from app.schemas.job import WorkerJob
from pydantic import ValidationError
from app.core.logger import configure_logging, log_event, logger
import logging

tracer = trace.get_tracer(__name__)

# Metrics
SUCCESSFUL_IMAGES = Counter(
    "worker_images_succeeded_total", "Successfully processed images"
)
TASK_COUNT = Counter("worker_tasks_total", "Total tasks processed")
TASK_LATENCY = Histogram("worker_task_duration_seconds", "Task duration")

# Start Prometheus metrics on port 8001 (to avoid conflict with API on 8000)
start_http_server(8001)

API_URL = os.getenv("API_URL", "http://api:8000")
INTERNAL_API_TOKEN = os.getenv("INTERNAL_API_TOKEN", "local-development-token")

celery = Celery(
    "worker",
    broker=CELERY_BROKER_URL,
    backend=CELERY_RESULT_BACKEND,
)
ENGINE_EXIT_CODES = Counter(
    "worker_engine_exit_codes_total", "Native engine result codes", ["result"]
)
OUTPUT_BYTES = Histogram("worker_output_bytes", "Generated artifact sizes in bytes")
celery.conf.update(
    task_serializer="json",
    result_serializer="json",
    accept_content=["json"],
    result_expires=CELERY_RESULT_EXPIRES_SECONDS,
    task_acks_late=True,
    task_reject_on_worker_lost=True,
    worker_prefetch_multiplier=1,
    task_soft_time_limit=CELERY_TASK_SOFT_TIME_LIMIT_SECONDS,
    task_time_limit=CELERY_TASK_TIME_LIMIT_SECONDS,
    broker_connection_retry_on_startup=True,
    broker_transport_options={
        "visibility_timeout": CELERY_VISIBILITY_TIMEOUT_SECONDS,
        "retry_on_timeout": True,
    },
    beat_schedule={
        "expire-artifacts": {
            "task": "tasks.expire_artifacts",
            "schedule": RETENTION_CLEANUP_INTERVAL_SECONDS,
        }
    },
    timezone="UTC",
)


def update_job(job_id: str, payload: dict) -> None:
    response = requests.patch(
        f"{API_URL}/internal/jobs/{job_id}",
        json=payload,
        headers={"X-Internal-Token": INTERNAL_API_TOKEN},
        timeout=10,
    )
    response.raise_for_status()


def is_terminal_transition_conflict(error: requests.HTTPError) -> bool:
    return error.response is not None and error.response.status_code == 409


@celery.task(name="tasks.expire_artifacts")
def expire_artifacts() -> int:
    db = SessionLocal()
    try:
        jobs = (
            db.query(Job)
            .filter(Job.expires_at <= datetime.utcnow(), Job.status.in_(("completed", "failed")))
            .order_by(Job.expires_at)
            .limit(RETENTION_CLEANUP_BATCH_SIZE)
            .all()
        )
        for job in jobs:
            artifact_store.delete(job.input)
            artifact_store.delete(job.output)
            db.query(JobEvent).filter(JobEvent.job_id == job.id).delete(synchronize_session=False)
            job.status = "expired"
            job.error = "Job artifacts have expired."
            job.logs = None
        db.commit()
        return len(jobs)
    except Exception:
        db.rollback()
        raise
    finally:
        db.close()


@celery.task(
    name="tasks.process_image",
    bind=True,
    autoretry_for=(Exception,),
    retry_backoff=5,
    retry_kwargs={"max_retries": 3},
)
def process_image(self, job: dict, carrier: dict):
    configure_logging()
    ctx = TraceContextTextMapPropagator().extract(carrier)

    with tracer.start_as_current_span("worker-process", context=ctx):
        start = time.time()
        TASK_COUNT.inc()

        raw_job_id = job.get("job_id") if isinstance(job, dict) else None
        try:
            payload = WorkerJob.model_validate(job)
        except ValidationError:
            if isinstance(raw_job_id, str) and raw_job_id:
                update_job(
                    raw_job_id,
                    {
                        "status": "failed",
                        "error": "Invalid processing payload.",
                        "event": "worker_payload_rejected",
                        "event_level": "error",
                        "event_message": "Worker rejected an invalid processing payload.",
                    },
                )
            return

        job = payload.model_dump()
        job_id = payload.job_id
        trace_id = payload.trace_id

        log_event(logging.INFO, "worker_task_received", component="worker", trace_id=trace_id, job_id=job_id)

        try:
            try:
                update_job(
                    job_id,
                    {
                        "status": "processing",
                        "event": "engine_execution_started",
                        "event_message": "Native image engine execution started.",
                    },
                )
            except requests.HTTPError as error:
                if is_terminal_transition_conflict(error):
                    return
                raise

            result = run_engine(job)
            ENGINE_EXIT_CODES.labels(result="success" if result["returncode"] == 0 else "failure").inc()
            log_event(
                logging.INFO if result["returncode"] == 0 else logging.ERROR,
                "engine_execution_finished",
                component="worker",
                trace_id=trace_id,
                job_id=job_id,
                duration_ms=result["duration_ms"],
                message="Native image engine execution completed.",
            )

            if result["returncode"] != 0:
                update_job(
                    job_id,
                    {
                        "status": "failed",
                        "error": sanitize_job_log(result["stderr"], job["input"], job["output"]),
                        "event": "engine_execution_failed",
                        "event_level": "error",
                        "event_message": "Native image engine execution failed.",
                        "event_details": {"duration_ms": result["duration_ms"]},
                    },
                )
                return

            update_job(
                job_id,
                {
                    "status": "completed",
                    "logs": sanitize_job_log(
                        "\n".join(part for part in (result["stdout"], result["stderr"]) if part),
                        job["input"],
                        job["output"],
                    ),
                    "event": "engine_execution_completed",
                    "event_message": "Native image engine execution completed successfully.",
                        "event_details": {
                            "duration_ms": result["duration_ms"],
                            "output_bytes": result["output_bytes"],
                            **({"preset": job["preset"]} if job.get("preset") else {}),
                        },
                },
            )
            SUCCESSFUL_IMAGES.inc()
            OUTPUT_BYTES.observe(result["output_bytes"])

        except Exception as error:
            logger.exception(
                "Worker task failed.",
                extra={
                    "event": "worker_task_error",
                    "component": "worker",
                    "trace_id": trace_id,
                    "job_id": job_id,
                },
            )
            try:
                update_job(
                    job_id,
                    {
                        "status": "retrying",
                        "error": "Processing failed unexpectedly and will be retried.",
                        "event": "worker_retry_scheduled",
                        "event_level": "error",
                        "event_message": "Worker processing failed; retry scheduled.",
                    },
                )
            except requests.HTTPError as update_error:
                if is_terminal_transition_conflict(update_error):
                    return
                raise
            raise self.retry(exc=error)

        finally:
            TASK_LATENCY.observe(time.time() - start)
