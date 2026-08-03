# backend/app/main.py


from contextlib import asynccontextmanager
from fastapi import FastAPI, Response
from fastapi.middleware.cors import CORSMiddleware
from app.api.routes.generate import router as generate_router
from app.core.db import Base, engine

from app.core.limiter import limiter
from app.api.routes.internal import router as internal_router
from prometheus_client import generate_latest
from opentelemetry.instrumentation.fastapi import FastAPIInstrumentor
from app.core.config import CORS_ORIGINS
from app.core.storage import artifact_store


@asynccontextmanager
async def lifespan(_: FastAPI):
    artifact_store.ensure_directories()
    Base.metadata.create_all(bind=engine)
    yield

app = FastAPI(title="ImgEngine API", version="0.1.0", lifespan=lifespan)
app.add_middleware(
    CORSMiddleware,
    allow_origins=CORS_ORIGINS,
    allow_credentials=False,
    allow_methods=["GET", "POST"],
    allow_headers=["Content-Type", "X-API-Key"],
)
FastAPIInstrumentor.instrument_app(app)


app.include_router(generate_router, prefix="/api")
app.include_router(internal_router, prefix="/internal")


@app.get("/healthz", include_in_schema=False)
def healthz():
    return {"status": "ok"}


@app.get("/metrics")
def metrics():
    return Response(generate_latest(), media_type="text/plain")


app.state.limiter = limiter
