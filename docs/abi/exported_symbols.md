# Exported Symbols — ABI v1 and additive ABI v2

`imgengine/abi/exported_symbols.json` is the canonical, version-controlled ABI manifest. `imgengine/scripts/check_exported_symbols.py` compares every dynamic export of `libimgengine.so` against it; unexpected exports fail verification.

## ABI v1 — unchanged

The original seven ABI-v1 symbols remain versioned `IMGENGINE_1.0` and retain
their existing contract.

| Symbol |
|---|
| `imgengine_abi_version` |
| `imgengine_status_message` |
| `imgengine_engine_create` |
| `imgengine_engine_destroy` |
| `imgengine_capability_supported` |
| `imgengine_process_encoded_image_to_jpeg` |
| `imgengine_output_release` |

## ABI v2 — layout render capability

The two additive symbols below are versioned `IMGENGINE_2.0`. They accept the
same opaque engine created by ABI v1, but the layout request/output structures
and ownership contract are defined separately in
`imgengine/api/v2/layout.h`. This addition does not alter ABI v1 or expose a
scheduler, allocator, or renderer implementation type.

| Symbol |
|---|
| `imgengine_layout_render_v2` |
| `imgengine_layout_output_release` |

## ABI v2.1 â€” bounded layout output capability

`imgengine_layout_render_v2_bounded` is the sole additive `IMGENGINE_2.1`
symbol. It leaves ABI v1 and the existing unbounded v2 operation unchanged.
The caller supplies a server-owned worst-case JPEG destination budget; a budget
below TurboJPEG's conservative destination bound returns the existing stable
`IMGENGINE_STATUS_RESOURCE_LIMIT` with an empty output. On success the output
uses the existing `imgengine_layout_output_release()` ownership contract.

| Symbol |
|---|
| `imgengine_layout_render_v2_bounded` |

Internal scheduler, allocator, codec, SIMD, logging, and legacy API symbols
remain deliberately hidden. The checker treats only `IMGENGINE_*` version-node
labels as metadata and compares every function/data export against the manifest.


## ABI v2.2 — bounded physical-sheet render capability

`imgengine_sheet_render_v1_bounded` is the sole additive `IMGENGINE_2.2`
symbol. It accepts a separately size/versioned sheet request, derives centered
crop geometry from the decoded source in C, and uses the existing v2 output
structure/release function. It does not change the FIT-only v2.0/v2.1 request
or their 4 MP resource limit.

| Symbol |
|---|
| `imgengine_sheet_render_v1_bounded` |

The export checker now validates both the exact exported symbol set and each
symbol's ELF version label against this manifest.
