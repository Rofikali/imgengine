# P7.2.5 - Capability Implementation Readiness

## 1. Purpose

This is the final planning/readiness gate before any native layout implementation. It identifies the smallest safe capability, the specific decisions still required, and the evidence that must exist before code is approved. It does not approve implementation globally, create a native API, or change ABI v1.

## 2. Current Architecture

    Browser/Nuxt presentation
      -> Rust authentication, policy, normalization, admission and lifecycle
      -> immutable RenderPlan
      -> future versioned native layout capability
      -> retained C scheduler, decode, raster work and encoding

Nuxt is advisory. Rust owns product semantics and resource/admission policy. C owns decoder truth, raster execution, native memory, defensive validation and scheduler execution. No second queue or scheduler is justified.

The current verified path remains file-only JPEG/PNG-to-JPEG at POST /api/v1/render. It is not a layout capability.

## 3. Evidence Reviewed

This readiness review uses and preserves:

- docs/P7_2_0_B_PRODUCT_LAYOUT_DOMAIN.md: product/domain terminology and ownership;
- docs/P7_2_1_NATIVE_LAYOUT_ARCHAEOLOGY.md: historical implementation evidence;
- docs/P7_2_2_EXACT_LAYOUT_SEMANTICS_AND_EVIDENCE_PLAN.md: exact geometry/no-silent-fallback semantics;
- docs/P7_2_3_RENDER_PLAN_CONTRACT.md: immutable plan and canonical placement semantics;
- docs/P7_2_4_NATIVE_LAYOUT_CAPABILITY_DESIGN.md: native boundary, ownership and validation design.

Source review confirms that ABI v1 exposes image.jpeg_encode only, while historical img_job_t/layout code is internal and A4/float coupled. It also confirms historical silent geometry shrink, skipped invalid placement with success, arena metadata allocation, slab-backed pixel buffers, and border -> bleed -> crop-mark post-processing order.

## 4. Minimal First Capability

The smallest safe native vertical slice is a conceptual internal layout.render.v1 capability, not a public HTTP feature and not an ABI-v1 extension:

    one validated encoded source
      + one concrete RGB raster canvas
      + explicit ordered destination rectangles
      + exact expected placement count
      + one closed scaling mode
      + explicit RGB background
      + one owned JPEG output
      -> structured native completion result

Rust provides already-normalized concrete pixel geometry. The capability does not interpret paper names, presets, physical dimensions, decimal units, orientation, client filenames, or layout intent. It consumes a resolved execution plan.

For the strict first slice, FIT is the ready scaling mode because it has a defined background behavior and needs no crop-anchor product choice. FILL is not included unless a capability version explicitly freezes and tests a named crop anchor; historical apparent centering is not permission for a hidden fallback.

The capability is useful as the core exact-placement/rendering vertical slice, but it is not by itself a product launch or a frontend feature.

## 5. Feature Scope

| Candidate feature | Readiness | First capability treatment |
| --- | --- | --- |
| One encoded source | READY WITH EXPLICIT DESIGN | Accept only formats separately proven through the new capability; start from currently verified JPEG/PNG policy but test each. |
| Concrete photo/canvas raster geometry | READY FOR IMPLEMENTATION | Passed as checked pixels; no physical conversion in C. |
| Explicit ordered placement rectangles | READY FOR IMPLEMENTATION | Canonical execution input; count is exact. |
| Copies less/equal capacity | READY FOR IMPLEMENTATION | Rust plan supplies exact rectangles/count; C verifies exact completion. |
| FIT | READY AFTER ONE SPECIFIC DECISION | Freeze exact RGB background and resize/reference behavior in capability specification. |
| FILL | READY AFTER ONE SPECIFIC DECISION | Add only with an explicit named crop anchor and reference tests. |
| Background | READY AFTER ONE SPECIFIC DECISION | Fixed explicit RGB representation; no implicit historical default. |
| JPEG output | READY WITH EXPLICIT DESIGN | One owned output representation/release rule required. |
| Concrete paper/canvas values | READY FOR IMPLEMENTATION | C uses pixels only; no paper catalogue. |
| Physical um/DPI conversion | READY IN RUST PLAN ONLY | Not a native v1 input; conversion is already specified. |
| A4/paper/passport presets | DEFERRED | Product catalogues, not native execution values. |
| AUTO_FIT | DEFERRED | No implicit or named policy in first capability. |
| Decorations | DEFERRED | Border/bleed/crop marks require footprint/unit/order contracts. |
| PDF/PNG output | DEFERRED | Require separate encoder/output/limit evidence. |
| Arbitrary crop anchors | DEFERRED | Requires explicit semantic and golden tests. |
| Multi-page/multi-source/streaming | NOT SUITABLE FOR FIRST CAPABILITY | Separate ownership/resource/lifecycle scope. |

