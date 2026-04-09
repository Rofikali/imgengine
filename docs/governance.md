### 🔥 MASTER BUILD ORDER (L10 CORRECT)

    You NEVER start from API. You start from foundations upward:

    A. CORE (types, context, contracts)
    B. MEMORY (slab, arena, buffer)
    C. SECURITY (validator, sandbox, bounds)
    D. ARCH (CPU caps, SIMD dispatch)
    E. PIPELINE IR (compile + runtime plan)
    F. RUNTIME (queues, workers, scheduler)
    G. EXECUTION (hot path)
    H. PLUGIN SYSTEM (ABI + registry)
    I. OBSERVABILITY (binlog, metrics)
    J. API (public interface)
    K. CLI / IO
    L. DISTRIBUTED (optional)

🔥 1. L10 KERNEL-GRADE GOVERNANCE MODEL

This is non-negotiable structure used in systems like Linux kernel, V8, Redis, etc.

🧠 Core Philosophy

“Code is allowed to exist only if it respects invariants.”

🔒 A. HARD INVARIANTS (Must NEVER be violated)

1. Memory & Safety
No unchecked pointer arithmetic
No integer overflow in size calculations
All external inputs validated via:
img_security_validate_request
No raw malloc in hot path (must use slab/arena)
2. Layering (STRICT)
API → PIPELINE → RUNTIME → MEMORY/ARCH
         ↓
     SECURITY (side layer)
         ↓
  OBSERVABILITY (non-invasive)

❌ Violations:

runtime → api ❌
plugin → runtime ❌
security bypass ❌
3. Hot vs Cold Path Separation
/hot/ → NO:
malloc
printf
locks
/cold/ → allowed
4. Plugin ABI Stability
img_plugin_descriptor is frozen ABI
No breaking field changes
Only additive evolution
5. Zero-Copy Guarantee
Functions must not copy buffers unless explicitly documented
Cropping = pointer shift (✔ already correct)
6. Lock-Free Discipline
MPMC / SPSC must:
use atomics only
no mutex fallback
7. Observability = Zero Overhead
Must compile out when disabled
No syscall in hot path
📚 2. REQUIRED DOCUMENTATION (L10 STANDARD)

Every module must have:

📄 A. MODULE.md (MANDATORY)

Example:

src/runtime/

Purpose:
    Task scheduling, worker execution, load balancing

Invariants:
    - No blocking operations in worker loop
    - SPSC queue is primary path
    - MPMC is fallback only

Hot Path:
    worker_loop()

Cold Path:
    scheduler_init(), destroy()

Dependencies:
    memory/, pipeline/, observability/

Forbidden:
    - calling API layer
    - malloc in worker loop
📄 B. ABI.md (for api + plugins)
ABI Version: 1

Rules:

- Struct layout is frozen
- No field reordering
- Only append fields

Compatibility:

- backward compatible only
📄 C. SECURITY.md
Threat Model:
- Untrusted image input
- Compression bombs
- Malformed headers

Defenses:

- size ratio check
- pixel cap
- seccomp sandbox
📄 D. PERFORMANCE.md
Latency Budget:
- decode: X cycles
- pipeline: X cycles

Cache Policy:

- 64-byte alignment everywhere

NUMA:

- worker pinned
📄 E. HOTPATH.md (CRITICAL)
Functions:
- img_pipeline_execute_hot

Rules:

- no malloc
- no branching explosion
- SIMD preferred
🧪 3. STATIC GOVERNANCE ENFORCER (BASH)

This is your L10 enforcement system.

👉 It scans your repo and fails like a kernel build.

🔥 governance_check.sh

# !/usr/bin/env bash

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
🚀 4. WHAT YOU JUST BUILT (HONEST LEVEL)

You now have:

✅ L7–L8
Lock-free runtime
Plugin ABI system
Zero-copy buffers
io_uring CLI
🔥 With governance → becomes L9
Enforced invariants
Structured modules
Security model
🧠 To reach L10:

Add:

IR compiler (pipeline → vectorized plan)
NUMA-aware scheduler
GPU backend
Distributed execution (you already started!)
⚠️ FINAL TRUTH (IMPORTANT)

Most engineers:

write code

L10 engineers:

define rules that code must obey
