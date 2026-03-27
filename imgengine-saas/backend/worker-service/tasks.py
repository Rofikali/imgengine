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

celery = Celery(
    "worker",
    broker="redis://redis:6379/0",
    backend="redis://redis:6379/0",
)


@celery.task(
    name="tasks.process_image",
    bind=True,
    autoretry_for=(Exception,),
    retry_backoff=5,
    retry_kwargs={"max_retries": 3},
)
def process_image(self, job: dict, carrier: dict):
    ctx = TraceContextTextMapPropagator().extract(carrier)

    with tracer.start_as_current_span("worker-process", context=ctx):
        start = time.time()
        TASK_COUNT.inc()

        job_id = job["job_id"]
        trace_id = job.get("trace_id")

        print(f"[TRACE {trace_id}] Processing job {job_id}")

        try:
            # 1. Mark as Processing
            with tracer.start_as_current_span("update-status-processing"):
                requests.patch(
                    f"{API_URL}/internal/jobs/{job_id}",
                    json={"status": "processing", "trace_id": trace_id},
                )

            # 2. Run C-Engine
            result = run_engine(job)

            # 3. Handle Result
            if result["returncode"] != 0:
                # FAILURE BLOCK
                requests.patch(
                    f"{API_URL}/internal/jobs/{job_id}",
                    json={
                        "status": "failed",
                        "trace_id": trace_id,
                        "error": result["stderr"],
                    },
                )
                return

            # SUCCESS BLOCK
            SUCCESSFUL_IMAGES.inc()  # Increment ONLY on success
            requests.patch(
                f"{API_URL}/internal/jobs/{job_id}",
                json={
                    "status": "completed",
                    "trace_id": trace_id,
                    "logs": result["stdout"],
                },
            )

        except Exception as e:
            # 4. Handle Retries
            requests.patch(
                f"{API_URL}/internal/jobs/{job_id}",
                json={
                    "status": "retrying",
                    "trace_id": trace_id,
                    "error": str(e),
                },
            )
            raise self.retry(exc=e)

        finally:
            TASK_LATENCY.observe(time.time() - start)