The only minimally necessary product-independent mode decision is FIT plus concrete RGB background. If that decision is not approved, no native implementation should begin.

## 6. Required Invariants

Every implementation must preserve all of the following:

    dimensions and required counts are positive
    all arithmetic is checked and widened before multiplication
    every placement rectangle is inside the concrete canvas
    requested_count == planned_count == executed_count
    source facts never redefine destination geometry
    same source + same plan + same capability version
        -> same semantic canvas/placement result
    exact resolved geometry is executed or the request fails
    no silent shrink, partial placement, paper change, mode change,
    output substitution, omitted feature or AUTO_FIT

A return code alone is insufficient. Native success also requires output presence and structured completion facts matching the immutable plan.

## 7. Native Boundary Readiness

| Boundary item | Classification | Requirement before code |
| --- | --- | --- |
| Fixed-width request header/version fields | IMPLEMENT NOW | Freeze a C-facing spec and static-layout checks before code. |
| Concrete canvas/cell pixels | IMPLEMENT NOW | Define channel model and staged byte/stride checks. |
| Explicit rectangle pointer plus count | IMPLEMENT NOW | Fixed element fields, alignment, count/byte validation and native copy rule. |
| Source bytes pointer plus length | IMPLEMENT NOW | Define synchronous borrow versus native copy/decode transfer. |
| Expected/executed count result | IMPLEMENT NOW | Fixed result semantics and mismatch failure. |
| C-owned output and explicit release | NEEDS MORE DESIGN | Freeze allocation family, zero-on-failure and safe Rust RAII behavior. |
| Stable native error categories | IMPLEMENT NOW | Freeze enum/category mapping, not browser text. |
| Synchronous terminal call | IMPLEMENT NOW | C must not retain Rust pointers after return. |
| Async native job handle | DEFERRED | Separate cancellation/shutdown/ownership contract. |
| Capability discovery/public ABI surface | NEEDS MORE DESIGN | Choose new ABI generation/surface before public FFI code. |
| Compact internal grid form | DEFERRED | Only after explicit-list equivalence evidence. |

“Implement now” means design is sufficiently bounded for an approved implementation phase after gates pass; it is not authorization in this planning task.

## 8. Ownership/Lifetime

| Object | Owner and required lifetime |
| --- | --- |
| Rust RenderPlan | Rust; immutable; read only while creating FFI input; never retained by C. |
| Request header and rectangles | Rust call-frame memory until C validates/copies; C retains no Rust pointer. |
| Native execution plan | C-owned after a successful deep copy; survives its scheduler queued/active lifetime; one C cleanup owner. |
| Encoded source | Rust owns initially; C may only borrow for synchronous decode/copy. Work that can outlive the call must use a C-owned copy or decoded representation. |
| Decoded source/canvas/intermediates | C-owned under existing native buffer/slab rules. |
| Arena metadata | C-owned transient metadata; cannot stand in for canvas/output storage. |
| Output buffer | C-owned after success; Rust safe wrapper releases exactly once through a future explicit release operation. |
| Result/error data | Caller-provided fixed-size result; no heap diagnostic strings; zero/reset on failure. |

