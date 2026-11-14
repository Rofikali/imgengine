# IMGENGINE C/Rust Documentation Map and Status Authority

**Audit base:** `e0f8a32044c8c2d9e68ae37299abdedaedf318a7` (2026-11-14). This maps **all 81 tracked `docs/**/*.md` files** by intended purpose and time period. A heading alone is not proof of implementation. See [native/Rust audit](NATIVE_C_RUST_DOCUMENTATION_AUDIT_2026-11-14.md).

## Reading order for C / Rust milestones

1. [Root engineering policy](../AGENTS.md) and [Native directory policy](../imgengine/AGENTS.md). The backend AGENTS in `imgengine-saas/backend/` applies only to files under that directory; Rust code under `imgengine/rust/` is under native AGENTS and root policy.
2. [Next action inventory](../next.md): finished, partial, blocked, planned, historical.
3. [Task recipes](../skills.md) and [change-scoped testing](CHANGE_SCOPED_VERIFICATION.md).
4. For **native C library**: [public ABI](ABI.md), [installed exports](abi/exported_symbols.md), [native development](NATIVE_DEVELOPMENT.md), [engine spec](ENGINE_SPEC.md), [C17 ADR](M0_C17_PORTABILITY_AND_WASM_ADR.md).
5. For **Rust FFI and backend**: [Rust FFI](RUST_FFI.md), [supervisor](RUST_SUPERVISOR.md), [request lifecycle](RUST_REQUEST_LIFECYCLE.md), [HTTP contract](api.md), [architecture](architecture.md).
6. For **security/operations**: [security target](security.md), [threat model](threat-model.md), [P14 deployment baseline](P14_PRODUCTION_BASELINE.md), [actual-host runbook](P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md), [MVP release checklist](MVP_RELEASE_CHECKLIST.md).
7. For **performance**: [native benchmarking](NATIVE_BENCHMARKING.md) and [scheduler measurement](SCHEDULER_CHARACTERIZATION_REPORT.md), never historical HLD throughput numbers by themselves.

## Normative source hierarchy

Public C exports and installed headers govern the ABI. Implementation and tests at an identified SHA govern what exists. The Rust wrapper and Rust service tests govern ownership and transport. Measured evidence with date/environment governs **validated** claims. Historical design documents capture intent but cannot silently override newer source/contract/evidence. Each security or performance assertion must cite its evidence and disclose residual limitations.

## Complete Markdown index

### Current native ABI, implementation contracts, and development procedures

**Category:** CURRENT / WITH GAPS. ABI 1.0–2.3 and tested native paths are current; some design files retain pre-implementation language. P13 C-side ABI 2.3 is implemented, but whole interactive transport is partial.

| Document | Indexed status |
| --- | --- |
| [ABI.md](ABI.md) | CURRENT / WITH GAPS |
| [abi/exported_symbols.md](abi/exported_symbols.md) | CURRENT / WITH GAPS |
| [ENGINE_SPEC.md](ENGINE_SPEC.md) | CURRENT / WITH GAPS |
| [NATIVE_DEVELOPMENT.md](NATIVE_DEVELOPMENT.md) | CURRENT / WITH GAPS |
| [LINUX_VERIFICATION.md](LINUX_VERIFICATION.md) | CURRENT / WITH GAPS |
| [REAL_IMAGE_VALIDATION.md](REAL_IMAGE_VALIDATION.md) | CURRENT / WITH GAPS |
| [REAL_IMAGE_VERIFICATION.md](REAL_IMAGE_VERIFICATION.md) | CURRENT / WITH GAPS |
| [M0_C17_PORTABILITY_AND_WASM_ADR.md](M0_C17_PORTABILITY_AND_WASM_ADR.md) | CURRENT / WITH GAPS |
| [P9_3_BOUNDED_OUTPUT_API_DESIGN.md](P9_3_BOUNDED_OUTPUT_API_DESIGN.md) | CURRENT / WITH GAPS |
| [P12_PHYSICAL_SHEET_CONTRACT.md](P12_PHYSICAL_SHEET_CONTRACT.md) | CURRENT / WITH GAPS |
| [P13_INTERACTIVE_CROP_CONTRACT.md](P13_INTERACTIVE_CROP_CONTRACT.md) | CURRENT / WITH GAPS |
| [ENGINEERING_PLAYBOOK.md](ENGINEERING_PLAYBOOK.md) | CURRENT / WITH GAPS |

