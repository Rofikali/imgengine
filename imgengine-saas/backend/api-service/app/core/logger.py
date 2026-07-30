# import logging

# logging.basicConfig(
#     level=logging.INFO,
#     format='{"time":"%(asctime)s","level":"%(levelname)s","msg":"%(message)s"}',
#     stream=sys.stdout,
# )

# logger = logging.getLogger("imgengine")


import logging
import json
import sys
from pathlib import Path

from app.core.config import LOG_DIR

logging.basicConfig(
    level=logging.INFO,
    format='{"time":"%(asctime)s","level":"%(levelname)s","msg":"%(message)s"}',
    stream=sys.stdout,
)


class JsonFormatter(logging.Formatter):
    def format(self, record):
        log_record = {
            "time": self.formatTime(record),
            "level": record.levelname,
            "message": record.getMessage(),
        }

        if hasattr(record, "trace_id"):
            log_record["trace_id"] = record.trace_id

        return json.dumps(log_record)


logger = logging.getLogger("imgengine")
Path(LOG_DIR).mkdir(parents=True, exist_ok=True)
handler = logging.FileHandler(Path(LOG_DIR) / "app.log")

handler.setFormatter(JsonFormatter())

logger.addHandler(handler)
logger.setLevel(logging.INFO)
