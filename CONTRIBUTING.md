# Contributing

Thank you for contributing to `imgengine`.

> **New-contributor technology gate (2026-11-14):** Read [approved IMGENGINE stack and directory policy](docs/TECHNOLOGY_AND_REPOSITORY_POLICY.md), root [AGENTS](AGENTS.md), scoped [Rust AGENTS](backend/AGENTS.md), and [next.md](next.md). **New backend = Rust/Axum**, not Python/FastAPI, Django/DRF, Flask, Celery, Redis, PostgreSQL or new Python scripts. The existing Python directory is legacy and may receive only separately authorized migration/security maintenance. No speculative folder moves, database or queue adoption without an approved ADR.

## Engineering standard

Changes should be small, reviewable, and backed by evidence. For native-code changes, correctness and memory safety come before performance claims.

Before opening a pull request:

1. Read `README.md` and the relevant design/specification documents under `docs/`.
2. Keep public ABI changes explicit and backward-compatible unless a versioned breaking change is intentional.
3. Add or update tests for changed behavior.
4. Run the relevant CTest, regression, sanitizer, ABI, and Rust FFI checks when available.
5. For image-processing changes, validate at least one real-image path in addition to synthetic/unit fixtures when practical.
6. Do not commit generated outputs, private images, credentials, or machine-specific paths.
7. Update documentation when behavior, interfaces, limits, or operational requirements change.

For an F2c Compress or related browser-local image-tool PR, follow the [F2c twelve-category CI acceptance contract](docs/FRONTEND_F2C_CI_QUALITY_GATES.md). Supply a G01–G12 evidence matrix with exact tested SHA, GitHub Actions links, no-upload and EXIF/metadata proof, browser/a11y results, measured performance and explicit NOT RUN/FAIL statuses. The existing seven jobs do not establish all twelve categories by themselves; do not mark requirements complete on documentation alone.

## Pull requests

A useful pull request should explain:

- what changed
- why it changed
- affected interfaces or behavior
- tests and verification performed
- performance impact, if measured
- security or resource-lifecycle impact
- known limitations or follow-up work

Do not report a benchmark number unless the benchmark method and environment are documented well enough to reproduce it.

## Native and ABI changes

Treat the public C ABI as a compatibility boundary. Changes to exported symbols, public structs, ownership rules, error semantics, threading assumptions, or capability discovery require corresponding ABI/FFI coverage.

Prefer adding a regression test before fixing a discovered native bug so the failure remains permanently represented in the test suite.
