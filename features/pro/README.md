# IMGENGINE Pro — product requirements and feature constitution

**Decision:** Pro selected by owner on 2026-11-21; **scope:** native C17 + Rust/FFI planning and verification first. **NO PRO FEATURE IMPLEMENTATION OR COMMERCIAL RELEASE IS CLAIMED.** Frontend requirements and implementation are intentionally deferred to a later owner conversation. This is a product-level contract proposed for review, not an authorization for billing, production deployments, changes to Free, or a live API.

## 1. Mission, positioning and customers

**Job-to-be-done:** "I have many images and recurring output requirements. Run the same trusted transformations on all of them and deliver organized, predictable results without repeating manual steps."

- Primary hypotheses: catalog operators, creators, freelancers and small businesses processing repeated image batches; validate with interviews and working pilots.
- Pro is **production automation**. IMGENGINE Free is five individual local tools; Studio is print/operator service work; Developer API is separately metered hosted compute; ImageReady is rule-based verification. Do not silently merge their entitlements or promises.
- Pro's differentiator must be measured saved time, predictable output and user trust; paid basic resizing alone is weak. Competing alternatives include mature batch/conversion tools.
- **Privacy default:** no server upload or permanent image storage simply to enable batch processing. Native CLI testing may use synthetic local files; future product execution model must be explicitly chosen and measured.
- Product version, native ABI version, recipe schema version and any later entitlement version are **independent**. Do not assume the current C17 `.so` already exposes a recipe/batch API.

## 2. End-to-end scenario and product boundaries

Import originals -> choose a named recipe and one or more output profiles -> validate inputs/resource budget -> preview/confirm destructive crop or color changes -> execute a bounded job -> collect per-output success/failure -> verify output dimensions/encoding -> export named outputs and a machine-readable manifest -> release resources.

Required behavior: preserve originals; explicit operation ordering; errors do not corrupt unrelated outputs; no silent unsupported codec fallback; avoid repeated lossy encoding when practical; stable output naming and collision policy; cancellation is **best effort at native boundaries**, never a false claim that a running C ABI call can be interrupted.

### In scope (after specific implementation approvals)

Local recipe schema, safe image operations, bounded batch orchestration, multiple destination outputs, output manifests, previews when the UI milestone is authorized, safe import/export, privacy and tested device/host limits.

### Out of scope until independently approved

Frontend implementation now; cloud sync, permanent image uploads, shared team workspaces, AI image generation/background removal, public developer API, marketplace platform integrations, distributed queues, Redis/Postgres/Python, unlimited processing, SaaS accounts/billing/SDK redistribution, guaranteed file-size targets, 99.9% SLA, automatic government/marketplace acceptance and production deployment. The current Rust/Axum server is not a Pro billing platform.

## 3. Feature register and sequential delivery

**Status authority:** [STATUS.md](STATUS.md) records verified repository/PR evidence. The below table is **approved direction / proposed scope**, not implementation-complete evidence. The first implementation milestone is a small native/Rust feasibility contract, not a new UI.

| Feature ID | Feature | Required result | Phase |
| --- | --- | --- | --- |
| PRO-00 | Customer/problem discovery and native capability audit | Validated recurring jobs, ROI hypotheses, gaps against C17/FFI | NOW: documentation/design |
| PRO-01 | Versioned named recipes | Create/validate/import/export safe recipe JSON; reject unsafe or unknown operation grammar | Native/Rust foundation |
| PRO-02 | Bounded batch executor | Process files with file/pixel/RSS/work/concurrency caps; per-file outcome, cancellation and cleanup | Native/Rust foundation |
| PRO-03 | Deterministic multi-step transforms | Explicit order: orient/crop/resize/convert/encode; defined color/alpha/metadata behavior | Native/Rust foundation |
| PRO-04 | Multiple output profiles | One original -> N independently specified outputs, bounded fan-out and resource budget | Native/Rust foundation |
| PRO-05 | Export manifest and names | Output path collision rules, hashes, dimensions, format, errors, provenance of recipe version | Native/Rust foundation |
| PRO-06 | Quality/size assistant | Visual tradeoff, best-effort target-size encoding, explicit failure when impossible | Later |
| PRO-07 | Crop/fit/fill/pad presets | Aspect presets, safe geometry and padding; destructive crop preview when UI exists | Later |
| PRO-08 | Color and metadata options | Orientation, alpha-to-JPEG background, tested ICC/EXIF policy, declared losses | Later |
| PRO-09 | ZIP and multi-directory exports | Path traversal protection, bounded archive construction, safe partial results | Later |
| PRO-10 | Batch retry/resume report | Retry only failed jobs, interrupted-job recovery defined and tested | Later |
| PRO-11 | Local recipe library | Search, duplicate, validate and migrate recipe schema versions | Later UI |
| PRO-12 | Accessible visual editor and previews | Keyboard/touch workflow and responsive progress/error UX | **FRONTEND DEFERRED** |
| PRO-13 | Offline/browser and Worker decision | Measure browser native / Wasm / local native options; no invented binary reuse | **FRONTEND DEFERRED** |
| PRO-14 | Optional watermark/border overlays | Composable export effects, rights and visual quality checks | Evidence-gated later |
| PRO-15 | Workflow templates for use cases | Customer-proven repeat jobs, not unverified platform-compliance guarantees | Evidence-gated later |
| PRO-16 | Licensing/entitlements | Owner-approved purchase, refund, customer identity and offline threat decision | Commercial gate |
| PRO-17 | Support, observability and release | Release manifest, incident/rollback owner, defects, compatibility and policies | Commercial gate |
| PRO-18 | Team/cloud automation or hosted processing | Separate risk, capacity, account, spend and data retention ADR before implementation | Explicitly deferred |

