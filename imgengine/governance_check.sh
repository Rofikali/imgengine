#!/usr/bin/env bash

set -e

echo "🔥 IMGENGINE GOVERNANCE CHECK START"

FAIL=0

# ===============================
# RULE 1: NO malloc in hot path
# ===============================
echo "[CHECK] No malloc in hot path..."

if grep -r "malloc\|calloc\|realloc" src/hot/; then
    echo "❌ ERROR: malloc used in hot path"
    FAIL=1
fi

# ===============================
# RULE 2: NO printf in hot path
# ===============================
echo "[CHECK] No printf in hot path..."

if grep -r "printf" src/hot/; then
    echo "❌ ERROR: printf used in hot path"
    FAIL=1
fi

# ===============================
# RULE 3: SECURITY VALIDATION
# ===============================
echo "[CHECK] Input validation present..."

if ! grep -r "img_security_validate_request" src/api/; then
    echo "❌ ERROR: Missing security validation in API"
    FAIL=1
fi

# ===============================
# RULE 4: FORBIDDEN DEPENDENCIES
# ===============================
echo "[CHECK] Layer violations..."

if grep -r "api/v1" src/runtime/; then
    echo "❌ ERROR: runtime depends on API layer"
    FAIL=1
fi

# ===============================
# RULE 5: PLUGIN ABI VERSION
# ===============================
echo "[CHECK] Plugin ABI version..."

if grep -r "img_plugin_descriptor" src/plugins/ | grep -v "IMG_PLUGIN_ABI_VERSION"; then
    echo "❌ ERROR: Plugin missing ABI version"
    FAIL=1
fi

# ===============================
# RULE 6: ATOMIC USAGE IN QUEUES
# ===============================
echo "[CHECK] Lock-free queues..."

if grep -r "pthread_mutex" src/runtime/queue; then
    echo "❌ ERROR: mutex used in lock-free queue"
    FAIL=1
fi

# ===============================
# RULE 7: HEADER GUARDS
# ===============================
echo "[CHECK] Header guards..."

if grep -r "#ifndef" include/ | wc -l | grep -q 0; then
    echo "❌ ERROR: Missing header guards"
    FAIL=1
fi

# ===============================
# RULE 8: ZERO COPY VIOLATION
# ===============================
echo "[CHECK] memcpy in plugins..."

if grep -r "memcpy" src/plugins/; then
    echo "⚠️ WARNING: memcpy used in plugin (verify zero-copy intent)"
fi

# ===============================
# RULE 9: SANDBOX REQUIRED
# ===============================
echo "[CHECK] Sandbox enabled..."

if ! grep -r "img_security_enter_sandbox" src/api/; then
    echo "❌ ERROR: sandbox not initialized"
    FAIL=1
fi

# ===============================
# FINAL RESULT
# ===============================
if [ "$FAIL" -eq 1 ]; then
    echo ""
    echo "🚨 GOVERNANCE CHECK FAILED"
    exit 1
else
    echo ""
    echo "✅ GOVERNANCE CHECK PASSED"
fi