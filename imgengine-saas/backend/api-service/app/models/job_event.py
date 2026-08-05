from datetime import datetime

from sqlalchemy import Column, DateTime, Integer, String, Text

from app.core.db import Base


class JobEvent(Base):
    __tablename__ = "job_events"

    id = Column(Integer, primary_key=True)
    job_id = Column(String, nullable=False, index=True)
    trace_id = Column(String, nullable=True, index=True)
    event = Column(String, nullable=False)
    component = Column(String, nullable=False)
    level = Column(String, nullable=False, default="info")
    message = Column(Text, nullable=False)
    details = Column(Text, nullable=True)
    created_at = Column(DateTime, default=datetime.utcnow, nullable=False, index=True)