Rust panic before FFI leaves C untouched. Rust panic after success must release output through RAII. C cleans every partially initialized native resource on failure. Client disconnect, deadline, shutdown and response abandonment remain P6 lifecycle events; ABI v1 has no native mid-operation cancellation and the first capability must not claim it.

## 9. Error Contract

Minimum native categories are:

| Category | Meaning | Rust-visible handling |
| --- | --- | --- |
| invalid_plan | Pointer/count/version/enum/geometry inconsistency | Safe request/boundary failure. |
| unsupported_capability | Unknown operation/version/feature | Explicit unsupported failure; no downgrade. |
| invalid_source/decode_failure | Source cannot meet decoder policy | Safe input failure. |
| invalid_geometry | Bounds, stride, zero/overflow or plan invariant failure | Safe validation/execution failure. |
| resource/allocation_failure | Native memory/arena/slab/copy failure | Safe resource/execution failure. |
| render_failure | Resize/FIT/FILL/raster failure | Safe execution failure. |
| placement_failure | Rectangle could not complete or count mismatch | Safe failure; no partial artifact. |
| encode/output_failure | Output allocation/encode/result invariant failure | Safe failure; no artifact. |
| internal_failure | Unexpected native state | Generic redacted internal error. |

Rust needs the stable category and safe result facts, not native paths, addresses, stack traces, source/output bytes, credentials, scheduler topology, or arbitrary diagnostic strings.

## 10. Completion Contract

A conceptual native result must contain:

    status/category
    executed capability/version
    expected/planned placement count
    executed placement count
    resolved canvas/output raster dimensions
    owned output pointer and byte length only on success

Success requires:

    status == success
    expected_count == planned_count == executed_count
    output pointer/length satisfy defined ownership invariants

Otherwise the result is failure, all partial output is released by C, and Rust returns no successful render response. Output bytes are a result fact; they are not proof of a future product output limit unless one is separately enforced.

## 11. Resource Model

| Measure | Native safety invariant | Future Rust admission policy |
| --- | --- | --- |
| Decoded source pixels/channels | Validate actual decoder facts and safe allocation math. | Measured allowed source policy. |
| Canvas pixels/stride/bytes | Checked staged arithmetic and available native storage. | Measured canvas/request budget. |
| Placement count/metadata | Pointer/count multiplication and allocation checks. | Measured count/work budget. |
| Arena use | Bounded/reset-safe metadata use. | Separate plan metadata budget. |
| Slab/pixel buffers | Actual block/pool allocation safety. | Capacity model, not inferred from incidental block size. |
| Output bytes | Safe allocation/encoding/release. | Measured output-size policy. |
| Workspace/time | C cleanup and terminal result. | Measured lifecycle/deadline/concurrency policy. |

No numerical threshold is selected. Values must come from representative-sheet RSS, CPU, latency, encoded-size, workspace, concurrency and failure measurements. Current P5/P6 limits do not automatically become layout limits.

## 12. Security Readiness

| Risk | Rust defense | C defense | Required test |
| --- | --- | --- | --- |
| Zero/negative/overflow values | Typed normalized plan; checked calculations | Recheck every range/add/multiply | Unit/property overflow tests. |
| Pointer/count mismatch | Safe FFI construction | Null/count/alignment/range checks before read/copy | Negative FFI tests and sanitizers. |
| Out-of-bounds/overlap rectangles | Canonical plan validation | Revalidate all rectangles | Malformed-plan/property tests. |
| Huge count/canvas/source | Admission/resource preflight | Byte/stride/allocation/slab checks | Large-count/canvas/source boundaries. |
| Output amplification | Future measured policy | Encoder/allocation cleanup | Output/resource measurements. |
| Malformed encoded image | Bounded authenticated ingress | Decoder truth/native guards | Malformed fixtures and fuzzing. |
| Allocation/partial init failure | Safe mapped failure/lifecycle cleanup | Single C owner cleans all partial state | Fault injection plus leak checks. |
| Double release/use-after-free | Rust RAII, no cloneable raw owner | Release/reset contract | Output-release and ASAN tests. |
| Rust panic/unwind/thread misuse | Contain unsafe FFI facade, terminal synchronous call | No retained Rust pointer/thread assumption | Panic-path and concurrency tests. |

