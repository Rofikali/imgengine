# F2b — browser-local Rotate

**State:** local implementation ready for review; no PR, merge, deployment, or GitHub CI claim yet.

F2b adds `/rotate-image` to **PRODUCT-FREE-01**. It processes one JPEG or PNG entirely in the browser and exports a PNG. It does not call a Nuxt proxy, Rust, C17, or a server endpoint.

## Contract

- The only transforms are `0`, `90`, `180`, and `270` degrees clockwise. Each output is derived from the original selected source, never from a previous export.
- A quarter-turn swaps width and height. The output remains bounded to 4 MiB input, 4 megapixels, and 4096 pixels per output edge; a source that would exceed the output bound is rejected rather than silently rescaled.
- `90`, `180`, and `270` use the shared local Worker and an `OffscreenCanvas`; output is a re-encoded PNG. Fully transparent pixels retain alpha, while browsers may normalize their invisible RGB channels during canvas readback.
- The tool uses decoded stored raster pixels. EXIF orientation is deliberately not interpreted as an automatic transform; explicit JPEG and PNG orientation fixtures confirm the displayed source geometry and output dimensions. Original metadata is not retained.
- Object URLs, Worker instances, decoded bitmaps, and canvases are released on replacement, cancellation, removal, errors, and route exit. No local/session/IndexedDB state is written and no image request is made.

## Acceptance evidence required for the review PR

- Unit validation for valid/invalid angles and width/height swapping.
- Desktop and mobile browser tests for asymmetric pixel mapping, alpha, dimensions, stored-raster EXIF behavior, invalid metadata, output bounds, cancellation, cleanup, keyboard-accessible controls, no-upload/privacy storage, responsive layout, and axe WCAG checks.
- Resize and Crop browser regressions because the Worker/composable message contract is shared.
- Nuxt typecheck and production build. The existing seven GitHub Actions jobs remain required on the exact PR head; they are not duplicated locally.

## Remaining boundaries

This is a browser implementation, not a claim of pixel identity across all browser engines, preservation of JPEG metadata, server-side processing, or a released Free product. Before a public release, the other three Free tools and independent product, hosting, legal, privacy, security, and owner-approval gates remain open.
