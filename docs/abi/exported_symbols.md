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

Internal scheduler, allocator, codec, SIMD, logging, and legacy API symbols
remain deliberately hidden. The checker treats only `IMGENGINE_*` version-node
labels as metadata and compares every function/data export against the manifest.