## 13. ABI/FFI Readiness

| Question | Status |
| --- | --- |
| Keep ABI v1 unchanged | FROZEN: yes. |
| Public ABI generation/surface for layout | OPEN. |
| Capability discovery representation | OPEN; must be fixed before public FFI. |
| Capability/RenderPlan version fields | READY AFTER ONE SPECIFIC DECISION: fixed-width values and compatibility rules. |
| Error category representation | READY: fixed stable native categories. |
| Output allocation/release family | OPEN; select and prove one allocator/release pairing. |
| Thread-safety guarantees | OPEN; do not mark handles Send/Sync without native proof. |
| Synchronous execution | READY: recommended first behavior. |
| Unsupported capability | READY: explicit rejection, never fallback. |

An internal C capability plus internal C tests can be implemented after its own design approval without deciding the public ABI surface. A safe Rust FFI exposure cannot begin until the OPEN ABI items are frozen.

## 14. Scheduler Integration

The smallest integration point is one C-private capability entry that transitions into the existing engine/render path and retained scheduler without exposing task objects or queue internals to Rust. Current source shows the historical render path through img_api_begin_job or img_api_begin_job_with_photo and img_runtime_prepare_render_stage; it does not establish a public async layout submit interface.

Required integration evidence:

- no second queue/worker pool/admission layer;
- one native execution object owns plan/source through queued and active work;
- exact completion count survives scheduler execution;
- failure, deadline, disconnect and graceful-drain cleanup remains correct;
- queued/active work behavior is proven through the existing P6 lifecycle model.

The precise internal entry/call graph is still an implementation design item, not a reason to alter the scheduler.

## 15. Memory Architecture

| Component | Readiness/use |
| --- | --- |
| Arena | Reuse for bounded transient placement/scratch metadata where reset/lifetime rules are explicit. |
| Slab | Reuse for canvas/intermediate pixel buffers where sizes/ownership fit existing rules. |
| Pixel buffers/canvas | C-owned and defensive-validated; do not replace with generic malloc for FFI convenience. |
| Native execution plan | New C-owned metadata object is required for safe scheduler lifetime; its allocator must be chosen/verified. |
| Encoded output | Requires an explicit C-owned allocation and release rule; cannot borrow arena/slab/call-frame memory after return. |

Arena is not large pixel memory, and slab availability is not SaaS resource policy. Existing components are reusable in principle; no code change is authorized by this statement. New native plan metadata and output ownership are required design elements, while async-handle memory is deferred.

## 16. Fixture Strategy

Use project-owned deterministic encoded fixtures and machine-checkable expected geometry. Do not use private images.

| Fixture | Required proof |
| --- | --- |
| A: one source/one rectangle | Baseline decode, exact destination, one completed placement. |
| B: multiple rectangles | Ordered explicit placements all execute. |
| C: copies below capacity | Exact occupied count; empty capacity has no implicit placement. |
| D: copies equal capacity | Exact complete grid/list count. |
| E: copies above capacity | Rust plan rejection before native work. |
| F: exact physical geometry | Rust conversion 35x45 mm at 300 DPI to 413x531 px; C receives those pixels unchanged. |
| G: FIT | Aspect preservation/background reference fixture. |
| H: FILL | Only after crop anchor is explicitly frozen; otherwise deferred. |
| I: infeasible layout | Rust plan rejection; no native silent shrink. |
| J: malformed source | Safe decoder failure and cleanup. |
| K: resource/overflow boundary | Checked rejection/no wrap and no unsafe allocation. |
| L: output ownership/release | Success, release once, failure/no-output and Rust panic-safe wrapper behavior. |

