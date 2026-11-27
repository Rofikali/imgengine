# Free frontend-only Docker environment — proposal

Status: **IMPLEMENTED LOCALLY / NOT INTEGRATED** at `99ca654ac72c121d31a5956888dfe6e6ea76ef5a`. `infra/Dockerfile.free-frontend-dev` and `infra/docker-compose.free-frontend-dev.yml` implement the bounded design below. They are not a CI change, release artifact, or permission to retire the existing P7 Compose stack.

## Intent and boundary

For an isolated IMGENGINE Free browser-local change, `infra/Dockerfile.free-frontend-dev` uses Ubuntu 24.04 with Node 24 and Corepack/pnpm only. `infra/docker-compose.free-frontend-dev.yml` exposes one `free-frontend` service with the repository bind mount, a named Linux `node_modules` volume and a Corepack cache volume. It has no `depends_on`, Rust API, C/C++ compiler, Cargo, database, external hostname, Docker socket, privileged mode, real credential, or published port.

P7 remains the approved integrated development environment. The proposed service is not a substitute for P7 when shared runtime/API/native work is in scope. Browser runs would use the CI-pinned `mcr.microsoft.com/playwright:v1.58.2-noble` image separately; the Free development image must not download browsers or claim browser coverage itself.

## Local command contract

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

The final browser command is **NOT RUN** unless the pinned Playwright image is locally available and executes it. Unit, typecheck, audit, static-output, browser, accessibility, privacy, no-upload, and release-guard results remain separate evidence.

`verify:free-publication-ready` is currently a compiled-candidate integrity check: notices, static boundaries, CSP, secret patterns, and the bundled licence report. Its successful exit does **not** verify detached owner/legal/security approval, a source freeze, artifact provenance, Cloudflare project configuration, billing, deployment, or release authority. Those manifest fields remain null at this checkpoint; Free v1.0.0 is not released and remains blocked outside this local environment milestone.

## Review and acceptance before integration

1. Compare the proposed image/packages and Compose isolation to P7; preserve P7 unchanged.
2. Confirm actual Ubuntu, Node, and pinned pnpm versions from the container.
3. Verify no backend process, database, native compiler, host port, secret, or paid provider is introduced.
4. Execute the scoped frontend checks in the container and four-profile static browser suite in the pinned browser image.
5. Review L1–L7 implementation/operability concerns plus CISSP privacy, CCIE loopback/no-upload, Linux/SRE volume/image lifecycle, CA spend boundary, and MBA/management scope/rollback. Obtain normal review and owner authorization before any remote integration.

## Local implementation evidence

At local branch commit pending after `99ca654`, Docker Desktop Linux engine `28.0.4` built the service image successfully. The container reported Ubuntu `24.04.5`, Node `v24.22.3`, and pnpm `10.34.9`. It passed a frozen install, 115 unit tests, typecheck, a production audit with zero critical/high/moderate/low findings, the Free static build, and static-output verification (49 files; tree SHA-256 `ec964aafd3972b02dc9d03ad987db1ab8c53bef37652d40c9e95fa6a9e8a96e7`). The isolation probe confirmed no `gcc`, `cargo`, `rustc`, `cmake`, `playwright`, or Docker socket in the container.

The CI-pinned Playwright image was unavailable after its Docker pull stalled, so local browser, accessibility, privacy, and no-upload matrix evidence is **NOT RUN**. No Windows fallback was used. The generated local `frontend/dist` link is intentionally untracked and not part of this change.
