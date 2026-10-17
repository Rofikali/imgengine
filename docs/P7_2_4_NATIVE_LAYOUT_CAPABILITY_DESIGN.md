# P7.2.4 - Native Layout Capability Design

## 1. Purpose

This is a design-only description of the future native boundary that executes Rust's immutable RenderPlan through the existing C engine. It does not add a C export, Rust binding, ABI struct, capability registry, test, or implementation.

The objective is a narrow capability that preserves P7.2.2/P7.2.3 semantics: exact physical-to-raster geometry, no implicit AUTO_FIT, exact copy completion, deterministic placement, no partial successful output, Rust control-plane ownership, and the retained C scheduler/execution plane.

## 2. Current Native/ABI Boundary

Current public ABI v1 is deliberately narrow. include/imgengine/api/v1/imgengine.h exports engine lifetime/capability query and imgengine_process_encoded_image_to_jpeg with an owned imgengine_output_t released by imgengine_output_release. It represents image.jpeg_encode, not layout.

The verified Rust endpoint is similarly narrow: rust/imgengine-api/src/lib.rs has file-only POST /api/v1/render for JPEG/PNG-to-JPEG. Neither surface accepts paper, physical dimensions, grid, copies, crop, decoration, or a RenderPlan.

The future chain is:

    Rust immutable RenderPlan
      -> narrow native layout capability adapter
      -> native-owned execution representation
      -> existing C scheduler and rendering components
      -> owned encoded output and structured completion result

This does not move scheduler admission into Rust or introduce a second scheduler.

## 3. Historical Layout Capability

The audited historical internal path is:

    CLI -> img_job_t -> begin/render stages -> A4 canvas
        -> resize/layout grid -> postfx -> encoding

include/pipeline/job.h combines product and execution concepts: float photo_w_cm/photo_h_cm, DPI, rows/columns, gap, padding, border_px, bleed_px, crop-mark fields, FIT/FILL and template identifiers. include/pipeline/canvas.h supplies float conversion and A4 dimensions. src/runtime/job_prepare_render_stage.c creates/resets a one MiB metadata arena, uses slab-backed canvas/layout work, and invokes img_layout_grid.

Important source facts:

- src/pipeline/canvas_geometry.c proportionally shrinks an oversized requested photo cell and returns success.
- src/pipeline/layout_grid_place_cells.c skips invalid cells with continue.
- src/pipeline/layout_grid.c records that reduced count and returns IMG_SUCCESS.
- src/runtime/job_postfx.c applies border, then bleed, then crop marks.
- src/pipeline/canvas_alloc_buffer.c uses slab-backed RGB canvas memory; src/pipeline/layout_grid_alloc_cells.c allocates checked cell metadata from an arena.

These are internal implementation/history facts, not a safe SaaS contract. The native capability may reuse proven raster components, arena and slab ownership, but must not inherit silent resize, implicit full-grid copy count, A4-only product semantics, float physical units, or partial success.

## 4. Why Historical img_job_t Is Not the New ABI

img_job_t is unsuitable as a public/native-crossing plan because it:

- carries float centimetres instead of normalized integer physical values or concrete pixels;
- conflates template/product labels, A4 assumptions and renderer controls;
- has ambiguous pixel terms such as gap and padding;
- does not model copies separately from rows times columns;
- allows historical silent shrink and partial-placement success;
- exposes unstable internal layout decisions and does not establish FFI ownership/version rules.

It must remain internal historical machinery. A future capability accepts a deliberately reviewed execution request; it does not make img_job_t public, reinterpret it as RenderPlan, or extend ABI v1 through it.

## 5. Candidate Native Boundary Designs

| Option | Description | Strengths | Material risks |
| --- | --- | --- | --- |
| A. Fully materialized native plan | Rust converts its plan to a C-shaped execution plan and calls C. | Concrete execution inputs; clear C boundary. | Directly sharing a Rust layout/struct couples Rust representation to C ABI, makes lifetime/version/allocation brittle, and can leak internal Rust choices. |
| B. Compact normalized parameters | Rust passes photo/paper/grid parameters and C derives geometry/placements. | Small request; resembles historical grid path. | Duplicates geometry; semantic drift can recreate silent scaling/count mismatch; weakens auditability and non-grid extensibility. |
| C. Explicit rectangles | Rust passes normalized canvas/cell policy plus each placement rectangle. | Exact execution targets, direct count verification, auditable, supports future non-grid layouts. | Rectangle memory/copy cost; pointer/count/ownership design required. |
| D. Hybrid adapter | Rust owns canonical explicit semantics; C receives a narrow validated request, copies it into native-owned execution data, validates invariants, and may use an equivalent compact internal form. | Keeps semantic authority in Rust and native defensive checks in C; isolates ABI/storage choices; preserves existing execution/memory architecture. | More conversion/validation; compact form requires equivalence proof, not assertion. |