### Current Rust core/control and resource evidence

**Category:** CURRENT / RELEASE OPEN. Rust FFI/supervisor/HTTP slices are documented as implemented; constrained local characterization does not establish real-host capacity or public readiness.

| Document | Indexed status |
| --- | --- |
| [RUST_FFI.md](RUST_FFI.md) | CURRENT / RELEASE OPEN |
| [RUST_SUPERVISOR.md](RUST_SUPERVISOR.md) | CURRENT / RELEASE OPEN |
| [RUST_REQUEST_LIFECYCLE.md](RUST_REQUEST_LIFECYCLE.md) | CURRENT / RELEASE OPEN |
| [api.md](api.md) | CURRENT / RELEASE OPEN |
| [architecture.md](architecture.md) | CURRENT / RELEASE OPEN |
| [API_CONTRACT_PARITY_GATE.md](API_CONTRACT_PARITY_GATE.md) | CURRENT / RELEASE OPEN |
| [SCHEDULER_BENCHMARK_CONTRACT.md](SCHEDULER_BENCHMARK_CONTRACT.md) | CURRENT / RELEASE OPEN |
| [SCHEDULER_CHARACTERIZATION_REPORT.md](SCHEDULER_CHARACTERIZATION_REPORT.md) | CURRENT / RELEASE OPEN |
| [HTTP_CAPACITY_CHARACTERIZATION_P6_5.md](HTTP_CAPACITY_CHARACTERIZATION_P6_5.md) | CURRENT / RELEASE OPEN |
| [P8_VPS_RESOURCE_AUDIT.md](P8_VPS_RESOURCE_AUDIT.md) | CURRENT / RELEASE OPEN |
| [P12_DEPLOYMENT.md](P12_DEPLOYMENT.md) | CURRENT / RELEASE OPEN |
| [P14_PRODUCTION_BASELINE.md](P14_PRODUCTION_BASELINE.md) | CURRENT / RELEASE OPEN |
| [P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md](P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md) | CURRENT / RELEASE OPEN |
| [MVP_RELEASE_CHECKLIST.md](MVP_RELEASE_CHECKLIST.md) | CURRENT / RELEASE OPEN |
| [LOCAL_DOCKER_REHEARSAL.md](LOCAL_DOCKER_REHEARSAL.md) | CURRENT / RELEASE OPEN |

### Native/Rust target security, architecture, patterns and performance

**Category:** MIXED / TARGET. HLD/LLD and GoF sketches contain aspirational claims. Native benchmark harness exists, enforced performance regression threshold and production deployment evidence remain open.

| Document | Indexed status |
| --- | --- |
| [hld.md](hld.md) | MIXED / TARGET |
| [lld.md](lld.md) | MIXED / TARGET |
| [design_patterns.md](design_patterns.md) | MIXED / TARGET |
| [performance.md](performance.md) | MIXED / TARGET |
| [security.md](security.md) | MIXED / TARGET |
| [threat-model.md](threat-model.md) | MIXED / TARGET |
| [deployment.md](deployment.md) | MIXED / TARGET |
| [NATIVE_BENCHMARKING.md](NATIVE_BENCHMARKING.md) | MIXED / TARGET |

### Historical native layout design and early implementation gates

**Category:** HISTORICAL / SUPERSEDED IN PART. Source archaeology and phase contracts remain valuable; pre-implementation 'planned' language is not a current-status override for later merged P8/P12 slices.

