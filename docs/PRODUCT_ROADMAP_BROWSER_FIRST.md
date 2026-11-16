# IMGENGINE — Browser-first image product roadmap

**Sequencing reconciliation (2026-11-16):** STRAT-00 browser-first priority is owner-approved and [ADR-F0](F0_BROWSER_FIRST_SEQUENCING_ADR.md) **MERGED in PR #55**. F1 browser-native local resize is the next ready product task, before optional measured Worker/Rust-Wasm. P13 M1.2 and bounded M1.3 merged in PRs #47 and #49. **The older M-stage table below is historical planning**, not a second live task queue. The first IMGENGINE Free product is exactly **Crop, Resize, Compress, Rotate, Convert**; print sheets, Flip and other tools are later. See the [canonical four-product portfolio](IMGENGINE_FOUR_PRODUCT_PORTFOLIO.md) and [live NOW / NEXT / LATER board](../next.md). F1/paid features/Free public launch have **not** been implemented or released.

**Status:** Product strategy and proposed sequence; not a claim that listed routes or features exist. The canonical P8/P12/P13 contracts govern current behavior. This roadmap does not supersede release/security requirements.

## North star
A privacy-first, image-only workstation for students, photo studios, printing shops, application users, and B2B customers. The customer browser should do supported work locally; a separately consented, metered server path is reserved for workloads needing it.

## Product and architecture
- UI: Nuxt/Vue/TypeScript; adopt Tailwind only after explicit dependency/build decision.
- Local: Web Worker + browser image APIs; use Rust/Wasm for measured CPU-heavy geometry/pixel operations, and optionally a small portable C17 Wasm geometry component after ADR approval.
- Server: existing Rust/Axum admission/lifecycle -> safe C FFI -> C17 native scheduler/engine; no scheduler rewrite.
- Keep native and browser behavior linked through versioned geometry, rounding, orientation, color, dimensions, and output fixtures. Pixel-perfect encoder bytes may differ across codecs; define acceptable parity explicitly.
- Privacy: never transmit image bytes by default for a browser-local operation. Ask consent before any server fallback; redact telemetry. Do not persist private images or ID documents by default.
- **Binding technology decision:** no new Python/FastAPI/Flask/Django/DRF, Celery, Redis, PostgreSQL, or Python-based build/test tooling in the target product, including later paid tiers. Existing legacy Python services/scripts remain until separately retired/replaced with verified parity. See [technology policy and folder ADR](TECHNOLOGY_AND_REPOSITORY_POLICY.md) and [active Rust AGENTS](../backend/AGENTS.md).

## Dependency-ordered milestones
| Stage | Scope | Exit evidence | Status |
| --- | --- | --- | --- |
| M0 | C17 baseline/C11 regression/C23 isolation and Wasm ADR | Ubuntu verifier, ABI checks, ADR | **Merged**, PR #29 |
| M1.1 | Wire native ABI 2.3 crop through Rust FFI, Axum, Nuxt server proxy | Recorded Ubuntu + exact-head 7/7 CI, P12 omission backward compatibility; [PR #43](https://github.com/Rofikali/imgengine/pull/43) | **Merged / recorded-verified**, not UI |
| M1.2 | Browser-facing typed optional crop fields and accessible editor (drag, keyboard, zoom, reset) | Browser component/UX tests, typed serializer, local preview cleanup; [Astra M1.2 handoff](ASTRA_P13_M1_2_FRONTEND_READY_HANDOFF_2026-11-15.md) | **Next / ready to start** |
| M1.3 | Crop end-to-end parity and release candidate | Exact-head cross-boundary evidence, no compliance claims | Planned |
| M2 | Astra-led Wasm architecture/threat review and small Worker prototype | Browser memory/latency/bundle/parity/security measurements, go/no-go | Planned |
| M3 | Browser crop, rotate, resize, JPEG/PNG/WebP conversion | Real-device tests, no-upload assertions, failure UX | Planned |
| M4 | Target-byte compression (20/50/100 KB and custom) | Explicit impossible-target behavior, bounded time/memory, fixture tests | Planned |
| M5 | Browser-local printable sheets and preset framework | Physical-size/DPI correctness, print/export tests, limits | Planned |
| M6 | Public free tool launch and economic telemetry | Accessibility, SEO usefulness, privacy, legal, deployment/rollback, margin evidence | Planned |
| M7 | Bounded local batch + saved advanced presets | Device limits, partial failure and ZIP export tests | Planned |
| M8 | Professional tools (watermark, backgrounds, text, layers in stages) | Customer demand, edit/undo memory and fidelity evidence | Planned |
| M9 | Paid B2B API, quotas, metering, bulk/PDF/prepress where justified | Tenant threat model, billing reconciliation, abuse and resource gates | Planned |

**Important boundary:** P13 M1.2 uses a **local interaction preview** and server-rendered JPEG when the customer presses Render (upload occurs). It is **not** a browser-only Wasm or zero-upload tool; those are planned for M2+. P13 M1.3 browser/native exact-image parity remains separate.

**Parallel release track:** P14 actual-VPS evidence can proceed on a separately approved candidate and host; it is not proof of browser-Wasm capability.

## Proposed routes (not yet implemented)
| Category | Proposed routes | Shared engine operation |
| --- | --- | --- |
| Core | `/crop-image`, `/resize-image`, `/rotate-image`, `/convert-image` | Crop, resample, transform, encode |
| Compression | `/compress-image`, `/compress-image-to-20kb`, `/compress-image-to-50kb` | Bounded encode/quality/resize strategy |
| Print | `/photo-sheet-maker`, `/a4-photo-layout`, `/passport-photo` | Versioned physical layout, preview, export |
| Applications | `/resize-signature`, `/merge-id-cards` | Local image processing with sensitive-data safeguards |
| Studio | `/watermark-image`, `/batch-image-resizer`, `/studio` | Reusable operations and composition |

SEO pages must offer useful differentiated content, not doorway pages. Country/visa photo presets need authoritative source, version, review date, tests and careful non-guarantee language.

## Free and paid hypotheses — not approved prices
- Free: reliable local crop, resize, compression, conversion and basic sheets.
- Studio Pro: saved/custom presets, larger *bounded* batch workflows, reusable templates, professional layout/watermarking and workspace convenience.
- B2B: authenticated metered server API, bulk processing, prepress only after demand evidence.
- **Batch workflow:** apply one validated recipe to many files, report per-file failures, export bounded ZIP.
- **Advanced preset:** saved image dimensions, crop, format, compression, DPI, margin, naming rules.
- **Professional tools:** precision positioning, text/watermarks, reusable sheets, layers/masks when justified.
- **Priority features:** early access or optional priority for *server* jobs; cannot promise priority on a customer's CPU.

## Business metrics
Measure task success, retry rate, mobile failure rate, privacy opt-in rate, per-tool conversion, weekly active users, paying customers, churn, support burden, cost per server job, bandwidth, contribution margin, and break-even. No promise of profitability or market-scale outcomes without data.

## Astra review gates
- Before M2 implementation: compare Rust-only Wasm, C17 geometry Wasm, browser-native codecs, and hybrid; review licensing, CSP/COOP/COEP, worker isolation, memory/DoS, fidelity and fallbacks.
- Before native ABI/FFI changes: independent security and ownership review.
- Before public launch: threat model, privacy/legal and operational readiness.
- Routine UI docs and bounded frontend changes should use a lower-cost working model and change-scoped tests.
