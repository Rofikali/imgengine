# AGENTS.md — IMGENGINE Engineering Constitution

## 1. Purpose

This file defines the global engineering rules for the IMGENGINE repository.

These instructions apply to the entire repository unless a deeper `AGENTS.md` provides more specific rules for a directory.

IMGENGINE is being developed as a production-grade image-processing SaaS with:

* a high-performance native C engine,
* a stable C ABI,
* a safe Rust control/orchestration layer,
* a Rust/Axum backend,
* a Nuxt/Vue/TypeScript frontend,
* ephemeral image processing,
* low-cost infrastructure,
* strong security boundaries,
* measurable performance,
* and a business model capable of becoming profitable.

The goal is not merely to make the code compile.

The goal is:

> Correct + Secure + Fast + Observable + Maintainable + Testable + Economically Viable.

---

# 2. Current Repository Architecture

The target architecture is:

```
Nuxt 4/5 + Vue + TypeScript
                |
                v
         Rust / Axum
                |
                v
      Rust application layer
                |
                v
         Safe Rust FFI
                |
                v
        libimgengine C ABI
                |
                v
         IMGENGINE C core
                |
                +--> SIMD / AVX2
                +--> TurboJPEG
                +--> decoding
                +--> layout/rendering
                +--> JPEG/PDF encoding
```

The C engine remains the performance-critical native execution layer.

Rust becomes the primary control-plane and orchestration language.

The frontend remains a separate presentation layer.

---

# 3. Technology Direction

## Keep

* C for proven performance-critical image-processing code.
* SIMD/AVX2 where benchmarks justify it.
* TurboJPEG and existing native codec investment.
* Rust for infrastructure, orchestration, validation, lifecycle, concurrency, and FFI safety.
* Axum/Tokio for the target HTTP backend.
* Nuxt/Vue/TypeScript for the frontend.
* Linux as the production target.
* Docker for reproducible environments.

## Retire progressively

The following are legacy technologies and must not be reintroduced into the target MVP architecture:

* Python
* FastAPI
* Celery
* Redis
* PostgreSQL for job-only persistence
* permanent image storage
* S3/MinIO for the MVP image path
* unnecessary ELK/Jaeger infrastructure

Existing legacy code may remain temporarily during migration.

Do not remove legacy components merely because the target architecture says they will eventually disappear.

Retirement requires:

1. replacement,
2. contract parity,
3. verification,
4. rollback window,
5. explicit retirement decision.

---

# 4. Current Migration State

Priority 1 — Security and Correctness:

```
COMPLETE / VALIDATED
```

Priority 2 — Stable C ABI:

```
CURRENT PHASE
```

Planned sequence:

```
P1 Security + correctness
  |
  v
P2 Stable C ABI
  |
  v
P3 Safe Rust FFI
  |
  v
P4 Selective orchestration migration
  |
  v
P5 Rust/Axum backend
  |
  v
P6 Legacy infrastructure retirement
  |
  v
P7 Nuxt modernization
  |
  v
P8 Production economics + scale
```

Do not skip phases simply because a later technology is attractive.

---

# 5. Engineering Philosophy

## 5.1 Correctness before optimization

Never sacrifice correctness for a benchmark result.

The order is:

```
Correctness
    ->
Security
    ->
Reliability
    ->
Performance
    ->
Cost optimization
```

A faster incorrect image engine is a failed product.

---

## 5.2 Evidence over assumptions

Do not say:

* "this should be faster"
* "this should be safe"
* "this should scale"
* "this should use less memory"
* "this should be production ready"

unless the statement is backed by appropriate evidence.

Prefer:

* tests,
* benchmarks,
* sanitizer results,
* fuzzing,
* profiling,
* load tests,
* security tests,
* resource measurements,
* CI evidence,
* production telemetry.

---

# 6. Business Engineering Principle

IMGENGINE is not only a programming project.

Every major architectural decision must eventually answer:

1. What problem does this solve?
2. Who benefits?
3. What does it cost?
4. What does it make faster?
5. What does it make safer?
6. What operational complexity does it introduce?
7. Can the business afford it?
8. Does it improve the product?
9. Can it scale economically?

Engineering metrics should eventually include:

* requests/second,
* images/second,
* processing latency,
* P50/P95/P99 latency,
* CPU per image,
* memory per request,
* peak RSS,
* failure rate,
* timeout rate,
* crash rate,
* cleanup failure rate,
* infrastructure cost,
* cost per processed image,
* revenue per processed image,
* gross margin,
* active users,
* paid users,
* conversion,
* churn,
* MRR/ARR.

