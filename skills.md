# IMGENGINE — Engineering Skills and Workflow Recipes

**Status:** Optional task playbook; not an executable skill, installed Codex skill, or automatic GitHub Copilot instruction. The tracked repository did not previously contain a root `skills.md` on 2026-11-14. **`AGENTS.md` and nested `AGENTS.md` are authoritative for agent rules**; this file provides reusable *how-to* recipes without replacing them. If using a tool-specific skill installer, follow that tool's supported skill format and path rather than assuming this markdown is auto-loaded.

## Authority and startup

Read, in this order:
1. `AGENTS.md` and relevant nested `AGENTS.md` (rules and boundaries).
2. `next.md` (current state and one active milestone).
3. The corresponding versioned contract in `docs/` and latest implementation/tests.
4. `docs/CHANGE_SCOPED_VERIFICATION.md` (which tests are justified).
5. This file for task-specific recipes.

Stop when the repository differs from the declared SHA or critical evidence cannot be verified. Preserve separate dirty worktrees; never silently modify them. No push/merge/deploy without explicit approval for the relevant action.

## Skill 1 — Scoped feature implementation

- Identify user outcome, current API/ABI contract, affected directories, acceptance criteria and rollback.
- Inspect only relevant files first; avoid dumping unrelated C source or test logs into the model context.
- Create one small feature branch; implement one layer/milestone at a time.
- Keep public contracts versioned; use typed boundaries and explicit errors.
- Run focused tests appropriate to touched files. Summarize pass/fail, revision and meaningful failure excerpts; link full CI logs instead of pasting them.
- Update `next.md` status and affected documentation in the same PR.

## Skill 2 — Nuxt frontend and browser privacy

- Prefer accessible components, keyboard and touch controls, typed API adapters, and no secrets in public config.
- For local image jobs, never upload image bytes or analytics containing image data without explicit authorization. Revoke object URLs, release buffers, handle tab/unmount cancellation and limit input dimensions.
- Run frontend tests, typecheck/build and relevant browser/E2E tests.
- **Do not manually run native CMake/sanitizer/fuzz for pure UI changes**. Required GitHub CI remains in force.

## Skill 3 — Rust/WebAssembly Worker

- Start with architecture and threat review; prototype a bounded single operation before introducing a full codec port.
- Compare browser-native codecs, Rust/Wasm geometry, portable C17-Wasm and hybrid execution. Evaluate memory allocations/copies, device compatibility, bundle and startup cost, SIMD/thread limitations, licensing, color/orientation parity and CSP/isolation.
- Test malformed images and input sizes, Worker crash/cancellation, clean resource release, no network upload, and mobile memory pressure.
- Run focused Wasm/Rust/Worker/browser checks; do not manually run native C tests unless shared C/ABI/FFI files or ownership changed.
- Never promise zero-copy, universal 30 ms compression, or 60 FPS without reproducible measurements.

## Skill 4 — Native C17, ABI and Rust FFI

- Shipping portable core is strict C17. Preserve installed C11 consumer behavior; C23 is isolated and capability-gated only.
- Version additive ABI changes, freeze public layouts, explicitly document allocation/release and unsafe Rust invariants.
- Run impacted ABI symbols/struct-layout/consumer tests, native regressions, C11/C17 builds when relevant, ASan/UBSan/fuzz where affected, and required authoritative verification before a native release.
- Do not replace the proven C scheduler without an evidence-backed architecture decision.

## Skill 5 — Security and deployment

- Treat every upload as hostile; validate at every trust boundary. Bound bytes, decoded pixels, intermediate buffers, canvas, CPU time, disk, concurrency and output.
- Keep processing ephemeral and logs redacted. No client secrets or unapproved permanent retention.
- Test deployment controls only in an approved environment. Real VPS inventory, TLS/firewall, monitoring, rollback, canary and ownership decisions are mandatory for public release.
- No readiness claim solely from Docker Desktop, local screenshot, or CI.

## Skill 6 — Product, CA/MBA and commercial review

- Define the customer, job-to-be-done, demand evidence, product scope, implementation/operating cost and commercial risk.
- Prefer free browser-local crop/resize/compress/basic print tools; monetize bounded batch operations, reusable presets and business workflows when willingness to pay is demonstrated.
- Calculate contribution margin as revenue net of direct variable costs; separately record fixed overhead, acquisition cost, retention, refunds, taxes and support burden.
- Do not introduce billing, tenant databases, queues or microservices before their customer and threat-model requirements are established.

## Model handoff — efficient tokens, preserved safety

| Task | Suggested model / effort | Validation |
| --- | --- | --- |
| Routine docs, frontend UI, well-specified PR | Terra / Sol, medium | Focused diff and component tests |
| New Rust/Wasm architecture, browser memory model | Astra, high | ADR + threat model, prototype parity |
| ABI/unsafe FFI/security-sensitive review | Astra, high | Independent review + relevant native gates |
| Release readiness | Human owner + independent reviewer | Required exact-revision CI and real-host evidence |

Keep one compact handoff report per milestone: baseline SHA, changed files, commands, results, unresolved risks, documentation updates, and PR link. Avoid rerunning unrelated suites manually.

## Maintenance rules
- Treat `next.md` as live state; do not duplicate the entire backlog here.
- If a workflow becomes obsolete, mark it superseded rather than silently deleting historical intent.
- Separate "planned", "implemented", "validated" and "production released" in every claim.
