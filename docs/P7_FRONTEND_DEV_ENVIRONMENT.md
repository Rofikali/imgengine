# P7.1.1 Frontend Development Environment

## Status

**IMPLEMENTED:** the P7 development stack is defined by
`infra/docker-compose.p7-frontend-dev.yml`. It is separate from the legacy
`imgengine-saas/infra/docker-compose.yml`, which remains untouched because it
models the legacy FastAPI/Celery/Redis/Postgres path.

**VERIFIED:** the evidence table records an Ubuntu 24.04 Docker Desktop run,
including both retained-cache recreation and a fresh named-volume bootstrap.
The rendered-request evidence exercises Nuxt's server proxy directly. P7.2.0
now makes that same proxy the active browser render workflow.

**DEFERRED:** this stack does not claim production deployment readiness,
production capacity, frontend migration completion, a Nuxt upgrade, or legacy
API retirement.

## Authoritative topology

```text
Windows 11 + VS Code
        |
        | bind mount (Git source of truth)
        v
Docker Desktop / Ubuntu 24.04
  browser -> http://127.0.0.1:3000 (Nuxt)
                    |
                    | Docker network: http://rust-api:8000
                    v
                Rust/Axum -> lifecycle/admission -> C scheduler/ABI v1 -> C engine
```

The browser reaches Nuxt at loopback port `3000`. The Rust service is not
published to the host or LAN. Nuxt's private server runtime supplies
`X-API-Key` when it calls the Compose service name `rust-api`; browser code
sends only the multipart `file` field to the same-origin `/api/render` route.

## Mount policy

| Location | Mount | Purpose | Persistence policy |
| --- | --- | --- | --- |
| `/workspace/imgengine` | Bind mount | Repository source for Windows VS Code and Linux tools | Git is authoritative; no source volume |
| `p7-node-modules` | Named volume | Linux-only Nuxt dependencies | Dependency cache only |
| `p7-cargo-registry`, `p7-cargo-git` | Named volumes | Cargo package/download cache | Dependency cache only |
| `p7-cargo-target`, `p7-native-build` | Named volumes | Rust and CMake build products | Rebuildable cache only |
| `/tmp/imgengine-workspaces` | `tmpfs`, mode `0700`, 64 MiB | Native request workspaces | Ephemeral; never a user-image volume |

No Docker volume stores uploads, outputs, user images, documents, or durable
application state. Named dependency/build volumes improve the Windows Docker
Desktop iteration loop but must not be considered verification inputs.

## Toolchain

The development image is explicitly `ubuntu:24.04`. It installs Node.js 24 to
match the existing frontend Dockerfile's Node 24 baseline, plus the existing
native verification prerequisites: Cargo/Rust, GCC, Clang, CMake, Ninja,
TurboJPEG development headers, NUMA/liburing headers, ImageMagick, Git, and
`pkg-config`. No Nuxt or Node upgrade is performed by this environment work.

Record the concrete versions from the running container:

```bash
docker compose --env-file infra/p7-frontend-dev.env \
  -f infra/docker-compose.p7-frontend-dev.yml exec frontend bash -lc '
  cat /etc/os-release; uname -a; node --version; npm --version;
  rustc --version; cargo --version; gcc --version; clang --version;
  cmake --version | head -1; ninja --version'
```

## Startup and verification

1. Copy `infra/p7-frontend-dev.env.example` to
   `infra/p7-frontend-dev.env` and replace its placeholder with a local,
   non-production key. The local file is ignored by Git.
2. Start the stack:

   ```bash
   docker compose --env-file infra/p7-frontend-dev.env \
     -f infra/docker-compose.p7-frontend-dev.yml up --build
   ```

3. Run the full path verifier from the repository root. On Windows PowerShell,
   use:

   ```powershell
   .\infra\scripts\verify-p7-frontend-stack.ps1
   ```

   On a Linux host, use:

   ```bash
   bash infra/scripts/verify-p7-frontend-stack.sh
   ```

It checks JPEG and PNG uploads through Nuxt to Rust/C, JPEG response decoding,
missing file (`400`), unsupported input (`415`), malformed image (`422`),
oversized input (`413`), unauthenticated direct Rust request (`401`), Nuxt's
Rust-unavailable path (`503`), and that the test key is absent from Nuxt's
client bundle. The verifier also stops Rust and proves that an oversized
multipart request is rejected by Nuxt with a safe local `413`, before upstream
admission; it verifies the documented `resource_limit` classification without
an H3 stack field. It never prints the key.

Inside the Ubuntu frontend container, the authoritative frontend commands are:

```bash
npm install
npm test
npm run typecheck
npm run build
```

`run-p7-nuxt.sh` uses the named Linux `node_modules` volume and performs
`npm ci --foreground-scripts` only when it is absent. Foreground lifecycle
scripts avoid a Docker Desktop fresh-volume `esbuild` binary-validation race.
For a fresh dependency path, remove only the P7 cache volumes and re-run the
commands; do not mount or copy a Windows `node_modules` tree into the
container:

```bash
docker compose --env-file infra/p7-frontend-dev.env \
  -f infra/docker-compose.p7-frontend-dev.yml down -v
```

## VS Code

The minimal `.devcontainer/devcontainer.json` attaches VS Code to the Compose
`frontend` service and therefore to the same Ubuntu 24.04 toolchain. Docker
Compose still requires the local ignored environment file before it can start.

## Evidence table

| Check | Result |
| --- | --- |
| Ubuntu container | PASS — Ubuntu 24.04.5 LTS on Docker Desktop/WSL2 |
| Toolchain | PASS — Node 24.21.0, npm 11.19.0, rustc/cargo 1.75.0, GCC 13.3.0, Clang 18.1.3, CMake 3.28.3, Ninja 1.11.1 |
| `npm install`, test, typecheck, build inside Ubuntu | PASS — 19 Vitest tests; Nuxt typecheck and production build passed |
| Nuxt -> Rust -> C JPEG and PNG path | PASS — the real project JPEG and a derived PNG both returned decodable JPEG responses through `/api/render` |
| Safe failure mapping and private-key boundary | PASS — 400 missing file, 415 unsupported, 422 malformed, 413 oversized, 401 direct unauthenticated Rust request, 503 unavailable Rust upstream; local test key absent from Nuxt client output |
| Container recreation with retained caches | PASS — Compose recreation preserved only the intended named dependency/build caches and the render-contract verifier passed |
| Fresh-volume dependency path | PASS — P7 volumes were removed with `down -v`; `npm ci --foreground-scripts`, native build, and both services bootstrapped successfully before the render-contract verifier passed |

## Known limitations

- The source bind mount can be slower on Windows Docker Desktop than a
  Docker-managed Linux workspace. It is intentionally retained so VS Code and
  Git work against the same checkout. Evaluate synchronized/Linux workspace
  alternatives only with measured evidence; do not move source into a named
  volume.
- The current P5/P6 body, admission, and deadline values remain provisional.
  This environment neither changes nor upgrades them.
- Docker development evidence is Linux integration evidence, not a production
  release claim.
- The active UI is intentionally limited to the current JPEG/PNG-to-JPEG
  contract. Layout, PDF, durable jobs, image history, and storage remain
  future capability-backed work.
