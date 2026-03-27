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
