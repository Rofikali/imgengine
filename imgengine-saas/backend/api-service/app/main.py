# backend/app/main.py


from fastapi import FastAPI
from app.api.routes.generate import router as generate_router
from app.core.db import Base, engine

from app.core.limiter import limiter
from app.api.routes.internal import router as internal_router
from prometheus_client import generate_latest
from fastapi import Response
from opentelemetry.instrumentation.fastapi import FastAPIInstrumentor
# from app.core.tracing import tracer


Base.metadata.create_all(bind=engine)

app = FastAPI()
FastAPIInstrumentor.instrument_app(app)


app.include_router(generate_router, prefix="/api")
app.include_router(internal_router, prefix="/internal")


@app.get("/metrics")
def metrics():
    return Response(generate_latest(), media_type="text/plain")


app.state.limiter = limiter
