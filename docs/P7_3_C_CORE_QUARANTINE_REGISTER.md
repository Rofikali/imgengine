# P7.3 C-Core Quarantine Register

| ID | Component | Current callers / reachability | Risk and evidence | Decision / removal condition |
| --- | --- | --- | --- | --- |
| Q-01 | `img_api_decode_input` / `img_api_release_input` | legacy `img_api_begin_job`; not selected by P7.2.8 | releases input with `munmap`; incompatible with borrowed HTTP/Rust bytes | QUARANTINE for borrowed input. Retain for proven mmap callers; remove only after caller/build/CLI replacement and rollback proof. |
| Q-02 | `api_file_mmap` and legacy CLI mmap paths | legacy file-oriented API/CLI | path/file ownership differs from API-byte contract | ADAPT. Document separate ownership; no reuse for HTTP. |
| Q-03 | float `img_layout_prepare_fit` | historical grid preparation | float geometry is not frozen P7.2.8 semantics | REQUIRES ADAPTER. Keep legacy support; private capability uses checked integer FIT. |
| Q-04 | `img_layout_grid_place_cells` | historical grid path | may clip / historical partial-success semantics conflict with exact completion | QUARANTINE for new explicit plans. Remove only after legacy layout retirement. |
| Q-05 | `img_blit_avx2` | legacy and P7.2.8 after validation | clips rather than expresses invalid bounds | ADAPT: only call after checked rectangle/source validation. |
| Q-06 | commented alternative worker/scheduler code | comments only | conflicting historical behavior can mislead maintenance | REMOVE-CANDIDATE documentation debris; verify no generated/source dependency before deletion in a separate cleanup scope. |
| Q-07 | SIMD grayscale / fused paths | hardware registry and plugin registry both populate `OP_GRAYSCALE` dispatch | P7.4-A corrected the architecture/fused entries, but P7.5 source tracing found `plugin_grayscale.c` registers a later byte-wise implementation that is not RGB-luma equivalent | BLOCKED / HARDEN: preserve the finding; no new grayscale caller or optimization. A separately authorized dispatch-wide RGB-oracle remediation gate must cover the plugin route before this item can be closed. |
| Q-08 | `queue_mpmc.c` stub and split queue implementation | build/source archaeology | topology is spread across one-function translation units | RESEARCH; inventory symbols/build references before consolidation. |

Quarantine means no new caller or public exposure without a dedicated adapter,
ownership contract, and regression evidence. It is not authorization to delete.
