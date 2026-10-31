# Build the native C library and the Rust/Axum API against Ubuntu 24.04.
FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive \
    CARGO_TARGET_DIR=/build/cargo-target \
    RUSTFLAGS="-L native=/opt/imgengine/lib" \
    LD_LIBRARY_PATH=/opt/imgengine/lib

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        cargo \
        cmake \
        libnuma-dev \
        libturbojpeg0-dev \
        liburing-dev \
        nasm \
        ninja-build \
        python3 \
        rustc \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . ./imgengine

RUN cmake -S /src/imgengine -B /build/native -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_LIBDIR=lib \
        -DIMGENGINE_LTO=OFF \
        -DIMGENGINE_BENCH=OFF \
        -DIMGENGINE_PORTABLE_BASELINE=ON \
        -DIMGENGINE_ENABLE_DSL_CODEGEN=OFF \
        -DBUILD_TESTING=OFF \
    && cmake --build /build/native --target imgengine --parallel 2 \
    && mkdir -p /opt/imgengine/lib \
    && cp -a /build/native/libimgengine.so* /opt/imgengine/lib/ \
    && ldd /opt/imgengine/lib/libimgengine.so \
    && ! ldd /opt/imgengine/lib/libimgengine.so | grep -q "not found"

RUN cargo build \
        --manifest-path /src/imgengine/rust/imgengine-api/Cargo.toml \
        --locked \
        --release \
        --bin imgengine-api-server

# Runtime contains only the native runtime libraries and the API executable.
FROM ubuntu:24.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive \
    IMGENGINE_API_BIND=0.0.0.0:8000 \
    IMGENGINE_WORKSPACE_ROOT=/tmp/imgengine-workspaces \
    LD_LIBRARY_PATH=/opt/imgengine/lib

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        ca-certificates \
        curl \
        libnuma1 \
        libturbojpeg0 \
        liburing2 \
    && rm -rf /var/lib/apt/lists/* \
    && groupadd --system --gid 10001 imgengine \
    && useradd --system --uid 10001 --gid 10001 --home-dir /nonexistent \
        --shell /usr/sbin/nologin imgengine \
    && mkdir -p /opt/imgengine/lib /tmp/imgengine-workspaces \
    && chown -R 10001:10001 /tmp/imgengine-workspaces

COPY --from=builder /opt/imgengine/lib/ /opt/imgengine/lib/
COPY --from=builder /build/cargo-target/release/imgengine-api-server /usr/local/bin/imgengine-api-server

RUN ldd /usr/local/bin/imgengine-api-server \
    && ldd /opt/imgengine/lib/libimgengine.so \
    && ! ldd /usr/local/bin/imgengine-api-server | grep -q "not found" \
    && ! ldd /opt/imgengine/lib/libimgengine.so | grep -q "not found"

WORKDIR /app
USER 10001:10001

EXPOSE 8000
HEALTHCHECK --interval=15s --timeout=3s --start-period=10s --retries=3 \
    CMD curl --fail --silent http://127.0.0.1:8000/readyz || exit 1

ENTRYPOINT ["/usr/local/bin/imgengine-api-server"]
