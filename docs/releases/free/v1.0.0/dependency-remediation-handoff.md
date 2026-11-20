# F5-SUPPLY-05 — Permanent dependency remediation handoff

**State (2026-11-20 UTC): READY FOR IMPLEMENTATION; not yet run, not released.** Verified GitHub `main`: `6c9e5ff7846d6d4e3b4ec63d62b19ac0d3af3008` after [PR #89](https://github.com/Rofikali/imgengine/pull/89) merged; [post-merge main CI](https://github.com/Rofikali/imgengine/actions/runs/44771824270) succeeded. This is a reviewable implementation contract, **not** an implemented security fix, frozen release candidate or owner authorization.

## Confirmed source risk and trial evidence

- Frontend `pnpm-lock.yaml` resolves `nitropack@2.13.6 -> @vercel/nft@1.11.1 -> glob@13.0.7 -> minimatch@10.2.6`, plus `nitropack -> archiver -> readdir-glob -> minimatch@10.2.6`.
- Two moderate advisories: [glob GHSA-3g3m-2fjr-6pw4](https://github.com/advisories/GHSA-3g3m-2fjr-6pw4) (previous audit fixed versions >=14.0.2) and [minimatch GHSA-g3hv-75wg-m249](https://github.com/advisories/GHSA-g3hv-75wg-m249) (previous audit fixed versions >=11.0.1). Revalidate advisories, version ranges and reachability at patch time; reported severity is not itself proof of runtime exposure.
- [PR #88](https://github.com/Rofikali/imgengine/pull/88) merged with [12/12 main-lockfile checks](https://github.com/Rofikali/imgengine/actions/runs/44742258560) and an independent [temporary patched-major probe](https://github.com/Rofikali/imgengine/actions/runs/44742258511) showing clean trial audit, 84/84 local tools and 200/200 Free static four-browser checks. It did **not** change tracked dependencies. Firefox timeout root cause remains unproven.
- [PR #89](https://github.com/Rofikali/imgengine/pull/89) implemented the stricter source-lockfile `pnpm run verify:free-release-dependencies` gate; that gate is expected to **fail** until the two source-lockfile moderate findings are resolved.

## One bounded implementation PR

1. Reinspect current main and open PRs. Use a short-lived feature branch and pinned Ubuntu 24.04 **Docker-only** development/toolchain. If Docker is unavailable, record NOT_RUN. Never regenerate the production lockfile by hand or substitute non-Docker local execution as project acceptance.
2. Trace upstream version ownership and package ranges through `nitropack`, `@vercel/nft`, `archiver`, `glob` and `minimatch`. Prefer a compatible upstream dependency upgrade; evaluate scoped overrides only with explicit risk/compatibility review, never blindly copy the disposable experiment into production.
3. Regenerate and commit `imgengine-saas/frontend/pnpm-lock.yaml` with **pinned pnpm 10.34.9** and the reviewed manifest changes. Require repeatable `pnpm install --frozen-lockfile` in Docker; preserve Nuxt, all five Free tools, C17/Rust source and CI policy.
4. Run the **actual tracked lockfile** `pnpm run verify:free-release-dependencies`, require zero moderate/high/critical in the release audit, and record the reported low vulnerabilities for separate review. Run frontend unit, typecheck, normal build, Free static generation, static artifact/CSP/privacy checks, browser-local 84-case regression and Free static 200-case desktop/mobile Chromium, Firefox, WebKit matrix. Retain unchanged assertions, timeouts and all privacy/no-upload guarantees.
5. Run the required **exact PR-head CI**, including native C17 fuzz/sanitizer, Rust/API, Ubuntu/Docker, frontend and aggregate fail-closed policy when lockfile/security changes select full scope. Record source SHA, reproducible command, workflow run URL, PASS/FAIL/NOT_RUN and identified third-party risk. A dry-run/probe pass is not the production lockfile pass.
6. Update `next.md` and changed contracts/security/skills/README/scoped AGENTS/docs index **only if applicable**, plus the Free release evidence matrix and vulnerability review. Mark unrelated files REVIEWED N/A rather than rewriting. After owner review and external merge, the next authorized small PR reconciles the actual merge SHA.

## Gate, rollback and release isolation

**READY_FOR_REVIEW** only after manager-generated lockfile, clean frozen install, source-locked zero-moderate release audit, required exact-head CI and four-browser/privacy matrix pass. Revert the manifest and generated lockfile as a unit if regression occurs. No weakening security, hidden retries, premature release state, or merger authority.

Even after this milestone, public Free v1.0.0 stays **BLOCKED** until immutable release artifact/digest, legal/privacy/support and operator review, provenance/license/SBOM, Cloudflare live HTTPS/CSP/privacy/rollback evidence, free-tier financial risk and explicit owner go/no-go are independently satisfied. This milestone does not permit cloud actions, deployments, tags, payment or billing.
