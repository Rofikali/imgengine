# Exported Symbols — ABI v1

`imgengine/abi/exported_symbols.json` is the canonical, version-controlled ABI manifest. `imgengine/scripts/check_exported_symbols.py` compares every dynamic export of `libimgengine.so` against it; unexpected exports fail verification.

| Symbol |
|---|
| `imgengine_abi_version` |
| `imgengine_status_message` |
| `imgengine_engine_create` |
| `imgengine_engine_destroy` |
| `imgengine_capability_supported` |
| `imgengine_process_encoded_image_to_jpeg` |
| `imgengine_output_release` |

All symbols are versioned `IMGENGINE_1.0`. Internal scheduler, allocator, codec, SIMD, logging, and legacy API symbols are deliberately hidden.
