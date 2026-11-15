# IMGENGINE — Seven-Level Engineering Operating Model

**Status:** Proposed operating policy for human review; documentation only (2026-11-15). This document adds an engineering decision and review method. It does **not** certify production readiness, grant an AI agent independent authority, implement features, create seven autonomous engineers, or override the repository's existing security, ABI, release, or Git rules.

**Start with:** [root AGENTS](../AGENTS.md), [current work/status](../next.md), [Astra kickoff](ASTRA_ENGINEERING_START_HERE.md), [technology ADR](TECHNOLOGY_AND_REPOSITORY_POLICY.md), [change-scoped verification](CHANGE_SCOPED_VERIFICATION.md), and the affected directory's scoped AGENTS. Use the [milestone/agent task template](AGENT_MILESTONE_HANDOFF_TEMPLATE.md) to operationalize this model.

## 1. Mission and non-negotiable constraints

Deliver a correct, secure, testable, maintainable, observable, financially sustainable image-processing product. An agent should **reason through seven engineering perspectives** as needed, not run seven parallel uncontrolled changes or assert a human job title. Its conclusions remain proposals until supported by source and measurements.

1. **Priority order:** correctness and user privacy -> security -> reliability -> measured performance -> maintainability -> total business cost. A performance improvement that breaks output parity, introduces races, or leaks images is a regression.
2. **Approved target:** strict C17 execution core in **imgengine/**; versioned C ABI; safe Rust FFI and bounded Rust/Tokio/Axum API in **backend/**; Nuxt/Vue/TypeScript frontend in **imgengine-saas/frontend/**; active Docker/Caddy in **infra/**.
3. **No new** Python/FastAPI/Django/DRF/Flask, Celery, Redis, PostgreSQL, Python scripts, speculative broker/database, or unrelated Kubernetes/microservices. Retained Python code in **imgengine-saas/backend/** and pre-existing verification scripts are legacy; do not silently retire, break or extend them as the target.
4. **Data:** privacy-first, ephemeral image processing; no automatic durable user-image/job storage, no leaking uploads, secrets, filenames or image bytes into diagnostics. Accounts, billing, taxes, durable metadata and external integrations need separate approved architecture and privacy decisions.
5. **Change control:** inspect HEAD/base, current source and contracts; make one bounded branch/PR per deliverable; do not rebase away other people's work, delete uncommitted changes, merge, publicly deploy, rotate secrets, alter billing or retire legacy without explicit human approval. Existing required CI/release checks remain in force.
6. **Truthfulness:** planned, source-implemented, previously tested, freshly tested, production deployed and commercially proven are separate states. State the tested SHA, environment and limitations. Never promise nanosecond images, universal 2 ms latency, infinite capacity, unconditional ARM64 support or guaranteed profit.

This model supplements, and cannot weaken, [root/scoped AGENTS](../AGENTS.md), binding ABI/API contracts, [MVP release checklist](MVP_RELEASE_CHECKLIST.md) and [P14 actual-host runbook](P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md). When instructions conflict, stop and surface the conflict for a human decision; do not choose the less safe rule.

## 2. The seven engineering perspectives

These are *review lenses*, not seven hired people, fixed job titles, or a promise of independent AI agents. Apply the smallest sufficient set on every task and escalate review depth when the risk or scope warrants it. Titles and leveling vary by employer.

| Perspective | Primary question | Expected contribution | Evidence of success |
| --- | --- | --- | --- |
| **L1 Junior** | Do I understand the requirement and existing style? | Locate the affected files, reproduce reported behavior, identify preconditions, propose focused tests. | Small correct diff; clear reproduction and peer-review questions. |
| **L2 Mid-Level** | Can this feature work end to end in its bounded scope? | Implement a cohesive vertical slice, handle errors, add contract/unit tests, document assumptions. | Passing scoped tests and working user-visible outcome. |
| **L3 Senior** | Is this system correct, secure and operable? | Own HLD/LLD for complex changes, API/ABI compatibility, threat/failure paths, production diagnostics and rollback. | Edge-case tests, failure-mode analysis, operational plan and maintainable implementation. |
| **L4 Staff** | Will the layers and teams integrate safely? | Define cross-boundary contracts, ownership, reusable standards and migration/compatibility plans. | C/Rust/Nuxt parity and reduced integration ambiguity. |
| **L5 Senior Staff** | Can large multi-workstream risks be resolved coherently? | Sequence dependent milestones, isolate systemic resource/supply-chain risks, mentor reviewers, expose blocking evidence. | Multiple bounded PRs with explicit dependencies and closed risk items. |
| **L6 Principal** | Is the architecture the right long-term trade-off? | Compare alternatives, address high-impact concurrency and security, preserve product simplicity, set quality and capacity decisions. | Decision record with measured trade-offs and a reversible implementation path. |
| **L7 Distinguished** | What enduring technical capability actually differentiates IMGENGINE? | Challenge foundational assumptions, assess portability/SDK/ecosystem and technical differentiation without speculative redesign. | A falsifiable strategic hypothesis supported by user value and engineering evidence. |

**Guardrail:** Junior-to-Principal reviews do not have to become separate files or handoffs for every typo. For a small UI copy edit, a focused implementer plus relevant reviewer is enough; for ABI, memory-safety, security boundary and production rollout changes, explicitly engage Senior/Staff/Principal checks. Distinguished review is for material, long-horizon decisions, not ordinary CRUD.

## 3. Additional independent review lenses

| Lens | Responsibility | Required decision/evidence |
| --- | --- | --- |
| **Security engineer / privacy engineer** | Threat-model untrusted image bytes, codecs, native memory, FFI, auth, proxy secrets, Docker isolation, dependency supply chain and abuse. | Trust boundaries, plausible attacks, controls, residual risk, reproducible tests and privacy impact; report hypothesis vs confirmed vulnerability. |
| **SRE / infrastructure engineer** | Model request admission, peak RSS/CPU/tmpfs, latency, capacity, failure recovery, health checks, observability, secret mounts, TLS, backup of configuration and rollback. | Exact environment, expected failure handling, release/restore rehearsal; never equate static Compose fields with live proof. |
| **Product manager / UX** | Define the customer job, quality oracle, accessible workflow, acceptance and support burden. | Explicit user value and measurable completion/quality criteria; mobile, keyboard and error recovery where relevant. |
| **CA / finance analyst** | Separate revenue, cash flow, margin, capex/opex, taxes and liabilities; avoid imagined sales. | Cost assumptions, unit economics, sensitivity, recordkeeping and cash-runway implications; seek qualified local advice for legal/tax decisions. |
| **MBA / business manager / founder** | Prioritize work, allocate scarce engineering effort, test willingness to pay, evaluate risk and strategic positioning. | Opportunity cost, stage gates, milestone owner, customer evidence and stop/go choices. |
| **CTO / engineering manager** | Make sequencing, review ownership, delivery, hiring and technical-investment trade-offs. | Single accountable decision-maker, realistic staffing and a clear escalation path. |

**Decision rights:** AI agents can analyze, draft code/docs and report evidence within the authorized scope. The user or authorized humans approve irreversible operations, spending, account/secret changes, deployment, main-branch merge, public SDK/legal commitments and deviations from accepted architecture. An AI review is not an independent security certification or a licensed accounting opinion.

## 4. Contract and folder ownership

| Boundary | Owns | Must not own | Canonical review sources |
| --- | --- | --- | --- |
| **imgengine/** (C17) | Decode/layout/render, safe allocation, scheduler, scalar/SIMD kernels, exported stable C ABI | HTTP, user identity, billing, Rust application policy | [native AGENTS](../imgengine/AGENTS.md), [C17 source audit](C17_CORE_PRINCIPAL_SOURCE_LLD_DSA_SECURITY_AUDIT_2026-11-15.md), [ABI](ABI.md) |
| **backend/** (Rust) | Safe native ownership, error/status mapping, bounded request admission, Axum transport, lifecycle and ephemeral cleanup | Duplicated image-processing kernels, duplicate C scheduler, permanent jobs by default | [Rust AGENTS](../backend/AGENTS.md), [FFI](RUST_FFI.md), [request lifecycle](RUST_REQUEST_LIFECYCLE.md), [API](api.md) |
| **imgengine-saas/frontend/** | Nuxt UI, input/UX validation, same-origin server proxy, accessibility, preview resource cleanup | Exposed Rust credentials, independent image backend, invented ABI fields | [frontend AGENTS](../imgengine-saas/frontend/AGENTS.md), [P12](P12_PHYSICAL_SHEET_CONTRACT.md), [P13](P13_INTERACTIVE_CROP_CONTRACT.md) |
| **infra/** | Approved local/simulated/prod-candidate Compose, Docker/Caddy, runtime hardening and deployment evidence | Unapproved public hosting, plaintext credentials in commits, assumed VPS proof | [infra AGENTS](../infra/AGENTS.md), [infra audit](INFRA_SECURITY_OPERATIONS_AUDIT_2026-11-15.md), [P14 actual VPS](P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md) |
| **imgengine-saas/backend/** | Retained Python legacy behavior during controlled migration | New target product features or substitute Rust services | [legacy AGENTS](../imgengine-saas/backend/AGENTS.md), [migration](migration.md) |

**Dependency direction:** Browser -> Nuxt same-origin proxy -> Rust/Axum -> safe Rust FFI -> stable C ABI -> C native engine. Preserve C allocator ownership and caller-allocated/borrowed buffer lifetimes. Browser cancellation alone does not imply mid-operation native cancellation.

## 5. Required review for a meaningful change

1. **Discovery:** identify task ID, exact base SHA, changed files, current behavior, authoritative contracts, prior tests and any dirty worktree.
2. **User outcome:** state whose problem is solved, acceptance criteria, non-goals and reversible rollout scope. No invented feature or API fields.
3. **HLD (when boundaries change):** actors, data/trust flow, responsibilities, alternatives, synchronous vs asynchronous behavior, failure and rollback paths.
4. **LLD:** structs/types, ownership/lifetimes, input validation, return errors, integer arithmetic, concurrency invariants, allocations, output caps, compatibility and deterministic tests.
5. **DSA/pattern review:** explain actual data structures, complexity, lock/atomic contract, cache behavior and algorithmic cost. Use SOLID, DRY, separation of concerns, adapters, strategies, or hexagonal seams only where they improve clarity/testability. DDD/event-driven architecture and a GoF pattern catalog are not mandatory features.
6. **Security/privacy:** enumerate attacker-controlled inputs and trust boundaries, decoder/process isolation, FFI and native UB risks, auth and abuse limits, secret redaction, resource exhaustion, supply chain and residual risk.
7. **Failure engineering:** overload, timeout, disconnect, partial output, cleanup, crash, restart, disk-full, unavailable dependency and rollback. Avoid unbounded queues and silent fallbacks that upload private images.
8. **Verification:** first make focused regressions and contract assertions; run tests appropriate to changed files per [change-scoped verification](CHANGE_SCOPED_VERIFICATION.md); do not weaken required CI. Reproduce or clearly label speculative concurrency findings.
9. **Performance:** record test hardware, input size/codec, image dimensions, DPI, batch/concurrency, scalar vs SIMD, warmup, p50/p95/p99, throughput, output correctness and peak total RSS. Compare against measured baseline; optimize only real bottlenecks.
10. **Business review:** estimate total cost per *successful* image, maintenance/support complexity, privacy and compliance trade-offs, customer willingness to pay, deployment economics and opportunity cost. Mark every unknown as unknown.
11. **Review and handoff:** show changed paths, diff summary, results/failures, exact commit SHA, residual risks, rollback, requested decision and next action. Update only relevant contracts/status; don't copy detailed evidence into next.md.
12. **Release:** obtain the separate human go/no-go and exact candidate P14 security/TLS/secret/capacity/alert/rollback proof. A merge, passing unit test or docs audit is **not** a public release.

### Test gates by change type

| Change | Minimum evidence; extend when a boundary changes |
| --- | --- |
| **Documentation only** | Markdown links/path resolution, consistency and diff inspection; existing required CI remains. No need to manually compile native C. |
| **Nuxt user flow** | Typecheck, focused UI tests, accessibility/keyboard/mobile/preview cleanup; browser E2E for the changed journey. |
| **Rust HTTP/proxy** | Input/response contract tests, negative security cases, admission/deadline/disconnect handling; end-to-end proxy parity. |
| **C ABI / Rust FFI** | ABI/export/layout checks, old consumer compatibility, ownership tests, Rust integration, appropriate C11/C17 and sanitizer gates. |
| **C core scheduler, decoder, SIMD** | Deterministic concurrency/error tests, native C17/C11, ASan/UBSan, applicable fuzz and scalar/SIMD quality parity, baseline benchmark. |
| **Container/CI/release** | Config and policy checks, artifact/digest provenance, runtime permissions, health/readiness, secrets, resource limits and rollback evidence for affected scope. |

For required workflows and conditional gates, [change-scoped verification](CHANGE_SCOPED_VERIFICATION.md) and existing CI remain authoritative. Do **not** promise that a proposed test has been run.

## 6. Existing high-priority risks and sequence (snapshot, not a second backlog)

The source-aligned register is [next.md](../next.md). This section guides review, **not** status changes:

- **Native C-A0 P0 review:** inspect work-stealing consumers of an SPSC queue and the metrics atomic/plain-access question. Reproduce concurrency reachability and exactly-once behavior before a scheduler/metrics fix. Findings are not confirmed exploits.
- **Native P0 resource evidence:** budget and measure whole-request decoded input, scratch, canvas, output, Rust and container aggregate memory at realistic concurrency; encoded-output caps alone do not prove safe peak RSS.
- **P13 M1.1:** review/freeze native ABI 2.3 -> safe Rust FFI -> Axum -> typed Nuxt proxy transport contract, preserve P12 ABI 2.2 centered-fill.
- **P13 M1.2/M1.3:** accessible keyboard/touch UI, deterministic crop coordinates, image parity and legacy behavior regression **after** M1.1 contract freeze.
- **P14:** Docker/Caddy candidate needs real-host secrets, rate/abuse, aggregate resources, DNS/TLS, monitoring, canary and rollback evidence before launch.
- **Public static .a, future browser Wasm, paid accounts/billing:** distinct later decisions; not implied by existing internal static library or product roadmap.

The infra, native and P13 tracks can be **reviewed** in parallel with independent branches, but edits crossing contracts must be sequenced. A review may recommend **no code change** if it cannot substantiate a fix.

## 7. Business scorecard: CA/MBA-level decisions without fictional numbers

For each meaningful milestone, record: customer segment, exact job-to-be-done, feature success rate, service latency/capacity, direct operating cost, support time, variable price (if known), privacy/compliance risk, engineering effort and competing priorities.

**Definitions and formulas (state currency, period and assumptions):**
- **Contribution per successful paid job** = net revenue actually attributable to that job - variable infrastructure/transaction/support costs for that job.
- **Variable cost per successful job** = total variable cost of attempted work including failed/abusive requests / number of successful chargeable jobs.
- **Gross margin** = (net revenue - cost of providing the service) / net revenue, when net revenue is nonzero and accounting scope is defined.
- **Break-even volume** (simple constant-margin model) = monthly fixed costs / positive contribution per job. If contribution is zero/negative, a finite break-even volume does not exist in that model.
- **Cash runway** = available unrestricted cash / positive net monthly cash burn, only when burn and cash are measured consistently.

Track **free-tier quota eligibility, actual bills, tax/payment fees and FX assumptions separately**. A cloud provider's free-tier description does not guarantee permanent capacity, zero charges or unlimited traffic. Favor one low-cost host if it passes security/capacity proof; do not add a broker or DB to look enterprise-grade.

### Decision memo for architecture or spending

Use a short ADR: **Context -> options (including do nothing) -> assumptions -> measured evidence -> security/privacy and operability -> implementation/migration cost -> cost per success -> irreversible risks -> recommendation -> human approval required.** Use a simple weighted ranking only when weights and evidence are explicit; avoid false quantitative precision.

## 8. Definition of done and review quality

A change is **DONE within its stated scope** only if:
- Its requirement and source/contract ownership are clear.
- A small, reviewable implementation and negative-path tests exist when code changes.
- Relevant compatibility, privacy, security and reliability invariants hold with recorded evidence.
- Reproducible commands, revision, environment and tests are reported; failures and skipped tests are disclosed.
- Documentation, decision register and **only** affected next.md status are aligned.
- A rollback or revert path and any operational impact are documented.
- Reviewer knows remaining risks and the exact next decision.
- CI is not bypassed; any required merge/release decision is separately authorized.

A **documentation-only PR** can satisfy its own writing/link gates but cannot make P13, concurrency, P14, pricing or production status DONE.

## 9. Agent delegation, escalation and output contract

**Agent-neutral:** Astra, Codex and other coding assistants can use the same template and this policy. The model name is not a quality, permission or security guarantee. Do not assume a role is a distinct running agent or that any model automatically reads skills.md; explicitly point it at the right files.

- One **task owner** per PR. The owner reports the seven lenses proportionally to risk and may request independent review; parallel agents must use separate branches and mutually exclusive files or an agreed integration plan.
- **Escalate to human:** conflicting contracts; unsupported platform claims; new persistent data, user identity, billing or spending; native breaking ABI; secret handling; CI bypass; forced Git history; release, merging or legacy removal.
- **Stop and disclose:** unrepeatable bug, incomplete source access, unverified security hypothesis, failing required tests, unsupported benchmark, unexpected diff or untracked user work.
- **Delivery format:** task/why; base revision; evidence vs uncertainty; design and alternatives; changed paths; tests with exact results; security/privacy/cost impact; residual risk; requested decision; next small milestone.

**Reusable task form:** [Agent Milestone Handoff Template](AGENT_MILESTONE_HANDOFF_TEMPLATE.md). Do not copy this whole operating model into every agent prompt; link the applicable sections and scoped AGENTS instead.

## 10. Maintenance and provenance

**Stable rules** belong in AGENTS and this operating model; **recipes** in skills.md; **current progress** in next.md; **versioned contracts/design decisions/evidence** in docs/. Revise this guide only for a deliberate policy change, not for each passing unit test. When a section has an as-of snapshot, refresh the source/SHA and distinguish prior from current evidence.

**Proposed review owner:** project owner, with architecture, security, operations and finance approval as applicable. **No implementation, new automated controls, real-host inspection or production verification is claimed by this guide.**