Option B is unacceptable as the sole semantic boundary because it reintroduces independent C geometry derivation. Option A is unsafe if interpreted as passing Rust memory/struct layout directly. Option C is the canonical semantic representation. Option D is the recommended operational design because it gives C an owned, validated execution object suitable for scheduler work.

## 6. Recommended Conceptual Boundary

The recommended future boundary is a new, separately versioned capability, conceptually named layout.render.pending. The final public name and ABI generation are intentionally undecided.

    Rust RenderPlan (semantic authority)
      -> FFI adapter writes fixed-width request header plus explicit placement array
      -> C validates and deep-copies required execution metadata
      -> C-native execution plan owned through scheduled work
      -> C executes and returns structured result plus owned output

The C-facing request contains only concrete execution values:

- required RenderPlan and native-capability version;
- concrete canvas/cell/footprint raster geometry;
- ordered explicit placement rectangles and count;
- resolved FIT/FILL and crop policy only when defined;
- concrete background/decoration instructions only when the capability supports them;
- encoded-input pointer/byte length and selected output capability;
- expected placement count and checked resource-policy reference/limits where relevant.

It does not contain product names, A4 labels, client paths, API keys, raw user metadata, or floating physical units. Physical um values may be retained in Rust-side plan provenance, but C does not need them to rasterize a plan already resolved to pixels. Avoiding duplicate unit conversion eliminates a major drift path.

C may construct a compact internal grid only after showing it generates exactly the same ordered rectangles, count, bounds and overflow behavior as the canonical list. That equivalence is a future test obligation. It is not a license to derive a different plan.

## 7. RenderPlan to Native Execution Mapping

| RenderPlan concept | C-facing execution input | C responsibility |
| --- | --- | --- |
| Plan/capability identity | fixed-width version fields and feature bits | Reject unsupported combination before work. |
| Source policy | encoded input byte slice plus explicit supported input/output identifiers | Decode facts and validate actual source. |
| Requested physical provenance | Not needed for raster execution; retained by Rust/result provenance if safe | Must not infer/alter physical intent. |
| Canvas | width_px, height_px, colour/channel expectations, checked byte expectations | Recheck stride/allocation/bounds before use. |
| Cell geometry | resolved photo rectangle pixels/footprint | Verify consistency with each placement and renderer mode. |
| Ordered placements | pointer plus count of fixed-width rectangles | Validate every rectangle, execute each or fail. |
| Rendering mode | closed FIT/FILL enum; crop field only when capability version defines it | Execute only supported enum values. |
| Decorations | concrete typed commands/geometry only for enabled/versioned features | Verify bounds/order; no historical implicit defaults. |
| Output | explicit supported encoder/output parameters | Encode or fail; allocate/release output by defined rule. |
| Completion | expected placement count | Return actual completed count and matching result facts. |

No implicit default changes are allowed at this mapping. An absent optional feature means disabled only where the capability version defines that absence; an unsupported supplied feature rejects.

## 8. Ownership and Lifetime

