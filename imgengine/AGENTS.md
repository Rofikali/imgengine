# AGENTS.md — IMGENGINE Native C Engine

## 1. Scope

These instructions apply to:

```
/imgengine
```

They supplement the repository-level `AGENTS.md`.

This directory contains the native IMGENGINE implementation.

The native engine is performance-critical and security-sensitive.

Do not treat it as ordinary application code.

---

# 2. Native Architecture

The native engine is responsible for performance-critical image processing.

Primary responsibilities include:

* image decoding,
* metadata validation,
* layout calculation,
* rendering,
* JPEG/PDF encoding,
* SIMD acceleration,
* memory management,
* worker execution,
* native scheduling where currently retained,
* security boundaries,
* native observability.

The target external architecture is:

```
Rust
  |
  v
stable libimgengine ABI
  |
  v
C implementation
  |
  +--> TurboJPEG
  +--> rendering
  +--> layout
  +--> SIMD/AVX2
  +--> JPEG/PDF
```

---

# 3. Current Migration Priority

Current phase:

```
Priority 2 acceptance — real-image verification
```

Priority 1 security/correctness work has already passed its Linux verification gate.

Do not use the completion of Priority 1 as permission to perform unrelated native rewrites.

Priority 3 Rust work must not begin until the public ABI is verified with
reproducible, real JPEG and PNG inputs through both C and Rust consumers.

---

# 4. C-to-Rust Boundary

Keep the following in C unless evidence proves otherwise:

* codec integration,
* image decode/encode kernels,
* layout math,
* rendering kernels,
* SIMD/AVX2,
* TurboJPEG integration,
* proven native performance paths.

Potential future Rust candidates:

* orchestration,
* process supervision,
* request validation,
* filesystem policy,
* lifecycle management,
* safe ownership facade,
* scheduling,
* non-kernel infrastructure.

Do not migrate scheduler, arena, slab, or other memory/concurrency components simply because they are difficult.

Migration requires evidence.

---

# 5. Stable C ABI Rules

The public ABI must be treated as a product contract.

The public API must NOT expose:

* private structures,
* private headers,
* internal allocator structures,
* scheduler internals,
* implementation-specific layout structures,
* unstable C types where fixed-width alternatives are appropriate.

Prefer:

* opaque handles,
* explicit lifecycle,
* explicit ownership,
* explicit allocation rules,
* explicit free functions,
* explicit error/status values,
* version information,
* documented thread-safety,
* documented lifetime rules.

---

# 6. Public Header Isolation

Every public header must be independently consumable.

A consumer should be able to include the public header without depending on:

* internal source headers,
* internal include ordering,
* implementation macros,
* private structures.

Create a consumer test that proves header isolation.

For example, conceptually:

```
#include <imgengine/...>
```

The consumer should compile using only the installed/public interface.

---

# 7. ABI Symbol Policy

Export only intended public symbols.

Do not accidentally export:

* static-library implementation symbols,
* allocator internals,
* scheduler internals,
* debugging helpers,
* test-only functions.

Maintain an explicit exported-symbol manifest.

ABI checking must detect both:

* missing required symbols,
* unexpected public symbols.

A library that still exports an obsolete implementation symbol is not automatically ABI-correct.

---

# 8. ABI Versioning

ABI versioning must be deliberate.

Document:

* major version,
* minor version,
* compatibility expectations,
* symbol policy,
* ownership compatibility,
* structure compatibility,
* error-code compatibility,
* deprecation policy.

Do not change the ABI merely to make an implementation easier.

If a breaking change is required, explicitly version it.

---

# 9. Ownership

Every pointer crossing the C ABI must have documented ownership.

For every returned resource, answer:

1. Who allocated it?
2. Who owns it?
3. Who frees it?
4. When may it be freed?
5. Can it be shared?
6. Can it be used across threads?
7. Can it outlive its parent handle?

Do not rely on comments such as "caller should probably free this."

The ownership contract must be unambiguous.

---

# 10. Error Handling

Do not use ambiguous return values for public operations.

Public API failures should distinguish meaningful categories such as:

* invalid argument,
* invalid image,
* unsupported format,
* resource limit,
* allocation failure,
* internal engine failure,
* cancelled operation,
* output failure.

Do not leak internal implementation details through public errors.

---

# 11. Integer and Boundary Safety

All size calculations must be reviewed for:

* integer overflow,
* multiplication overflow,
* addition overflow,
* truncation,
* signed/unsigned conversion,
* allocation-size overflow,
* dimension overflow,
* stride overflow.

Never assume a client-controlled width/height is safe.

Validate before multiplication/allocation.

---

# 12. Memory Safety

Use sanitizers during native development when appropriate:

* ASAN,
* UBSAN,
* leak detection.

Existing arena/slab behavior must remain protected by regression tests.

When changing allocator behavior, test:

* boundary allocation,
* zero-size behavior where relevant,
* overflow,
* ownership,
* double free,
* invalid free,
* reuse,
* teardown,
* concurrent behavior if supported.

Do not weaken validation merely because it makes a test inconvenient.

---

# 13. Scheduler Safety

The scheduler is a concurrency boundary.

When modifying scheduler behavior, verify:

* task delivery,
* overflow behavior,
* worker visibility,
* shutdown,
* cancellation,
* teardown,
* queue ownership,
* no stranded tasks,
* no use-after-free,
* no double execution.

Any queue topology change requires a regression test.

---

# 14. SIMD Rules

SIMD dispatch must never assume that CPU instruction support automatically means safe execution.

