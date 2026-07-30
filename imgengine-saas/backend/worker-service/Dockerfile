FROM debian:bookworm-slim AS engine-builder

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    libnuma-dev \
    libturbojpeg0-dev \
    liburing-dev \
    ninja-build \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src/imgengine
COPY imgengine/ ./

RUN cmake -S . -B build -G Ninja \
      -DCMAKE_BUILD_TYPE=Release \
      -DIMGENGINE_BENCH=OFF \
      -DIMGENGINE_ENABLE_DSL_CODEGEN=OFF \
      -DIMGENGINE_LTO=OFF \
    && cmake --build build --parallel \
    && ctest --test-dir build --output-on-failure \
    && build/imgengine_cli --help

FROM python:3.12-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
    libnuma1 \
    libturbojpeg0 \
    liburing2 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY imgengine-saas/backend/api-service/pyproject.toml \
     imgengine-saas/backend/api-service/uv.lock ./
RUN pip install --no-cache-dir uv \
    && uv sync --frozen --no-install-project

COPY imgengine-saas/backend/api-service/app ./app
COPY imgengine-saas/backend/worker-service/tasks.py ./tasks.py
COPY imgengine-saas/backend/worker-service/engine_runner.py ./engine_runner.py
COPY --from=engine-builder /src/imgengine/build/imgengine_cli /usr/local/bin/imgengine_cli
COPY --from=engine-builder /src/imgengine/build/libimgengine.so /usr/local/lib/libimgengine.so

RUN ldconfig \
    && imgengine_cli --help

CMD ["uv", "run", "celery", "-A", "tasks", "worker", "--loglevel=info"]