| Object | Creator/owner | Borrower/lifetime | Destruction/failure cleanup |
| --- | --- | --- | --- |
| Rust RenderPlan | Rust | FFI adapter reads immutable plan for call construction | Rust drops after native call/result is complete; never borrowed asynchronously by C. |
| FFI request header/placement input | Rust call frame | C reads only during entry validation/copy | Rust releases on return; C must not retain pointers. |
| Native execution plan | C after successful deep copy/initialisation | C scheduler worker/render stages | C frees/reset/releases on all queued, failed, completed, timeout or shutdown paths. |
| Placement array inside native plan | C-owned copy | Native execution only | Released with native execution plan; no Rust pointer retained. |
| Encoded source bytes | Rust owns ingress buffer initially | C may borrow only for synchronous decode/copy; for scheduled/deferred work C must own copied/decoded input before Rust call returns | Rust owns pre-transfer failure; C owns its copy/decoded buffer after successful transfer. |
| Decoded image/pixel buffers | C | C renderer only | Existing native ownership/slab rules; release/recycle on every path. |
| Canvas/intermediate/arena data | C | C renderer/cache only | Continue existing slab/arena ownership; arena metadata is not a pixel buffer. |
| Encoded output buffer | C after success | Rust copies/borrows according future safe binding | Future capability must provide explicit release, analogous to imgengine_output_release; release exactly once. |
| Error/result object | Prefer caller-provided fixed-size result structure with stable status/category | Rust reads after C completes | No heap diagnostic object or borrowed error string; C zero-initializes outputs on failure. |

A safe first capability should have synchronous call semantics: C may use its internal scheduler but does not return control to Rust until the native execution has reached a terminal result and all Rust-borrowed inputs are no longer referenced. If future asynchronous submission is needed, it requires a separately designed native job handle, cancellation/abandonment semantics, ownership transfer and shutdown lifecycle. It is not implied here.

Rust panic before entry leaves C untouched. Rust panic after a successful call must use safe binding/RAII to release C output. C failure after accepting copied data owns and cleans its partial native state. ABI v1 has no mid-operation cancellation; P6 disconnect/deadline/shutdown semantics remain response/lifecycle behavior, not proof that native work is stopped.

## 9. Pointer and Length Rules

Every pointer-like field is explicit:

| Input/output | Required rule |
| --- | --- |
| Encoded input | const uint8_t pointer plus size_t byte length; null is permitted only with length zero if capability defines empty input, otherwise reject. No filename or NUL-terminated string. |
| Placement list | const pointer plus size_t count; null requires count zero, but a successful layout requires positive count. C checks count against capability/resource policy before multiplication or copy. |
| Typed plan header | pointer to an explicitly sized/versioned fixed-width request view; non-null, aligned for its declared C type, read-only to C. |
| Output | caller supplies initialized output/result storage; C writes only on success and zeroes/reset fields on failure. Output bytes are C-owned until explicit release. |
| Optional arrays/fields | Presence is explicit by capability feature/version and pointer/count, never inferred from sentinel values or global state. |
| Strings/identifiers | Prefer fixed capability enums/integers. If a future textual identifier is necessary, use pointer plus byte length with a documented encoding and bounded validation. |

C does not retain Rust pointers after the synchronous call boundary. C validates pointer/count consistency, count-size multiplication, alignment appropriate for native typed access, and every copied byte range. FFI conversion must not transmute Rust layout or rely on C padding/packing matching a Rust type.

## 10. Integer and Overflow Rules

| Concept | Conceptual type/domain | Required checks |
| --- | --- | --- |
| Raster width/height, x/y, cell dimensions, rows, columns, copies, DPI | uint32_t domain where the capability fixes that maximum | Reject zero where a positive value is required; widen before arithmetic. |
| Physical values | Rust integer um only; not necessary in C raster request | Rust checked conversion/provenance only. |
| Placement count and byte lengths | size_t | Check count times element size before allocation/copy. |
| Stride, canvas/intermediate/output byte sizes | size_t | Checked staged multiplication: width times channels, then stride times height. |
| Pixel counts/work estimates | uint64_t or checked size_t, according operation | Reject if unrepresentable or policy-disallowed. |
| Version/enums/feature bits | fixed-width uint32_t or explicit enum | Reject unknown required values; no coercion/default. |

The historical pattern uint32 multiplied by uint32 is not sufficient for new geometry or byte calculations. Rust preflight and C defensive validation both use widened checked arithmetic. Historical canvas allocation's single size_t pw * ph * 3 expression is not a model for a parameterized future capability.

## 11. Source Image Boundary

Rust knows authenticated request policy, encoded byte count, selected input/output capability, plan geometry and product resource policy. It does not know decoded dimensions/channels as authoritative facts without C decode.

C receives encoded bytes, decodes, validates actual format/dimensions/channels/stride, and establishes source-image truth. The decoded source is independent of requested photo physical/raster geometry. C must not infer plan semantics from extension, filename, MIME claim, EXIF metadata, or source dimensions.

