# C17 native and Rust documentation audit — 2026-11-14

**Audited revision:** e0f8a32044c8c2d9e68ae37299abdedaedf318a7. **Coverage:** all 83 Markdown files within docs/ were inspected for purpose, status and claimed scope; selected C/Rust source, public headers, ABI map and CMake were cross-checked. This is a document classification and targeted source audit, NOT an exhaustive source audit, newly executed verification, penetration test, or production certification. No code, build or CI changes are authorized by this report.

## Source-of-truth hierarchy

1. Public C ABI headers, exported-symbol manifest and C/Rust implementation at the checked revision.
2. Versioned operation contracts: [ABI](ABI.md), [Rust API](api.md), [P12 sheet](P12_PHYSICAL_SHEET_CONTRACT.md), [P13 crop](P13_INTERACTIVE_CROP_CONTRACT.md), [Rust lifecycle](RUST_REQUEST_LIFECYCLE.md).
3. Dated executable evidence: [M0](M0_C17_PORTABILITY_AND_WASM_ADR.md), [P7.4](P7_4_EVIDENCE_REGISTER.md), [P7.5](P7_5_BRANCH_CONSOLIDATION.md), [benchmark contract](NATIVE_BENCHMARKING.md).
4. Snapshot interpretation: this audit, [native/Rust architecture](NATIVE_RUST_ARCHITECTURE_RECONCILIATION.md), [security and observability](NATIVE_RUST_SECURITY_OBSERVABILITY_AUDIT.md), [performance, patterns and algorithms](NATIVE_RUST_PERFORMANCE_DESIGN_AUDIT.md).
5. [AGENTS](../AGENTS.md), [native AGENTS](../imgengine/AGENTS.md) govern agent rules; [skills](../skills.md) gives optional techniques; [next](../next.md) tracks tasks.
6. Historical designs retain meaning for their own revision only. The August 2026 PRD is authoritative to its legacy durable-job plan, not to the newer synchronous, ephemeral Rust/C first-MVP when contracts conflict.

## Status vocabulary

- **DONE-RECORDED:** narrow feature or fix completed with dated evidence; no new test run claimed.
- **CURRENT CONTRACT / MIXED:** useful existing document; current and historical assertions may coexist.
- **PARTIAL / RELEASE OPEN:** an implementation slice exists; integration or release evidence remains incomplete.
- **PLAN / OPEN:** guidance or an intended feature; code or operating proof not established.
- **HISTORICAL:** design, audit or old milestone snapshot; not a current mandate.
- **LEGACY:** retained FastAPI/Celery/Redis/Postgres/durable-job path; not the new native/Rust product contract.

## Capability reconciliation

| Native C or Rust area | Status | Grounding and next evidence |
| --- | --- | --- |
| C17 shipping profile, C11 whole-core compatibility | DONE-RECORDED | M0 PR #29 and Ubuntu 24.04 gate; C23 deliberately outside shipping |
| Shared libimgengine.so, exported ABI 1.0–2.3 | IMPLEMENTED | CMake, installed include/imgengine/api headers and cmake/libimgengine.map |
| Internal static imgengine_internal library | IMPLEMENTED-INTERNAL | CMake builds static internals for CLI; public installable libimgengine.a is NOT established |
| C decode, JPEG encode, FIT layout, centered-fill, normalized crop | IMPLEMENTED-NARROWLY | Native symbols through 2.3, P8/P12/P13; not independent general-purpose crop/resize SDK |
| PDF/bleed/border/crop-mark functionality | PARTIAL/INTERNAL | Internal source and CLI contracts exist; no shipping PDF/P12 public ABI claim |
| Rust safe FFI, lifecycle, bounded admission, Axum | IMPLEMENTED-RECORDED | Rust wrapper/supervisor/API; ABI 2.3 user-crop Rust transport remains to be connected |
| C scheduler, borrowed-buffer ownership, allocator, scalar/SIMD | DONE-RECORDED for bounded test cases | P7.4 and P7.5; not arbitrary concurrent engine or cancellation guarantee |
| Native security/sandbox, fuzz, sanitizer | IMPLEMENTED CONTROLS | Production decoder isolation, aggregate memory/RSS and malicious input envelopes remain unproven |
| C logger/metrics/tracing and Rust admission counters | IMPLEMENTED SUBSYSTEMS | Thread safety/production export/redaction evidence incomplete |
| Native performance benchmark harness | IMPLEMENTED HARNESS | No universal sub-2ms, 100k/s, nanosecond image pipeline guarantee |
| VPS, release operations, public SDK package, license provenance | OPEN | Real host, release checklist, static consumer/package and licensing needed |
| Retained Python-based async service | LEGACY | Do not delete until parity and rollback are explicitly approved |