For AVX/AVX2:

* verify CPU capability,
* verify operating-system support for required register state,
* preserve scalar fallback,
* test equivalence.

SIMD output must remain semantically equivalent to the scalar implementation.

Performance optimization must not become a correctness fork.

---

# 15. Decoder Security

Image files are hostile input.

Decoder boundaries must validate:

* file type,
* actual decoded dimensions,
* pixel count,
* memory requirements,
* supported formats,
* malformed input,
* resource limits.

Never fabricate image dimensions.

Never trust dimensions supplied only by the client.

Use actual decoder-derived metadata where the architecture requires it.

---

# 16. Sandbox

Sandboxing must be validated as part of actual engine lifecycle.

The test should prove:

* initialization works,
* required engine operations work,
* rendering works,
* shutdown works,
* prohibited operations are denied.

Do not weaken sandbox restrictions merely to make a test pass.

If a required syscall is genuinely necessary, document why and allowlist the smallest required capability.

---

# 17. Fuzzing

Fuzzing must exercise the actual native library code.

A fuzz harness that validates only its own wrapper is insufficient.

When changing parser/decoder/validator code:

* update fuzz targets,
* preserve instrumentation,
* run bounded smoke fuzzing,
* inspect findings,
* add regression tests for discovered bugs.

Bounded fuzzing is smoke verification, not proof of absence of bugs.

Long-running corpus-based fuzzing belongs in the appropriate CI/performance/security pipeline.

---

# 18. Testing Requirements

Before accepting a significant native change, select appropriate tests from:

* CTest,
* regression tests,
* ASAN,
* UBSAN,
* leak detection,
* fuzzing,
* sandbox tests,
* ABI tests,
* header consumer tests,
* symbol tests,
* benchmark tests.

Do not run every expensive test for every trivial documentation change.

Choose evidence proportional to risk.

---

# 19. Performance Engineering

Do not benchmark debug builds and call the result production performance.

Native benchmarks should record enough environment information to make comparisons meaningful.

Compare:

* baseline,
* changed implementation,
* representative inputs,
* output correctness,
* throughput,
* latency,
* CPU,
* memory where relevant.

A benchmark regression should be investigated before claiming completion.

---

# 19.1 Real-Image Verification

Synthetic tests, fuzzing, and generated micro-fixtures are necessary but do
not replace end-to-end real-image evidence. Before closing a native security or
ABI gate, run reproducible Ubuntu 24.04/Docker verification with generated or
project-owned JPEG and PNG inputs covering representative color spaces,
progressive encoding, unusual aspect ratios, and malformed inputs.

The verification must exercise the same public interface intended for
production. ABI verification must use only installed/public headers and public
symbols; Rust verification must use its safe wrapper. Validate output using an
independent decoder, record input/output dimensions and sizes, status results,
timing methodology, sanitizer result, and toolchain version. Do not commit
personal images or large binary fixtures; generate deterministic fixtures in
the verifier or document their license and provenance.

---

# 20. Linux Verification

The authoritative native release environment is:

```
Ubuntu 24.04
Docker
```

Use the existing verification infrastructure rather than inventing parallel verification paths.

Relevant files include:

* `Dockerfile.verify`
* `scripts/verify_linux.sh`
* native CMake configuration
* CTest
* native benchmark tooling

The Windows native build is not the release verification environment when POSIX-specific code is involved.

---

# 21. Code Style

Follow the repository's existing:

* C standard,
* compiler settings,
* CMake conventions,
* clang-format configuration,
* warning policy.

Do not perform broad formatting changes during functional work unless explicitly requested.

Avoid unnecessary renaming.

Avoid unrelated refactors.

---

# 22. Documentation

When changing native public behavior, update the appropriate documentation.

Especially consider:

* `docs/api.md`
* `docs/architecture.md`
* `docs/ENGINE_SPEC.md`
* `docs/lld.md`
* `docs/security.md`
* `docs/threat-model.md`
* `docs/performance.md`
* `docs/NATIVE_DEVELOPMENT.md`
* `docs/NATIVE_BENCHMARKING.md`
* `docs/LINUX_VERIFICATION.md`

For ABI work also maintain:

* ABI documentation,
* exported-symbol documentation,
* migration guidance.

---

# 23. ABI Phase Definition of Done

Priority 2 is complete only when all applicable items are satisfied:

* public headers are self-contained,
* public structures are appropriately opaque,
* ownership is documented,
* error/status semantics are documented,
* symbol visibility is controlled,
* ABI versioning is defined,
* missing-symbol checks exist,
* unexpected-symbol checks exist,
* C consumer test exists,
* header isolation test exists,
* ABI compatibility test exists,
* ownership tests exist,
* error tests exist,
* ASAN/UBSAN passes,
* Linux/Docker verification passes,
* minimal Rust FFI proof passes,
* documentation is updated,
* benchmark impact is measured,
* git diff is reviewed.

---

# 24. What NOT To Do

Do not:

* rewrite the engine in Rust,
* remove working C code without evidence,
* add Redis,
* add PostgreSQL,
* add Kafka,
* add Kubernetes,
* redesign the frontend,
* introduce a full Rust backend,
* change the business model

while performing a focused ABI task.

Finish the boundary first.

---

# 25. Native Engineering Principle

The native engine should become:

> A small, stable, well-tested, high-performance machine.

Rust should eventually be able to treat it as a dependable component rather than knowing its internal implementation.

That is the objective of the stable ABI.