Before enqueueing work that outlives the call-frame borrow, C must have copied encoded bytes or taken ownership of its own decoded representation. Native decode facts that materially affect resource safety must be checked against Rust-provided source policy before allocating/rendering; the detailed policy/result handshake remains a future capability decision.

## 12. Capability Definition

A future capability is conceptually a named, versioned operation such as layout.render, with a versioned feature set. This is not final public naming.

Its minimum v1 semantic scope should be deliberately small:

- one encoded source image;
- one concrete raster canvas;
- explicit ordered placement rectangles;
- exact placement completion count;
- closed FIT/FILL modes;
- one explicitly supported encoded output;
- no implicit AUTO_FIT, paper preset interpretation, crop-position extension or decoration default.

The actual first output format, supported source formats, decorations, crop anchor and paper catalogue are PRODUCT DECISIONS and remain deferred. Capability discovery must expose supported operation/version/features without claiming unsupported behavior.

## 13. Versioning

These version dimensions remain independent:

| Version | Meaning |
| --- | --- |
| HTTP API version | Public browser/proxy-to-Rust wire contract. |
| RenderRequest version | Product-intent vocabulary accepted by Rust. |
| RenderPlan version | Immutable normalized semantic plan Rust produces. |
| Native capability version | What C layout.render implementation can validate/execute. |
| ABI version | Public binary ABI generation; current v1 remains image.jpeg_encode. |
| Preset/catalogue version | Product provenance resolved before plan creation. |

The plan declares the exact capability and version it requires. Rust must not send RenderPlan v2 to a native v1 capability. C rejects unknown/unsupported plan or capability versions before allocation/execution. There is no downgrade that removes copies, changes geometry, omits decoration or silently changes a mode.

Whether layout.render is a separate ABI generation, a separate library surface, or a capability registry behind a new ABI is DEFERRED. It must be selected only after symbol/header/FFI compatibility design and tests. It must not casually extend ABI v1.

## 14. Compatibility

| Condition | Required deterministic behavior |
| --- | --- |
| Rust has a plan requiring unavailable native capability/version | Reject as unsupported capability before admission where discoverable, otherwise before native execution. |
| C receives unknown plan/feature/version | Reject with native unsupported/invalid-plan category; no defaults. |
| Rust requests unenabled paper/output/decoration/crop behavior | Reject as product/capability unsupported; no substitution. |
| Native feature cannot meet expected count/geometry | Fail execution; no partial output. |
| Optional future field absent | Use only the explicitly defined default for that exact capability version; otherwise reject. |

Compatibility is semantic, not merely struct-size compatibility. A matching byte layout cannot authorize changed physical or output behavior.

## 15. Completion Semantics

A return code alone cannot prove exact semantic completion. The future native result must contain, at minimum:

    native status/category
    required capability and executed capability version
    planned_placement_count
    executed_placement_count
    resolved canvas/output raster dimensions
    output buffer pointer and output byte length on success only

Success is valid only if:

    status == success
    planned_placement_count == expected_placement_count
    executed_placement_count == planned_placement_count
    output is present and passes defined output invariants

Any mismatch is a placement/internal execution failure. C must clean partial canvas/output state before returning failure. Rust additionally compares returned counts to its immutable plan before mapping a successful HTTP response.

Output byte count is a result fact, not proof it meets future product output limits unless a separately enforced limit exists.

## 16. Error Model

| Native category | Meaning | Rust/API treatment |
| --- | --- | --- |
| invalid_plan | Malformed pointer/count/version/enum/geometry inconsistency | Safe request/internal boundary failure; no details leaked. |
| unsupported_capability | Operation/feature/version not implemented | Stable capability rejection; no fallback. |
| invalid_source | Malformed/disallowed decoded input | Safe input failure. |
| invalid_geometry | Bounds, stride, overflow, impossible plan invariant | Safe validation/execution failure. |
| resource_failure | Native resource/slab/arena/policy-enforced failure | Safe resource/execution failure. |
| allocation_failure | Allocation/copy initialization failure | Safe resource/execution failure. |
| decode_failure | Decode could not establish valid source | Safe input/execution failure. |
| render_failure | Raster/resize/FIT/FILL failure | Safe execution failure. |
| placement_failure | Any planned placement cannot complete or count mismatches | Safe execution failure; no partial success. |
| encode_failure/output_failure | Encoder/output invariant failure | Safe execution failure; no artifact. |
| internal_failure | Unexpected native invariant breach | Generic internal error; redacted diagnostics. |

