# backend/models/job.py


from sqlalchemy import Column, String, DateTime, Text
from datetime import datetime
from app.core.db import Base


class Job(Base):
    __tablename__ = "jobs"

    id = Column(String, primary_key=True, index=True)
    trace_id = Column(String, nullable=True, index=True)
    owner_key_hash = Column(String, nullable=True, index=True)
    idempotency_key = Column(String, nullable=True)

    input = Column(String)
    output = Column(String)

    status = Column(String, default="queued")

    error = Column(Text, nullable=True)  # 🔥 NEW
    logs = Column(Text, nullable=True)  # 🔥 NEW

    created_at = Column(DateTime, default=datetime.utcnow)
    updated_at = Column(DateTime, default=datetime.utcnow, onupdate=datetime.utcnow)
    expires_at = Column(DateTime, nullable=True, index=True)
