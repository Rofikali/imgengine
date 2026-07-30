# api-service/app/api/routes/internal.py

from fastapi import APIRouter, Depends, HTTPException, status
from sqlalchemy.orm import Session
from app.core.db import SessionLocal
from app.models.job import Job
from app.core.security import verify_internal_token

router = APIRouter()


def get_db():
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()


@router.patch("/jobs/{job_id}", dependencies=[Depends(verify_internal_token)])
def update_job(job_id: str, data: dict, db: Session = Depends(get_db)):
    job = db.query(Job).filter(Job.id == job_id).first()

    if not job:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Job not found")

    if "status" in data:
        job.status = data["status"]

    if "logs" in data:
        job.logs = data["logs"]

    if "error" in data:
        job.error = data["error"]

    db.commit()

    return {"message": "updated"}
