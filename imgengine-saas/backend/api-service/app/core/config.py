# backend/core/config.py 

import os


DATABASE_URL = os.getenv(
    "DATABASE_URL", "postgresql://imgengine:imgengine@db:5432/imgengine"
)
API_KEYS = frozenset(filter(None, os.getenv("API_KEYS", "test-key-123").split(",")))
INTERNAL_API_TOKEN = os.getenv("INTERNAL_API_TOKEN", "local-development-token")
MAX_UPLOAD_BYTES = int(os.getenv("MAX_UPLOAD_BYTES", str(20 * 1024 * 1024)))
UPLOAD_DIR = os.getenv("UPLOAD_DIR", "/data/uploads")
OUTPUT_DIR = os.getenv("OUTPUT_DIR", "/data/outputs")
LOG_DIR = os.getenv("LOG_DIR", "/data/logs")
CELERY_BROKER_URL = os.getenv("CELERY_BROKER_URL", "redis://redis:6379/0")
CELERY_RESULT_BACKEND = os.getenv("CELERY_RESULT_BACKEND", CELERY_BROKER_URL)
CORS_ORIGINS = [
    origin.strip()
    for origin in os.getenv("CORS_ORIGINS", "http://localhost:3000").split(",")
    if origin.strip()
]
