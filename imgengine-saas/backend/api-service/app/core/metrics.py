# api-service/app/core/metrics.py

from prometheus_client import Counter, Histogram

IMAGE_PROCESSED_TOTAL = Counter(
    "imgengine_images_processed_total",
    "Total number of images processed by the C-engine",
)

REQUEST_COUNT = Counter(
    "http_requests_total", "Total HTTP Requests", ["method", "endpoint"]
)

REQUEST_LATENCY = Histogram(
    "http_request_duration_seconds", "HTTP Request Latency", ["endpoint"]
)

JOB_TRANSITIONS = Counter(
    "imgengine_job_transitions_total",
    "Job lifecycle transitions accepted by the API",
    ["from_status", "to_status"],
)

QUEUE_PUBLICATION_FAILURES = Counter(
    "imgengine_queue_publication_failures_total",
    "Jobs that could not be published to the processing queue",
)

UPLOAD_BYTES = Histogram(
    "imgengine_upload_bytes",
    "Accepted upload sizes in bytes",
)
