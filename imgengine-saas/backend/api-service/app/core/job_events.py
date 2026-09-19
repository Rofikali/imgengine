import json
from typing import Any

from sqlalchemy.orm import Session

from app.models.job import Job
from app.models.job_event import JobEvent

MAX_EVENT_MESSAGE_CHARS = 2_000
MAX_EVENT_DETAILS_CHARS = 4_000


def record_job_event(
    db: Session,
    job: Job,
    *,
    event: str,
    component: str,
    message: str,
    level: str = "info",
    details: dict[str, Any] | None = None,
) -> None:
    serialized_details = json.dumps(details or {}, separators=(",", ":"), default=str)
    db.add(
        JobEvent(
            job_id=job.id,
            trace_id=job.trace_id,
            event=event[:128],
            component=component[:64],
            level=level[:16],
            message=message[:MAX_EVENT_MESSAGE_CHARS],
            details=serialized_details[:MAX_EVENT_DETAILS_CHARS],
        )
    )