| Document | Indexed status |
| --- | --- |
| [P7_2_0_B_PRODUCT_LAYOUT_DOMAIN.md](P7_2_0_B_PRODUCT_LAYOUT_DOMAIN.md) | HISTORICAL / SUPERSEDED IN PART |
| [P7_2_1_NATIVE_LAYOUT_ARCHAEOLOGY.md](P7_2_1_NATIVE_LAYOUT_ARCHAEOLOGY.md) | HISTORICAL / SUPERSEDED IN PART |
| [P7_2_2_EXACT_LAYOUT_SEMANTICS_AND_EVIDENCE_PLAN.md](P7_2_2_EXACT_LAYOUT_SEMANTICS_AND_EVIDENCE_PLAN.md) | HISTORICAL / SUPERSEDED IN PART |
| [P7_2_3_RENDER_PLAN_CONTRACT.md](P7_2_3_RENDER_PLAN_CONTRACT.md) | HISTORICAL / SUPERSEDED IN PART |
| [P7_2_4_NATIVE_LAYOUT_CAPABILITY_DESIGN.md](P7_2_4_NATIVE_LAYOUT_CAPABILITY_DESIGN.md) | HISTORICAL / SUPERSEDED IN PART |
| [P7_2_5_CAPABILITY_IMPLEMENTATION_READINESS.md](P7_2_5_CAPABILITY_IMPLEMENTATION_READINESS.md) | HISTORICAL / SUPERSEDED IN PART |
| [P7_2_6_MINIMAL_FIT_CAPABILITY_DECISION.md](P7_2_6_MINIMAL_FIT_CAPABILITY_DECISION.md) | HISTORICAL / SUPERSEDED IN PART |
| [P7_2_7_C_PRIVATE_IMPLEMENTATION_PLAN.md](P7_2_7_C_PRIVATE_IMPLEMENTATION_PLAN.md) | HISTORICAL / SUPERSEDED IN PART |
| [P7_2_8_C_PRIVATE_LAYOUT_RENDER_IMPLEMENTATION.md](P7_2_8_C_PRIVATE_LAYOUT_RENDER_IMPLEMENTATION.md) | HISTORICAL / SUPERSEDED IN PART |
| [P7_2_8_IMPLEMENTATION_AUDIT.md](P7_2_8_IMPLEMENTATION_AUDIT.md) | HISTORICAL / SUPERSEDED IN PART |
| [P8_PRODUCT_CONTRACT.md](P8_PRODUCT_CONTRACT.md) | HISTORICAL / SUPERSEDED IN PART |
| [P10_MVP_PRODUCT_CONTRACT.md](P10_MVP_PRODUCT_CONTRACT.md) | HISTORICAL / SUPERSEDED IN PART |
| [P12_2_ABI_2_2_CROP_DESIGN.md](P12_2_ABI_2_2_CROP_DESIGN.md) | HISTORICAL / SUPERSEDED IN PART |

### Historical hardening, ownership, dispatch and migration evidence

**Category:** HISTORICAL EVIDENCE / CLOSED IN SCOPE. Keep original test results and old findings. Later P7.4/P7.5 closure must be linked before treating earlier P7.3 issues as currently unmitigated. Old local dirty worktree remains unverified remotely.

