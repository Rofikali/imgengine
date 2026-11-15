# Astra / Codex / human engineer — P13 M1.2 Nuxt crop editor handoff

**Status: READY TO START A BOUNDED FRONTEND IMPLEMENTATION, NOT A FINISHED EDITOR OR RELEASE.** Dated 2026-11-15. Inspected `main` at **`39e7099fbb31a2b879c4a9b2c22fca73de42419d`**, merge of [PR #43](https://github.com/Rofikali/imgengine/pull/43). PR #43 head: `3f7819c16a793500f0668dfdb91af961072646d6`; the [GitHub Actions run](https://github.com/Rofikali/imgengine/actions/runs/43683324448) reports **seven successful jobs** at that head. PR text records Ubuntu 24.04 authoritative verification at integrated `7ef95249a9068987338689eb6c665f5d876f8edc`. These are **recorded, already-run tests**, not newly executed for this docs-only handoff.

## 1. Start decision and hard boundary

**YES: M1.1 is merged, M1.2 is ready to implement.** The additive C ABI 2.3 normalized user crop, safe Rust FFI/supervisor/Axum validation and Nuxt same-origin **server-proxy transport** exist. P13 is **not end-to-end complete**: the browser-facing types, form serializer, workflow state, accessible editor and browser/native parity proof still need work.

**Do not start M1.1 again.** The normal M1.2 change is frontend-only under `imgengine-saas/frontend/`, using the existing same-origin endpoint `/api/sheet-render`; C17/Rust services are authoritative and unchanged. If the exact source shows an essential contract defect, produce a **separate** narrowly-scoped Rust/ABI proposal and its required tests; never quietly expand the UI PR into native surgery.

**M1.2 is server-rendered image cropping, NOT browser-only WebAssembly:** a local preview does not upload on every interaction, but choosing **Render** uploads the source photo to the existing private-key-protected Nuxt -> Rust -> C17 service. Distinguish this from planned M2+ browser-local no-upload free tools. Never advertise M1.2 as zero-bandwidth/local-only.

**Next after M1.2 is M1.3**, the separate exact-image parity, expanded P12 backward-compatibility, adversarial browser E2E and release-candidate verification. P14 live-VPS release readiness is a separate human-approved milestone; starting Astra does not imply production ready.

## 2. Mandatory document / source reading order

1. Root `AGENTS.md`, `next.md`, `skills.md`, [seven-level model](SEVEN_LEVEL_ENGINEERING_OPERATING_MODEL.md), [milestone template](AGENT_MILESTONE_HANDOFF_TEMPLATE.md), [Astra start](ASTRA_ENGINEERING_START_HERE.md), [technology policy](TECHNOLOGY_AND_REPOSITORY_POLICY.md), [scoped verification](CHANGE_SCOPED_VERIFICATION.md).
2. [frontend AGENTS](../imgengine-saas/frontend/AGENTS.md), [P13 contract](P13_INTERACTIVE_CROP_CONTRACT.md), [P12 baseline](P12_PHYSICAL_SHEET_CONTRACT.md), [Rust HTTP contract](api.md).
3. Source in current `main`: `imgengine-saas/frontend/components/SheetRenderPanel.vue`, `composables/useSheetRenderWorkflow.ts`, `composables/useImgengineApi.ts`, `types/api.ts`, `utils/render-form.ts`, `utils/sheet-crop.ts`, `server/api/sheet-render.post.ts` and related `*.test.ts`.
4. Inspect actual current HEAD/branch/worktree and updated source before any edit; do not assume this audit still matches a later commit.

## 3. Exact implementation gap — established by inspected frontend source

| Layer | Evidence / existing code | Actual M1.2 need |
| --- | --- | --- |
| **C ABI 2.3 / Rust FFI / Axum** | M1.1 merged PR #43 and P13/API contracts | **Do not rewrite** for an ordinary frontend change |
| **Nuxt server proxy** `server/api/sheet-render.post.ts` | Accepts optional four `crop_*` fields via `SHEET_CROP_FIELDS`, validates all-or-none and forwards to Rust with server-side API key | Preserve existing shape and byte/admission/error handling; no new route or browser key |
| **Proxy validator** `utils/sheet-crop.ts` | Validates complete fixed-point integer rectangles: `crop_x`, `crop_y`, `crop_width`, `crop_height`, scale 1,000,000 | Reuse/align contract; share pure geometry helpers only if safe and tested, never bypass backend authority |
| **Browser input type** `types/api.ts` | `SheetRenderInput` contains physical settings but **no optional crop rectangle** | Add backward-compatible optional **all-or-none** typed crop state; do not invent a new HTTP field |
| **Form serializer** `utils/render-form.ts` | `createSheetRenderForm` appends only P12 physical fields | Serialize exactly four crop text fields **only when explicit user crop is selected**; preserve omitted fields as P12 centered-fill |
| **Workflow** `composables/useSheetRenderWorkflow.ts` | Manages selected file, server result, abort and response preview URLs, but no user crop state | Add transient crop state, safe reset/file-change and stale-request behavior, cleanup, error announcements |
| **UI** `components/SheetRenderPanel.vue` | Physical-size/copy form with no crop editor | Add user-adjustable crop frame, drag/pointer, touch, keyboard/numeric alternative, bounded zoom and reset, accessible preview |
| **Tests** `utils/sheet-crop.test.ts`, `utils/render-form.test.ts` etc. | M1.1 proxy transport tests; no complete M1.2 browser interaction evidence | Add unit/component/contract tests, relevant browser/mobile/a11y evidence; leave expanded native parity for M1.3 |

**Warning:** A Nuxt proxy accepting crop fields is not proof the browser is sending them. **Do not mark M1.2 complete** until real user interactions produce correct multipart fields and server output.

## 4. Authoritative crop and compatibility contract

- Only these **optional** multipart fields exist: `crop_x`, `crop_y`, `crop_width`, `crop_height`. Values are unsigned decimal integers in **[0, 1,000,000]**; width and height > 0, and x+width/y+height <= 1,000,000. No browser-controlled scale field, file paths, image dimensions, pixel pointers or API credentials.
- **All four or none**. Absent crop = unchanged **P12 centered-fill** path. Complete crop = **P13 ABI 2.3**. Reset-to-default must explicitly omit crop fields when intended to return to centered-fill; do not send a guessed zero rectangle.
- The crop is a source-relative, orientation-corrected normalized rectangle, **not** absolute canvas coordinates. The preview must present the correct physical photo-cell aspect ratio, zoom/position bounds and accurately converted fixed-point values. Use checked integer rounding per [P13 normative geometry](P13_INTERACTIVE_CROP_CONTRACT.md). Treat EXIF orientation, extreme source aspect ratios, odd dimensions and edge crop as parity risks.
- Server/native validate the actual decoded image independently. Invalid HTTP crop -> **400 `invalid_request`**; decoded-source aspect incompatibility currently maps to **413 `resource_limit`**. Do not invent new error classes; show a meaningful accessible message without suppressing the status semantics.
- Preserve existing P5 render, P8 FIT, P12 physical sheet and all successful JPEG accounting headers (expected/planned/executed counts). No PDF/PNG sheet output, jobs, accounts, billing, arbitrary server fetch, durable image storage or local-only Wasm claims.
- Browser preview is **not the authoritative image**; the completed Rust/C17 JPEG result is. Do not upload image bytes for every mouse/touch gesture, and do not log source pixels, private filenames or credentials. A successful Render explicitly sends source image to server. Tell users accurately.
- Browser abort cancels client waiting/request where supported; it **does not guarantee cancellation of native processing**.

## 5. M1.2 LLD and user experience requirements

1. **State model:** idle/no file -> file validated -> image decoded and orientation accounted for -> crop ready (default centered-fill or selected) -> submitting -> success/failure/reset. Describe transition and cleanup. Keep image source ephemeral and avoid accidental retained `Blob`/`ImageBitmap` references.
2. **Editor geometry:** choose preview coordinate system and transformation matrix; represent selected crop rectangle deterministically at source-normalized scale, with one reusable conversion path. Prevent NaN, overflow, invalid extents and out-of-bounds dragging/zooming. Crop aspect must reflect actual photo width/height in mm, not paper dimensions.
3. **Interactions:** pointer drag (with Pointer Events/capture), touch pan/pinch only if warranted by testable scope, visible keyboard arrow controls or sliders with explicit step and focus semantics, bounded zoom, reset/default action, and a non-drag accessible alternative. Do not rely solely on touch gestures, color, or keyboard focus traps.
4. **Responsive design:** support representative desktop and narrow mobile viewports without fixed oversized canvases; respect reduced-motion; avoid introducing Tailwind/UI frameworks until a separate dependency/design review approves them. Existing custom CSS and Nuxt/Vue are sufficient for a bounded M1.2.
5. **Image fidelity:** proper local decode/orientation handling, bounded preview size, clip/mask overlay, represent black bars/letterboxing only where actual behavior permits, clear “preview vs server output” messaging. Avoid promising pixel-perfect results before M1.3.
6. **Cleanup/security:** revoke all `URL.createObjectURL` results when replaced/reset/unmounted; dispose/cancel active image decode if applicable; clear selected crop when source changes; avoid stale responses overwriting a newer selection; no image payload in telemetry; enforce current ~1 MiB encoded upload cap and reasonable preview dimension/memory thresholds without claiming a cap is native decode protection.
7. **UX errors:** file too large, decode invalid, extreme crop, server 400/413, unavailable/overloaded/timeouts, repeated submit, reset while pending. Provide `aria-live` announcements/focus recovery and user-visible action to retry.
8. **Frontend tests:** unit test coordinate conversion, normalization and edge/corner cases; `createSheetRenderForm` omitted/all-four behavior and exact typed strings; workflow cleanup/abort/stale callbacks; interaction keyboard/touch pointer accessibility; manual/automated browser checks on real fixtures. Avoid flaky heavy browser tooling if it is not already present—justify any package addition.
9. **Acceptance split:** M1.2 passes when UI can reliably send validated selected crop and visually inspect server output, with source-specific tests. **M1.3** owns comprehensive browser-to-C exact-image fixture parity, invalid/adversarial E2E and broad P12 regression/release candidate evidence; do not claim M1.3 complete from a component test.

## 6. Reviews by seven engineering levels plus security/finance/management

| Lens | Expected M1.2 review |
| --- | --- |
| Junior | Confirm current file/component structure, reproduce P12 baseline, add simple fixtures and scoped tests |
| Mid-Level | Deliver one complete crop editor journey and its typed form/workflow wiring |
| Senior | Review a11y/error paths, cleanup, crop math and maintainability |
| Staff | Safeguard existing Nuxt-Rust ABI P12/P13 boundary and typed API contracts |
| Senior Staff | Review M1.2 vs M1.3 dependencies, browser-test coverage and risk ownership |
| Principal | Compare editor math/state/UX implementation choices and avoid unwarranted dependencies or rewrite |
| Distinguished | Challenge whether the feature meaningfully improves trustworthy studio workflows and stays portable |
| Security/SRE | Prevent image/key leaks, malformed controls, memory exhaustion and invalid server-side assumptions; honor P14 |
| CA/MBA/Product | Define task-success/time-to-correct-sheet/retry metrics, accessibility cost and support burden; do not implement paywall, billing, ad SDK or telemetry collecting images |

## 7. Execution and explicit definition of done

- Before writing code: inspect latest SHA and scoped instructions, state a short HLD/LLD approach, risk/evidence gap, **exact changed files**, test plan, and what is deliberately out of scope.
- Implement on one dedicated **`feat/m1-2-interactive-crop-ui`-style branch** from latest main; do not touch legacy Python, Rust/C17 API symbols, infra, secrets or paid plans without separate approval. Small scoped commits/PR.
- Run the existing pnpm `test`, `typecheck`, `build` in `imgengine-saas/frontend/` and meaningful browser/keyboard/mobile tests at exact commit; `git diff --check`. Check current GitHub CI; **do not waive required CI**. Manual native C17/C11/ASan/fuzz is not necessary solely for a frontend change; rerun required shared-boundary tests if you change that boundary.
- Report tests **run and not run**, exact SHA/OS/browser/versions/fixtures, errors/failures, URL/worker memory observations, accessibility results and remaining M1.3 gaps.
- Reconcile P13 and `next.md` with source-backed **M1.2-specific** evidence; no “ready for production” or measured-cost claims. Draft/review PR; **do not merge, deploy or spend** without explicit human go/no-go.

### Copy/paste task for Astra (also valid for Codex / human engineer)

> You are implementing **IMGENGINE P13 M1.2 FRONTEND ONLY**, using the seven engineering levels (Junior through Distinguished) and Security/SRE/CTO/CA-MBA/Product reviews proportionately. Start with latest `Rofikali/imgengine` `main`; PR #43 is merged (M1.1 ABI 2.3 Rust FFI/Axum/supervisor/Nuxt proxy transport). Read `AGENTS.md`, `imgengine-saas/frontend/AGENTS.md`, `docs/ASTRA_ENGINEERING_START_HERE.md`, `docs/P13_INTERACTIVE_CROP_CONTRACT.md`, `docs/api.md`, `docs/ASTRA_P13_M1_2_FRONTEND_READY_HANDOFF_2026-11-15.md`, `next.md`, and all currently affected frontend source/tests. Before editing, give a source-backed short HLD/LLD, typed crop contract, privacy/resource/a11y threat review, acceptance matrix and smallest file diff plan. **M1.1 is done; do NOT reimplement it.** Add a keyboard/touch/pointer-accessible normalized crop UI with bounded zoom/reset, local preview and cleanup; wire the **still-unimplemented browser-facing** `SheetRenderInput`/`createSheetRenderForm` to the existing four optional `crop_*` fields, sending **all four only when explicitly selected**. Omission must preserve P12 centered-fill; no secrets in browser, no image uploads on drag, and rendering still intentionally uploads the image to the existing Rust/C17 endpoint. Preserve P5/P8/P12 and error mappings, verify unit/form/workflow/browser-a11y tests + pnpm typecheck/build and GitHub CI at the actual head, and update `next.md`/P13 with exact evidence. Keep comprehensive browser/native pixel parity for **M1.3**, and public release/P14, Rust/Wasm and pricing as separate milestones. Use one reviewed PR; **no merge, deployment, native changes or new services without separate explicit approval.**

**Source-backed conclusion:** You may start Astra on M1.2 immediately after accepting this handoff. This document is review guidance, not proof M1.2 was implemented or tested.
