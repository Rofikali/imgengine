# P13 M1.3 — bounded browser/native parity and P12 compatibility

Base: `de8b1708ba4e54cd219c93cc15f7923773ce8c9e` (PR #47 merged).
M1.2's seven-job CI passed at PR head `b3aa42c2b89b0831a48448e8c4529cd67131ed75`
([run](https://github.com/Rofikali/imgengine/actions/runs/43808582807)). This
change adds tests and documentation only. Production C17, Rust, ABI, Nuxt proxy,
editor, dependency lockfile and CI workflows are unchanged.

## Oracle and acceptance limits

The new `tests/browser/crop-parity.spec.ts` exercises the real UI through Nuxt,
Rust and C17. It captures the actual multipart crop, checks all-four/none and
absence of browser credentials, and forwards the request unmodified.

The oracle imports no production geometry helpers. BigInt arithmetic independently
converts left/top with floor and right/bottom with ceil at scale 1,000,000.
Expected extents and corner coordinates are independently derived from source
dimensions, target aspect and zoom; integer geometry must match exactly.
For P12, the oracle computes centered fill using integer cross-products and
floor centering, without sending any crop fields.

Synthetic fixtures contain spatial RGB ramps and asymmetric sinusoidal detail.
The browser decodes each encoded PNG/JPEG fixture, crops using the reference
rectangle, and resizes it with Canvas2D. The returned JPEG is decoded and compared
over **every RGB channel of every output pixel**, with no border exclusion.
Preselected acceptance limits are mean absolute error (MAE) <= 4 and 99th
percentile absolute error (P99) <= 24, on the 0–255 channel scale. These allow
JPEG/chroma and resampler differences; dimensions, copy accounting and geometry
are exact. JPEG file bytes are not compared.

Matrix: sources 401×203, 203×401, 257×257 and 640×480; targets 80×120 and 120×80;
PNG and JPEG; P12 omission, top-left, bottom-right, 4× zoom and fractional 1.05×
zoom. This gives 80 image comparisons, including up/downsampling and odd edges.
The production editor is driven with native inputs/keyboard events, not bypassed
with a production geometry function.

Additional live tests freeze a 3-copy partial row on a 200×200 sheet with 80×80
cells, blank unused fourth cell, white margins, and a normalized last-source-pixel
crop. Landmark channels permit absolute error <= 12; white samples must be >=250.
These small raster fixtures make verification inexpensive without new packages.

## Security, orientation and lifecycle

Stored-raster EXIF semantics from M1.2 remain authoritative; no auto-rotation is
introduced. The existing live suite checks JPEG/PNG orientations 1–8, mirrored
values and both TIFF byte orders, original upload byte identity, preview landmarks
and rendered landmarks. M1.3 adds malformed orientation values 0/9/65535, a
dimension-header limit rejection before decode, ten repeated selections with
exactly one live object URL, zero after removal, and zero automatic POSTs.
URL counts are ownership evidence, **not a heap/RSS bound or garbage-collection
measurement**. Existing unit/browser suites cover decode timeout, stale responses,
abort, replacement/unmount, keyboard/touch, focus, axe and P12 retry.

The new HTTP matrix covers invalid decimal geometry, DPI/copy resource limits,
unsupported output, exponent/fractional/overflow crop fields, forbidden source
dimensions, does-not-fit, canvas resource limits and >1 MiB multipart input.
Problem response keys and status/code pairs are checked. Existing live tests
retain partial/duplicate/negative/zero/aspect-incompatible crop cases.

Development initially expected 400 for DPI=1201. Source inspection confirmed
`parse_sheet_plan` intentionally returns `SheetInputError::ResourceLimit` (413),
as does the copy-count envelope. The test expectation was corrected; production
behavior was not changed and the failed log is retained.

## Reproduction and evidence

Use supported Node/pnpm, the production Nuxt build and the existing local Rust
service. Supply only disposable local server credentials via private environment:
`NUXT_IMGENGINE_API_BASE` and `NUXT_IMGENGINE_RENDER_API_KEY`. Never use production.

```sh
pnpm exec nuxi prepare
pnpm test
pnpm run typecheck
pnpm run build
pnpm run test:browser
pnpm run test:browser:live
pnpm exec playwright test --config playwright.parity.config.ts
```

`PLAYWRIGHT_CHANNEL=chrome` selects an installed Chrome; otherwise install the
pinned Playwright Chromium. Set `PLAYWRIGHT_OUTPUT_DIR` and `PARITY_REPORT` to
external log paths. JSON report attachments contain per-case crop/rectangle,
dimensions, MAE/P99/maximum error, not source image bytes. Preserve failed logs.
Inspect only compact summaries and failed excerpts. Before each test capture
HEAD/base, branch diff, staged/unstaged and untracked paths.

Development measurements: 64 initial image cases plus the 16 fractional-zoom
cases passed their image/geometry limits. Across these cases, largest per-case
MAE was 1.858403 and largest P99 was 9. The exact committed revision's results
and CI URL belong in the PR/handoff; these development observations are not
blanket evidence for an untested revision.

Environment: Windows 11, Node 24.15.0, pnpm 10.34.9, Chrome 157.0.8103.78,
Playwright 1.58.2. Linux API is rebuilt from current unchanged Rust source against
the cached M1.1 native library (SHA-256
`927f11d819783167418d565dd977cf510a5303d9968742135dc64fec94520e91`). This supports
focused integration, not new native artifact provenance. Required seven-job PR
CI supplies the fresh full native/sanitizer/fuzz/container verification; it is
not duplicated manually. The new live browser suite is explicitly run locally;
existing CI is not claimed to run it and no workflow is changed.

## Review and remaining risk

Review spans test correctness and reproducibility, independent oracle design,
HTTP/ABI compatibility, malformed-input containment, URL ownership and affordable
fixture sizes. No customer images, new infrastructure, runtime dependencies or
native rewrites are introduced. Revert the tests/docs to roll back this change.
This is one agent's proportionate seven-level/security/SRE/product review, not
an independent audit. No server cost, latency or RSS improvement is claimed.

Coverage is bounded, not a proof for every image: high-frequency textures,
ICC/CMYK/color management, progressive-codec permutations, physical mobile,
Firefox/Safari and manual screen-reader coverage remain open. Smooth-reference
tolerances must not be reused blindly for those workloads. Aggregate process
memory, native concurrency C-A0 and P14 actual-host release evidence remain open.
Next action: review exact-SHA results and all seven CI checks; then a separate
cross-browser/color-management fixture extension. CI-SCOPE-01 remains separate.
No merge or deployment is authorized by passing this suite.
