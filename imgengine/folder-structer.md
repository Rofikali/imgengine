### 📁 FOLDER STRUCTURE

    imgengine/
    │
    ├── CMakeLists.txt
    │
    ├── include/imgengine/
    │   ├── api.h
    │   ├── context.h
    │   ├── memory_pool.h
    │   ├── image.h
    │   ├── resize.h
    │   ├── crop.h
    │   ├── border.h
    │   └── layout.h
    |   └── common.h
    │
    ├── src/
    │   ├── core/
    │   │     memory_pool.c
    │   │     image.c
    │   │     context.c
    │   │     libt_avx2.c
    │   │
    │   ├── ops/
    │   │     resize.c
    │   │     crop.c
    │   │     border.c
    │   │
    │   ├── layout/
    │   │     grid_layout.c
    │   │     crop_marks.c
    │   │
    │   ├── io/
    │   │     stb_impl.c
    │   │     image_io.c
    |   |
    │   └── api/
    │         api.c
    │   └── pipeline/
    │         plugin_runner.c
    │
    |└── plugins/
    |   ├── plugin_registry.c
    |   ├── bleed_plugin.c
    |   └── crop_plugin.c
    |   └── plugin_registry.c
    |   └── registers_all.c
    |   └── pdf_plugin.c
    ├── tests/
    │   ├── assets/             # Minimal test images (e.g., 8x8, 16x16, 127x127)
    │   ├── unit/               # Pure logic (math, memory pool, SIMD)
    │   │   ├── test_mem_pool.c
    │   │   └── test_simd_ops.c
    │   ├── integration/        # Pipeline & Plugin flow
    │   │   └── test_pipeline.h
    │   ├── e2e/                # CLI interface & file I/O
    │   │   └── cli_suite.sh
    │   └── test_helpers.h      # Macros for assertions
    |
    ├── third_party/stb/
    │   ├── stb_image.h
    │   └── stb_image_write.h
    │
    └── cli/
        └── main.c

### FINAL ARCHITECTURE

    core/
    api.c

    layout/
    grid_layout.c

    plugins/
    plugin_registry.c
    bleed_plugin.c
    crop_plugin.c
    (future: cmyk, icc, pdf)

    pipeline/
    plugin_runner.c


### 🔥 3. IMPORTANT ORDER (CRITICAL)

    👉 Plugin execution order must be:

    1. layout_grid
    2. bleed plugin   ✅ FIRST
    3. crop marks     ✅ AFTER