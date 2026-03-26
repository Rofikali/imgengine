# # worker-service/tasks.py

from celery import Celery
from engine_runner import run_engine
import requests
import os

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
def process_image(self, job: dict):

    job_id = job["job_id"]

    try:
        # 🔥 mark processing
        requests.patch(
            f"{API_URL}/internal/jobs/{job_id}",
            json={"status": "processing"},
        )
        print("CALLING API 1 :", f"{API_URL}/internal/jobs/{job_id}")

        # 🔥 run engine
        result = run_engine(job)

        # 🔥 success / failure
        if result["returncode"] != 0:
            requests.patch(
                f"{API_URL}/internal/jobs/{job_id}",
                json={
                    "status": "failed",
                    "error": result["stderr"],
                },
            )
            print("CALLING API 2 :", f"{API_URL}/internal/jobs/{job_id}")
            return

        # ✅ success
        requests.patch(
            f"{API_URL}/internal/jobs/{job_id}",
            json={
                "status": "completed",
                "logs": result["stdout"],
            },
        )
        print("CALLING API 3 :", f"{API_URL}/internal/jobs/{job_id}")

    except Exception as e:
        # 🔥 retry + mark retrying
        requests.patch(
            f"{API_URL}/internal/jobs/{job_id}",
            json={
                "status": "retrying",
                "error": str(e),
            },
        )
        print("CALLING API 4 :", f"{API_URL}/internal/jobs/{job_id}")
        raise self.retry(exc=e)
