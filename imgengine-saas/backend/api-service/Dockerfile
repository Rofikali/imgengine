# api-service/Dockerfile

FROM python:3.12-slim

ENV PIP_DISABLE_PIP_VERSION_CHECK=1 \
    PIP_ROOT_USER_ACTION=ignore \
    UV_HTTP_RETRIES=5 \
    UV_HTTP_TIMEOUT=300

WORKDIR /app

COPY pyproject.toml uv.lock ./

RUN pip install --no-cache-dir --retries 5 --timeout 300 uv \
    && uv sync --frozen --no-install-project

COPY . .

CMD ["/bin/sh", "-c", "/app/.venv/bin/alembic upgrade head && exec /app/.venv/bin/uvicorn app.main:app --host 0.0.0.0 --port 8000"]
