# F5-SUPPLY-05 — Source lockfile remediation and risk disposition

**Checkpoint: REVIEW CANDIDATE — NOT MERGED, NOT RELEASED.**
Generated on 2026-11-21 using exact source branch `fix/free/f5-supply-05-production-lockfile`.

## Root cause and resolution

The tracked Nuxt/Nitro production resolution previously contained two moderate advisories:

| Dependency | Before | Candidate pinned version | Advisory |
| --- | --- | --- | --- |
| `glob` | 13.0.7 | 14.0.2 | [GHSA-3g3m-2fjr-6pw4](https://github.com/advisories/GHSA-3g3m-2fjr-6pw4) |
| `minimatch` | 10.2.6 | 11.0.1 | [GHSA-g3hv-75wg-m249](https://github.com/advisories/GHSA-g3hv-75wg-m249) |

The source manifest adds explicit **pnpm 10.34.9 overrides** for these two transitive dependencies, preserving existing Vue and node-forge overrides. The prior [isolated major-version compatibility experiment](https://github.com/Rofikali/imgengine/actions/runs/44742258511) demonstrated compatibility with these exact versions against 84 existing local browser tests and 200 Free static browser tests. However, the owners of the indirect dependency ranges (notably the Nuxt/Nitro, `@vercel/nft`, `archiver` resolution chain) have not all explicitly adopted these major versions. The override is therefore an **upstream-range compatibility exception requiring independent review**, not a claim that the upstream semver contract changed.

Reviewers must evaluate any changed `glob` argument semantics, `minimatch` pattern/optimization behavior, platform portability and package graph consumers. The broad override impacts more than the two original paths: it is intentionally visible and must not be hidden as a safe upstream update. Prefer removing/reducing it once compatible upstream releases have been proved and pinned.

## Manager-generated lockfile and provenance

A *read-only* Ubuntu 24.04 container job, with Playwright `v1.58.2-noble`, Node 24 and pnpm `10.34.9`, generated the lockfile with `pnpm install --no-frozen-lockfile`, then reinstalled it with `pnpm install --frozen-lockfile` and ran the existing strict source-resolved release audit. Its output was captured as a CI artifact and transferred unchanged into the tracked `imgengine-saas/frontend/pnpm-lock.yaml` by the GitHub connector. The temporary generation workflow was subsequently removed from the PR branch to avoid recurring CI cost and embedded lockfile logs.

**Exact generation/audit evidence:** [GitHub Actions run #44876436903](https://github.com/Rofikali/imgengine/actions/runs/44876436903) — **SUCCESS**:
- Pinned Ubuntu container and package-manager validation: PASS
- pnpm-generated lockfile and frozen install: PASS
- Existing `pnpm run verify:free-release-dependencies`: **critical=0 high=0 moderate=0 low=0**
- Nuxt preparation, TypeScript typecheck and Vitest unit tests: PASS
- No production tag, deployment, billing action or server image upload took place

**Tracked candidate:** manifest override commit `438f834c521318033a0e68ee31a5b934d7e02c5b`; pnpm-generated lockfile commit `4d8daa131e041d9bf715fb115568bb8c72a5837e`. The final PR head is newer and **requires independent exact-head CI**.

## Required acceptance gates before merge

1. Recheck `pnpm install --frozen-lockfile` and `pnpm run verify:free-release-dependencies` **using the checked-in lockfile**, not the CI generator's temporary worktree.
2. Require complete ordinary CI at the final PR head, including native C fuzz/sanitizers, Rust, Ubuntu 24.04 Docker, production audit, Nuxt type/unit/build, 84 existing local-browser tests and 200 Free static-browser tests across Chromium desktop/mobile, Firefox and WebKit. Retain all existing assertions, timeouts and security headers.
3. Validate no changes to the five browser-local Free image workflows, privacy/no-upload guarantee, static allowlist, C ABI, Rust FFI or Cloudflare deployment policy. Run independent security code review on the major-version override and supply-chain provenance; document future upstream retirement.
4. Reconcile `next.md` generated NOW/NEXT facts and applicable release docs **after actual owner-reviewed merge**, using real merge SHA and CI URLs. No claim of success before the final exact-head CI result.

## Explicitly still blocked after dependency remediation

The repo's `verify:free-publication-ready` is intentionally fail-closed. Final operator-approved terms/privacy/support notices, detached source-and-artifact attestation and digest, SBOM/license checks, independent Cloudflare Pages HTTPS/CSP/privacy/404/rollback, free-tier cost review and explicit owner **go/no-go** must pass before publication. **Do not create `free-v1.0.0` or deploy from this candidate.**
