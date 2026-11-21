# Rust TCP-disconnect test ownership — CI regression

**2026-11-21 | TEST-ONLY REVIEW CANDIDATE. Do not treat this as a production engine or release fix.**

## Evidence

[PR #98 native CI run #44878044976](https://github.com/Rofikali/imgengine/actions/runs/44878044976) reported failure of the Ubuntu 24.04 Docker verification job's `cargo test` stage. Its `imgengine-api` library test `tcp_disconnect_during_upload_never_enters_lifecycle` failed at API-state construction (`LifecycleUnavailable`), before the request was sent. The other 20 tests in that binary passed. The existing [post-merge main native CI #44877676683](https://github.com/Rofikali/imgengine/actions/runs/44877676683) had succeeded with the same API source. **This does not prove a data-race root cause**; a native engine already owned by an in-flight connection is one plausible mechanism.

## Scoped change

Two TCP-disconnect tests previously called `start_loopback(runtime.router())` and, after checking results, `server.abort()` + `await` + `drop(runtime)`. Aborting the Tokio listener task alone does not establish that all detached Axum connection tasks have stopped or that their API lifecycle references have been released before the next test acquires the `ENGINE_LOCK`.

Use the **existing production-owned `ApiRuntime::serve` and `ApiServer::shutdown`** via the existing test helper `start_runtime_server` for the two tests. Shutdown closes admission, waits for accepted work, joins the server and transitions lifecycle to stopped before releasing the test-level engine mutex. All TCP input, request assertion, response-abandonment semantics, event counts, native processing and timeouts remain unchanged. Remove the now-unused abort-only helper.

## Validation

Require exact-head PR CI, particularly the Ubuntu 24.04 Docker verification `imgengine-api` tests; repeat the failing test under the test's original concurrent runner configuration to check for recurrence. No changed production source or C ABI; no skipped test, retry workaround, timeout extension, waiver or modification to the CI verification gate. If the failure persists, inspect the deeper native lifetime independently rather than asserting this test-only cleanup solves it.

Keep this fix **separate from Free release-candidate evidence PR #98**. That PR remains blocked by its independent CI gate until test ownership is resolved and a fresh exact-head run passes.

No Git tag, Cloudflare deployment, billing action or Free release authorization.
