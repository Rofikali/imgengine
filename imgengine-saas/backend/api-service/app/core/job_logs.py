from app.core.config import MAX_JOB_LOG_CHARS
from app.core.storage import artifact_store


def sanitize_job_log(value: str | None, input_key: str, output_key: str) -> str:
    text = value or ""
    for key in (input_key, output_key):
        text = text.replace(str(artifact_store.path_for(key)), key)
    if len(text) > MAX_JOB_LOG_CHARS:
        return text[:MAX_JOB_LOG_CHARS] + "\n[log truncated]"
    return text
