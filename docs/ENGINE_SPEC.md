# IMGENGINE Native Engine Specification

**Status:** Living contract  
**Applies to:** C public API, CLI, bindings, worker invocation, and native tests.

## 1. Purpose

The native engine converts one source image into a print-ready sheet. It owns decode, center crop/fill or fit scaling, grid placement, borders, bleed, crop marks, and encoding. It is invoked through the CLI in the initial SaaS release and may later be called through the stable C API.

The engine is not responsible for authentication, tenant policy, browser uploads, persistence, retries, or billing.

## 2. Canonical Layout Model

`img_job_t` in `imgengine/include/pipeline/job.h` is the canonical native job descriptor. Its current ABI version is `2`.

| Field | Unit | Meaning | Current default |
| --- | --- | --- | --- |
| `photo_w_cm` | cm | Printed width of one photo cell. | `4.5` |
| `photo_h_cm` | cm | Printed height of one photo cell. | `3.5` |
| `dpi` | dots/inch | Print resolution used to derive photo pixels. | `300` |
| `cols`, `rows` | count | Number of cells horizontally and vertically. | `2`, `3` |
| `gap`, `padding` | px | Inter-cell separation and outer canvas margin. | `15`, `20` |
| `border_px` | px | Border drawn around every rendered cell. | `2` |
| `bleed_px` | px | Extra safe image area for physical cutting. | `10` |
| `crop_mark_px` | px | Crop-mark segment length. | `20` |
| `crop_thickness` | px | Crop-mark line thickness. | `2` |
| `crop_offset_px` | px | Distance of crop mark from image edge. | `8` |
| `mode` | enum | `IMG_FILL` crops to fill the cell; `IMG_FIT` preserves full image inside it. | `IMG_FILL` |
| `bg_r`, `bg_g`, `bg_b` | 0–255 | Canvas background color. | `255`, `255`, `255` |

### Dimension Conversion

For a printed cell, derive pixel dimensions as follows before layout:

```text
photo_width_px  = round(photo_w_cm / 2.54 × dpi)
photo_height_px = round(photo_h_cm / 2.54 × dpi)
```

The layout must reject any request whose derived dimensions, row/column count, padding, gap, border, or bleed would overflow the output buffer or exceed configured resource limits.

## 3. Render Semantics

1. Decode a supported source image.
2. Validate decoded dimensions, pixel format, and arithmetic bounds.
3. Apply `FIT` or `FILL` to produce a cell-sized image. `FILL` center-crops excess content; `FIT` leaves background where aspect ratios differ.
4. Place cells in row-major order inside the grid.
5. Apply each cell border.
6. Render bleed and crop marks according to configured safety parameters.
7. Encode output based on the output-path extension.

The public C API documents the complete order as decode → resize → grid layout → bleed → crop marks → border → encode. Any change to this order is a behavior change and requires regression fixtures.

## 4. Supported Interfaces

### C API — implemented

- `img_api_init(workers)` creates engine resources.
- `img_api_run_job(engine, input_path, output_path, job)` runs the full file-to-file pipeline.
- `img_api_run_job_raw(...)` returns encoded output bytes.
- `img_api_run_job_rgb24(...)` accepts an already decoded RGB24 frame.
- `img_api_shutdown(engine)` drains and releases the engine.

The C API must return `IMG_ERR_*` values rather than terminate the process for user input failures.

### CLI — implemented contract

```text
imgengine_cli --input <file> [--output <file>] [options]
```

| CLI option | Maps to |
| --- | --- |
| `--width`, `--height`, `--dpi` | cell size and print resolution |
| `--cols`, `--rows`, `--gap`, `--padding` | grid configuration |
| `--border`, `--bleed`, `--crop-mark`, `--crop-thickness`, `--crop-offset` | print-safety configuration |
| `--input-format raw-rgb24`, `--input-width`, `--input-height`, `--input-stride` | pre-decoded raw ingress |

The CLI help currently declares JPEG/PNG input and JPEG/PNG/PDF output by extension. Each format must be covered by a smoke fixture before it is exposed in the SaaS UI.

## 5. Built-In Templates

The native engine currently names three templates:

| Template ID | Intended layout |
| --- | --- |
| `IMG_JOB_TEMPLATE_PASSPORT_45X35` | 4.5 × 3.5 cm, 6 × 6 grid, fill mode. |
| `IMG_JOB_TEMPLATE_PASSPORT_38X35` | 3.8 × 3.5 cm, 6 × 6 grid, fill mode. |
| `IMG_JOB_TEMPLATE_PRINTREADY_6X6` | 6 × 6 print-ready grid. |

Templates are native defaults, not legal claims of country-specific passport compliance. Country rules must be versioned, reviewed presets before commercial use.

## 6. Performance and Correctness Boundaries

The RFC target is `<2 ms` for a defined 4K render-only native benchmark. It does **not** include upload, queue latency, decode of arbitrary untrusted input, storage, API work, or browser time.

Native hot-path rules:

- no heap allocation, file I/O, syscalls, logging, or locks in the defined execution hot path;
- scalar fallback must remain behaviorally compatible with SIMD implementations;
- validation occurs before ownership transfer and before pixel processing;
- public ABI additions/changes require symbol audit and ABI-version review.

## 7. Portability Contract

Linux is the deployment target. Windows developer support is incomplete until the POSIX `mmap` use in `src/api/api_file_mmap.c` has a `CreateFileMapping`/`MapViewOfFile` implementation or an explicitly tested alternative. NASM is required for JPEG SIMD dependency builds where SIMD is expected.

## 8. Native Acceptance Matrix

| Case | Required evidence |
| --- | --- |
| JPEG → PNG sheet | Build test plus image dimensions/layout fixture. |
| PNG → PNG sheet | Build test plus image dimensions/layout fixture. |
| PDF output | Valid PDF open/parse smoke test. |
| FIT and FILL | `regression_geometry` uses a deterministic wide RGB fixture and proves the two modes produce different A4 rasters. |
| Border, bleed, crop marks | `regression_geometry` proves each enabled control changes the expected raster while preserving A4 dimensions. |
| Layout bounds | `layout_properties` executes 1,000 deterministic combinations of DPI, grid, gap, padding, and photo dimensions; every computed cell grid must remain inside A4 bounds. |
| Corrupt input | `regression_security` verifies malformed data returns `IMG_ERR_FORMAT`, unsafe dimensions return `IMG_ERR_SECURITY`, and neither creates an output artifact. |
| No SIMD support | CI compares the optimized binary with a portable baseline build that excludes AVX objects and requires pixel-equivalent output. A direct AVX2/scalar CTest validates the resize-kernel arithmetic before AVX2 dispatch is released. |
| Sanitizers | ASan/UBSan clean native test run on supported platform. |
| Memory and arithmetic boundaries | Pure-C CTest covers validation and slab exhaustion; bounded libFuzzer covers the untrusted dimension/file-size parser plus real JPEG/PNG decoder dispatch with ASan/UBSan. |