## 17. Golden/Reference Strategy

Geometry goldens are mandatory for canvas dimensions, rectangle order, count, bounds and resolved cell dimensions. Placement goldens verify row-major source plans and explicit-list semantics. Pixel goldens are required for FIT/background and later FILL/decorations.

JPEG byte identity is not a required oracle unless encoder determinism is independently proven. Semantic output checks should decode output, verify dimensions, expected placement regions/background, and selected source-reference behavior. Metadata goldens apply only after a metadata contract is frozen.

## 18. Test Layers

| Layer | What it proves |
| --- | --- |
| Rust unit/RenderPlan tests | Unit normalization, conversion, geometry, copies/capacity and plan immutability. |
| C native unit tests | Defensive plan validation, rectangle execution, count invariant, allocation/error cleanup. |
| FFI contract tests | Fixed-width/versions, pointer/count/alignment, output ownership/release and error classification. |
| Property/reference tests | Overflow, bounds, non-overlap, determinism and equivalence to independent geometry logic. |
| Sanitizer/leak tests | Memory safety under valid/failing plans. |
| Fuzzing | Hostile encoded sources and malformed plan inputs. |
| Integration tests | Rust-to-C real JPEG/PNG, completion/result/cleanup. |
| End-to-end tests | Nuxt-to-Rust-to-C only after API/UI work is separately approved. |
| Docker gate | Authoritative Ubuntu 24.04 native/FFI/lifecycle/security evidence. |

Tests must layer rather than duplicate. No P7.2.5 fixture/test is created here.

## 19. Performance Evidence Plan

No boundary performance claim is established. Measure before optimizing:

    RenderPlan compilation
    Rust-to-C conversion/copy
    explicit-rectangle metadata allocation/validation
    decode, resize, placement, canvas and encoding time
    peak RSS, slab/arena use and output bytes
    total latency and scheduler/concurrency effects

For first correctness implementation, explicit rectangles need not be benchmarked against a compact form before code exists; they are bounded by selected policy and give clearer semantics. A compact internal representation may be considered only after functional, sanitizer, fuzz and exact-equivalence gates pass, then compared under reproducible small/large/concurrent workloads.

## 20. Implementation Gates

| Gate | Evidence required | Current state |
| --- | --- | --- |
| 1. Semantics frozen | Exact geometry, exact copies, no fallback, FIT/background scope written and approved. | READY AFTER ONE SPECIFIC DECISION |
| 2. Native request/result | Fixed header/rectangle/result fields, integer rules and completion invariant reviewed. | READY AFTER ONE SPECIFIC DECISION |
| 3. Ownership frozen | Source transfer, native-plan allocator, output allocation/release and panic behavior approved. | NOT READY |
| 4. Error contract | Stable categories and safe Rust/API mapping reviewed. | READY AFTER ONE SPECIFIC DECISION |
| 5. Fixtures/goldens | A-L fixture sources/oracles/licensing and fault-injection approach approved. | NOT READY |
| 6. Resource model | Measurements to select policy values and safety/estimate separation defined. | READY AFTER ONE SPECIFIC DECISION |
| 7. Security checklist | Pointer/count, overflow, ownership, sanitizer/fuzz matrix approved. | READY AFTER ONE SPECIFIC DECISION |
| 8. ABI strategy | Public ABI generation/discovery/ownership decision approved before Rust FFI exposure. | NOT READY |
| 9. Scheduler proof | Exact integration path and queued/active lifecycle tests designed. | NOT READY |

No gate is marked PASS merely because this document exists.

## 21. Future Implementation Sequence

