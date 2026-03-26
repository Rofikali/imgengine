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


router = APIRouter()


def get_db():
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()


@limiter.limit("5/minute")
@router.post("/generate", dependencies=[Depends(verify_api_key)])
def generate(
    request: Request, file: UploadFile = File(...), db: Session = Depends(get_db)
):
    job_id = str(uuid.uuid4())
    logger.info(f"Job created: {job_id}")

    input_path = f"/data/uploads/{job_id}_{file.filename}"
    output_path = f"/data/outputs/{job_id}.png"

    # save file
    with open(input_path, "wb") as buffer:
        shutil.copyfileobj(file.file, buffer)

    # 1. Create default settings using your Pydantic model
    # This fills in 'cols', 'rows', etc., with the defaults you defined
    settings = GenerateJob(input=input_path, output=output_path)

    # save job in DB
    job = Job(id=job_id, input=input_path, output=output_path, status="queued")
    db.add(job)
    db.commit()

    # 3. Send the FULL dictionary to the worker
    celery.send_task(
        "tasks.process_image",  # ✅ FIXED",
        args=[
            {
                "job_id": job_id,
                "input": input_path,
                "output": output_path,
                **settings.model_dump(),
            }
        ],
    )

    # return {"job_id": job_id}
    return {
        "job_id": job.id,
        "status": job.status,
        "output": job.output,
        "error": job.error,
    }


@router.get("/status/{job_id}")
def status(job_id: str, db: Session = Depends(get_db)):
    job = db.query(Job).filter(Job.id == job_id).first()

    return {"job_id": job.id, "status": job.status, "output": job.output}