Native return values and safe structured counters are for Rust classification. Browser responses must not disclose filesystem paths, memory addresses, credentials, internal scheduler/topology, stack data, raw source/output bytes, or arbitrary native diagnostic strings.

## 17. Native Defensive Validation

Rust product validation does not eliminate C validation. C must independently validate:

- request/header version and size;
- pointer/count/feature consistency and alignment;
- every count/byte/stride/coordinate multiplication and addition;
- canvas, source crop and placement rectangle bounds;
- non-overlap when required by the plan;
- enum values and output capability;
- allocation/slab/arena availability and partial-initialization cleanup;
- expected versus actual executed count and output-result invariants.

Rust owns product values such as paper catalogue, preset authority and selected resource ceilings. C need not reproduce every business policy, but it must reject values that would make native execution unsafe or semantically impossible.

## 18. Scheduler Integration

The design keeps the existing C scheduler. The exact current historical render entry descends through img_api_begin_job or img_api_begin_job_with_photo and img_runtime_prepare_render_stage; current source evidence does not establish a public asynchronous layout submission API suitable for Rust.

Therefore the future capability must provide one native entry that owns the transition into the established scheduler/render path without exposing queue internals. It must not add a Rust queue, duplicate admission, or require the caller to manipulate native task objects. It should preserve synchronous terminal-call behavior at the FFI boundary while allowing C to schedule internally. Any scheduler-specific execution object is C-private and must own copied plan/input data for its entire queued/active lifetime.

This integration route is an ARCHITECTURAL REQUIREMENT; its exact internal call graph is a future implementation detail to be confirmed by focused scheduler tests.

## 19. Memory Model

The plan/control metadata and pixel memory are distinct:

- fixed request header may remain on the Rust call frame until C copies/validates it;
- placement metadata becomes a native-owned copy, suitable for C scheduler lifetime; it may use an arena only if bounded and reset safely;
- decoded source, canvas, resize intermediates and raster work remain C-owned;
- canvas/intermediates continue to use existing slab/pixel-buffer practices where compatible;
- transient metadata can use existing arena allocation where bounded;
- encoded output uses an explicit C allocator/owner and release function;
- no native pointer into Rust plan/input survives the allowed borrow lifetime;
- no borrowed arena/slab memory survives its owner/reset/destroy event.

Arena capacity is metadata/scratch policy, not proof that canvas/output resource use is bounded. The capability must not replace slab/arena behavior with generic malloc merely to ease FFI, nor expose those internals to Rust.

Every stage has one cleanup owner: C cleans native plan/source/canvas/arena/slab state after transfer; Rust releases returned C output exactly once through the safe binding; lifecycle workspace cleanup remains Rust-owned.

## 20. Performance Considerations

No benchmark establishes the cost of this boundary. Explicit rectangles add O(copies) metadata and one validated copy, while a compact grid may reduce bytes but requires proof against semantic drift. For small jobs, FFI conversion/allocation may be material; for large jobs, decode/resize/canvas/encode may dominate. Neither claim is measured.

Future benchmarks must compare canonical explicit rectangles with any compact internal representation for small and large jobs, varying placement count, source size, canvas size and concurrency. Measure latency, CPU, allocations, peak RSS, cache behavior and scheduler interaction. Performance may guide internal representation only after exact-count/geometry equivalence and sanitizer/security evidence pass.

## 21. Security Model

| Attack | Rust defense | C defense | Expected failure | Evidence needed |
| --- | --- | --- | --- | --- |
| Huge count/array | Product policy and checked estimate before admission | pointer/count multiplication and allocation checks | resource/invalid-plan rejection | overflow and large-count tests |
| Malicious coordinates/overlap | Validate canonical rectangles | revalidate bounds/overlap/checked additions | invalid geometry/plan | property and malformed-plan tests |
| Pointer/count mismatch or invalid alignment | Safe FFI construction | null/count/alignment/range checks before read/copy | invalid plan | FFI negative tests, sanitizers |
| Huge raster/canvas/stride | Checked Rust geometry/resource calculation | staged byte/stride/slab checks | resource/invalid geometry | boundary and ASAN/UBSAN tests |
| Hostile encoded input | Bounded ingress/source policy | decoder truth and native input guards | invalid source/decode failure | real-image, malformed, fuzz tests |
| Output/workspace amplification | Future measured policy | allocation/encoder failure cleanup | resource/output failure | representative-sheet measurements |
| Repeated expensive plans | Bounded Rust admission/lifecycle | retained C scheduler constraints | overload/deadline behavior | P5/P6 plus layout load evidence |

