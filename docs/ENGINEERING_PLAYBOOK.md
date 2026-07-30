# IMGENGINE Engineering Playbook

## 1. Loop Engineering

One loop owns one user-visible outcome. It starts with a contract change and finishes only with executable evidence. Do not combine unrelated native, API, UI, and infrastructure refactors in one loop.

```text
Observe → Specify → Implement → Verify → Measure → Document → Decide next loop
```

### Loop Template

1. Link one PRD backlog item and the affected contract section.
2. Record baseline behavior, command, environment, and result.
3. Add the smallest test or fixture that fails for the missing behavior.
4. Implement only the change needed to make it pass.
5. Run focused tests, then build/integration checks.
6. Record behavior, performance, and operational evidence in the relevant document.
7. Mark the PRD item complete only when its definition of done is met.

## 2. Required Design Review

Before modifying a subsystem, answer:

- What external contract changes?
- Which layer owns the change: UI, API, worker, engine, or infrastructure?
- What input bounds, ownership rules, and failure modes apply?
- What regression test proves the behavior?
- Does it alter native ABI, job payload version, database schema, or observability?

## 3. Native Change Checklist

- Keep dependencies downward: control plane → execution plane only.
- Preserve `img_job_t` ABI or version it deliberately.
- Validate size arithmetic before allocation or pointer calculations.
- Keep the hot path free of allocation, I/O, locks, and logging.
- Test scalar fallback and any SIMD leaf independently.
- Run sanitizer, ABI, regression, and benchmark gates on the supported Linux toolchain.

## 4. SaaS Change Checklist

- Define Pydantic request/response schemas before route logic.
- Use migrations for persistent schema changes; do not rely on runtime table creation in production.
- Treat queue publication and worker execution as failure-prone external operations.
- Keep secrets server-side and internal routes worker-authenticated.
- Add correlation IDs, safe logs, metric labels, and bounded timeouts.
- Test actual HTTP behavior with Redis/PostgreSQL before declaring an asynchronous flow complete.

## 5. First Implementation Loops

### Loop A — Native Build Truth

Document one supported Linux build command, make it pass in a clean environment, run CLI smoke fixtures, and add CTest coverage. Windows support is either completed through a file-mapping abstraction or documented as unsupported for this release.

### Loop B — Worker Artifact

Build the native CLI inside the worker image or copy it from a named builder stage. The worker must print its engine version at startup and fail fast when the binary is missing.

### Loop C — Real Job Lifecycle

Run FastAPI, Redis, PostgreSQL, worker, and storage together. Submit a fixture image and prove `queued → processing → completed` plus a download assertion. Add corrupt-input and engine-failure tests.

### Loop D — Layout Product Controls

Expose the complete layout model in Nuxt and FastAPI, validate it once at the API boundary, serialize it into a versioned worker payload, and compare output fixtures for border/bleed/crop behavior.

## 6. Definition of Done

A loop is complete only when source, tests, documentation, local run instructions, and observability agree. A green compile alone is not done; a working manual demo without a repeatable test is not done; a benchmark without its command and input corpus is not a performance claim.

