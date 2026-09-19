# backend/app/main.py


from contextlib import asynccontextmanager
import logging
import time
import uuid

from fastapi import FastAPI, HTTPException, Request, Response, status
from fastapi.middleware.cors import CORSMiddleware
from app.api.routes.generate import router as generate_router

from app.core.limiter import limiter
from slowapi import _rate_limit_exceeded_handler
from slowapi.errors import RateLimitExceeded
from app.api.routes.internal import router as internal_router
from prometheus_client import generate_latest
from opentelemetry.instrumentation.fastapi import FastAPIInstrumentor
from app.core.config import CORS_ORIGINS, validate_runtime_configuration
from app.core.storage import artifact_store
from app.core.celery_client import assert_broker_available
from app.core.db import engine
from sqlalchemy import text
from app.core.logger import configure_logging, log_event


@asynccontextmanager
async def lifespan(_: FastAPI):
    configure_logging()
    validate_runtime_configuration()
    artifact_store.ensure_directories()
    log_event(logging.INFO, "service_started", component="api")
    yield
    log_event(logging.INFO, "service_stopped", component="api")

app = FastAPI(title="ImgEngine API", version="0.1.0", lifespan=lifespan)
app.add_middleware(
    CORSMiddleware,
    allow_origins=CORS_ORIGINS,
    allow_credentials=False,
    allow_methods=["GET", "POST"],
    allow_headers=["Content-Type", "Idempotency-Key", "X-API-Key"],
)
FastAPIInstrumentor.instrument_app(app)


@app.middleware("http")
async def request_observability(request: Request, call_next):
    request_id = request.headers.get("X-Request-ID") or str(uuid.uuid4())
    started_at = time.perf_counter()
    try:
        response = await call_next(request)
    except Exception:
        log_event(
            logging.ERROR,
            "http_request_failed",
            component="api",
            request_id=request_id,
            duration_ms=round((time.perf_counter() - started_at) * 1000, 2),
            message=f"{request.method} {request.url.path} failed",
        )
        raise
    response.headers["X-Request-ID"] = request_id
    log_event(
        logging.INFO if response.status_code < 500 else logging.ERROR,
        "http_request_completed",
        component="api",
        request_id=request_id,
        status_code=response.status_code,
        duration_ms=round((time.perf_counter() - started_at) * 1000, 2),
        message=f"{request.method} {request.url.path} completed",
    )
    return response


app.include_router(generate_router, prefix="/api")
app.include_router(internal_router, prefix="/internal")


@app.get("/healthz", include_in_schema=False)
def healthz():
    return {"status": "ok"}


@app.get("/readyz", include_in_schema=False)
def readyz():
    components: dict[str, str] = {}
    try:
        with engine.connect() as connection:
            connection.execute(text("SELECT 1"))
        components["database"] = "ok"
    except Exception:
        components["database"] = "unavailable"
    try:
        assert_broker_available()
        components["broker"] = "ok"
    except Exception:
        components["broker"] = "unavailable"
    try:
        artifact_store.is_ready()
        components["storage"] = "ok"
    except Exception:
        components["storage"] = "unavailable"
    if all(value == "ok" for value in components.values()):
        return {"status": "ready", "components": components}
    raise HTTPException(status_code=status.HTTP_503_SERVICE_UNAVAILABLE, detail=components)


@app.get("/metrics")
def metrics():
    return Response(generate_latest(), media_type="text/plain")


app.state.limiter = limiter
app.add_exception_handler(RateLimitExceeded, _rate_limit_exceeded_handler)