No plan/log/result contains secrets, client paths, source bytes, output bytes or unsafe native diagnostics.

## 22. Test and Evidence Plan

Before implementation is accepted, require:

| Area | Evidence |
| --- | --- |
| ABI/FFI shape | Fixed-width field/static-layout assertions in C/Rust bindings, pointer/count/size/alignment rejection, version/feature rejection, output-release tests. |
| Geometry | Exact 35x45 mm at 300 DPI = 413x531 px; conversion boundaries; checked paper/grid/canvas calculations; explicit rectangle and row-major golden cases. |
| Completion | requested == planned == executed for fewer/equal capacity; copies above capacity rejects; injected failed placement cannot return success. |
| Source/render | Real JPEG/PNG cases, malformed inputs, source/crop bounds, FIT/FILL reference fixtures once crop semantics are frozen. |
| Failure/cleanup | Allocation/decode/render/encode failures, partial initialisation, result zeroing, output release, workspace/lifecycle cleanup. |
| Memory safety | ASAN, UBSAN, leak checking, malformed plan fuzzing, input fuzzing, large-count/overflow cases. |
| Determinism | Same source plus same RenderPlan plus same capability version yields the same semantic placement/canvas result. Do not claim byte-identical JPEG absent encoder evidence. |
| Scheduler/lifecycle | Existing admission, deadline, disconnect and graceful-drain evidence with queued/active layout work; no hidden second queue. |
| Performance | Reproducible comparison of explicit and any compact internal form after correctness/security gates. |

## 23. Evidence Classification

| Decision | Classification |
| --- | --- |
| ABI v1 lacks layout; img_job_t is A4/float/internal; historical shrink/partial placement; arena/slab usage | PROVEN BY SOURCE |
| Existing narrow JPEG/PNG-to-JPEG behavior and prior lifecycle gates | PROVEN BY TEST/SOURCE as documented in P5/P6 evidence |
| Exact size, exact copy count, no silent fallback, Rust/C responsibility split | ARCHITECTURAL DECISION from P7.2.2/P7.2.3 |
| Explicit rectangles as canonical semantics; C-owned copied execution plan; synchronous FFI terminal boundary | ARCHITECTURAL RECOMMENDATION |
| First paper/output/decoration/crop/source capability scope and resource values | PRODUCT DECISION / DEFERRED |
| Public ABI generation, binary encoding, registry shape and async job-handle model | DEFERRED |
| Boundary performance and production capacity | UNKNOWN pending benchmarks/evidence |
| Scheduler internal invocation route for the new capability | UNKNOWN implementation detail; must be verified |

## 24. Open Questions

1. What first small capability feature set has product evidence: output, source set, paper and decorations?
2. Which public ABI generation/surface and discovery mechanism best preserve ABI v1?
3. Is a native explicit rectangle array sufficient, or is a proven equivalent compact encoding justified?
4. Which crop anchor and output DPI-metadata semantics are required?
5. What resource budgets follow representative-sheet memory/output/workspace/capacity measurements?
6. Does native source decoding occur before or after internal scheduling in the chosen implementation, and what copy/ownership cost is acceptable?
7. Is a future async native job handle needed, or is synchronous terminal FFI sufficient?
8. What stable error-token mapping is appropriate for a future public layout API?

## 25. Next Gate

The next gate is not layout implementation. First conduct a focused P7.2.5 capability-implementation readiness review that selects the minimal product feature set and resolves the remaining native ABI/ownership decisions with a proposed test fixture matrix. Only then may an explicitly approved implementation phase add a new native capability, safe Rust binding and API contract. ABI v1 and the C scheduler remain unchanged unless separately approved. Production readiness is not claimed.
