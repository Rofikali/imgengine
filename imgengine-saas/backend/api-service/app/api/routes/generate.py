# backend/app/api/routes/generate.py

import uuid
import shutil
from fastapi import APIRouter, UploadFile, File, Depends, Request
from sqlalchemy.orm import Session
from app.schemas.job import GenerateJob
from app.core.db import SessionLocal
from app.models.job import Job


from app.core.celery_client import celery
from app.core.security import verify_api_key
from app.core.limiter import limiter

from app.core.logger import logger

from app.core.tracing import tracer
import time

# Change your import at the top
from app.core.metrics import REQUEST_COUNT, REQUEST_LATENCY, IMAGE_PROCESSED_TOTAL


router = APIRouter()


def get_db():
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()


# To this (for testing):
@limiter.limit("1000/minute")
# @limiter.limit("5/minute")    # here is Actually limit to 5 per minute for testing, change to 1000 in production
@router.post("/generate", dependencies=[Depends(verify_api_key)])
async def generate(
    request: Request, file: UploadFile = File(...), db: Session = Depends(get_db)
):
    from opentelemetry.trace.propagation.tracecontext import (
        TraceContextTextMapPropagator,
    )

    with tracer.start_as_current_span("generate-job"):
        start = time.time()

        REQUEST_COUNT.labels(method="POST", endpoint="/generate").inc()
        job_id = str(uuid.uuid4())

        trace_id = str(uuid.uuid4())
        # carrier = job_id.get("carrier", {})
        # job_id = job_id["job_id"]

        # print("TRACE CARRIER:", carrier)
        logger.info("job started", extra={"trace_id": trace_id})

        input_path = f"/data/uploads/{job_id}_{file.filename}"
        output_path = f"/data/outputs/{job_id}.png"

        # save file
        with open(input_path, "wb") as buffer:
            shutil.copyfileobj(file.file, buffer)

        # 1. Create default settings using your Pydantic model
        # This fills in 'cols', 'rows', etc., with the defaults you defined
        settings = GenerateJob(input=input_path, output=output_path)

        # save job in DB
        job = Job(
            id=job_id,
            trace_id=trace_id,
            input=input_path,
            output=output_path,
            status="queued",
        )
        db.add(job)
        db.commit()

        carrier = {}
        TraceContextTextMapPropagator().inject(carrier)

        # 3. Send the FULL dictionary to the worker

        job_payload = {
            "job_id": job_id,
            "trace_id": trace_id,
            "input": input_path,
            "output": output_path,
            **settings.model_dump(),
        }

        celery.send_task(
            "tasks.process_image",
            args=[job_payload, carrier],  # ✅ CORRECT
        )

        REQUEST_LATENCY.labels(endpoint="/generate").observe(time.time() - start)
        IMAGE_PROCESSED_TOTAL.inc()  # Increment every time an image is made

        return {
            "job_id": job.id,
            "trace_id": job.trace_id,  # ✅ real trace
            "status": job.status,
            "output": job.output,
            "error": job.error,
            "carrier": carrier,
        }


@router.get("/status/{job_id}")
def status(job_id: str, db: Session = Depends(get_db)):
    job = db.query(Job).filter(Job.id == job_id).first()
    trace_id = str(uuid.uuid4())

    return {
        "job_id": job.id,
        "trace_id": trace_id,
        "status": job.status,
        "output": job.output,
    }
