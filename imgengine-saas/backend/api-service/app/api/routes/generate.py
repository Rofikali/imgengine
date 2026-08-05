# backend/app/api/routes/generate.py

import logging
import uuid
from pathlib import Path
from datetime import datetime, timedelta

from fastapi import APIRouter, UploadFile, File, Form, Depends, HTTPException, Request, status as http_status
from fastapi.responses import FileResponse, RedirectResponse
from sqlalchemy.orm import Session
from app.schemas.job import GenerateJob
from app.core.db import SessionLocal
from app.models.job import Job


from app.core.celery_client import assert_broker_available, celery
from app.core.security import verify_api_key
from app.core.limiter import limiter

from app.core.logger import log_event
from app.core.job_events import record_job_event
from app.models.job_event import JobEvent

from app.core.tracing import tracer
import time
from app.core.config import GENERATE_RATE_LIMIT, JOB_RETENTION_HOURS, MAX_UPLOAD_BYTES
from app.core.storage import StoragePathError, artifact_store

# Change your import at the top
from app.core.metrics import IMAGE_PROCESSED_TOTAL, QUEUE_PUBLICATION_FAILURES, REQUEST_COUNT, REQUEST_LATENCY, UPLOAD_BYTES


router = APIRouter()


def output_url(job: Job) -> str | None:
    return f"/api/output/{job.id}" if job.status == "completed" else None


def serialize_expiry(job: Job) -> str | None:
    return job.expires_at.isoformat() if job.expires_at else None


def get_db():
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()