## Document discrepancy register

| ID | Risk | Concrete observation | Documentation remedy |
| --- | --- | --- | --- |
| D01 | HIGH | hld.md calls itself production-ready, promises zero allocation, less-than-2ms and 100k+ ops/s without general benchmark/release proof | Historical/aspirational notice and current benchmark links |
| D02 | HIGH | lld.md calls include/api/v1 the public API; actual installed ABI is include/imgengine/api | Historical/conceptual notice and current ABI source links |
| D03 | HIGH | ABI.md describes v1 one-capability boundary, while additive ABI 2.0–2.3 exists | Clarify v1 scope and link new symbols/headers |
| D04 | HIGH | docs/README claims PRD is binding generally, while PRD describes legacy durable asynchronous jobs | Make authority generation-specific |
| D05 | HIGH | P13 contract starts with no implementation; native crop ABI 2.3 later merged through PRs #24–26 | Dated status note: native done; Rust/API/user controls incomplete |
| D06 | MED | P8 and P10 contracts remain future-tense despite newer implemented P8/P12 | Preserve as historical snapshot, link newer contracts |
| D07 | MED | P12.2 ABI design refers to PR #16 as draft; implementation later merged | Mark old design snapshot |
| D08 | HIGH | OBSERVABILITY, SECURITY_OPERATIONS, SRE and DATABASE_RECOVERY describe legacy jobs and DB | Distinguish legacy from current Rust/C policies |
| D09 | MED | LINUX_VERIFICATION describes Sept 8/8 CTests versus M0 Nov 16/16 | Explicitly date evidence; don't overwrite history |
| D10 | LOW | docs/README contains literal backslash-n characters in intro | Correct Markdown spacing |
| D11 | HIGH-REVIEW | C metrics.c uses atomics for totals but plain max/min writes; concurrent reachability not proven | Trace call sites and concurrency; focused test before asserting data race |
| D12 | MED | Default compiler includes -march=native; cross-machine distributed binary portability not proved | Document portable CPU baseline and dispatch gates |
| D13 | MED | design_patterns.md contains examples; SOLID/GoF compliance not fully code-verified | Label sketches as proposals, require source/test proof |
| D14 | HIGH | JPEG destination budget does not constrain aggregate decode/canvas/intermediate RSS | Resource-envelope audit required |
| D15 | MED | internal .a exists but public static SDK does not | Static link consumer/dependency/package decision needed |
| D16 | HIGH | security.md describes networkless isolated decode as target, not demonstrated mandatory runtime | Explicit actual sandbox scope and evidence |

## Document-by-document classification (83 Markdown files)

These are DOCUMENT classifications, not assertions that each feature was freshly tested. A document can contain multiple historical/implemented states.

