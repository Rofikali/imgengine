# IMGENGINE Documentation

This directory is the documentation source of truth. Historical notes remain useful, but a requirement is binding only when it is stated in the PRD or a linked canonical contract below.

## Start Here

| Document | Purpose | Audience |
| --- | --- | --- |
| [PRD](PRD.md) | Product scope, release gates, delivery loops, and backlog. | Product, engineering, operations |
| [Engine Specification](ENGINE_SPEC.md) | Native C/assembly layout contract: size, border, crop, bleed, output, and performance boundaries. | Native engineers, bindings authors |
| [SaaS Contract](SAAS_CONTRACT.md) | Nuxt, FastAPI, queue, worker, storage, and API behavior. | Full-stack and platform engineers |
| [Engineering Playbook](ENGINEERING_PLAYBOOK.md) | How a loop moves from a requirement to verified production evidence. | All contributors |
| [Native Development](NATIVE_DEVELOPMENT.md) | Supported Linux toolchain and reproducible Loop A commands. | Native engineers, CI owners |
| [HLD](hld.md) | Native architecture and performance constraints. | Native engineers |
| [LLD](lld.md) | Native module mappings, ownership, and call flows. | Native engineers |
| [Design Patterns](design_patterns.md) | Pattern guidance for the native implementation. | Native engineers |
| [v2 RFC](v2.0%20RFC.md) | Historical high-performance architecture proposal. | Architecture reference |

## Documentation Rules

1. Mark each statement as **implemented**, **planned**, or **validated** when that distinction matters.
2. Define public behavior in a contract before implementing it in C, FastAPI, Nuxt, or a worker.
3. Update the matching contract and acceptance criteria in the same change as an externally visible behavior change.
4. Do not advertise benchmark targets until CI produces a reproducible benchmark result.
5. Prefer stable names and versioned payloads over undocumented path or process assumptions.

## Product Vocabulary

- **Photo:** a source JPEG or PNG.
- **Cell:** one positioned, scaled copy of the source photo on a sheet.
- **Sheet/canvas:** the final printable output containing one or more cells.
- **Layout job:** dimensions, grid, print-safety, color, and output parameters used to render a sheet.
- **Job:** a durable SaaS record that tracks one submitted layout job.
- **Control plane:** Nuxt/FastAPI validation, authorization, state, and scheduling.
- **Execution plane:** worker and native engine processing of pixels.

## Status Legend

- **Implemented:** present in the tracked source; may still need tests.
- **Validated:** built or tested with recorded evidence.
- **Planned:** accepted requirement not yet implemented.
