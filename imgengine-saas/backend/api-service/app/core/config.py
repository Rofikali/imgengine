# backend/core/config.py 

import os


DATABASE_URL = os.getenv(
    "DATABASE_URL", "postgresql://imgengine:imgengine@db:5432/imgengine"
)
API_KEYS = frozenset(filter(None, os.getenv("API_KEYS", "test-key-123").split(",")))
INTERNAL_API_TOKEN = os.getenv("INTERNAL_API_TOKEN", "local-development-token")
MAX_UPLOAD_BYTES = int(os.getenv("MAX_UPLOAD_BYTES", str(20 * 1024 * 1024)))
STORAGE_ROOT = os.getenv("STORAGE_ROOT", "/data")
STORAGE_BACKEND = os.getenv("STORAGE_BACKEND", "local")
S3_ENDPOINT_URL = os.getenv("S3_ENDPOINT_URL")
S3_ACCESS_KEY_ID = os.getenv("S3_ACCESS_KEY_ID")
S3_SECRET_ACCESS_KEY = os.getenv("S3_SECRET_ACCESS_KEY")
S3_BUCKET = os.getenv("S3_BUCKET", "imgengine")
S3_REGION = os.getenv("S3_REGION", "us-east-1")
S3_PRESIGN_TTL_SECONDS = int(os.getenv("S3_PRESIGN_TTL_SECONDS", "300"))
UPLOAD_PREFIX = "uploads"
OUTPUT_PREFIX = "outputs"
LOG_DIR = os.getenv("LOG_DIR", "/data/logs")
CELERY_BROKER_URL = os.getenv("CELERY_BROKER_URL", "redis://redis:6379/0")
CELERY_RESULT_BACKEND = os.getenv("CELERY_RESULT_BACKEND", CELERY_BROKER_URL)
JOB_RETENTION_HOURS = int(os.getenv("JOB_RETENTION_HOURS", "24"))
RETENTION_CLEANUP_INTERVAL_SECONDS = int(os.getenv("RETENTION_CLEANUP_INTERVAL_SECONDS", "3600"))
RETENTION_CLEANUP_BATCH_SIZE = int(os.getenv("RETENTION_CLEANUP_BATCH_SIZE", "100"))
CORS_ORIGINS = [
    origin.strip()
    for origin in os.getenv("CORS_ORIGINS", "http://localhost:3000").split(",")
    if origin.strip()
]
