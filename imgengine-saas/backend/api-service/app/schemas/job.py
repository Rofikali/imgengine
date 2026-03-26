# backend/schemas/job.py

from pydantic import BaseModel, Field


class GenerateJob(BaseModel):
    input: str

    output: str = "output.png"

    cols: int = 6
    rows: int = 6
    gap: int = 15
    dpi: int = 300
    border: int = 2
    padding: int = 20

    crop_mark: int = 15
    crop_thickness: int = 2
    bleed: int = 0
    crop_offset: int = 8

    width: float = Field(4.5, alias="w_cm")
    height: float = Field(3.5, alias="h_cm")
