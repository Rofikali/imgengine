# backend/app/main.py


from fastapi import FastAPI

from app.api.routes.generate import router as generate_router
from app.core.db import Base, engine

from app.core.limiter import limiter
from app.api.routes.internal import router as internal_router


Base.metadata.create_all(bind=engine)

app = FastAPI()


app.include_router(generate_router, prefix="/api")
app.include_router(internal_router, prefix="/internal")

app.state.limiter = limiter
