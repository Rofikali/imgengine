# ADR-F0: browser-local resize before optional Wasm

Date: 2026-11-16. Direction: **STRAT-00 OWNER APPROVED**. ADR delivery:
**ACCEPTED / MERGED in [PR #55](https://github.com/Rofikali/imgengine/pull/55) as `e26a9194a3c9f58059fcd81c8bd78cffd2199c33`; final PR-head CI succeeded at `073a16ab84901f67d78f5964622aed524adbc089`.**
Source baseline: `67f6bbb2e9e9f80198a10e204fcbcc252db37fc8`.
The [live board](../next.md) governs delivery state; this document is not a
feature implementation, release, or permission to merge/deploy.

## Context and decision

P13 M1.1 transport, M1.2 editor and bounded M1.3 parity are already merged.
M1.3 PR #49 merged as `8fa2cc3faa651b99599b750978662cd85ce272fb`; its
[seven CI jobs passed](https://github.com/Rofikali/imgengine/actions/runs/43825725494)
at head `77ac72044848676f201ad0ece3d76aa136842349`. Its server-render action
uploads the source intentionally; that local interaction preview is not a
zero-upload image tool. Cross-browser/color and release evidence remain open.

The owner approved privacy-first free browser tools before speculative platform
expansion. The first customer job is resizing one photo/signature to requested
pixel dimensions without sending it to a server. F1 uses existing Nuxt/Vue/TS
and browser decode/resample/encode APIs. It does not require Rust/Wasm first.
A Worker may isolate processing where supported and useful; browser capability
and responsiveness measurements determine that implementation choice. Unsupported
local capability produces an honest local error, never an automatic server fallback.

This supersedes the *ordering* in the historical [M0 Wasm recommendation](M0_C17_PORTABILITY_AND_WASM_ADR.md),
[M1→M2 roadmap](PRODUCT_ROADMAP_BROWSER_FIRST.md) and [strategy brief](IMGENGINE_STRATEGY_AGENT_START_2026-11-15.md)
where Wasm appears to be a prerequisite for free resize. Their native C17, ABI,
security and measured feasibility requirements remain binding. The approved
[technology policy](TECHNOLOGY_AND_REPOSITORY_POLICY.md) is unchanged.

## Sequence and first implementation contract

| Milestone | Bounded result | Dependency / exit |
| --- | --- | --- |
| F0 | This ADR, evidence reconciliation and live queue | Reviewed docs PR, exact-SHA CI, owner-authorized merge |
| F1 | One browser-local resize vertical slice | Starts after F0 merge; acceptance below and separate reviewed PR |
| F2 | Crop/rotate/flip, then compression in separate PRs | Preserve F1 privacy/lifecycle guarantees; define each operation's fidelity and limits |
| F3 | Measured Worker/Rust-Wasm feasibility only where justified | Compare browser-native baseline, cold/warm latency, bundle, memory, fidelity and support burden before adoption |
| F4/F5 | Conversion/basic print sheets, then free-tools launch | Separate feature acceptance, browser/privacy/hosting/legal/accessibility evidence and explicit launch approval |

F1 scope: single local JPEG/PNG selection, positive integer width/height,
aspect-ratio lock, local result preview and explicit PNG download. No batch,
conversion menu, target-KB compression, account, billing, database or server
fallback. A dedicated local-tool route must distinguish itself from the existing
server-render workflow. Preserve existing P12/P13 routes and omission behavior.

F1 must document byte, decoded-pixel, per-axis and output-allocation caps before
decode/render; start from existing preview bounds and reduce them if device
evidence requires. Reject malformed/unsupported files and overflow/nonfinite,
zero or fractional dimensions. Validate actual decoded dimensions, not filenames
or client header claims alone. Aspect-lock rounding must be deterministic and
tested on portrait, landscape, odd and one-pixel cases; output dimensions exact.
State the local orientation contract explicitly with fixtures; do not silently
change P13's stored-raster contract. Document metadata/color limitations and
browser resampler differences; do not promise byte-identical encoded output.

Privacy/lifecycle acceptance: no source/result bytes or filenames in HTTP,
analytics, logs, URLs or persistent browser storage. Loading app assets is not
an image upload. Verify the full choose→resize→download flow with network capture,
including errors and replacement. Release object URLs/bitmaps/Workers on replace,
remove, failure and unmount; abandon stale completions after cancellation. URL
counts alone are not heap/RSS evidence. Test repeated operations under documented
bounds, keyboard labels/focus/error announcements and desktop/mobile layouts.
Use synthetic public fixtures, never customer images.

## Alternatives, cost and independent safety gates

Browser APIs first minimize dependency/download and codec-maintenance costs and
test the user job quickly. Browser fidelity/capability variance is the tradeoff.
Mandatory Wasm first adds build/bundle/ownership work before a measured benefit;
it remains an F3 option. A server-first resize contradicts the approved no-upload
job. Neither choice establishes universal speed, low-end memory safety, revenue
or launch readiness. Measure task success, end-to-end latency and support burden;
no new telemetry upload or paid infrastructure is authorized by this ADR.

C-A0 concurrency/resource investigations remain independent P0 work. They are
hard gates for relevant native/FFI changes and server release; unconfirmed native
hypotheses do not block an isolated browser prototype that never invokes native
code. P8-SIM-02 merging is not simulator runtime evidence. P14 host security,
capacity, abuse controls and rollback remain required when exposing server compute.
Public free-tool launch also needs its own privacy/browser/hosting review.
Paid Studio, metered API, SDK and desktop stay deferred to demonstrated demand
and separately approved scope. No existing native, Rust or legacy path is retired.

## Verification, review and rollback

F0 is docs-only: inspect scope, links, status consistency and diff. Required CI
still runs all seven jobs; do not duplicate native/sanitizer/fuzz/Docker locally.
F1 requires focused unit, browser/privacy/negative/accessibility tests, typecheck
and production build at exact SHA, plus existing CI. Form/proxy changes, if ever
needed, require relevant Rust integration; native/FFI work requires its safety
matrix. [CI-first policy](CI_FIRST_MINIMAL_AI_CONTEXT_POLICY.md) governs logs.

Revert this sequencing ADR/queue change to roll back planning; no runtime migration
is introduced. Any later priority reversal must preserve recorded owner decisions
and get a new reviewed decision. Next eligible implementation is **F1 after F0
merge**; CI success alone does not unlock it. Merge and deployment remain human
decisions.
