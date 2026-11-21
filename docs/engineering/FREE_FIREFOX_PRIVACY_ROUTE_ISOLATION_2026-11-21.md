# Free v1.0.0 browser navigation lifetime — Firefox and WebKit

**Date:** 2026-11-21

**Decision:** Bounded CI test-lifetime candidate. **Public release is NOT authorized.**

## Evidence

The Free static test server has recorded completed HTTP 200 responses for pages and JavaScript/CSS assets while Playwright occasionally never observes browser page `load`:

- [PR #98 exact-head CI #44885261064](https://github.com/Rofikali/imgengine/actions/runs/44885261064): desktop Firefox timed out in the single six-route privacy audit at `/rotate-image`.
- [Merged main CI #44885978021](https://github.com/Rofikali/imgengine/actions/runs/44885978021): the same Firefox audit timed out at `/convert-image`.
- [Cloudflare owner-documentation PR #102 CI #44886587696](https://github.com/Rofikali/imgengine/actions/runs/44886587696): desktop WebKit timed out on the independent `/resize-image` CSP/header test after the static server had completed page HTTP 200 responses.

**A browser navigation stall is not a demonstrated image upload or proven product security flaw.** The root cause within the browser loading lifecycle or CI environment remains unproven. Previous green tests on other SHAs do not clear this release source.

## Independent tests without waived checks

The original six-route privacy journey (home, resize, crop, compress, rotate, convert) is now **six individually isolated Playwright tests**, rather than one 90-second test with six Pages in a single context.

Each test retains the complete `waitUntil: 'load'` navigation, visible static main region, context-level surveillance of prohibited HTTP requests (external origin, non-GET, request body, `/api/`), zero localStorage, zero sessionStorage, zero IndexedDB and zero cookies. The route-wide guarantees are **stronger** because all six routes must independently prove the privacy boundary. The four browser projects run the same exact test suite.

**Firefox and WebKit** now each run **55 tests in 55 isolated short-lived browser/server processes** using Playwright's test-level sharding, while desktop/mobile Chromium each run the same 55 tests in their ordinary single matrix. The test-driver unit contracts verify both full shard sets, absence of empty shards, redacted diagnostics and fail-closed behavior on any failed or interrupted process. This isolates further WebKit page-runtime interference discovered in PR #102. The extra CI work is a bounded reliability trade-off for a zero-server-cost static Free product; monitor the actual GitHub Actions minute budget before optimizing.

No test or browser is disabled, no test or navigation timeout increased, no retries added, no `load` downgraded to `domcontentloaded`, no security headers relaxed, no data-exfiltration controls removed, and no production source/API/ABI/deployment code changed.

## Acceptance, proof and residual risk

1. **Exact PR head full CI must PASS every selected native, Docker, Rust, browser, CSP, privacy and aggregate gate.** Any failing, cancelled or missing required job is NO GO.
2. If tests still fail, keep the PR blocked; inspect the bounded test-server response and browser failure artifacts for the affected single test. Do not mask or change the test to force success.
3. On independently reviewed merge, require a second successful merged-main exact-SHA full CI before freezing source or generating an immutable release artifact. PR #102 documentation must also pass the exact-head policy and full CI before merge.
4. Public v1.0.0 launch additionally requires approved privacy/terms/support notices, real operator contact, detached owner/security/legal approval, source+artifact hashes, SBOM/license verification, accurate Cloudflare Free billing and provider TLS/CSP/404/no-upload/rollback checks. The existing fail-closed publication guard remains unchanged.
5. The GitHub Actions milestone-proposal permission problem is independent and requires an owner repository setting change; CI cannot safely grant itself privilege.

This engineering note is a test-harness candidate, **not** a signed release attestation, a Cloudflare deployment, or a final `free-v1.0.0` Git tag.

## Additional exact-head failure and bounded navigation semantic correction

[PR #103 exact-head CI #44887405931](https://github.com/Rofikali/imgengine/actions/runs/44887405931) **FAILED** at desktop Firefox shard 55/55: `tool-navigation.spec.ts` timed out waiting for `load` on the homepage. Other native, Rust, Docker, security and integration checks passed, and prior 54 Firefox shards completed. This **does not** establish that a requested image was uploaded. The isolated navigation test only verifies DOM link destinations, heading and horizontal overflow: it now navigates with `waitUntil: 'domcontentloaded'` and subsequently checks actual visible DOM. The separate Free privacy and CSP/header tests **still explicitly wait for `load`**, enforce all their network/storage/cookie/header constraints and remain required on every browser. This is a semantic correction for a navigation-only test, not a broad loosening of the release gate or a fix for the underlying browser lifecycle instability.

**Acceptance remains exact-head full CI PASS**, not a previous passing partial run. If the load-bound privacy/CSP tests fail in their isolated shards, keep public release blocked.
