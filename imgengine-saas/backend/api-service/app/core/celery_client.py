import socket
from urllib.parse import urlparse

from celery import Celery

from app.core.config import CELERY_BROKER_URL, CELERY_RESULT_BACKEND

celery = Celery("api", broker=CELERY_BROKER_URL, backend=CELERY_RESULT_BACKEND)
celery.conf.task_publish_retry = False
celery.conf.broker_connection_timeout = 2
celery.conf.broker_transport_options = {
    "socket_connect_timeout": 2,
    "socket_timeout": 2,
    "retry_on_timeout": False,
}


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
