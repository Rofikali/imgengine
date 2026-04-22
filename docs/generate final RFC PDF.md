# RFC: imgengine v1.0 (FINAL — KERNEL-GRADE)

**Title:** High-Performance, Deterministic Image Processing Engine
**Author:** Principal Engineering Track
**Status:** Final
**Date:** 2026-03-30

---

# 1. Overview

## 1.1 Problem Statement

Existing image processing systems (e.g., ImageMagick-class tools) suffer from:

* Unpredictable latency due to heap allocation and fragmentation
* Scalar execution models with limited SIMD utilization
* Unsafe parsing surfaces (RCE risk)
* Poor suitability for high-throughput, multi-tenant SaaS environments

## 1.2 Objective

Design and implement a **deterministic, zero-allocation, SIMD-accelerated image engine** with:

* Sub-2ms latency per 4K image
* 100K+ operations/sec throughput
* Kernel-grade memory safety
* Stable P99 latency under load

## 1.3 Non-Goals

* GPU-first execution (CPU-first design)
* GUI or interactive editing tools
* General-purpose image editing workflows

---

# 2. SLO / KPIs

| Metric      | Target       |
| ----------- | ------------ |
| P50 Latency | < 1 ms       |
| P99 Latency | < 2 ms       |
| Throughput  | 100K ops/sec |
| Error Rate  | < 0.01%      |
| Allocations | 0 (hot path) |

---

# 3. Execution Flow (Kernel Model)

```
USER
 ↓
cmd
 ↓
api          (validation + IO boundary)
 ↓
runtime      (scheduler + orchestration)
 ↓
pipeline     (pure execution engine)
 ↓
arch         (SIMD kernels)
 ↓
memory       (allocation system)
 ↓
core         (types only)
```

---

# 4. Core Design Principles

1. **Strict Downward Dependency DAG**
2. **Hot Path Purity (Zero Side Effects)**
3. **Deterministic Execution**
4. **Data-Oriented Design (DOD)**
5. **Separation of Control vs Execution**
6. **Kernel-Grade Isolation of Subsystems**

---

# 5. Dependency Rules (MANDATORY)

## 5.1 Main Flow Constraint

```
cmd → api → runtime → pipeline → arch → memory → core
```

## 5.2 Hard Rules

* No upward dependencies
* Pipeline must NEVER call:

  * IO
  * Plugins
  * Security
* Runtime is the ONLY execution controller
* API is the ONLY IO boundary

---

# 6. Side System Rules (STRICT)

| System        | Rule                                         |
| ------------- | -------------------------------------------- |
| security      | Runs BEFORE api/runtime only                 |
| io            | Accessible ONLY from API                     |
| plugins       | Controlled ONLY by runtime                   |
| observability | Passive (write-only, no influence)           |
| startup       | Allowed to touch everything (ignored in DAG) |

---

# 7. Layer Responsibilities

## 7.1 core/

* Pure type definitions
* No logic
* No allocation

## 7.2 memory/

* Slab allocator
* Arena allocator
* NUMA-aware allocation
* Provides all memory to upper layers

## 7.3 arch/

* SIMD kernels (AVX2, AVX-512, NEON)
* No branching in hot loops
* No allocation

## 7.4 pipeline/

* Executes precomputed operations
* Jump-table dispatch
* No IO, no plugins, no allocation
* Deterministic execution only

## 7.5 runtime/

* Scheduler
* Worker threads
* Queue management
* Plugin execution
* Controls entire execution flow

## 7.6 api/

* Public contract
* Input validation
* IO boundary
* Job orchestration entry

## 7.7 cmd/

* CLI interface
* Argument parsing only

## 7.8 io/

* Pure decode/encode logic
* No file/network operations

## 7.9 vfs/ (Platform Layer)

* File IO
* Network IO
* S3/HTTP adapters ( not now )

## 7.10 plugins/

* Static extension logic
* Registered at startup
* Executed ONLY via runtime

## 7.11 observability/

* Metrics, logging, tracing
* Lock-free, non-blocking
* No effect on execution

## 7.12 security/

* Input validation (API entry)
* Sandbox (startup only)
* Decoder bounds enforcement

---

# 8. Memory Model (CRITICAL)

## 8.1 Architecture

```
GLOBAL (NUMA / HugePages)
 ↓
THREAD-LOCAL (Slab / Arena)
 ↓
HOT PATH (Zero-copy buffers)
```

## 8.2 Guarantees

* O(1) allocation
* Zero fragmentation
* No malloc/free in hot path
* Cache-aligned memory

---

# 9. Execution Model

