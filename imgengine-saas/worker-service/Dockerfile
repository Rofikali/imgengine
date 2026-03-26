# # worker-service/Dockerfile

# FROM python:3.11-slim

# WORKDIR /app

# # install uv
# RUN pip install uv

# # copy SAME backend (important)
# COPY ../api-service /app

# # install deps
# RUN uv sync --frozen

# # copy binary
# COPY imgengine_cli /usr/local/bin/imgengine_cli
# RUN chmod +x /usr/local/bin/imgengine_cli

# CMD ["uv", "run", "celery", "-A", "app.workers.tasks.celery", "worker", "--loglevel=info"]

# # FROM python:3.11-slim

# # WORKDIR /app

# # RUN pip install celery redis

# # COPY tasks.py .
# # COPY engine_runner.py .
# # COPY imgengine_cli /usr/local/bin/imgengine_cli

# # RUN chmod +x /usr/local/bin/imgengine_cli

# # CMD ["celery", "-A", "tasks", "worker", "--loglevel=info"]


FROM python:3.11-slim

WORKDIR /app

# 1. System dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    libc6 \
    && rm -rf /var/lib/apt/lists/*

# 2. Install 'uv'
RUN pip install uv

# 3. Copy API Service files (Context is '..', so we are in imgengine-saas/)
COPY api-service/ /app/

# 4. Install dependencies
RUN uv sync --frozen

# 5. Copy Worker specific files and binary
COPY worker-service/tasks.py /app/tasks.py
COPY worker-service/engine_runner.py /app/engine_runner.py
COPY worker-service/imgengine_cli /usr/local/bin/imgengine_cli
RUN chmod +x /usr/local/bin/imgengine_cli

# 6. Run Celery
# Note: Since api-service files are in /app, Celery will find 'app.core'
CMD ["uv", "run", "celery", "-A", "tasks", "worker", "--loglevel=info"]