| Document | Classification |
| --- | --- |
| [ABI.md](ABI.md) | CURRENT CONTRACT / MIXED |
| [ABI_AUDIT.md](ABI_AUDIT.md) | HISTORICAL |
| [API_CONTRACT_PARITY_GATE.md](API_CONTRACT_PARITY_GATE.md) | CURRENT CONTRACT / MIXED |
| [CHANGE_SCOPED_VERIFICATION.md](CHANGE_SCOPED_VERIFICATION.md) | CURRENT CONTRACT / MIXED |
| [DATABASE_RECOVERY.md](DATABASE_RECOVERY.md) | LEGACY |
| [ENGINEERING_PLAYBOOK.md](ENGINEERING_PLAYBOOK.md) | CURRENT CONTRACT / MIXED |
| [ENGINE_SPEC.md](ENGINE_SPEC.md) | CURRENT CONTRACT / MIXED |
| [HTTP_CAPACITY_CHARACTERIZATION_P6_5.md](HTTP_CAPACITY_CHARACTERIZATION_P6_5.md) | PARTIAL / RELEASE OPEN |
| [LINUX_VERIFICATION.md](LINUX_VERIFICATION.md) | CURRENT CONTRACT / MIXED |
| [LOCAL_DOCKER_REHEARSAL.md](LOCAL_DOCKER_REHEARSAL.md) | CURRENT CONTRACT / MIXED |
| [M0_C17_PORTABILITY_AND_WASM_ADR.md](M0_C17_PORTABILITY_AND_WASM_ADR.md) | DONE-RECORDED |
| [MVP_RELEASE_CHECKLIST.md](MVP_RELEASE_CHECKLIST.md) | CURRENT CONTRACT / MIXED |
| [NATIVE_BENCHMARKING.md](NATIVE_BENCHMARKING.md) | PLAN / OPEN |
| [NATIVE_C_RUST_DOCS_MAP.md](NATIVE_C_RUST_DOCS_MAP.md) | CURRENT CONTRACT / MIXED |
| [NATIVE_C_RUST_DOCUMENTATION_AUDIT_2026-11-14.md](NATIVE_C_RUST_DOCUMENTATION_AUDIT_2026-11-14.md) | CURRENT CONTRACT / MIXED |
| [NATIVE_DEVELOPMENT.md](NATIVE_DEVELOPMENT.md) | CURRENT CONTRACT / MIXED |
| [OBSERVABILITY.md](OBSERVABILITY.md) | LEGACY |
| [P10_MVP_PRODUCT_CONTRACT.md](P10_MVP_PRODUCT_CONTRACT.md) | HISTORICAL |
| [P12_2_ABI_2_2_CROP_DESIGN.md](P12_2_ABI_2_2_CROP_DESIGN.md) | HISTORICAL |
| [P12_DEPLOYMENT.md](P12_DEPLOYMENT.md) | PARTIAL / RELEASE OPEN |
| [P12_PHYSICAL_SHEET_CONTRACT.md](P12_PHYSICAL_SHEET_CONTRACT.md) | PARTIAL / RELEASE OPEN |
| [P13_INTERACTIVE_CROP_CONTRACT.md](P13_INTERACTIVE_CROP_CONTRACT.md) | PARTIAL / RELEASE OPEN |
| [P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md](P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md) | PLAN / OPEN |
| [P14_PRODUCTION_BASELINE.md](P14_PRODUCTION_BASELINE.md) | PARTIAL / RELEASE OPEN |
| [P7_2_0_B_PRODUCT_LAYOUT_DOMAIN.md](P7_2_0_B_PRODUCT_LAYOUT_DOMAIN.md) | HISTORICAL |
| [P7_2_1_NATIVE_LAYOUT_ARCHAEOLOGY.md](P7_2_1_NATIVE_LAYOUT_ARCHAEOLOGY.md) | HISTORICAL |
| [P7_2_2_EXACT_LAYOUT_SEMANTICS_AND_EVIDENCE_PLAN.md](P7_2_2_EXACT_LAYOUT_SEMANTICS_AND_EVIDENCE_PLAN.md) | HISTORICAL |
| [P7_2_3_RENDER_PLAN_CONTRACT.md](P7_2_3_RENDER_PLAN_CONTRACT.md) | HISTORICAL |
| [P7_2_4_NATIVE_LAYOUT_CAPABILITY_DESIGN.md](P7_2_4_NATIVE_LAYOUT_CAPABILITY_DESIGN.md) | HISTORICAL |
| [P7_2_5_CAPABILITY_IMPLEMENTATION_READINESS.md](P7_2_5_CAPABILITY_IMPLEMENTATION_READINESS.md) | HISTORICAL |
| [P7_2_6_MINIMAL_FIT_CAPABILITY_DECISION.md](P7_2_6_MINIMAL_FIT_CAPABILITY_DECISION.md) | HISTORICAL |
| [P7_2_7_C_PRIVATE_IMPLEMENTATION_PLAN.md](P7_2_7_C_PRIVATE_IMPLEMENTATION_PLAN.md) | HISTORICAL |
| [P7_2_8_C_PRIVATE_LAYOUT_RENDER_IMPLEMENTATION.md](P7_2_8_C_PRIVATE_LAYOUT_RENDER_IMPLEMENTATION.md) | PARTIAL / RELEASE OPEN |
| [P7_2_8_IMPLEMENTATION_AUDIT.md](P7_2_8_IMPLEMENTATION_AUDIT.md) | HISTORICAL |
| [P7_3_C_CORE_DECISION_REGISTER.md](P7_3_C_CORE_DECISION_REGISTER.md) | HISTORICAL |
| [P7_3_C_CORE_INVENTORY.md](P7_3_C_CORE_INVENTORY.md) | HISTORICAL |
| [P7_3_C_CORE_PRINCIPAL_AUDIT.md](P7_3_C_CORE_PRINCIPAL_AUDIT.md) | HISTORICAL |
| [P7_3_C_CORE_QUARANTINE_REGISTER.md](P7_3_C_CORE_QUARANTINE_REGISTER.md) | HISTORICAL |
| [P7_3_PRINCIPAL_REVIEW.md](P7_3_PRINCIPAL_REVIEW.md) | HISTORICAL |
| [P7_4_B_BORROWED_INPUT_OWNERSHIP.md](P7_4_B_BORROWED_INPUT_OWNERSHIP.md) | DONE-RECORDED |
| [P7_4_C2_ALLOCATOR_QUEUE_OWNERSHIP.md](P7_4_C2_ALLOCATOR_QUEUE_OWNERSHIP.md) | DONE-RECORDED |
| [P7_4_C_SCHEDULER_SHUTDOWN_REMEDIATION.md](P7_4_C_SCHEDULER_SHUTDOWN_REMEDIATION.md) | DONE-RECORDED |
| [P7_4_EVIDENCE_REGISTER.md](P7_4_EVIDENCE_REGISTER.md) | DONE-RECORDED |
| [P7_4_SIMD_EVIDENCE.md](P7_4_SIMD_EVIDENCE.md) | DONE-RECORDED |
| [P7_5_BRANCH_CONSOLIDATION.md](P7_5_BRANCH_CONSOLIDATION.md) | DONE-RECORDED |
| [P7_5_DIAGNOSTIC_OBSERVABILITY_POLICY.md](P7_5_DIAGNOSTIC_OBSERVABILITY_POLICY.md) | DONE-RECORDED |
| [P7_5_GRAYSCALE_DISPATCH_EVIDENCE.md](P7_5_GRAYSCALE_DISPATCH_EVIDENCE.md) | DONE-RECORDED |
| [P7_FRONTEND_ARCHITECTURE_AUDIT.md](P7_FRONTEND_ARCHITECTURE_AUDIT.md) | HISTORICAL |
| [P7_FRONTEND_DEV_ENVIRONMENT.md](P7_FRONTEND_DEV_ENVIRONMENT.md) | HISTORICAL |
| [P7_RECOVERY_INTEGRATION_REVIEW.md](P7_RECOVERY_INTEGRATION_REVIEW.md) | HISTORICAL |
| [P8_PRODUCT_CONTRACT.md](P8_PRODUCT_CONTRACT.md) | HISTORICAL |
| [P8_VPS_RESOURCE_AUDIT.md](P8_VPS_RESOURCE_AUDIT.md) | PARTIAL / RELEASE OPEN |
| [P9_3_BOUNDED_OUTPUT_API_DESIGN.md](P9_3_BOUNDED_OUTPUT_API_DESIGN.md) | DONE-RECORDED |
| [PRD.md](PRD.md) | LEGACY |
| [PRINCIPAL_AUDIT_2026-10-31.md](PRINCIPAL_AUDIT_2026-10-31.md) | HISTORICAL |
| [PRODUCT_ROADMAP_BROWSER_FIRST.md](PRODUCT_ROADMAP_BROWSER_FIRST.md) | PLAN / OPEN |
| [PROJECT_STATUS_AND_DECISION_REGISTER.md](PROJECT_STATUS_AND_DECISION_REGISTER.md) | CURRENT CONTRACT / MIXED |
| [README.md](README.md) | CURRENT CONTRACT / MIXED |
| [REAL_IMAGE_VALIDATION.md](REAL_IMAGE_VALIDATION.md) | CURRENT CONTRACT / MIXED |
| [REAL_IMAGE_VERIFICATION.md](REAL_IMAGE_VERIFICATION.md) | DONE-RECORDED |
| [RELEASE_CHECKLIST.md](RELEASE_CHECKLIST.md) | LEGACY |
| [RUST_FFI.md](RUST_FFI.md) | CURRENT CONTRACT / MIXED |
| [RUST_REQUEST_LIFECYCLE.md](RUST_REQUEST_LIFECYCLE.md) | CURRENT CONTRACT / MIXED |
| [RUST_SUPERVISOR.md](RUST_SUPERVISOR.md) | CURRENT CONTRACT / MIXED |
| [SAAS_CONTRACT.md](SAAS_CONTRACT.md) | LEGACY |
| [SCHEDULER_BENCHMARK_CONTRACT.md](SCHEDULER_BENCHMARK_CONTRACT.md) | CURRENT CONTRACT / MIXED |
| [SCHEDULER_CHARACTERIZATION_REPORT.md](SCHEDULER_CHARACTERIZATION_REPORT.md) | DONE-RECORDED |
| [SECURITY_OPERATIONS.md](SECURITY_OPERATIONS.md) | LEGACY |
| [SRE.md](SRE.md) | LEGACY |
| [abi/exported_symbols.md](abi/exported_symbols.md) | CURRENT CONTRACT / MIXED |
| [api.md](api.md) | CURRENT CONTRACT / MIXED |
| [architecture.md](architecture.md) | CURRENT CONTRACT / MIXED |
| [deployment.md](deployment.md) | PLAN / OPEN |
| [design_patterns.md](design_patterns.md) | CURRENT CONTRACT / MIXED |
| [hld.md](hld.md) | HISTORICAL |
| [instructions_data.md](instructions_data.md) | HISTORICAL |
| [lld.md](lld.md) | HISTORICAL |
| [migration.md](migration.md) | CURRENT CONTRACT / MIXED |
| [performance.md](performance.md) | PLAN / OPEN |
| [security.md](security.md) | PLAN / OPEN |
| [threat-model.md](threat-model.md) | PLAN / OPEN |
| [v2.0 RFC.md](v2.0%20RFC.md) | HISTORICAL |
| [whatiwant.md](whatiwant.md) | HISTORICAL |

## Next native/Rust documentation-only reconciliation

- P0: fix authority and status labels in docs index/HLD/LLD/ABI/P13, preserving historical text.
- P0: define current C/Rust trust boundaries, explicit no-native-cancellation, ownership and aggregate resource gaps.
- P0: note potential metrics race as unverified; request call-site/thread analysis, not a speculative code fix.
- P1: map actual C/Rust HLD/LLD, architecture/DSA/patterns, and test evidence to implementation.
- P1: separate legacy observability and SRE goals from native/Rust current behavior.
- P1: document portable static/shared SDK packaging and license/provenance review before commercial distribution.
- P1: enforce performance claims by measured workload, hardware, compiler, median, p95/p99, peak RSS and correctness parity.

**Non-claims:** This PR does not run tests, change source/CI, deploy, close a release gate, authorize deleting old code, or prove the absence/presence of vulnerabilities.
