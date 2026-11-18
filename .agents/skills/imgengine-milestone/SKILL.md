---
name: imgengine-milestone
description: Execute or continue the next approved, unblocked IMGENGINE engineering milestone from next.md in VS Code Codex. Use when the owner says "go ahead", "continue next milestone", asks for a bounded feature/infra milestone, or resumes work in a new Codex session. Keep local-first context and testing, risk-based engineering/security/business review, truthful PR/CI evidence, and owner approval gates. Do not invoke for discussion-only questions.
---

# IMGENGINE milestone executor (reusable, low-context)

**Inputs:** The owner's current instruction, repository-local `AGENTS.md`, and short live `next.md`. Use a fresh VS Code Codex session in the actual IMGENGINE checkout; the skill is **instruction-only** and cannot autonomously schedule work, authorize writes, bypass permissions or read private Usage metrics. Follow applicable nested `AGENTS.md` and explicit owner scope. Historical `AGENTS-history-2026-11-18.md`, `next-history.md` and full `skills.md` are **on-demand only**, not required startup reads.

## 1. Reconcile once and select one task

1. Inspect `git status -sb`, current HEAD, branch, remotes and worktree; read the compact NOW/NEXT/LATER rows in `next.md`. Check concise current `main`, open PR, exact head and required check outcomes **only when needed** to avoid duplicating completed work. Prefer local Git; use remote GitHub metadata, **not** repeated whole remote diffs/logs.
2. Confirm **one** owner-approved READY child task. Distinguish `PRODUCT-FREE-01` (exactly five initial tools) from `PRODUCT-PRO-01`, `PRODUCT-STUDIO-01` and `PRODUCT-API-01`; a child merge is **not** product release. Keep P7/P8/P12/P14 independent. If a PR is awaiting review, do not open a duplicate; if a feature is already merged, reconcile it and select the next eligible task.
3. If current code is dirty or an existing unpushed branch contains relevant work, **preserve and inspect it**; do not reset, overwrite, recreate or silently switch branches. If permission, task readiness, or environment is blocked, report exact blocker rather than fabricating progress. When the owner says discuss/read-only, do not edit.

## 2. Preflight the affected boundary, not the whole repo

- Identify the relevant contract/acceptance criteria, 2–5 likely files, test commands, negative cases, rollback and any scoped nested `AGENTS.md`. Do not automatically load all docs.
- **Frontend:** verify current Node/pnpm, Nuxt generated tsconfig, required local production build before Playwright, configured Chromium/Chrome channel and target E2E config. Treat a missing tool as `ENVIRONMENT_BLOCKED`; resolve once with the existing approved local setup where possible. Do not repeat futile failing runs or conflate a system-Chrome run with pinned-browser verification.
- **C17/Rust/FFI:** verify native compiler/CMake, Cargo, ABI version, ownership/thread bounds and focused sanitizer/fuzz contract; use Ubuntu 24.04/Docker lane as required. **Infra P8:** PowerShell script defect must be independently fixed and parser-tested before executing its simulation. **P14:** never assume Docker Desktop equals an actual approved VPS.
- Keep full local logs outside model context (ignored local path); return commands, SHA, short PASS/FAIL/NOT RUN, with a minimal relevant failure excerpt and a link/path to retained evidence.

## 3. One bounded implementation and verification

1. Plan the smallest change in the active technology: browser-local Nuxt/Vue/TS; server Rust/Axum + safe C17 FFI only when the task needs it. No new Python backend, speculative database or unrelated infrastructure. Use sound boundaries, modular design, SOLID/DRY and evidence-led performance.
2. Inspect adversarial input/failure paths, privacy, accessibility, cancellation, resource ceilings, cleanup and cross-feature regressions. For code touching shared image Worker/composable, run related sibling feature tests. Keep source, tests, contract and only affected milestone docs synchronized.
3. Iterate with focused tests. After implementation is stable, freeze the change and verify final exact revision once with the applicable local acceptance suite; **do not rerun identical passing checks** without changed evidence or a new SHA. Save full output locally and inspect only failures. Never downgrade existing seven-job GitHub CI or native/security release gates.
4. Review local diff, staged secrets, extra files, actual tests and optional benchmark delta; commit only if within owner authorization. Do **not push without explicit owner approval**. For authorized push/PR, publish a single bounded review PR, then fetch concise seven-job exact-head outcomes. Do not merge/deploy/change cloud/billing/secrets without separate approval.

## 4. Engineering + CTO + security + CA/MBA decision gate

Perform the checks **in proportion to impact**, not as seven verbose agent role-play reports:

| Lens | Acceptance question |
| --- | --- |
| Junior / mid / senior | Does it satisfy contract, integrate correctly and have unit/regression tests? |
| Lead / staff | Is it a small, maintainable change with clear ownership, lifecycle and rollback? |
| Principal / CTO | What are concurrency, ABI, measurable latency/memory, capacity, long-term architecture, product priority and operations consequences? |
| Security / SRE | How can malformed inputs, failures, permissions, leaked images/keys, overload and unavailable dependencies harm users? How will we detect/recover? |
| CA / MBA / management | **When relevant**: what user/shop value, support effort, hosting/egress costs, unit margin, pricing/tax/legal risk, milestone dependency and approval are affected? No invented costs/revenue. |
| Logic / evidence | What is verified at this SHA, what remains a hypothesis, and what test or customer evidence could disprove it? |

No full CFO/marketing report for a purely local CSS fix. Public paid/API/Studio releases require their *separate* commercial/security/host decision gates.

## 5. Close with a truthful handoff

Update only the relevant live `next.md` entry **when a docs change is authorized**, with task state, PR, test/merge SHA, CI link, pending risk and **one next READY task**; preserve historical evidence. A PR description does not update `main`, and a MERGED feature does not mean a RELEASED product. Do not create status-only commits that repeatedly restart full CI without need.

Provide at most ~12 concise lines:

```text
Milestone/parent: ...
State: READY | IN_PROGRESS | PR_OPEN | READY_FOR_REVIEW | MERGED | BLOCKED
Scope: ...
Base / exact tested SHA: ...
Tests: focused PASS/FAIL/NOT RUN with counts
GitHub CI: 7/7 PASS | PENDING | FAIL | NOT RUN, exact-head URL
Security/privacy/infra/business gate: passed or explicitly not verified
PR/approval: ... ; merge/deploy: NOT DONE unless independently authorized
Next: one READY milestone or precise blocker
```

**For every new Codex session:** invoke `$imgengine-milestone Go ahead`. The skill's short metadata is discoverable; its full body is used for this workflow and detailed references only when necessary. No copying a long prompt each time. See [six low-context rules](../../../docs/CODEX_VSCODE_LOW_CONTEXT_SIX_RULES.md) if troubleshooting excessive context.
