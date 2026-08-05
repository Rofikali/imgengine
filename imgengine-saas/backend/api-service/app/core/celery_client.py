import socket
from urllib.parse import urlparse

from celery import Celery

from app.core.config import (
    CELERY_BROKER_URL,
    CELERY_RESULT_BACKEND,
    CELERY_RESULT_EXPIRES_SECONDS,
    CELERY_VISIBILITY_TIMEOUT_SECONDS,
)

celery = Celery("api", broker=CELERY_BROKER_URL, backend=CELERY_RESULT_BACKEND)
celery.conf.update(
    task_serializer="json",
    result_serializer="json",
    accept_content=["json"],
    result_expires=CELERY_RESULT_EXPIRES_SECONDS,
    task_publish_retry=True,
    task_publish_retry_policy={
        "max_retries": 3,
        "interval_start": 0.2,
        "interval_step": 0.2,
        "interval_max": 1.0,
    },
    broker_connection_timeout=2,
    broker_connection_retry_on_startup=True,
    broker_transport_options={
        "socket_connect_timeout": 2,
        "socket_timeout": 2,
        "retry_on_timeout": True,
        "visibility_timeout": CELERY_VISIBILITY_TIMEOUT_SECONDS,
    },
)


def assert_broker_available(timeout_seconds: float = 2) -> None:
    broker = urlparse(CELERY_BROKER_URL)
    if broker.scheme not in {"redis", "rediss"}:
        return

    if not broker.hostname:
        raise RuntimeError("Celery broker URL does not include a hostname")

    with socket.create_connection(
        (broker.hostname, broker.port or 6379), timeout=timeout_seconds
    ):
        pass
