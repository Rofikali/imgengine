# F5-SUPPLY-01 — Free v1.0.0 moderate dependency audit evidence (2026-11-20)

**Decision state: OPEN / NO PATCH OR RISK ACCEPTANCE APPROVED.** This is an *intermediate development-CI finding*, not a source-frozen release candidate audit, immutable SBOM, final dependency disposition or public deployment clearance. [F5 release evidence matrix](evidence-matrix.md) remains authoritative for launch gates.

## Exact verified report

- **Source:** [PR #86 CI run 44725108058](https://github.com/Rofikali/imgengine/actions/runs/44725108058), job **Nuxt frontend verification**, `pnpm install --frozen-lockfile` and `pnpm audit --prod --json`, with `scripts/check-production-audit.mjs` printing bounded, sanitized finding details. The run's full aggregate CI result is **not presumed** from this individual frontend job. PR HEAD and all checks must be reverified before merging.
- **Observed job summary:** `critical=0 high=0 moderate=2 low=0`. **The frontend verification job passed.** The existing CI checker does not fail solely on moderate severity; this does **not** mean moderate vulnerabilities are cleared for release.
- **Advisory A:** `glob`, **moderate**, [GHSA-3g3m-2fjr-6pw4](https://github.com/advisories/GHSA-3g3m-2fjr-6pw4), production audit reports `patched_versions: >=14.0.2`. The advisory describes cancellation/AbortSignal handling that can allow filesystem traversal to continue after timeout. Applicability to the actual Nuxt/static artifact **UNVERIFIED**.
- **Advisory B:** `minimatch`, **moderate**, [GHSA-g3hv-75wg-m249](https://github.com/isaacs/minimatch/security/advisories/GHSA-g3hv-75wg-m249), production audit reports `patched_versions: >=11.0.1`. The advisory describes input-controlled glob-pattern expansion with potential CPU/memory denial of service under specified processing settings. Applicability to project inputs and resolved version **UNVERIFIED**.
- **Previously reported picomatch high advisory:** [GHSA-fc5q-39xc-vjpf](https://github.com/advisories/GHSA-fc5q-39xc-vjpf) is **withdrawn** as of 2026-11-18 and did not appear in this run's high counts. The narrow time-limited *development CI* exception remains in source; remove it only in a separately reviewed fail-closed security change. It is never a production authorization.

## Verified current lockfile graph (static inspection, NOT reachability proof)

The tracked `imgengine-saas/frontend/pnpm-lock.yaml` blob **`7cbd9c37f1aa5bfcfd16dfd3df8913afc8e29026`** resolves:

- `nitropack@2.13.6` → `@vercel/nft@1.11.1` → **`glob@13.0.7`** → **`minimatch@10.2.6`**.
- `nitropack@2.13.6` → `archiver@8.0.0` → `readdir-glob@3.0.0` → **`minimatch@10.2.6`**.

This is **two transitive paths to the resolved minimatch version**, not an independently verified runtime exposure. Nitro is involved in Nuxt tooling and normal server builds; static Free export must still prove no executable copy or reachable Node/Nitro code in the final output. Do not hand-edit the lockfile or force a major upgrade across these dependency contracts without compatibility tests. The package graph may change with later merges; re-verify from the frozen candidate lockfile.

## Required review and resolution sequence

1. **Dependency graph and lockfile evidence:** reconfirm the graph above, all resolved versions and all importing paths with pinned pnpm `10.34.9` (`pnpm why glob minimatch --depth 20` and lockfile inspection) in the mandated Ubuntu 24.04 Docker environment. Distinguish build/dev/test tree from executable shipped static browser output. A `--prod` audit classifies dependency entries; it does **not** alone prove runtime reachability.
2. **Threat model / exposure:** assess whether untrusted patterns, input filenames, directory trees or AbortSignal cancellations reach either vulnerable implementation during build, static serving or any separately deployed Node/Rust/C runtime. Verify the Free static artifact contains no Node/Nitro/server code, rather than inferring this from package names.
3. **Patch feasibility:** select an upstream-compatible Nuxt/transitive dependency upgrade only after inspecting actual semver/ABI compatibility and the lockfile. Published patched *major* versions are **not** permission for blind global overrides or hand-edited lockfiles. If non-applicable, document a dated, scoped owner/security acceptance and expiration; otherwise prepare a separate minimal patch PR.
4. **Reverification:** frozen install, production audit, checker unit regressions, typecheck/unit/build, static artifact/headers/CSP audit and all affected four-browser checks on **the exact patch HEAD**. Current critical/high gate must remain fail-closed.
5. **Independent release gates:** final frozen-source SHA and static artifact SHA-256; license/SBOM/reproducibility review; real Cloudflare headers/TLS/analytics, privacy/terms/support, rollback and explicit owner launch approval. Development CI success alone closes **none** of these.

## Non-actions and controls

No dependency version/lockfile, audit threshold, temporary exception, deployment settings or release artifact was modified by F5-SUPPLY-01. No severity count was artificially set to zero. Keep the audit diagnostic output bounded/sanitized, and do not include raw environment variables, tokens, personal files or registry credentials in logs. Any remediation decision requires its own exact-SHA evidence and reviewer disposition.
