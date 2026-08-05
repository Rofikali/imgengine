import json
import logging
import sys
from datetime import UTC, datetime
from logging.handlers import RotatingFileHandler
from pathlib import Path
from typing import Any

from app.core.config import LOG_DIR, LOG_FILE_BACKUP_COUNT, LOG_FILE_MAX_BYTES, LOG_LEVEL, SERVICE_NAME


class JsonFormatter(logging.Formatter):
    def format(self, record: logging.LogRecord) -> str:
        payload: dict[str, Any] = {
            "timestamp": datetime.now(UTC).isoformat(),
            "level": record.levelname.lower(),
            "service": SERVICE_NAME,
            "event": getattr(record, "event", record.getMessage()),
            "message": record.getMessage(),
        }
        for field in ("component", "trace_id", "job_id", "request_id", "duration_ms", "status_code"):
            value = getattr(record, field, None)
            if value is not None:
                payload[field] = value
        if record.exc_info:
            payload["exception"] = self.formatException(record.exc_info)
        return json.dumps(payload, ensure_ascii=False, default=str)


logger = logging.getLogger("imgengine")


def configure_logging() -> None:
    if logger.handlers:
        return
    Path(LOG_DIR).mkdir(parents=True, exist_ok=True)
    formatter = JsonFormatter()
    stdout = logging.StreamHandler(sys.stdout)
    stdout.setFormatter(formatter)
    file_handler = RotatingFileHandler(
        Path(LOG_DIR) / f"{Path(SERVICE_NAME).name or 'imgengine'}.log",
        maxBytes=LOG_FILE_MAX_BYTES,
        backupCount=LOG_FILE_BACKUP_COUNT,
        encoding="utf-8",
    )
    file_handler.setFormatter(formatter)
    logger.setLevel(LOG_LEVEL)
    logger.propagate = False
    logger.addHandler(stdout)
    logger.addHandler(file_handler)


def log_event(level: int, event: str, message: str | None = None, **context: Any) -> None:
    configure_logging()
    logger.log(level, message or event, extra={"event": event, **context})