| Document | Indexed status |
| --- | --- |
| [P7_3_C_CORE_DECISION_REGISTER.md](P7_3_C_CORE_DECISION_REGISTER.md) | HISTORICAL EVIDENCE / CLOSED IN SCOPE |
| [P7_3_C_CORE_INVENTORY.md](P7_3_C_CORE_INVENTORY.md) | HISTORICAL EVIDENCE / CLOSED IN SCOPE |
| [P7_3_C_CORE_PRINCIPAL_AUDIT.md](P7_3_C_CORE_PRINCIPAL_AUDIT.md) | HISTORICAL EVIDENCE / CLOSED IN SCOPE |
| [P7_3_C_CORE_QUARANTINE_REGISTER.md](P7_3_C_CORE_QUARANTINE_REGISTER.md) | HISTORICAL EVIDENCE / CLOSED IN SCOPE |
| [P7_3_PRINCIPAL_REVIEW.md](P7_3_PRINCIPAL_REVIEW.md) | HISTORICAL EVIDENCE / CLOSED IN SCOPE |
| [P7_4_B_BORROWED_INPUT_OWNERSHIP.md](P7_4_B_BORROWED_INPUT_OWNERSHIP.md) | HISTORICAL EVIDENCE / CLOSED IN SCOPE |
| [P7_4_C2_ALLOCATOR_QUEUE_OWNERSHIP.md](P7_4_C2_ALLOCATOR_QUEUE_OWNERSHIP.md) | HISTORICAL EVIDENCE / CLOSED IN SCOPE |
| [P7_4_C_SCHEDULER_SHUTDOWN_REMEDIATION.md](P7_4_C_SCHEDULER_SHUTDOWN_REMEDIATION.md) | HISTORICAL EVIDENCE / CLOSED IN SCOPE |
| [P7_4_EVIDENCE_REGISTER.md](P7_4_EVIDENCE_REGISTER.md) | HISTORICAL EVIDENCE / CLOSED IN SCOPE |
| [P7_4_SIMD_EVIDENCE.md](P7_4_SIMD_EVIDENCE.md) | HISTORICAL EVIDENCE / CLOSED IN SCOPE |
| [P7_5_BRANCH_CONSOLIDATION.md](P7_5_BRANCH_CONSOLIDATION.md) | HISTORICAL EVIDENCE / CLOSED IN SCOPE |
| [P7_5_DIAGNOSTIC_OBSERVABILITY_POLICY.md](P7_5_DIAGNOSTIC_OBSERVABILITY_POLICY.md) | HISTORICAL EVIDENCE / CLOSED IN SCOPE |
| [P7_5_GRAYSCALE_DISPATCH_EVIDENCE.md](P7_5_GRAYSCALE_DISPATCH_EVIDENCE.md) | HISTORICAL EVIDENCE / CLOSED IN SCOPE |
| [P7_RECOVERY_INTEGRATION_REVIEW.md](P7_RECOVERY_INTEGRATION_REVIEW.md) | HISTORICAL EVIDENCE / CLOSED IN SCOPE |
| [ABI_AUDIT.md](ABI_AUDIT.md) | HISTORICAL EVIDENCE / CLOSED IN SCOPE |

### Legacy durable-job platform-specific documents

**Category:** LEGACY. Legacy FastAPI/Celery/Redis/Postgres requirements must not be automatically imposed on synchronous/ephemeral Rust. Migration is a live boundary plan; historical PRD is not current first-MVP authority.

| Document | Indexed status |
| --- | --- |
| [PRD.md](PRD.md) | LEGACY |
| [SAAS_CONTRACT.md](SAAS_CONTRACT.md) | LEGACY |
| [OBSERVABILITY.md](OBSERVABILITY.md) | LEGACY |
| [SECURITY_OPERATIONS.md](SECURITY_OPERATIONS.md) | LEGACY |
| [SRE.md](SRE.md) | LEGACY |
| [DATABASE_RECOVERY.md](DATABASE_RECOVERY.md) | LEGACY |
| [RELEASE_CHECKLIST.md](RELEASE_CHECKLIST.md) | LEGACY |
| [migration.md](migration.md) | LEGACY |

### Frontend/plan/management and miscellaneous docs — indexed, excluded from detailed native work

**Category:** OUT OF SCOPE / INDEXED. Indexed for completeness only. Frontend/Wasm feature implementation excluded. The 2026-10-31 principal audit and v2 RFC are historical/aspirational; instructions_data is scratch installation notes and whatiwant a short intent fragment.

