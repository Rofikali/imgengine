from enum import StrEnum


class JobStatus(StrEnum):
    QUEUED = "queued"
    PROCESSING = "processing"
    RETRYING = "retrying"
    COMPLETED = "completed"
    FAILED = "failed"


ALLOWED_TRANSITIONS: dict[JobStatus, set[JobStatus]] = {
    JobStatus.QUEUED: {JobStatus.PROCESSING, JobStatus.FAILED},
    JobStatus.PROCESSING: {JobStatus.RETRYING, JobStatus.COMPLETED, JobStatus.FAILED},
    JobStatus.RETRYING: {JobStatus.PROCESSING, JobStatus.FAILED},
    JobStatus.COMPLETED: set(),
    JobStatus.FAILED: set(),
}


def can_transition(current: str, target: JobStatus) -> bool:
    try:
        current_status = JobStatus(current)
    except ValueError:
        return False
    return current_status == target or target in ALLOWED_TRANSITIONS[current_status]