1. Approve the minimal native feature set: explicit pixels/rectangles, FIT, RGB background, JPEG output, exact count.
2. Freeze C-private request/result field contracts, error categories, source-transfer and output-release ownership.
3. Define deterministic A-L fixtures, geometry/pixel oracles, fault-injection seams and test ownership.
4. Implement and test C-private capability adapter/execution representation with existing scheduler/memory architecture.
5. Add C unit, property, error/cleanup, ASAN/UBSAN and fuzz evidence.
6. Decide and implement a versioned public ABI/capability surface only after review; add ABI/FFI compatibility tests.
7. Add safe Rust binding and RAII output owner; run Rust/FFI real-image/completion tests.
8. Compile immutable RenderPlan in Rust and enforce product resource policy from measured values.
9. Integrate through existing Rust lifecycle/admission; prove deadline, disconnect, drain and cleanup with layout work.
10. Add a new HTTP/API contract only after Rust/native evidence passes.
11. Add Nuxt controls only after the API contract and end-to-end verification exist.

## 22. Deferred Features

- AUTO_FIT;
- paper/passport/custom-paper catalogues and orientation policy;
- FILL until its explicit crop-anchor contract is approved;
- border, bleed, crop marks and all decoration footprint/unit behavior;
- PDF, PNG output, streaming, output metadata/DPI metadata;
- multi-source, multi-page, batch/background jobs and persistent storage;
- async native job handles/native cancellation;
- compact grid encoding until equivalent to explicit rectangles;
- public capability registry/ABI generation until FFI review;
- production resource limits, capacity claims and performance optimization.

## 23. Open Questions

1. Is exact FIT plus a fixed RGB background the approved minimal semantic feature set?
2. Which C allocation family and release function own layout JPEG output?
3. What C-private request/result field layout safely represents explicit rectangles and completion facts?
4. Does source decoding/copy occur before scheduler queueing, and what resource accounting follows?
5. What is the public ABI generation/surface and discovery model after internal evidence exists?
6. Which deterministic images and expected pixel/geometry oracles are approved fixtures?
7. Which representative workloads select Rust product resource limits?
8. What exact C scheduler call path safely hosts the capability without a new queue?
9. Is/when a named centre crop anchor approved for FILL?

## 24. Final Readiness Assessment

| Area | Classification | Reason |
| --- | --- | --- |
| Concrete raster geometry and explicit rectangles | READY FOR IMPLEMENTATION | Semantics/invariants are specified. |
| Exact completion/count result | READY FOR IMPLEMENTATION | Required result facts are known. |
| FIT/background | READY AFTER ONE SPECIFIC DECISION | Freeze RGB/background and resize reference semantics. |
| C-private native adapter | READY AFTER ONE SPECIFIC DECISION | Freeze source-transfer/native-plan allocation contract. |
| Output ownership/release | NOT READY | Allocator/release/panic contract remains open. |
| Public ABI/FFI exposure | NOT READY | ABI generation/discovery/threading/ownership open. |
| Rust RenderPlan compiler/API | DEFERRED | Depends on safe native boundary and product policy. |
| FILL/crop position | DEFERRED | Crop-anchor semantics not frozen. |
| Decorations/paper/presets/PDF/multi-page | DEFERRED | Product and resource semantics incomplete. |
| Production readiness/capacity | DEFERRED | No layout implementation/evidence or measured limits. |

## 25. Recommended Next Step

Do not implement yet. Request a narrowly scoped P7.2.6 design decision that freezes: (1) the minimal FIT/background/JPEG feature set, (2) C-native source transfer and plan/output ownership, (3) C-private request/result fields and their fixed-width validation rules, and (4) deterministic fixture/oracle ownership. After those four decisions are approved, implementation may begin first as C-private capability plus native tests; public ABI, Rust, HTTP and Nuxt remain later gates. Production readiness is not claimed.
