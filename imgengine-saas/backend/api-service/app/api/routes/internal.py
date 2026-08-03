# api-service/app/api/routes/internal.py

from fastapi import APIRouter, Depends, HTTPException, status
from sqlalchemy.orm import Session
from app.core.db import SessionLocal
from app.models.job import Job
from app.core.security import verify_internal_token
from app.core.job_states import can_transition
from app.schemas.job import JobStatusUpdate
from app.core.metrics import JOB_TRANSITIONS

router = APIRouter()


def get_db():
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()


@router.patch("/jobs/{job_id}", dependencies=[Depends(verify_internal_token)])
def update_job(job_id: str, data: JobStatusUpdate, db: Session = Depends(get_db)):
    job = db.query(Job).filter(Job.id == job_id).first()

    if not job:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Job not found")

    if not can_transition(job.status, data.status):
        raise HTTPException(
            status_code=status.HTTP_409_CONFLICT,
            detail=f"Cannot transition job from {job.status} to {data.status}",
        )

    previous_status = job.status
    job.status = data.status

    if data.logs is not None:
        job.logs = data.logs

    if data.error is not None:
        job.error = data.error

    db.commit()
    if previous_status != job.status:
        JOB_TRANSITIONS.labels(from_status=previous_status, to_status=job.status).inc()

    return {"job_id": job.id, "status": job.status}