Do not optimize for "millions of users" before measuring the economics of one request.

First establish:

```
cost(request)
cost(image)
capacity(server)
revenue/customer
```

Then model scale.

---

# 7. Security Rules

Treat uploaded images and request data as untrusted.

Never assume:

* file extensions are trustworthy,
* MIME types are trustworthy,
* dimensions are trustworthy,
* metadata is trustworthy,
* paths are trustworthy,
* client-generated filenames are trustworthy,
* request sizes are safe,
* image dimensions are safe.

Validate at the correct boundary.

Never trust client-provided dimensions when actual decoding can establish dimensions.

Never expose:

* filesystem paths,
* internal diagnostics,
* stack traces,
* secrets,
* environment variables,
* internal identifiers,
* image bytes,
* private filenames.

Logs must be sanitized.

---

# 8. Ephemeral Data Policy

The target MVP must not permanently store user images or generated outputs.

Each request should use an isolated temporary workspace.

Target policy:

* random private request directory,
* restrictive permissions,
* server-generated filenames,
* bounded temporary storage,
* cleanup on success,
* cleanup on failure,
* cleanup on timeout,
* cleanup on cancellation,
* cleanup on client disconnect,
* startup stale-directory sweeper,
* stale retention no greater than five minutes.

Do not introduce a database merely to track temporary image jobs.

Metrics and carefully redacted logs are separate from image retention.

---

# 9. Observability

Production behavior must be measurable.

Lifecycle events should use structured logging.

Where appropriate, record:

* request ID,
* trace ID,
* route,
* operation,
* result class,
* duration,
* safe size buckets,
* safe pixel buckets,
* engine result.

Never log raw image content.

Never log secrets.

Never log arbitrary filesystem paths.

Never expose internal error details to clients.

Metrics should include:

* request count,
* success count,
* failure count,
* timeout count,
* overload count,
* processing duration,
* queue/concurrency utilization,
* memory pressure,
* native failures,
* cleanup failures.

---

# 10. Codex Operating Model

Codex is an implementation/review assistant.

The human developer remains responsible for:

* architecture,
* requirements,
* acceptance criteria,
* security decisions,
* business decisions,
* release decisions,
* final merge/push authorization.

Codex must not silently redefine product architecture.

For significant work, follow:

```
Understand
   ->
Inspect
   ->
Plan
   ->
Implement
   ->
Test
   ->
Review
   ->
Adversarial review
   ->
Fix
   ->
Verify
   ->
Commit
   ->
Ask before push
```

Do not immediately edit a large system without first understanding the current implementation.

---

# 11. Codex Planning Rules

For non-trivial tasks, Codex should first identify:

* current behavior,
* affected files,
* architectural boundary,
* risks,
* dependencies,
* tests,
* rollback strategy,
* documentation impact.

A good implementation task should contain:

```
Problem
Current behavior
Desired behavior
Constraints
Acceptance criteria
Tests
Documentation
Git requirements
```

---

# 12. Definition of Done

A task is not complete merely because compilation succeeds.

For production-impacting work, Definition of Done normally includes:

* implementation complete,
* relevant unit tests,
* relevant integration/regression tests,
* sanitizer verification where applicable,
* security verification where applicable,
* benchmark comparison where performance matters,
* documentation updated,
* migration notes updated where applicable,
* no unrelated changes,
* git diff reviewed,
* commit created when requested/appropriate,
* push performed only after explicit user authorization.

---

# 13. Git Policy

Current repository branch:

```
codex/fix-saas-candidate-fixtures
```

Remote:

```
origin -> GitHub repository
```

Git policy:

> Codex may create commits, but Codex must NEVER push to a remote repository without explicit user approval.

Before committing:

1. inspect `git status`,
2. inspect the diff,
3. verify tests,
4. ensure no secrets are staged,
5. ensure no unrelated files are included,
6. use a clear commit message.

After committing:

* report commit hash,
* report working tree state.

Before pushing:

* stop and ask for permission.

Never force-push unless explicitly instructed.

Never rewrite published history without explicit authorization.

---

# 14. Linux and Docker Environment

Production target:

```
Ubuntu 24.04 Linux
```

Development/verification environment:

```
Windows 11
  +
Ubuntu 24.04
  +
Docker Desktop
```

Linux/Docker is the authoritative environment for Linux release verification.

Do not waste migration work attempting to make POSIX-specific native code behave like a Windows-native build unless portability is explicitly part of the task.

Native Windows compatibility may be treated as separate work.

---

# 15. Testing Philosophy

