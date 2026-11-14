# IMGENGINE — Browser-first image platform roadmap

**Status: product strategy and proposed milestones, NOT a claim of implemented browser processing.**  
**Audience:** product, principal engineering, browser/Wasm, security, operations, and finance.

## North star and operating principles

Become a reliable image-only toolkit for individuals, photo studios, print shops, students, and API customers. Native C17 ABI + C scheduler stay authoritative for the current server-backed product; Rust/Axum owns server control/admission; Nuxt/Vue/TypeScript owns interface and SEO. Use browser-local execution **where proven** to improve cost, privacy, and UX. Reject a universal mandate to run every image operation through Rust/Wasm.

No Python runtime in the target architecture; legacy Python services remain until approved retirement. No permanent image retention in the first-MVP image path. No uploading image data to fallback servers without clear user consent. Avoid unfounded compliance, zero-copy, 30-ms, 60-FPS, zero-cost, or print-color-fidelity guarantees.

## Dependency-ordered stages (proposed future scope; retain historical P-number names)

| Stage | Outcome | Acceptance / exit evidence | Owner/review |
| --- | --- | --- | --- |
| M0 — portability | **Merged PR #29**; C17 default, C11 regressions, C23 isolated, Wasm ADR | Recorded Ubuntu 24.04 verifier; not VPS proof | Native engineering |
| M1.1 — crop transport | ABI 2.3 crop through Rust FFI → Axum → Nuxt, absent crop still P12 centered-fill | Focused cross-boundary tests + integration and ABI compatibility | Rust + native boundary; independent Astra review |
| M1.2 — crop editor | Accessible drag/zoom/reset/keyboard crop, local preview, object-URL cleanup | Browser tests, keyboard/mobile behavior, geometry parity | Frontend |
| M1.3 — crop E2E | Customer-selected crop is rendered correctly for exact copies | End-to-end, invalid-input, ownership/security verification | Integration |
| W0 — browser architecture gate | Compare browser codecs+JS, Rust-only Wasm, portable C17 geometry-Wasm; select minimally risky experiment | ADR, memory/bundle/browser matrix, threat model, measurable go/no-go | **Astra architecture review before coding** |
| W1 — local browser prototype | Web Worker, no network image uploads, bounded geometry/crop experiment; no product claim | Cross-browser and low-memory fixtures, cancellation/cleanup, bundle/latency profile | Browser/Wasm |
| W2 — image MVP | Crop, rotate, resize and JPEG/PNG/WebP export where supported, sharing processing contracts | Error cases, orientation/color handling, consistent downloads and privacy test | Frontend + Wasm as justified |
| W3 — compression | Target size presets (20/50/100 KB or custom), explicit cannot-satisfy case | Representative quality, CPU/memory benchmarks, hard byte-ceiling output check | Browser engine |
| W4 — print layouts | Browser sheet construction using reviewed geometry; server option only with consent | 300 DPI physical-size/print validation, memory and export integrity | Browser + print domain |
| W5 — commercial MVP | Helpful route-specific pages; pricing experiment, paid convenience | Conversion, retention, support, gross margin/unit economics | Product/finance |
| W6+ — advanced | Batch image tools, saved presets, watermark/text, layered editor, export & eventually paid B2B APIs | Customer demand and independent security/release gate | Product and engineering |
| P14 — operational release | Actual VPS security/capacity, rollback, monitoring, canary | `docs/MVP_RELEASE_CHECKLIST.md` complete | SRE/security/release owner |

P14 is an **independent release gate**, not something W0/W1 can substitute for. A browser-only first release still needs HTTPS, accessibility, dependency/security checks, privacy claims verified, and deployment discipline.

## Proposed routes mapped to reusable operations (not implemented claims)

| Category | Example public routes | Shared engine operation |
| --- | --- | --- |
| Everyday | `/crop-image`, `/resize-image`, `/rotate-image`, `/compress-image`, `/convert-image` | crop, resample, transform, encode |
| Search-specific tasks | `/compress-image-to-20kb`, `/compress-image-to-50kb`, `/resize-signature` | presets over the same bounded compressor/resizer |
| Print/studio | `/photo-sheet-maker`, `/a4-photo-layout`, `/passport-photo`, `/studio` | sheet geometry, compositing, export |
| Professional | `/batch-image-resizer`, `/watermark-image`, `/merge-id-cards` | batch coordinator, overlay, layout |

Routes are examples and should not be published without working, differentiated features and helpful content. Passport/visa presets require official source provenance, review date, and non-guarantee wording. ID and signature images are sensitive; prefer fully local processing.

## Architecture proposal

```text
Nuxt/Vue/TypeScript routes + typed UI + shared product contracts
  |
  +-- default: Web Worker -> browser-native codecs and/or bounded Rust/Wasm geometry
  |              (local file bytes; no implicit upload)
  |
  +-- opt-in server: same-origin Nuxt proxy -> authenticated Rust/Axum
                    -> bounded admission -> safe FFI -> portable C17 core/C scheduler
```

Start from browser-native codecs and a small Wasm geometry proof; only bring native C into Wasm when measured parity/performance/reuse warrants it. Wasm does not make memory transfers inherently zero-copy, C/FFI inherently safe, or browser codec output reproducible across engines. Verify EXIF orientation, alpha, ICC/CMYK, memory peaks, lossless/lossy differences, and browser fallback behavior.

## Commercial model — hypotheses, not forecasts

- **Free:** useful local crop/resize/format conversion/compression/basic sheets to acquire trust and discoverability.
- **Studio Pro candidates:** batch workflows (multiple files with a saved recipe and per-file error report), advanced presets (named, repeatable print/exam/custom configurations), professional tools (watermark, alignment, cut marks, templates, non-destructive editing later), and early access to *optional* features.
- **Server/B2B:** paid high-resource jobs and authenticated APIs only once cost and abuse/tenant boundaries are proven.
- **Do not charge solely for CPU priority in a customer's own browser**; that cannot be reliably promised.
- **Measure:** product activation, jobs/customer, retries, completion, p50/p95 latency by device, free→paid conversion, churn, support load, bytes transferred, server CPU/RSS, infrastructure spend, payment fees, customer acquisition cost, contribution margin. Profit = collected revenue minus all associated costs, not gross receipts.
- **Go/no-go:** interview real shop operators and run low-cost pricing experiments before advanced editor or billing infrastructure.

## Review workflow and model budgeting

Use Terra/Sol for small implementation scopes. Use **Astra** at W0 to challenge JS vs Rust/Wasm vs C17/Wasm design *before coding*, at high-risk FFI/privacy/format milestones, and for pre-release independent review. An AI review is advice, not a substitute for tests, security audit, or owner approval.

Every milestone has a scope/evidence receipt, and only relevant tests run during local edits; see `docs/CHANGE_IMPACT_TEST_POLICY.md`. The native C heavy suite remains mandatory at native/ABI/security-boundary or release gates, not as a default for browser-only changes.
