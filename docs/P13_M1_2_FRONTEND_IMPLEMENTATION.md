# P13 M1.2 frontend implementation and verification

Implementation branch: `codex/p13-m1-2-crop-editor`, based on
`d73e321566c1cd58854a03b4ea051a6a9f3cb322` (PR #45 merged).
Status: merged in PR #47 as `de8b1708ba4e54cd219c93cc15f7923773ce8c9e`. All seven CI jobs passed at PR head `b3aa42c2b89b0831a48448e8c4529cd67131ed75` ([CI](https://github.com/Rofikali/imgengine/actions/runs/43808582807)). This is not a production release.
The final handoff records the exact tested implementation SHA and log location.

## Design and ownership

`SheetRenderInput.crop` is one optional complete rectangle. The serializer
validates and writes precisely four unsigned integer text fields, or omits all
four. Existing Nuxt proxy, Rust API, FFI and C code are unchanged. Resetting the
crop or changing photo dimensions/DPI selects the existing P12 centered-fill path.

`SheetCropEditor.vue` owns presentation and pointer capture. Native range inputs
provide a non-drag, touch and keyboard alternative; the focusable frame supports
arrows (1% step) and Shift+arrow (10%). Zoom is 1–4×. A responsive clipped image
shows the selected region without animation. Pinch gestures are not implemented.
Success moves focus to the result heading; removal returns focus to file selection.

`crop-editor.ts` computes the preview cell aspect using the selected millimetres
and DPI with integer half-up raster rounding. It picks an integer source-pixel
rectangle, then rounds normalized left/top inward with ceil and right/bottom
inward with floor. Native floor/ceil reconstruction recovers those integer edges.
The tests check the native inequality
`abs(crop_width * target_height - crop_height * target_width) <= target_width + target_height`.
No source dimensions, scale, credentials, paths or resource limits enter HTTP.
The server remains authoritative; preview is approximate, especially at tiny cells.

The workflow owns source preview, crop selection, completed JPEG and request
controllers. Selection validates the existing encoded-byte/type limits, preflights
JPEG/PNG dimensions, then decodes locally. An in-flight decode blocks submission.
Preview is capped at 16 million pixels and 8192 per edge, with a ten-second decode
timeout. This is a browser UX guard, not a native decoder/RSS security guarantee.
Reset, replacement, unmount, render success and render failure release source
URLs. Stale decodes dispose their result; stale network responses cannot replace
new state. Failure clears the explicit selection and explains that retry uses
centered fill; the same photo can be selected again to reopen the editor.
Request abort stops browser waiting and does not promise native cancellation.

## Orientation contract resolved without backend changes

M1.2 coordinates and output use the **stored raster**, ignoring EXIF orientation.
This explicitly supersedes the original proposal's orientation-corrected-source
assumption. C17/Rust behavior and P12 compatibility remain unchanged.
The browser removes JPEG EXIF segments or PNG eXIf chunks only from an ephemeral
preview copy. It does not rotate/re-encode pixels or modify uploaded file bytes.
All eight valid orientation values, including mirrored values, are accepted;
malformed metadata fails closed. The editor explains stored orientation and tells
users to export upright when they want rotated output. Automatic upright rendering
is a separate future product capability, not claimed by this contract.

Live acceptance covers orientations 1–8 in JPEG and PNG, alternating TIFF byte
order: preview dimensions and red/blue landmarks, byte-identical original upload,
and the real Rust/C right-hand crop producing a blue JPEG. Comprehensive M1.3
pixel parity and cross-browser/manual screen-reader testing remain separate gates.

## Verification lanes and reproduction

Class: frontend UI + HTTP serialization + development-only browser dependencies.
Before each test, record branch, exact HEAD, merge base, branch diff, staged and
unstaged diffs and untracked files. Keep full logs outside the repository.

From `imgengine-saas/frontend` with supported Node and pnpm 10.34.9:

```sh
pnpm install --frozen-lockfile
pnpm exec nuxi prepare
pnpm exec vitest run utils/crop-editor.test.ts utils/crop-preview.test.ts utils/render-form.test.ts utils/sheet-crop.test.ts composables/useSheetRenderWorkflow.test.ts
pnpm test
pnpm run typecheck
pnpm run build
pnpm exec playwright install chromium
pnpm run test:browser
```

An installed Chrome can be selected with `PLAYWRIGHT_CHANNEL=chrome`.
`PLAYWRIGHT_OUTPUT_DIR` places artifacts outside the worktree. Tests use synthetic
401×203 red/blue PNGs, desktop Chromium and Pixel 7 touch/viewport emulation.
They check keyboard, actual pointer/touch events, sliders, reset/aspect changes,
no POST while editing, no browser API key, multipart shape, 413 recovery,
same-file reselection, focus and URL counts on replacement/reset/unmount.
Axe checks WCAG 2 A/AA and 2.1 AA rules for the sheet panel. This is not a manual
screen-reader audit or proof on physical mobile devices, Firefox or Safari.

For live integration, run the existing Rust API locally with a disposable test
credential, set `NUXT_IMGENGINE_API_BASE` and `NUXT_IMGENGINE_RENDER_API_KEY` in
the test shell, and run `pnpm run test:browser:live`. The config starts the built
Nuxt server at loopback port 3100. The test forwards real requests to Rust,
verifies decoded JPEG dimensions/counts for explicit and omitted crop, and checks
partial, negative, zero, duplicate and aspect-incompatible requests (400/413).
Synthetic result screenshots are saved for visual inspection. Never point this
test at a public/production service. `test:browser` uses mocked failures and
does not require the Rust service; the separate live suite is an additional gate.

Focused Rust command (Linux, unchanged native link prerequisite):

```sh
cargo test --offline --manifest-path backend/imgengine-api/Cargo.toml physical_sheet -- --test-threads=1
```

Local verification used Windows 11, Node 24.15.0, pnpm 10.34.9, Chrome
157.0.8103.78, Playwright 1.58.2 and axe-core 4.11.4. The Rust tests use the existing
Ubuntu cached M1.1 library, SHA-256
`927f11d819783167418d565dd977cf510a5303d9968742135dc64fec94520e91`.
This reuse supports focused contract testing; it is not a fresh exact-candidate
native build/provenance certificate. Full native C17/C11, ASan/UBSan, fuzz,
Ubuntu verifier and release/host suites are deliberately NOT RUN manually.
All seven existing GitHub CI jobs remain unchanged and mandatory.

Dependency audit found zero high/critical advisories, but one low (`deepmerge`)
and two moderate (`glob`, `minimatch`) findings in the pre-existing dependency
graph. These remain release-disposition items; the new Playwright/axe packages
are not the reported vulnerable packages. The lockfile only adds their test
dependency graph; it does not update the existing Nuxt dependency versions.

Initial failures: Vitest could not load `.nuxt/tsconfig.json` before `nuxi prepare`;
no test bodies ran. The first live test used Chromium's unavailable unintercepted
multipart body; observing and continuing the real request fixed the harness.
Neither failed run is passing evidence. A stalled bundled-browser download was
cancelled; the installed Chrome was used and its version recorded above.

## Review, cost and next gates

Junior/Mid-level review: complete typed selection-to-result journey and negative
fixtures. Senior: cleanup, cancellation races, keyboard/error recovery and integer
edges. Staff: P12 omission and untouched M1.1/ABI boundary. Senior Staff: scoped
M1.2 evidence versus pending M1.3/CI/release gates. Principal/Distinguished:
no editor framework, new service, storage or native rewrite; one reversible
frontend change. Security/SRE: header preflight, bounded decode lifetime, no
image telemetry or exposed credential, explicit stored-raster contract and no native
cancellation claim. These are one agent's review lenses, not independent reviewers.

Playwright and axe are development-only additions because existing Vitest cannot
exercise browser pointer capture, touch, focus, native sliders or accessibility
trees. They add test installation/maintenance cost, not client runtime dependencies.
Editing makes no server request; one intentional Render invokes the existing
service. User task success/retry reduction, aggregate browser/native memory and
cost per successful sheet are NOT MEASURED. No revenue or cost-saving claim is made.

Rollback: revert this frontend change; omission still selects P12. No persistent
data migration or backend rollout is needed. Keep
full M1.3 parity, dependency disposition, C-A0 and P14 release gates open.

Next bounded milestone: M1.3 image-parity fixture matrix and expanded orientation regressions, after review/required CI. Separately propose CI-SCOPE-01 dry-run path
classification (known, mixed, renamed/deleted and unknown paths; fail closed),
keeping all seven jobs running. No selective skipping, branch-rule change,
merge or deployment is authorized by this implementation.
