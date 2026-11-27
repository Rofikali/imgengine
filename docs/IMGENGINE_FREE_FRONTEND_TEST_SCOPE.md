# IMGENGINE Free — isolated frontend test scope

Owner direction 2026-11-27. Candidate on existing PR #130, no new PR. #128/#129 merged; repaired measurement retained from main 74b4eedaf7373ea345f6a4291e2bb603727f1bbe. Tests NOT RUN for new candidate.

## Product and scope boundaries
Free is active product. Do not modify or test unrelated Pro, Studio, C17/ABI, Rust/FFI/API, legacy backend, databases or infra. Shared contracts, app.vue and common CSS are not Free-only by default. Initial exact allowlist: modified imgengine-saas/frontend/free-static/FreeSiteHeader.vue, landing.vue, tools.ts plus known non-runtime docs. Added/deleted/renamed files, other shared/unknown/config/lock/security paths or mixed scope do not opt in. No broad directory exemptions. Extend only after dependency review and test evidence.

## Verification contract
Preserve trusted-base nine selected job flags, three always-on jobs and aggregate gate. Add free_only output; absent flag from old base means normal frontend tests. Full/main/merge queue never use submode. Selected frontend jobs retain locked install, audit, security/allowlist/header/release/license/measurement fixtures, focused Vitest local- and free-static/ filters, shared TypeScript preparation/typecheck, Free static build, verify:free-static, verify:free-publication-ready, audited measurement and entire four-profile static browser matrix. Skip only normal unit suite (replace with focused units), normal server-backed build in both jobs and normal browser matrix for proven Free-only submode. Shared compile checks intentionally retained. Full and broader scopes retain existing gates. No paid product runtime or unrelated native/infra jobs for eligible PRs. Separate exact-artifact release approval unchanged.

Ubuntu 24.04 Docker commands from frontend: pnpm install --frozen-lockfile; pnpm exec nuxi prepare; pnpm exec vitest run local- free-static/; pnpm run typecheck; pnpm run build:free-static; pnpm run verify:free-static; pnpm run verify:free-publication-ready; pnpm run test:browser:free-static. Run existing security/audit fixtures per CI. No skip tokens or waiver.

## Rollout
Selector fixtures include allowed, mixed, unknown, native, configuration, added/deleted and full fallbacks. Browser coverage unchanged. Trusted old main means this policy PR runs full CI; actual selective lane execution NOT VERIFIED until an eligible later PR. No local Docker tests or measured savings claimed. One active integration PR; five attempts maximum, agent-maintained ledger; owner merge/release authorization required. L1–L7, CISSP, CCIE, Linux/SRE, CA/MBA and management review: bounded scope, fail-closed tests, no new service, no weakened release. Domain/finance/monetization deferred.
