#!/usr/bin/env bash

set -e

echo "🔍 Running L10 Include Guard Checker..."

FAIL=0

# ===== RULES =====
BUFFER_HEADER="core/buffer.h"
PIPELINE_HEADER="pipeline/pipeline_types.h"

# ===== SCAN ALL C FILES =====
FILES=$(find . -type f \( -name "*.c" -o -name "*.h" \))

for file in $FILES; do

    # Skip build dir
    if [[ "$file" == *"/build/"* ]]; then
        continue
    fi

    CONTENT=$(cat "$file")

    # =========================
    # RULE 1: img_buffer_t field access
    # =========================
    if grep -qE "->(width|height|stride|channels|data)" "$file"; then
        if ! grep -q "$BUFFER_HEADER" "$file"; then
            echo "❌ ERROR: $file uses img_buffer_t fields but missing #include \"$BUFFER_HEADER\""
            FAIL=1
        fi
    fi

    # =========================
    # RULE 2: img_pipeline_desc_t usage
    # =========================
    if grep -q "img_pipeline_desc_t" "$file"; then
        if ! grep -q "$PIPELINE_HEADER" "$file"; then
            echo "❌ ERROR: $file uses img_pipeline_desc_t but missing #include \"$PIPELINE_HEADER\""
            FAIL=1
        fi
    fi

    # =========================
    # RULE 3: Forbidden API type duplication
    # =========================
    if grep -q "typedef struct .*img_pipeline_desc_t" "$file"; then
        if [[ "$file" != *"pipeline_types.h"* ]]; then
            echo "❌ ERROR: $file illegally defines img_pipeline_desc_t (ONLY allowed in pipeline_types.h)"
            FAIL=1
        fi
    fi

    # =========================
    # RULE 4: buffer struct duplication
    # =========================
    if grep -q "struct img_buffer" "$file"; then
        if [[ "$file" != *"buffer.h"* ]]; then
            echo "❌ ERROR: $file illegally defines img_buffer (ONLY allowed in core/buffer.h)"
            FAIL=1
        fi
    fi

done

# ===== FINAL RESULT =====
if [ $FAIL -eq 1 ]; then
    echo ""
    echo "🚨 BUILD BLOCKED: Fix include violations"
    exit 1
else
    echo "✅ Include rules passed (L8 clean)"
fi