**R1 native milestone:** prove a small versioned recipe and a bounded 2-image transform using **existing** C/FFI APIs, with golden fixtures; avoid adding public ABI functions until an evidence-backed need is confirmed. No feature marked complete by existing Free tools alone. **R2:** batch contracts and failure behavior. **R3:** multi-output and manifest. **R4:** optional frontend discovery with owner; **R5:** paid beta only after commercial/security gates.

## 4. Functional acceptance contracts

1. **Recipe:** include schema version, operation list, parameters/units and declared output profiles; reject malformed JSON, NaN/overflow, invalid dimensions, unknown algorithms, arbitrary file paths or scripts; migrations and stable ordering tested.
2. **Batch:** record files attempted/succeeded/failed/cancelled, require deterministic per-item diagnostics, cap bytes/pixels/CPU time/memory/concurrency, avoid unbounded decoded buffers; no promise of native mid-call cancellation.
3. **Execution:** no source overwrite, ordered composable operations; FIT/FILL details and rounding defined by tested contract; explicit alpha/ICC/EXIF/orientation handling; no invented format support.
4. **Multi-output:** output identities include profile and recipe version; a failure in one profile cannot silently mark all profiles successful; reserve output budget before processing.
5. **Export:** safe filenames, Unicode/path normalization where supported, overwrite-disabled default, ZIP protection and size limits, verifiable output byte length/hash and manifest error enums.
6. **Privacy:** no image bytes or sensitive filenames in logs/metrics/analytics; ephemeral scratch by default with cleanup on success/error/crash within documented limits.
7. **Interop:** C ABI remains C11-header consumable / strict C17 implementation; safe Rust FFI isolates unsafe, ownership and panics; consumer tests prove compatibility.
8. **Verification:** every implemented feature requires fixture correctness, invalid/malicious inputs, cancellation/failure cleanup, peak RSS and time/throughput measurements; per-PR exact SHA CI and documented NOT_RUN states.
9. **Frontend later:** implement accessible previews, mobile performance, privacy network capture and visual export only after separate scope/approval. No UI build work as part of PRO-00.
10. **Commercial later:** require real pilot data, price sensitivity, margin and payment/refund/tax/retention decision; proposed prices are hypotheses, not live terms.

## 5. Product metrics and management gates

- North-star metric: completed *repeat* workflows per retained active customer **and median time saved against a credible baseline**. Count *successful outputs* separately from attempted files.
- Activation: first working recipe + completed multi-image result; retention: repeated jobs in a later period; reliability: error rates, duplicate/incorrect outputs, peak memory, p95 latency at explicit image size/concurrency.
- Finance: recognized revenue, collected cash, payment fees, variable compute/bandwidth/support, refunds, contribution margin, fixed costs, CAC and payback. Prepaid credit receipt != immediately recognized revenue by default.
- Pilot gate: at least 10 relevant users interviewed and a small number of repeat trials with recorded time savings and willingness-to-pay before introducing entitlements or subscriptions.
- Risk register: underestimated browser memory, decoder vulnerabilities, silent quality degradation, unsupported color paths, device variance, cheap competitor alternatives, paid entitlement bypass and high support cost. Mitigate with limits, fixtures, independent review and narrow customer scope.

## 6. Technical governance and dependencies

See [NATIVE_TESTING.md](NATIVE_TESTING.md) for Docker Desktop/Ubuntu 24.04 and CI; [STATUS.md](STATUS.md) for NOW/STARTED/IN_PROGRESS/COMPLETED/MERGED/NOT_MERGED; [portfolio](../../docs/IMGENGINE_FOUR_PRODUCT_PORTFOLIO.md), [technology policy](../../docs/TECHNOLOGY_AND_REPOSITORY_POLICY.md), [C/Rust evidence](../../docs/NATIVE_RUST_CONTRACT_EVIDENCE_MATRIX.md), and [live next](../../next.md).

Keep `main` as integration source of truth; use short-lived `feat/pro/<milestone>` branches (e.g. `feat/pro/pro-01-recipe-contract`), one bounded, tested PR per feature. `features/pro/` is a documentation/Pro test namespace, not a permanent product fork or authorization to move existing C/Rust source. Each merged feature updates affected docs and status while main Free release takes its own owner-approved route.
