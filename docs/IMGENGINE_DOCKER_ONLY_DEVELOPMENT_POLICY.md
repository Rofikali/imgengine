# IMGENGINE — Docker-only development and new-session bootstrap

This standing local-development policy applies to every agent and engineer across milestones, worktrees and handoffs. Windows 11 provides the editor, Git and Docker Desktop orchestration. Ubuntu 24.04 containers provide the project runtime.

At session start, run `docker info --format '{{.OSType}}'` and require `linux`. Select the service from the checked-out Compose configuration or the image from the relevant CI job. For browser verification use the image pinned by that revision's browser workflow. Have Docker acquire the image. Inside the selected container, check `/etc/os-release` for `ID=ubuntu` and `VERSION_ID="24.04"` before installing dependencies or running the project. If acquisition or runtime verification fails, record the failed step in the task board. Keep recovery within Docker as described in section B.

## A. Every new session: read only the minimum

For **each new Codex/agent/engineering session**, after a context reset, or when moving worktrees:

1. Apply root `AGENTS.md` binding instructions (do not manually re-ingest the entire file when the agent host already supplies them). Inspect local `git status -sb`, `git branch --show-current`, `git rev-parse HEAD`, and `git worktree list`. Preserve occupied branches, commits and dirty files.
2. Read the **active** rows in `next.md`, not `next-history.md`. Reconcile GitHub merge/PR/check metadata **only as necessary**. As of this policy, PR #72 **F2c Compress is merged**; F4 Convert is next, reportedly active in an owner's local branch (not verified/published by this PR). No duplicate work.
3. Read **only the scoped AGENTS** for changed paths (F4: `imgengine-saas/frontend/AGENTS.md`), the relevant source/contract, and a targeted part of `skills.md` if necessary. Read other docs/large CI logs **on demand**—never load the entire documentation index, archive or seven-persona narrative by default.
4. **Before any dependency download, install, build, app execution, typecheck or test**, verify the Docker-only boundary below and the minimal existing Docker image/Compose/runtime files. Once verified in the same session and revision, reuse that evidence; do not repeatedly load all Dockerfiles or rebuild a working image without a reason.
5. Confirm the active owner permission boundary. F4 continues on its existing local worktree; **no push, PR, merge, deploy, secrets, spending, or unrelated changes** without explicit owner authorization. Send only a compact exact-SHA handoff.

**Permanent `go ahead` behavior:** resume the **highest-priority approved READY/in-progress job** in `next.md`, repair its blockers before advancing, and reconcile already merged work. It does not authorize automatic push/merge/deploy or public release.

## B. Host/toolchain security boundary

On Windows 11, run `docker pull`, `docker build` or `docker compose` through Docker Desktop in Linux container mode. Docker's engine or builder fetches registry layers into Docker-managed storage. Run package and browser downloads, builds and tests inside the project's Ubuntu 24.04 containers. Keep dependency caches in container storage or Docker volumes. The source checkout may stay on the Windows bind mount.

This boundary includes recovery from failed downloads. Retry the Docker operation or record the registry error and outstanding test. Do not fetch layers, toolchains, browser archives or dependencies with Windows curl, PowerShell download commands or a Windows Temp staging directory. For the Free browser matrix, use the Playwright Noble image and engine versions specified in the [browser CI job](../.github/workflows/ci.yml).

| Windows 11 host — allowed | Docker Desktop Ubuntu 24.04 Linux container — required |
| --- | --- |
| VS Code, local Git, Docker Desktop, Docker CLI/Compose, file management | Node.js 24, Corepack, pinned pnpm, Nuxt/Vue/TS, Playwright browsers, C/C++/CMake/Ninja, Rust/Cargo, dependencies, builds and project tests |

A **workspace-local Windows `node.exe`**, ZIP or executable is still a Windows-native runtime and is **prohibited**. Do not search/download/install Windows Node binaries, use host-native `npm`/`pnpm`/`npx`/`cargo`/`cmake`, modify Windows PATH/registry, or scan host drives for a fallback toolchain. A Windows terminal may run `git` and `docker compose` only for orchestration. The Docker Desktop WSL2 backend does not permit a separate unmanaged Windows/WSL app toolchain.

Do not add Docker socket mounts, privileged containers, unnecessary publicly exposed ports, extra Python/FastAPI/Redis/Celery/PostgreSQL services, paid cloud infrastructure or real credentials for a browser-local Free milestone. Use synthetic development values in ignored env files.

