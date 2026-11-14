# IMGENGINE Documentation

This directory is the documentation source of truth. Historical notes remain useful, but a requirement is binding only when it is stated in the PRD or a linked canonical contract below.

## Start Here

| Document | Purpose | Audience |
| --- | --- | --- |
| [Project Status & Decision Register](PROJECT_STATUS_AND_DECISION_REGISTER.md) | Living inventory of implemented, partial, deferred, legacy and blocked work, with evidence and decisions. | All contributors |\n| [Browser-First Product Roadmap](PRODUCT_ROADMAP_BROWSER_FIRST.md) | Proposed image-tool routes, Rust/Wasm milestones, free/paid features, and business gates. | Product, architecture, engineering |\n| [Change-Scoped Verification](CHANGE_SCOPED_VERIFICATION.md) | Fast developer-loop testing and AI token/time budget policy without weakening release gates. | All contributors, CI owners |\n| [PRD](PRD.md) | Product scope, release gates, delivery loops, and backlog. | Product, engineering, operations |
| [Engine Specification](ENGINE_SPEC.md) | Native C/assembly layout contract: size, border, crop, bleed, output, and performance boundaries. | Native engineers, bindings authors |
| [Rust API Contract](api.md) | Implemented synchronous Rust/Axum render, layout, and physical-sheet contracts. | Frontend, backend, and platform engineers |
| [Local Docker Rehearsal](LOCAL_DOCKER_REHEARSAL.md) | Local P12 candidate rehearsal and its explicit evidence limits. | Developers and release owners |
| [SaaS Contract (legacy)](SAAS_CONTRACT.md) | Historical FastAPI, queue, worker, storage, and durable-job contract. | Legacy-path maintainers |
| [Engineering Playbook](ENGINEERING_PLAYBOOK.md) | How a loop moves from a requirement to verified production evidence. | All contributors |
| [SRE Contract](SRE.md) | SLOs, alert rules, and incident response expectations. | Platform and on-call engineers |
| [Security Operations](SECURITY_OPERATIONS.md) | Implemented controls and beta tenancy/security gates. | Security and platform engineers |
| [Release Checklist](RELEASE_CHECKLIST.md) | Required verification evidence and rollout governance. | Release owners |
| [Database Recovery](DATABASE_RECOVERY.md) | Verified local PostgreSQL backup, restore, and recovery boundaries. | Platform and on-call engineers |
| [Native Development](NATIVE_DEVELOPMENT.md) | Supported Linux toolchain and reproducible Loop A commands. | Native engineers, CI owners |
| [Native Benchmarking](NATIVE_BENCHMARKING.md) | Reproducible native performance measurement contract and evidence policy. | Native and performance engineers |
| [Linux Native Verification](LINUX_VERIFICATION.md) | Ubuntu 24.04 Docker verification gate for C correctness and security. | Native engineers and CI owners |
| [Rust FFI Boundary](RUST_FFI.md) | Safe ownership boundary over `libimgengine` ABI v1. | Rust and native engineers |
| [Rust Supervisor Foundation](RUST_SUPERVISOR.md) | Ephemeral single-engine request lifecycle proof. | Rust and platform engineers |
| [Rust Request Lifecycle](RUST_REQUEST_LIFECYCLE.md) | Transport-neutral request ownership, cleanup, timeout, and error contract. | Rust and platform engineers |
| [Scheduler Benchmark Contract](SCHEDULER_BENCHMARK_CONTRACT.md) | Decision gate for C execution and Rust admission evidence. | Rust and native engineers |
| [Scheduler Characterization Report](SCHEDULER_CHARACTERIZATION_REPORT.md) | Recorded decision gate result: keep C scheduler ownership. | Architecture reviewers |
| [HLD](hld.md) | Native architecture and performance constraints. | Native engineers |
| [LLD](lld.md) | Native module mappings, ownership, and call flows. | Native engineers |
| [Design Patterns](design_patterns.md) | Pattern guidance for the native implementation. | Native engineers |
| [v2 RFC](v2.0%20RFC.md) | Historical high-performance architecture proposal. | Architecture reference |
| [Architecture](architecture.md) | Implemented Rust/C/Nuxt first-MVP boundaries, retained legacy path, and open release gates. | Architecture and platform engineers |
| [Migration Plan](migration.md) | Reversible legacy-to-Rust migration position and retirement criteria. | Engineering and release owners |
| [Security Design](security.md) | Target security controls and migration gates. | Security and platform engineers |
| [Threat Model](threat-model.md) | Assets, threats, mitigations, and verification. | Security and engineering |
| [Deployment Design](deployment.md) | Hardened low-cost VPS operating model. | Platform and on-call engineers |
| [Performance Strategy](performance.md) | Native and end-to-end measurement gates. | Native and performance engineers |

## Documentation Rules

1. Mark each statement as **implemented**, **planned**, or **validated** when that distinction matters.
2. Define public behavior in a contract before implementing it in C, Rust/Axum, Nuxt, FastAPI, or a worker.
3. Update the matching contract and acceptance criteria in the same change as an externally visible behavior change.
4. Do not advertise benchmark targets until CI produces a reproducible benchmark result.
5. Prefer stable names and versioned payloads over undocumented path or process assumptions.

## Product Vocabulary

- **Photo:** a source JPEG or PNG.
- **Cell:** one positioned, scaled copy of the source photo on a sheet.
- **Sheet/canvas:** the final printable output containing one or more cells.
- **Layout job:** dimensions, grid, print-safety, color, and output parameters used to render a sheet.
- **Job:** a durable SaaS record in the legacy asynchronous architecture; the target synchronous architecture has no durable job artifact.
- **Control plane:** implemented Rust/Axum validation, authorization, lifecycle, and bounded admission; FastAPI remains the retained legacy control plane.
- **Execution plane:** retained C scheduler and native engine process pixels; legacy workers remain only on the retained legacy path.

## Status Legend

- **Implemented:** present in the tracked source; may still need tests.
- **Validated:** built or tested with recorded evidence.
- **Planned:** accepted requirement not yet implemented.
