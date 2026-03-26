# backend/models/job.py


from sqlalchemy import Column, String, DateTime, Text
from datetime import datetime
from app.core.db import Base


class Job(Base):
    __tablename__ = "jobs"

    id = Column(String, primary_key=True, index=True)

    input = Column(String)
    output = Column(String)

    status = Column(String, default="queued")

    error = Column(Text, nullable=True)  # 🔥 NEW
    logs = Column(Text, nullable=True)  # 🔥 NEW

    created_at = Column(DateTime, default=datetime.utcnow)
    updated_at = Column(DateTime, default=datetime.utcnow)
