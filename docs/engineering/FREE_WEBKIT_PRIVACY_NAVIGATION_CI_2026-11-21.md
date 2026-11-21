# Free static WebKit privacy navigation: CI regression investigation

**Date:** 2026-11-21  
**Status:** test-harness repair candidate, NOT RELEASED.  
**Issue scope:** browser test runtime; no product JavaScript, C/Rust API/ABI, hosting or release security policy changes.

## Exact failure

At [PR #98 exact-head full CI #44883232543](https://github.com/Rofikali/imgengine/actions/runs/44883232543), all native, Docker, Rust, fuzz, sanitizers, Nuxt frontend verification, and other selected integration checks passed. Its `Nuxt local-tool browser quality` job failed at the **49/50 passed** point of the desktop WebKit static distribution test: `tests/browser/free-static.spec.ts:48`, “static Free browsing sends no image, filename, or server-bound requests”. The browser emitted `Error: page.goto: WebKit encountered an internal error` while navigating to `http://127.0.0.1:3100/convert-image` after five earlier full-load page navigations within the same browser Page. The CI gate correctly blocked the PR and did not claim partial static browser success.

**Important distinction:** this is a browser-engine error during the test's page transition; the log does not show a forbidden image-bearing network request or a proven product JavaScript fault. The WebKit runtime/resource-lifetime explanation is **a hypothesis**, not an independently established root cause. A green targeted/full CI run is necessary to validate this bounded test-harness change.

## Scoped repair and stronger invariants

The six privacy route checks keep the same Playwright **browser context**, but now allocate a fresh Page per route, close each page after its full `load`, and do not reuse six serial Nuxt navigations on one WebKit Page.

The request listener is upgraded from `page.on('request')` to **`context.on('request')`** so every route still contributes to a single comprehensive audit. It continues to reject any outbound origin, HTTP method other than GET, request body, or `/api/` path. A forbidden request cannot be hidden by a Page rollover.

The test retains all six routes and their full-load waits and now asserts **no localStorage, sessionStorage, or IndexedDB after each route**, **no cookies after the entire journey**, and a visible main region for each page. Privacy controls are stricter, not skipped. No retry, timeout increase, WebKit bypass, degraded CSP check, early-exit waiver, relaxed browser matrix, or cherry-picking of passed tests was introduced.

## Acceptance criteria and limitations

1. The exact-head test in pinned Playwright v1.58.2 / Ubuntu 24.04 must pass in **desktop WebKit**, and the full Free static and local-tool browser matrix must pass all projects.
2. The complete main project CI gate must pass, not only the isolated browser test. A separate GitHub Actions status-sync permission denial is an owner-controlled setting, independent of application/browser CI.
3. If WebKit still produces an internal error, keep the PR unmerged and isolate a minimal reproducer using the existing CI browser failure artifact and diagnostic logs. Do **not** disable WebKit, a privacy test, or a safety gate to force green.
4. After the repair passes and is reviewed/merged, update Draft [Free release evidence PR #98](https://github.com/Rofikali/imgengine/pull/98) from main, revalidate exact-head CI, and only then consider it ready for review. No final `free-v1.0.0` tag or Cloudflare deploy is authorized by a CI repair.

**Risk and accounting:** one bounded test-only change and CI run; no new dependency, API/ABI surface, account credential, paid resource or production privacy data transfer. Maintain a visible review/merge approval boundary.