@limiter.limit(GENERATE_RATE_LIMIT)
@router.post("/generate", dependencies=[Depends(verify_api_key)])
async def generate(
    request: Request,
    file: UploadFile = File(...),
    cols: int = Form(6, ge=1, le=20),
    rows: int = Form(6, ge=1, le=20),
    gap: int = Form(15, ge=0, le=500),
    padding: int = Form(20, ge=0, le=1000),
    width: float = Form(4.5, gt=0, le=50),
    height: float = Form(3.5, gt=0, le=50),
    dpi: int = Form(300, ge=72, le=1200),
    border: int = Form(2, ge=0, le=100),
    bleed: int = Form(0, ge=0, le=500),
    crop_mark: int = Form(15, ge=0, le=500),
    crop_thickness: int = Form(2, ge=1, le=100),
    crop_offset: int = Form(8, ge=0, le=500),
    db: Session = Depends(get_db),
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
        log_event(logging.INFO, "job_submission_started", component="api", trace_id=trace_id, job_id=job_id)

        if file.content_type not in {"image/jpeg", "image/png"}:
            raise HTTPException(
                status_code=http_status.HTTP_415_UNSUPPORTED_MEDIA_TYPE,
                detail="Only JPEG and PNG uploads are supported",
            )

        filename = Path(file.filename or "upload").name
        try:
            input_key = artifact_store.upload_key(job_id, filename)
        except StoragePathError as exc:
            raise HTTPException(
                status_code=http_status.HTTP_415_UNSUPPORTED_MEDIA_TYPE,
                detail="Only JPEG and PNG uploads are supported",
            ) from exc
        output_key = artifact_store.output_key(job_id)
        input_path = artifact_store.path_for(input_key)

        bytes_written = 0
        with input_path.open("wb") as buffer:
            while chunk := await file.read(1024 * 1024):
                bytes_written += len(chunk)
                if bytes_written > MAX_UPLOAD_BYTES:
                    buffer.close()
                    input_path.unlink(missing_ok=True)
                    raise HTTPException(
                        status_code=http_status.HTTP_413_REQUEST_ENTITY_TOO_LARGE,
                        detail=f"Upload exceeds the {MAX_UPLOAD_BYTES} byte limit",
                    )
                buffer.write(chunk)
        artifact_store.upload(input_key)
        UPLOAD_BYTES.observe(bytes_written)

        # 1. Create default settings using your Pydantic model
        # This fills in 'cols', 'rows', etc., with the defaults you defined
        settings = GenerateJob(
            input=input_key,
            output=output_key,
            cols=cols,
            rows=rows,
            gap=gap,
            padding=padding,
            width=width,
            height=height,
            dpi=dpi,
            border=border,
            bleed=bleed,
            crop_mark=crop_mark,
            crop_thickness=crop_thickness,
            crop_offset=crop_offset,
        )

        # save job in DB
        job = Job(
            id=job_id,
            trace_id=trace_id,
            input=input_key,
            output=output_key,
            status="queued",
            expires_at=datetime.utcnow() + timedelta(hours=JOB_RETENTION_HOURS),
        )
        db.add(job)
        record_job_event(
            db,
            job,
            event="job_queued",
            component="api",
            message="Upload validated and job queued for processing.",
            details={"upload_bytes": bytes_written},
        )
        db.commit()

        carrier = {}
        TraceContextTextMapPropagator().inject(carrier)

        # 3. Send the FULL dictionary to the worker

        job_payload = {
            "version": 1,
            "job_id": job_id,
            "trace_id": trace_id,
            "input": input_key,
            "output": output_key,
            **settings.model_dump(),
        }

        try:
            assert_broker_available()
            celery.send_task(
            "tasks.process_image",
            args=[job_payload, carrier],  # ✅ CORRECT
            )
        except Exception as exc:
            QUEUE_PUBLICATION_FAILURES.inc()
            job.status = "failed"
            job.error = "The processing queue is unavailable. Please retry shortly."
            record_job_event(
                db,
                job,
                event="queue_publication_failed",
                component="api",
                level="error",
                message="The processing queue was unavailable.",
            )
            db.commit()
            log_event(logging.ERROR, "queue_publication_failed", component="api", trace_id=trace_id, job_id=job_id)
            raise HTTPException(
                status_code=http_status.HTTP_503_SERVICE_UNAVAILABLE,
                detail="Image processing is temporarily unavailable",
            ) from exc

        REQUEST_LATENCY.labels(endpoint="/generate").observe(time.time() - start)
        IMAGE_PROCESSED_TOTAL.inc()  # Increment every time an image is made
        log_event(logging.INFO, "job_enqueued", component="api", trace_id=trace_id, job_id=job_id, duration_ms=round((time.time() - start) * 1000, 2))

        await file.close()
        return {
            "job_id": job.id,
            "trace_id": job.trace_id,  # ✅ real trace
            "status": job.status,
            "output_url": output_url(job),
            "expires_at": serialize_expiry(job),
            "error": job.error,
            "carrier": carrier,
        }


@router.get("/status/{job_id}", dependencies=[Depends(verify_api_key)])
def status(job_id: str, db: Session = Depends(get_db)):
    job = db.query(Job).filter(Job.id == job_id).first()
    if not job:
        raise HTTPException(status_code=http_status.HTTP_404_NOT_FOUND, detail="Job not found")

    return {
        "job_id": job.id,
        "trace_id": job.trace_id,
        "status": job.status,
        "output_url": output_url(job),
        "expires_at": serialize_expiry(job),
        "error": job.error,
    }


@router.get("/jobs/{job_id}/logs", dependencies=[Depends(verify_api_key)])
def job_logs(job_id: str, db: Session = Depends(get_db)):
    job = db.query(Job).filter(Job.id == job_id).first()
    if not job:
        raise HTTPException(status_code=http_status.HTTP_404_NOT_FOUND, detail="Job not found")
    if job.status == "expired":
        raise HTTPException(status_code=http_status.HTTP_410_GONE, detail="Job logs have expired")
    events = (
        db.query(JobEvent)
        .filter(JobEvent.job_id == job.id)
        .order_by(JobEvent.created_at, JobEvent.id)
        .all()
    )
    return {
        "job_id": job.id,
        "trace_id": job.trace_id,
        "status": job.status,
        "logs": job.logs or "",
        "events": [
            {
                "timestamp": event.created_at.isoformat(),
                "event": event.event,
                "component": event.component,
                "level": event.level,
                "message": event.message,
                "details": event.details,
            }
            for event in events
        ],
    }


@router.get("/output/{job_id}", dependencies=[Depends(verify_api_key)])
def download_output(job_id: str, db: Session = Depends(get_db)):
    job = db.query(Job).filter(Job.id == job_id).first()
    if not job:
        raise HTTPException(status_code=http_status.HTTP_404_NOT_FOUND, detail="Job not found")
    if job.status == "expired":
        raise HTTPException(status_code=http_status.HTTP_410_GONE, detail="Job output has expired")
    if job.status != "completed":
        raise HTTPException(status_code=http_status.HTTP_409_CONFLICT, detail="Output is not available until the job completes")

    try:
        output_path = artifact_store.path_for(job.output)
    except StoragePathError as exc:
        raise HTTPException(status_code=http_status.HTTP_500_INTERNAL_SERVER_ERROR, detail="Job output path is invalid") from exc
    if not output_path.is_file():
        output_path = artifact_store.download(job.output)
    signed_url = artifact_store.download_url(job.output)
    if signed_url:
        return RedirectResponse(signed_url)
    return FileResponse(output_path, filename=f"{job_id}{output_path.suffix}")
