# F1: bounded browser-local resize

Status: implemented for review; not released. Base main:
`e26a9194a3c9f58059fcd81c8bd78cffd2199c33` (F0 PR #55 merged).
[ADR-F0](F0_BROWSER_FIRST_SEQUENCING_ADR.md) governs the approved sequence.
Exact test-head SHA, PR and CI are recorded in the PR handoff and [live board](../next.md).

## User contract

`/resize-image` selects one still JPEG/PNG, edits width/height with an optional
aspect lock, previews a local result and downloads `imgengine-resized.png`.
Changing dimensions invalidates the previous result. Aspect lock uses the original
source ratio, nearest integer rounding (halves up) and a minimum one-pixel edge.
Unlocked dimensions may intentionally change aspect. Original inputs are not
modified. PNG preserves alpha; browser color management/resampling can vary.

Input: at most 4 MiB encoded, 4,000,000 decoded pixels and 8192 per axis. Output:
positive integers, at most 4096 per axis and 4,000,000 pixels (16 MB nominal RGBA
canvas); PNG output must be nonempty and at most 20 MiB. These are operation
limits, not a total browser-process memory guarantee. Invalid headers, animation,
metadata, actual decode/header dimension mismatch and unsupported formats fail.
Existing preview preflight is reused unchanged; the lower F1 pixel cap is checked
before invoking its decoder. Extensions/MIME alone are not trusted.

Orientation is explicitly **stored raster, without EXIF rotation/mirroring**.
The existing preview helper strips JPEG EXIF/PNG eXIf from an ephemeral copy;
the Worker receives that sanitized local Blob. Tests cover values 1–8 for JPEG
and PNG plus malformed metadata. Original bytes stay local and unchanged.
Export is a fresh browser PNG; original metadata is not preserved. This does not
change the P13 stored-raster contract or claim universal color/profile parity.

No source/result bytes or filenames are sent to HTTP, telemetry or persistent
browser storage. App/Worker assets and Nuxt's static build manifest may be fetched;
reading a `blob:` object URL is local. There is no server fallback. Unsupported
Worker/OffscreenCanvas/image decode produces a safe local error.

## Ownership and architecture

The existing server-render screen moves intact into `ServerRenderApp.vue` behind
the `/` page; global styles remain in `app.vue`, which now hosts `NuxtPage`.
The only server-screen addition is a link to local resize. Existing P5/P8/P12/P13
forms, proxies, Rust and C17 code are unchanged. Dedicated routes distinguish
zero-upload resize from server print layouts that upload on Render.

The composable owns one source URL and at most one result URL. Replacement,
removal and unmount revoke both, abort preview work and terminate active Workers.
Each render uses a fresh Worker; successful encoding closes its ImageBitmap and
clears its canvas, then the owner terminates the Worker. Cancel/timeout also
terminate it; generation checks discard stale asynchronous completions. Cancel
preserves the current source for retry. Decode and resize each have a 10-second
timeout; bounded file reads may finish after cancellation but cannot publish a
stale selection. No C/native cancellation semantics are changed.

Worker processing keeps resampling/encoding off the UI thread; source preview
still uses browser image decoding. This is isolation, not proof of a latency or
RSS improvement. No packages, Wasm toolchain, queue, database or runtime service
are added. Costs are static route/Worker assets, browser CPU/memory and existing
site delivery; no new server image-processing cost or business revenue is claimed.

## Acceptance and reproduction

Use supported Node/pnpm and the existing lockfile:

```sh
pnpm exec nuxi prepare
pnpm test
pnpm run typecheck
pnpm run build
pnpm exec playwright test --config playwright.resize.config.ts
pnpm run test:browser
```

The resize browser config covers desktop Chromium and emulated Pixel 7. It tests
exact output dimensions and color landmarks, aspect rounding, PNG download,
same-origin asset-only HTTP/no storage, axe, mobile overflow, EXIF, malformed and
oversized input, allocation limits, five repeated operations, URL/Worker cleanup,
cancel, timeout, unsupported Worker and accepted 4MP output. The existing browser
suite guards server-screen routing, crop interactions/accessibility and error
behavior. Unit tests preserve existing proxy/form contracts and add invalid
dimensions, allocation caps and rounding cases. Full CI runs all seven jobs;
new resize browser tests are local evidence, not automatically run by existing CI.

Full logs stay outside chat in `E:/imgengine-f1-logs`; browser attachments record
bounded fixture click-to-preview duration, not customer content. An early privacy
assertion omitted Nuxt's static build-manifest JSON and was corrected to allow
that specific same-origin GET path without query/body; it did not reveal upload.
A test Worker subclass also needed TypeScript's explicit `override` modifier.
Failed development logs are retained. Final exact-SHA results must supersede,
not silently reuse, working-tree evidence.

## Remaining risks and next action

URL/Worker counts demonstrate ownership, not aggregate heap/RSS. Physical mobile,
Firefox/Safari, screen readers, high-frequency resampling, ICC/CMYK and broader
codec fidelity remain unverified. Browser memory pressure can still abort valid
inputs; errors offer a smaller-image retry without uploading. Header inspection
is a resource guard, not a sandbox or a substitute for browser security updates.
C-A0, dedicated P8 runtime and P14 release evidence remain separate open work.

Rollback reverts this route/component extraction and its new local files; no
native API or data migration exists. Next: review F1 evidence and obtain owner
merge approval; after confirmed merge, select bounded F2 crop/rotate/flip under
the live queue. Public launch and Wasm adoption require their separate gates.
