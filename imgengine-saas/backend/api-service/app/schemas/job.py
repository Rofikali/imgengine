# backend/schemas/job.py

from pydantic import BaseModel, Field

from app.core.job_states import JobStatus


class GenerateJob(BaseModel):
    input: str

    output: str = "output.png"

    cols: int = Field(6, ge=1, le=20)
    rows: int = Field(6, ge=1, le=20)
    gap: int = Field(15, ge=0, le=500)
    dpi: int = Field(300, ge=72, le=1200)
    border: int = Field(2, ge=0, le=100)
    padding: int = Field(20, ge=0, le=1000)

    crop_mark: int = Field(15, ge=0, le=500)
    crop_thickness: int = Field(2, ge=1, le=100)
    bleed: int = Field(0, ge=0, le=500)
    crop_offset: int = Field(8, ge=0, le=500)

    width: float = Field(4.5, gt=0, le=50)
    height: float = Field(3.5, gt=0, le=50)


class JobStatusUpdate(BaseModel):
    status: JobStatus
    error: str | None = Field(default=None, max_length=10_000)
    logs: str | None = Field(default=None, max_length=100_000)