* Worker-per-core architecture
* CPU affinity (thread pinning)
* Lock-free SPSC queues
* Batch processing (32–128 images)

### Result

* No contention
* High cache locality
* Predictable latency

---

# 10. Pipeline Execution Model

## 10.1 Precomputed Execution

* No runtime graph construction
* DAG built in API/startup
* Pipeline executes only

## 10.2 Dispatch

```
g_jump_table[op_code](ctx, buffer, params)
```

* O(1) dispatch
* Branch prediction friendly
* Fully deterministic

---

# 11. Plugin System (Kernel-Grade)

## 11.1 Design

* Static registration at startup
* No dynamic loading (no dlsym in hot path)
* Integrated into jump table

## 11.2 Rules

* Pipeline MUST NOT know plugins exist
* Runtime handles plugin execution
* ABI strictly versioned

---

# 12. IO Model (Corrected)

## 12.1 Split Design

| Layer | Responsibility        |
| ----- | --------------------- |
| io    | decode/encode only    |
| vfs   | file/network/syscalls |

## 12.2 Rules

* Only API can call IO/VFS
* No runtime or pipeline access

---

# 13. Observability Model

* Lock-free ring buffer
* Asynchronous flushing
* RDTSC-based profiling

### Hard Rules

* No blocking
* No allocation in hot path
* No influence on execution decisions

---

# 14. Security Model

## 14.1 Threats

* Malicious inputs
* Decoder exploits
* Buffer overflows

## 14.2 Enforcement

* Input validation at API boundary
* Bounds checking in decoders
* Sandbox at startup

## 14.3 Tools

* ASAN / UBSAN
* LibFuzzer

---

# 15. Latency Budget (Per Image)

| Stage           | Budget |
| --------------- | ------ |
| Decode          | 0.5 ms |
| Dispatch        | 0.2 ms |
| SIMD Processing | 0.8 ms |
| Output          | 0.3 ms |
| **Total**       | < 2 ms |

---

# 16. CPU Optimization Strategy

* 64-byte cache alignment
* Prefetching
* False-sharing avoidance
* Sequential memory access

---

# 17. NUMA Strategy

* Per-node memory pools
* Thread-to-node pinning
* No cross-node memory access

---

# 18. Failure Handling

| Scenario          | Strategy        |
| ----------------- | --------------- |
| Corrupt input     | Fail-fast       |
| SIMD unsupported  | Scalar fallback |
| Plugin failure    | Skip + log      |
| Memory exhaustion | Reject request  |

---

# 19. Backpressure

* Bounded queues
* Drop policies
* Adaptive throttling

---

# 20. Repository Structure

```
imgengine/
├── api/
├── src/
│   ├── core/
│   ├── memory/
│   ├── arch/
│   ├── pipeline/
│   ├── runtime/
│   ├── io/
│   ├── vfs/
│   ├── plugins/
│   ├── observability/
│   ├── security/
├── bindings/
├── tests/
├── scripts/
├── docs/
├── build/
```

---

# 21. Kernel-Grade Rules (MANDATORY)

1. No malloc/free in hot path
2. No locks in execution path
3. No dynamic dispatch (only jump tables)
4. No cache-line sharing
5. No unpredictable branching
6. No IO outside API
7. No plugin execution in pipeline

---

# 22. CI/CD Requirements

## 22.1 Performance Gate

CI MUST fail if:

* P99 latency > 2ms
* Throughput drops > 5%
* Allocation detected in hot path

## 22.2 Testing

* Unit tests
* Integration tests
* Fuzz testing
* Performance benchmarks

## 22.3 Security

* ASAN / UBSAN mandatory
* Fuzz coverage required
* No undefined behavior allowed

---

# 23. Header & ABI Policy

## 23.1 Opaque Types

```
typedef struct img_ctx img_ctx_t;
```

## 23.2 Rules

* Headers expose interfaces only
* No heavy includes in headers
* Forward declarations preferred
* ABI stability enforced

---

# 24. Hot Path Definition

Hot path includes ONLY:

* pipeline/
* arch/
* runtime worker execution

### Forbidden in hot path:

* malloc/free
* printf/logging
* syscalls
* file IO

---

# 25. Final Definition

This system is defined as:

> **A Kernel-Grade, Deterministic, Data-Oriented, Zero-Allocation Execution Engine**

With:

* Strict dependency DAG
* Jump-table execution
* Slab-based memory model
* Full CPU-level optimization

---

# 26. Final Status

✅ Deterministic
✅ SIMD-optimized
✅ Zero-allocation hot path
✅ Kernel-grade architecture

**Status: PRODUCTION-READY (Subject to CI Enforcement)**
