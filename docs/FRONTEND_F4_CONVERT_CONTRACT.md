# F4 — Browser-local image Convert contract

**Status:** implemented and merged in [PR #76](https://github.com/Rofikali/imgengine/pull/76) as `9216306c93dd21c5011992b11ed68dc1051ac12d`; its exact head passed required CI. This does not release IMGENGINE Free.

F4 converts one still JPEG or PNG to a newly encoded JPEG or PNG entirely in the browser. No original bytes, output bytes, filename, EXIF, request body, telemetry, storage, Rust API, or server fallback is used.

## Inputs and bounds

- Accept only JPEG and PNG after bounded signature/header inspection and browser decode.
- Reject empty files, files over 4 MiB, dimensions over 8192 per side, animated PNG, and decoded rasters over 4 megapixels.
- The output retains stored-raster dimensions. It intentionally does not apply EXIF orientation.

## Output behavior

- User chooses PNG or JPEG. Unsupported choices fail closed.
- Re-encoding strips EXIF and other source metadata. Color output can vary by browser codec.
- PNG preserves alpha. JPEG composites transparent source pixels over opaque white.
- Require a nonempty requested-MIME Blob no larger than 20 MiB before publishing a preview/download.
- Object URLs, temporary image elements, canvases, timeout handles, and state are released on replacement, cancellation, failure, and unmount.

## UX and verification

The UI is keyboard usable, names the output format, provides actionable errors and cancel/remove controls, and does not invent server progress. Tests cover format validation, alpha behavior, metadata removal, resource cleanup, bounded/invalid input, accessibility, responsive layout, download name, and no image-bearing network or browser storage. Required repository CI remains unchanged.