Tests must validate behavior, not merely implementation details.

Prefer layered verification:

```
Unit tests
    ->
Integration tests
    ->
Regression tests
    ->
Sanitizers
    ->
Fuzzing
    ->
Security tests
    ->
Performance tests
    ->
End-to-end tests
```

When changing a boundary, add a regression test for the boundary.

When fixing a bug, preserve the bug as a regression test whenever practical.

---

# 16. ABI and API Stability

Public interfaces are contracts.

Never expose internal implementation structures merely for convenience.

Public native interfaces should prefer:

* opaque handles,
* explicit lifecycle,
* explicit ownership,
* fixed-width types where appropriate,
* explicit error/status codes,
* documented allocation/free rules,
* versioning,
* symbol visibility,
* compatibility tests.

The installed public header must not accidentally depend on private implementation headers.

---

# 17. Rust Safety

Rust FFI is an unsafe boundary.

Keep `unsafe`:

* small,
* localized,
* documented,
* reviewed.

Do not automatically mark native handles as `Send` or `Sync`.

Only implement thread-safety traits when the underlying C object is proven safe for that usage.

Never hide ownership ambiguity behind convenient wrappers.

The safe Rust API must make invalid states difficult to represent.

---

# 18. Performance Rules

Do not rewrite C code in Rust merely because Rust is preferred architecturally.

Keep C when:

* it is performance-critical,
* benchmarks justify it,
* the implementation is well-tested,
* the code has established codec/SIMD integration,
* migration risk exceeds expected benefit.

Consider Rust when:

* ownership is difficult to prove in C,
* orchestration dominates,
* filesystem/process management is involved,
* HTTP/API handling is involved,
* concurrency safety is improved,
* lifecycle management becomes clearer.

Every performance migration must compare:

* correctness,
* latency,
* throughput,
* CPU,
* memory,
* binary size where relevant,
* operational complexity.

---

# 19. No Premature Infrastructure

Do not introduce:

* Kubernetes,
* Kafka,
* Redis,
* PostgreSQL,
* service meshes,
* complex distributed queues,
* expensive observability stacks

without a measured requirement.

The initial system should be capable of operating on a low-cost VPS.

Scale architecture should be designed intelligently, but infrastructure should be introduced according to measured demand.

---

# 20. Documentation Rules

Canonical product/engineering behavior belongs in `docs/`.

Important documentation currently includes:

* `docs/PRD.md`
* `docs/ENGINE_SPEC.md`
* `docs/SAAS_CONTRACT.md`
* `docs/ENGINEERING_PLAYBOOK.md`
* `docs/architecture.md`
* `docs/migration.md`
* `docs/security.md`
* `docs/threat-model.md`
* `docs/api.md`
* `docs/deployment.md`
* `docs/performance.md`
* `docs/LINUX_VERIFICATION.md`
* `docs/NATIVE_DEVELOPMENT.md`
* `docs/NATIVE_BENCHMARKING.md`
* `docs/RELEASE_CHECKLIST.md`

When behavior changes, update the relevant contract.

Clearly distinguish:

* Planned
* Implemented
* Validated

Do not describe planned functionality as implemented.

Do not advertise benchmark numbers without reproducible evidence.

---

# 21. Migration Discipline

Migration must remain reversible until the retirement phase.

Prefer:

```
Add new path
   ->
Verify
   ->
Compare
   ->
Canary
   ->
Rollback window
   ->
Retire old path
```

Do not delete a working legacy path before replacement parity is proven.

---

# 22. Change Scope

Every task should minimize unrelated changes.

Do not combine:

* ABI migration,
* backend rewrite,
* frontend redesign,
* database removal,
* infrastructure redesign,
* unrelated refactoring

into one uncontrolled change.

One architectural boundary at a time.

---

# 23. Required Engineering Mindset

Think simultaneously as:

### Software Engineer

Does the code work?

### Systems Engineer

What happens under load, failure, concurrency, and resource pressure?

### Security Engineer

What happens when the input is malicious?

### SRE

How will we detect and recover from failure?

### Product Manager

Does this improve the actual product?

### CA / MBA / Business Owner

What does this cost, what does it earn, and what is the economic model?

### Philosopher / Logician

What assumptions are we making?

What evidence supports them?

What would prove us wrong?

---

# 24. Final Rule

Do not optimize for code volume.

Optimize for:

```
Trust
Correctness
Simplicity
Security
Performance
Evidence
Maintainability
Profitability
```

The best implementation is not the one with the most technology.

It is the smallest system that satisfies the real requirements with strong evidence.
