# Free frontend-only Docker environment — proposal

Status: **PROPOSAL ONLY** at `99ca654ac72c121d31a5956888dfe6e6ea76ef5a`. It is not an approved environment, CI change, release artifact, or permission to retire the existing P7 Compose stack.

## Intent and boundary

For an isolated IMGENGINE Free browser-local change, the proposed `infra/Dockerfile.free-frontend-dev` would use Ubuntu 24.04 with Node 24 and Corepack/pnpm only. The proposed `infra/docker-compose.free-frontend-dev.yml` would expose one `free-frontend` service with the repository bind mount and a named Linux `node_modules` volume. It would have no `depends_on`, Rust API, C/C++ compiler, Cargo, database, external hostname, Docker socket, privileged mode, real credential, or published port by default.

P7 remains the approved integrated development environment. The proposed service is not a substitute for P7 when shared runtime/API/native work is in scope. Browser runs would use the CI-pinned `mcr.microsoft.com/playwright:v1.58.2-noble` image separately; the Free development image must not download browsers or claim browser coverage itself.

## Proposed local command contract

From `imgengine-saas/frontend` inside the approved Ubuntu container, after Docker Desktop reports a Linux engine and `/etc/os-release` reports Ubuntu 24.04:

```bash
node --version
corepack enable
pnpm --version
pnpm install --frozen-lockfile
pnpm exec nuxi prepare
pnpm test
pnpm run typecheck
pnpm run verify:free-release-dependencies
pnpm run build:free-static
pnpm run verify:free-static
pnpm run test:browser:free-static
```

The final browser command is **NOT RUN** unless the pinned Playwright image is locally available and executes it. Unit, typecheck, audit, static-output, browser, accessibility, privacy, no-upload, and release-guard results remain separate evidence. The publication guard is expected to fail until external release gates are met.

## Review and acceptance before implementation

1. Compare the proposed image/packages and Compose isolation to P7; preserve P7 unchanged.
2. Confirm actual Ubuntu, Node, and pinned pnpm versions from the container.
3. Verify no backend process, database, native compiler, host port, secret, or paid provider is introduced.
4. Execute the scoped frontend checks in the container and four-profile static browser suite in the pinned browser image.
5. Review L1–L7 implementation/operability concerns plus CISSP privacy, CCIE loopback/no-upload, Linux/SRE volume/image lifecycle, CA spend boundary, and MBA/management scope/rollback. Obtain normal review and owner authorization before any remote integration.
