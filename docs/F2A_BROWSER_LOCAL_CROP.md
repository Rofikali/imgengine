# F2a: browser-local pixel crop

Implementation for review, not a release. Base main:
`aeeb2ce2bb4c6a540b9dfe36fa49a0009585b445`. F1 PR #57 is merged as
`1523d82f9549bcc3e54b57976e7cd5c539900e27`; all seven checks passed at
`19f25914acd61affa64c98ca260dbbf04a652787`
([CI](https://github.com/Rofikali/imgengine/actions/runs/44043494900)).
The [live board](../next.md) and PR handoff record this task's exact-SHA results.

## Contract and ownership

`/crop-image` chooses one local JPEG/PNG, positions a crop by pointer/touch or
arrow keys (Shift = ten pixels), and edits left/top/width/height using labeled
numeric controls. Free and square aspects are explicit; changing aspect resets
to a centered selection. Reset selects the largest allowed centered rectangle.
Source images beyond the 4096 output-edge cap therefore reset to a partial crop,
not a scaled full image. Numeric controls are the non-drag resizing alternative.
Output is a downloadable `imgengine-cropped.png`; selection changes invalidate it.

Coordinates are integer decoded-source pixels, with zero-based left/top and
exclusive right/bottom. Pointer deltas round once to the nearest source pixel
and clamp to the raster. Numeric invalid input is rejected rather than silently
clamped. Crop export copies the selected rectangle at 1:1 size: no normalized
conversion, aspect-fill or resize. The composable and Worker both validate bounds;
the Worker checks against actual decoded dimensions before canvas allocation.

Input/output caps, 10-second deadlines, cancellation/generation checks, URL and
Worker ownership, no-upload/storage policy and safe errors reuse
[F1](F1_BROWSER_LOCAL_RESIZE.md). Input remains ≤4 MiB, ≤4 MP, ≤8192 per side;
output ≤4 MP and ≤4096 per side, PNG ≤20 MiB. F1 resize remains the default
Worker operation when no crop is provided. Its route, limits and tests remain.
No native/Rust/proxy/ABI, package lockfile or CI changes are made.

Orientation remains **stored raster**: the existing ephemeral JPEG EXIF/PNG eXIf
stripping precedes decoding; no automatic rotation or mirroring is introduced.
Original metadata is not preserved. PNG alpha is retained; tests cover an opaque
pixel rectangle and fully transparent final pixel. Browser premultiplication,
color profiles and JPEG decoding can affect channel values; this is not a claim
of universal encoded-byte or semi-transparent/color-profile parity.

All image bytes and filenames remain local. Serving application assets is
allowed; source/result network or telemetry transmission and silent server fallback
are not. No persistence, account, billing, new dependency or additional server
processing is introduced. Worker reuse minimizes maintenance; it does not prove
faster rendering, a total RSS ceiling or reduced business cost.

## Verification and review

After scope inspection, use existing frontend commands: `pnpm test`,
`pnpm run typecheck`, `pnpm run build`, then
`pnpm exec playwright test --config playwright.crop.config.ts`,
`pnpm exec playwright test --config playwright.resize.config.ts` and
`pnpm run test:browser`. Browser fixtures are synthetic; output checks include
odd rectangles/all opaque pixels, exact dimensions, last-pixel transparency,
portrait pointer/touch, keyboard, square/reset, malformed input, EXIF mirrors/
rotations, no HTTP image traffic/storage, axe, repeated URL cleanup, cancellation,
removal and navigation. F1 regression covers the shared Worker timeout/stale
selection/resource bounds. Unit cases cover overflow, decimals, negative values,
thin rasters and rounding/clamping. All seven required CI jobs still run; local
browser suites are not misrepresented as part of existing CI.

Full logs: `E:/imgengine-f2a-logs`. Early test-only errors (request URL getter and
pointer coordinates before scrolling preview into view) remain in failed logs;
only final exact-SHA results establish acceptance. Do not manually duplicate
native/sanitizer/fuzz/Docker gates for this browser-only change.

Physical mobile, Firefox/Safari, manual screen readers, aggregate heap/RSS and
broader codec/color fidelity remain open. URL counts prove ownership only.
INFRA-P8-01, C-A0 and P14 remain independent; no simulator or release claim is made.
Rollback removes the crop route and optional shared-Worker crop path. Next after
review and confirmed merge: F2b Rotate, separately bounded. No merge/deploy approval
is granted by this feature or its tests.