| Document | Indexed status |
| --- | --- |
| [P7_FRONTEND_ARCHITECTURE_AUDIT.md](P7_FRONTEND_ARCHITECTURE_AUDIT.md) | OUT OF SCOPE / INDEXED |
| [P7_FRONTEND_DEV_ENVIRONMENT.md](P7_FRONTEND_DEV_ENVIRONMENT.md) | OUT OF SCOPE / INDEXED |
| [PRODUCT_ROADMAP_BROWSER_FIRST.md](PRODUCT_ROADMAP_BROWSER_FIRST.md) | OUT OF SCOPE / INDEXED |
| [PROJECT_STATUS_AND_DECISION_REGISTER.md](PROJECT_STATUS_AND_DECISION_REGISTER.md) | OUT OF SCOPE / INDEXED |
| [CHANGE_SCOPED_VERIFICATION.md](CHANGE_SCOPED_VERIFICATION.md) | OUT OF SCOPE / INDEXED |
| [PRINCIPAL_AUDIT_2026-10-31.md](PRINCIPAL_AUDIT_2026-10-31.md) | OUT OF SCOPE / INDEXED |
| [README.md](README.md) | OUT OF SCOPE / INDEXED |
| [instructions_data.md](instructions_data.md) | OUT OF SCOPE / INDEXED |
| [whatiwant.md](whatiwant.md) | OUT OF SCOPE / INDEXED |
| [v2.0 RFC.md](v2.0%20RFC.md) | OUT OF SCOPE / INDEXED |

## Known documentation corrections and supersession

- `hld.md` title “Production-ready” and `lld.md` introductory wording are retained for historical context but must not be cited as a verified current deployment or low-latency guarantee.
- `PRD.md` (2026-08) and old SaaS observability/SRE/database documentation describe the **legacy durable job architecture**. Current synchronous Rust controls belong to `api.md`, `RUST_REQUEST_LIFECYCLE.md`, and `MVP_RELEASE_CHECKLIST.md`.
- `P13_INTERACTIVE_CROP_CONTRACT.md` was created as a proposal; native ABI 2.3 became implemented after its original draft. Transport and user interaction remain partial.
- `P12_2_ABI_2_2_CROP_DESIGN.md` draft PR #16 state is a time-specific checkpoint; the later P12 native path was merged.
- `docs/OBSERVABILITY.md` is intentionally uppercase, so do not create `docs/observability.md` as a different file on a case-sensitive system.
- Avoid conflating internal C PDF/preset/plugin source presence with stable public ABI exposure.
- `docs/README.md` is a navigation file. Do not make a historical PRD automatically binding on a newer Rust scope without explicit version and applicability.

## Required audit record for every future native/Rust milestone

| Field | What to record |
| --- | --- |
| Revision | Exact baseline and test-head SHA; branch and PR |
| Status | PLANNED / IMPLEMENTED / VERIFIED (recorded)/PARTIAL / BLOCKED / LEGACY |
| Scope | Public ABI operation, Rust wrapper, request path or internal code |
| Trust boundary | Untrusted bytes, pointers, threads, resources, child process and filesystem |
| Ownership | Who allocates, owns, borrows, releases; exact failure cleanup |
| DSA/architecture | Algorithm, complexity, memory bounds, queue/scheduler state invariants |
| Observability | Redacted fields, emitted metrics, measurement overhead, trace propagation |
| Performance | Fixture, input/output hashes, hardware, compiler, exact timed region and p50/p95/p99 |
| Tests | Focused tests and required full/release gates, passing/failing by exact revision |
| Open risks | Severity, accountable owner, mitigation or explicit non-claim |
| Product economics | Support/compute/bandwidth/maintenance trade-off when materially affected |
| Decision | Keep, harden, quarantine, deprecate or proceed; next review trigger |

Documentation-only changes require link/consistency review, not manual C rebuild. Native/FFI/security changes still require the applicable ABI, sanitizer, fuzz and integration gates. Required CI is not waived.
