# P12 Rust API container and frontend routing

## What is deployed in this slice

The Compose stack now includes a separate `rust-api` service built from
`imgengine/rust/Dockerfile`. It serves the verified synchronous Rust/Axum
capabilities:

- `POST /api/v1/render` — P5 single-image render
- `POST /api/v1/layout-render` — P8 FIT-only layout
- `POST /api/v2/sheet-render` — P12 physical sheet with centered crop-to-fill
- `GET /healthz` — process liveness
- `GET /readyz` — admission readiness

The existing Python API, worker, cleanup, Redis, and PostgreSQL services remain
in Compose for legacy job flows. They are not dependencies of the Rust API
container. This is a side-by-side migration, not a deletion or replacement of
legacy services.

The Nuxt server proxies the P5, P8, and P12 browser requests to
`http://rust-api:8000` using the private runtime overrides
`NUXT_IMGENGINE_API_BASE` and `NUXT_IMGENGINE_RENDER_API_KEY`. The browser never receives the backend API key.

The P12 UI aborts its browser-side fetch when the sheet panel is reset or
unmounted and ignores stale responses. This is response abandonment, not a
guarantee that already-admitted native C work stops mid-operation; the current
ABI does not provide mid-operation native cancellation.

## Container properties

The Rust API image is built on Ubuntu 24.04 and contains:

- the portable-baseline C shared library,
- the Rust API server binary,
- only the runtime libraries required by the C library.

The Compose service:

- runs as UID/GID 10001 (non-root),
- has a read-only root filesystem,
- uses an ephemeral `/tmp` tmpfs for workspaces,
- drops Linux capabilities and enables `no-new-privileges`,
- exposes port 8001 only on host loopback,
- uses `/readyz` for container health,
- has no database, Redis, object-store, or worker dependency.

## Configuration

Required local variables:

- `API_KEYS` — server-side key set accepted by Rust and the legacy API
- `WEB_API_KEY` — one key from `API_KEYS`, used only by the Nuxt server proxy
- `INTERNAL_API_TOKEN` — retained for the legacy Python worker stack

Optional Rust output destination budgets:

- `IMGENGINE_LAYOUT_MAX_OUTPUT_BYTES` — default 16,777,216 bytes (P8)
- `IMGENGINE_SHEET_MAX_OUTPUT_BYTES` — default 67,108,864 bytes (P12)

The P12 64 MiB value limits the conservative encoded-JPEG destination allocation.
It is not a complete per-request RSS limit or a concurrency capacity claim.

## Local verification

From the repository root:

```bash
cp imgengine-saas/infra/.env.example imgengine-saas/infra/.env
# Replace all local example secrets before exposing any service beyond loopback.
cd imgengine-saas/infra
docker compose config
docker compose build rust-api
docker compose up -d rust-api
cd ../..
API_URL=http://127.0.0.1:8001 \
API_KEY=local-test-key \
bash imgengine-saas/scripts/rust_api_smoke.sh imgengine/photo.jpg
```

The smoke script checks liveness/readiness and sends real HTTP requests through
P5, P8, and P12. The P12 check renders A4 at 300 DPI and verifies the server's
canvas dimensions and exact copy accounting in the response headers and JPEG.

The Nuxt app can then be started with the Compose stack:

```bash
cd imgengine-saas/infra
docker compose up --build -d
```

Open `http://localhost:3000` and select **Physical sheet**.

## Remaining production gates

This is not a production release approval. Before canary or public exposure:

1. Measure peak RSS for representative inputs and A4/10 MP outputs in the final
   runtime container.
2. Measure concurrency and queue behavior under load; do not infer safe
   concurrency from the 64 MiB output allocation cap.
3. Set an evidence-based container memory/CPU limit and admission capacity.
4. Verify deployment health, graceful shutdown, restart, and rollback.
5. Run a controlled canary while retaining the legacy route as the rollback path.
6. Decide separately when the Python/Redis/PostgreSQL legacy stack can be
   retired; this PR intentionally does not remove it.
