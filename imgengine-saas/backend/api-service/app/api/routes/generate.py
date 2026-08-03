# backend/app/api/routes/generate.py

import uuid
from pathlib import Path

from fastapi import APIRouter, UploadFile, File, Form, Depends, HTTPException, Request, status as http_status
from fastapi.responses import FileResponse
from sqlalchemy.orm import Session
from app.schemas.job import GenerateJob
from app.core.db import SessionLocal
from app.models.job import Job


from app.core.celery_client import assert_broker_available, celery
from app.core.security import verify_api_key
from app.core.limiter import limiter

from app.core.logger import logger

from app.core.tracing import tracer
import time
from app.core.config import MAX_UPLOAD_BYTES
from app.core.storage import StoragePathError, artifact_store

# Change your import at the top
from app.core.metrics import REQUEST_COUNT, REQUEST_LATENCY, IMAGE_PROCESSED_TOTAL


router = APIRouter()


def output_url(job: Job) -> str | None:
    return f"/api/output/{job.id}" if job.status == "completed" else None


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
        logger.info("job started", extra={"trace_id": trace_id})

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
        )
        db.add(job)
        db.commit()

        carrier = {}
        TraceContextTextMapPropagator().inject(carrier)

        # 3. Send the FULL dictionary to the worker

        job_payload = {
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
            job.status = "failed"
            job.error = "The processing queue is unavailable. Please retry shortly."
            db.commit()
            logger.exception("job queue submission failed", extra={"trace_id": trace_id})
            raise HTTPException(
                status_code=http_status.HTTP_503_SERVICE_UNAVAILABLE,
                detail="Image processing is temporarily unavailable",
            ) from exc

        REQUEST_LATENCY.labels(endpoint="/generate").observe(time.time() - start)
        IMAGE_PROCESSED_TOTAL.inc()  # Increment every time an image is made

        await file.close()
        return {
            "job_id": job.id,
            "trace_id": job.trace_id,  # ✅ real trace
            "status": job.status,
            "output_url": output_url(job),
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
    }


@router.get("/output/{job_id}", dependencies=[Depends(verify_api_key)])
def download_output(job_id: str, db: Session = Depends(get_db)):
    job = db.query(Job).filter(Job.id == job_id).first()
    if not job:
        raise HTTPException(status_code=http_status.HTTP_404_NOT_FOUND, detail="Job not found")
    if job.status != "completed":
        raise HTTPException(status_code=http_status.HTTP_409_CONFLICT, detail="Output is not available until the job completes")

    try:
        output_path = artifact_store.path_for(job.output)
    except StoragePathError as exc:
        raise HTTPException(status_code=http_status.HTTP_500_INTERNAL_SERVER_ERROR, detail="Job output path is invalid") from exc
    if not output_path.is_file():
        raise HTTPException(status_code=http_status.HTTP_404_NOT_FOUND, detail="Output file not found")
    return FileResponse(output_path, filename=f"{job_id}{output_path.suffix}")