**Docker unavailable or wrong runtime?** Record `BLOCKED — Docker Desktop/Linux engine` or the exact failing image/install step. Report unexecuted tests as **NOT RUN**, not PASS; do not silently substitute Windows execution or a different commit's green GitHub CI.

## C. Inspect these existing sources only as relevant

| Check | File |
| --- | --- |
| Ubuntu 24.04 + Node 24/C/Rust dev image | `infra/Dockerfile.p7-frontend-dev` |
| Linux frontend service + named dependency volume | `infra/docker-compose.p7-frontend-dev.yml` |
| VS Code Dev Containers attach | `.devcontainer/devcontainer.json` |
| Linux startup script (Corepack/pnpm) | `infra/scripts/run-p7-nuxt.sh` |
| Node engine, pinned pnpm and test scripts | `imgengine-saas/frontend/package.json` |
| Synthetic local key template | `infra/p7-frontend-dev.env.example` |
| Minimum-context source/CI evidence rules | `docs/CODEX_VSCODE_LOW_CONTEXT_SIX_RULES.md`, `docs/CI_FIRST_MINIMAL_AI_CONTEXT_POLICY.md` |

Use **P7 active Rust/C/Nuxt Compose**, not `imgengine-saas/infra/docker-compose.yml` (legacy Python/worker/database stack).

## D. Focused frontend tests in Ubuntu container

### Free-only lightweight environment proposal

`FREE-FRONTEND-DOCKER-LOCAL-01` adds a dedicated Ubuntu 24.04 `free-frontend` service with only Node/Corepack/pnpm and named Linux dependency/cache volumes. It does not replace P7, start a backend/native/database dependency, publish a host port, mount the Docker socket, or include real credentials. Run isolated Free commands through this service with `--no-deps`; run browser checks only in the revision-pinned Playwright image. See [the environment contract](engineering/FREE_FRONTEND_LOCAL_DOCKER_PROPOSAL.md). A successful compiled-candidate check is not external release approval.

From the repository root, on the **Windows host** use only Git/Docker orchestration. First check `docker info --format '{{.OSType}}'` returns `linux`. Create the ignored `infra/p7-frontend-dev.env` from the example **only if missing**, substitute a synthetic development key, never overwrite an existing file or print real secrets.

```bash
git status -sb
git worktree list
docker info --format '{{.OSType}}'
docker compose --env-file infra/p7-frontend-dev.env -f infra/docker-compose.p7-frontend-dev.yml config --quiet
docker compose --env-file infra/p7-frontend-dev.env -f infra/docker-compose.p7-frontend-dev.yml build frontend
docker compose --env-file infra/p7-frontend-dev.env -f infra/docker-compose.p7-frontend-dev.yml run --rm --no-deps frontend bash -lc 'set -euo pipefail; node --version; corepack enable; pnpm --version; pnpm install --frozen-lockfile --reporter=append-only; pnpm test; pnpm run typecheck; pnpm run build'
```

The frontend pins `pnpm@10.34.9` and supports Node `^24.15.0`; **check actual container versions** rather than trusting the broad NodeSource 24.x major. If version mismatch or Linux package failure occurs, correct the Docker image in a separate authorized scope—not the Windows host. For Playwright, install pinned browser packages **inside Linux** and run the relevant browser suite there. Do not claim Playwright PASS unless executed. GitHub's mandatory native, sanitizer/fuzz, Rust, frontend and browser gates remain intact for a published PR; unpushed F4 changes have **no exact-head GitHub verification**.

## E. F4 recovery, evidence and clean continuation

Keep the existing F4 branch, worktree, commits and uncommitted code. Navigation links alone do **not** prove conversion correctness, privacy, cancellation, metadata handling or resource limits. The former agent attempted `E:\imgengine\.tmp-test-tools\node-v24.15.0\node-verified.exe`; **never resume or run it**. Before any cleanup, identify the specific owning process and exact partial download; remove only confirmed temporary artifacts, not unrelated test logs or source. No blanket deletion of `.tmp-*`.

Use proportionate L1–L7 engineering reviews plus CISSP privacy, CCIE networking, Linux/SRE, CA economics, MBA/product and management perspectives. Keep HLD/LLD, DRY/SOLID, secure code and measured claims when relevant; do not emit seven repetitive role reports.

**Handoff (8–12 lines):** milestone + branch/SHA; Docker image/service and real Node/pnpm versions; focused commands and PASS/FAIL/NOT RUN; touched files; security/privacy issues; outstanding PR/CI gates; exact next approved READY job. Store full logs outside the model context in ignored local files/CI, inspecting just failed excerpts first. **Never** claim a docs-only PR ran the F4 app/tests.
