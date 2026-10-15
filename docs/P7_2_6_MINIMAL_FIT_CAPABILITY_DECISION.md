# P7.2.6 - Minimal FIT Capability Decision

## 1. Purpose

This focused design gate freezes the four prerequisites for a first C-private layout capability:

1. FIT plus RGB background semantics;
2. source, plan and output ownership/lifetime;
3. C-private request/result representation; and
4. deterministic fixtures and oracle strategy.

It does not implement the capability, create a public ABI, bind Rust, change HTTP/Nuxt, or select product resource limits.

## 2. Evidence Reviewed

This decision follows P7.2.0-B through P7.2.5. Source inspection reconfirms:

- historical FIT in src/pipeline/layout_prepare_fit.c calculates a smaller scale, truncates positive float results, clamps to [1, cell dimension], and calls img_layout_prepare_fit_commit;
- the commit helper fills an RGB cell background and uses (cell - fitted) / 2 offsets, making an odd remaining pixel fall on right/bottom;
- src/pipeline/canvas_fill_background.c fills the complete RGB canvas before layout;
- historical geometry and placement have known silent-shrink/partial-success defects and are not the new contract;
- src/api/api_job_begin.c puts a render task on its stack, submits it, then img_runtime_wait_task waits until IMG_TASK_DONE before returning;
- worker tasks contain pointer payloads and can execute after queueing, so any data not guaranteed valid for that lifetime must be native-owned;
- src/api/api_encoded.c implements img_encoded_free with free for historical encoded output; ABI v1 instead provides its own imgengine_output_release.

The historical float FIT implementation is evidence for behavior, not a sufficient cross-platform authoritative arithmetic contract. P7.2.6 freezes a checked integer equivalent.

## 3. Minimal Capability Boundary

The first capability is a C-private, synchronous terminal operation conceptually called layout.render.v1:

    borrowed encoded bytes (decode completes before enqueue)
      -> C-owned decoded source
      -> C-owned execution metadata/explicit rectangles
      -> retained C scheduler and renderer
      -> RGB canvas filled once
      -> FIT each rectangle
      -> C-owned JPEG output plus structured completion result

It accepts resolved pixels only. It does not receive physical um, DPI, A4/paper/preset names, client paths, credentials, crop settings, FILL, decorations, metadata, or API/UI objects.

## 4. FIT Semantics

For positive decoded source dimensions Sw, Sh and positive destination dimensions Dw, Dh, FIT preserves every source pixel's semantic coverage, uses uniform scaling, never crops, never stretches, and never changes Dw/Dh.

Determine the limiting axis with checked 64-bit products:

    if Dw * Sh <= Dh * Sw:
        render_width  = Dw
        render_height = max(1, floor(Sh * Dw / Sw))
    else:
        render_height = Dh
        render_width  = max(1, floor(Sw * Dh / Sh))

All operands originate in the fixed positive uint32 raster domain. Each product is performed in uint64 and checked before use; the computed result must be in [1, Dw] or [1, Dh] respectively. Any zero dimension, impossible conversion, overflow, or bound violation fails before raster work.

This floor rule is the integer/rational equivalent of the existing positive float/truncation behavior, without using floating point as an authoritative contract. The minimum one-pixel clamp is retained only for a positive source/destination pair whose ideal fitted minor axis is less than one pixel. It never permits a result outside the destination. The later resize kernel's sampling semantics must be established by pixel-reference tests; this decision freezes output dimensions and placement, not an undocumented filter formula.

The fitted RGB image is centred in its destination:

    offset_x = floor((Dw - render_width) / 2)
    offset_y = floor((Dh - render_height) / 2)

Odd residual pixels go to the right and bottom. Thus left/top receive floor(residual/2), right/bottom receive ceil(residual/2). This matches historical commit placement and is now explicit.

FILL is deferred. No hidden center-crop or FILL fallback is permitted.

## 5. RGB Background Semantics

The first capability has an explicit, required RGB background triple (r, g, b), each an 8-bit channel. There is no alpha, transparency, gradient, pattern, colour profile or implicit default.

Rendering order is fixed:

    1. allocate and validate the concrete RGB canvas;
    2. fill every canvas pixel with the explicit RGB background;
    3. for each ordered placement, create/fill the placement-local FIT target
       with the same RGB background, render the fitted image at its offsets,
       then blit it into the canvas;
    4. encode the completed canvas as JPEG.

Consequently, pixels outside all rectangles and letterbox/pillarbox pixels inside a rectangle have identical, deterministic background semantics. Exact-aspect FIT has no remaining background inside its rectangle. A one-pixel residual follows the right/bottom rule in section 4.

The historical code already fills RGB backgrounds at canvas and FIT-cell stages. The new capability carries the same explicit value but does not inherit historical job defaults.

## 6. Rectangle Semantics

A placement is exactly four non-negative integer pixel values:

    x, y, width, height

The canvas origin is its upper-left pixel edge. x increases right; y increases down. The rectangle denotes the half-open set:

    [x, x + width) x [y, y + height)

width and height must be positive. C validates checked x + width and y + height and requires each far edge not exceed the concrete canvas dimensions. Width/height are not inclusive end coordinates, and no other rectangle convention is allowed.

The first capability rejects placement overlap before scheduling. It has no compositing, z-order or blending semantics. Ordered rectangles determine processing order and result/provenance order, but not overlap resolution. A rejected overlap is not repaired, shifted or clipped.

## 7. Placement Semantics

The caller provides an ordered explicit placement list and expected_count. The C-private execution plan stores an owned copy in that exact order. A successful result requires:

    expected_count == copied_placement_count == executed_placement_count

Every rectangle must be validated before any output is reported. A failed allocation, invalid rectangle, render failure or blit failure produces failure; no smaller canvas is returned as success. This freezes the resolution of historical skip-and-success behavior.

Grid/capacity are Rust plan concerns. If the source plan was derived from a grid, Rust must already have enforced copies <= capacity and row-major order. C neither recreates the grid nor converts capacity into copies.

## 8. Source Ownership

The first capability borrows encoded source bytes only for synchronous decode at entry. This is the minimum-copy choice supported by the current historical flow: img_api_begin_job decodes input before it calls img_api_begin_job_with_photo, while the latter may queue a render-stage task.

Rules:

- Rust owns encoded bytes for the complete native entry call.
- C validates pointer plus byte length and decodes before queueing any render task.
- On successful decode, C owns the decoded native buffer through the queued/active render lifetime.
- C never queues or stores the borrowed encoded-byte pointer.
- On decode failure, C owns no Rust source memory and returns a safe category.
- A future asynchronous native handle, deferred decode, or return-before-terminal execution must copy/take ownership of source bytes under a separately approved contract.

Borrowing is sufficient only because the first capability is terminal synchronous and decode completes before enqueue. It must be tested; it is not a general async ownership rule.

## 9. RenderPlan and Scheduler Lifetime

Rust owns the immutable canonical RenderPlan for the request. The FFI adapter may borrow its resolved scalar values only to construct a C-private request view. C must copy all execution metadata that a scheduled worker can observe:

- placement array;
- concrete canvas/cell dimensions;
- render mode/background;
- expected count and feature/version fields.

This copy occurs before scheduler submission. The resulting C-owned execution object remains valid through queueing, execution and terminal cleanup. C retains no pointer into Rust plan memory.

Current source demonstrates a stack task and synchronous wait, but its worker payload contains pointers. The new capability therefore does not rely on stack-task/Rust-borrow lifetime beyond entry validation. Deep-copying small plan metadata before enqueue is required; it is not a source-byte copy.

## 10. Output Ownership

The first C-private capability uses one allocator/release family for encoded JPEG output:

    C allocation/encoder creates output
      -> on success ownership transfers to Rust safe wrapper
      -> Rust invokes only the paired native release function exactly once

The C-private release operation must be the native wrapper around the same allocation family used by the encoder. Existing historical encoded outputs are released by img_encoded_free(), which calls free; that is evidence for a possible internal family, not permission for Rust to call free directly or for ABI v1 to change. The future public ABI may expose a different named release operation after a separate ABI decision.

On entry C initializes result output pointer to null and length to zero. Before successful handoff C owns any output allocation and releases it on later failure. After success, C must not access/release the output. The Rust wrapper is single-owner/non-cloneable, clears its stored pointer after release, and releases on normal drop/panic unwind. The output never outlives the request wrapper unless a later public ownership API explicitly transfers it.

## 11. Failure Cleanup

| Stage | Resources that may exist | Cleanup owner on failure | Returned category |
| --- | --- | --- | --- |
| Request validation | No transferred resources | C resets result only | invalid_plan/unsupported_capability |
| Decode | C partial decoder/decoded buffer | C | invalid_source/decode_failure |
| Native-plan copy | C execution object/rectangle copy | C | allocation_failure/resource_failure |
| Scheduler submission/wait | C execution object, decoded source | C after task terminal/failure | resource_failure/internal_failure |
| Canvas/FIT temporary | Slab/arena/canvas/intermediate buffers | C through existing owner_pool/arena cleanup | allocation_failure/render_failure |
| Placement/blit | Canvas and execution state | C discards canvas/output; no partial result | placement_failure/render_failure |
| JPEG encode | Encoder-owned provisional bytes | C releases provisional output | encode_failure/output_failure |
| Result handoff | C-owned JPEG bytes | C before handoff; Rust after successful handoff | internal_failure if invariant fails |

Every failure leaves output pointer null, output length zero, and executed_count either zero or a non-success diagnostic fact that Rust never treats as a completed artifact. No raw native diagnostic is returned to a browser.

## 12. C-Private Request

This is a semantic field contract, not an implemented C struct:

    NativeLayoutRequest
      request_version and required_capability_version
      encoded_source_bytes: const pointer + size_t length
      canvas_width_px, canvas_height_px
      background_r, background_g, background_b
      render_mode = FIT only
      placements: const pointer + size_t count
      expected_count
      output_requirement = JPEG only

Required validation:

- non-null source with non-zero length;
- non-null placements with positive count;
- fixed/known version, FIT enum and JPEG requirement;
- positive canvas/rectangle dimensions;
- checked canvas byte, placement byte and coordinate arithmetic;
- expected_count equals placement count;
- no rectangle overlap and all rectangles are in canvas.

The request contains no physical units, paper/preset names, HTTP/client data, paths, API keys, client MIME claims, layout-grid controls, decoration settings, or output metadata.

## 13. C-Private Result

This is a semantic result contract, not an implemented C struct:

    NativeLayoutResult
      status category
      executed_capability_version
      expected_count
      planned_count
      executed_count
      canvas_width_px, canvas_height_px
      output_width_px, output_height_px
      jpeg_output: owned pointer + size_t length (success only)

C initializes the full result to a safe empty state before validation. On success only, output is non-null and length is positive, all counts agree, and output dimensions equal the completed concrete canvas for this JPEG-sheet capability. On failure, output is null/zero and no count mismatch can be classified as success.

Result metadata is caller-provided fixed storage and never needs a heap error object. Status category is sufficient for Rust; detailed logging remains redacted/native-side.

## 14. Completion Semantics

A successful terminal call requires all of:

    status == success
    expected_count == planned_count
    planned_count == executed_count
    executed_count > 0
    JPEG pointer != null
    JPEG length > 0
    output dimensions == completed canvas dimensions

Rust compares returned count and dimensions with its immutable plan before any future HTTP success mapping. Any difference is failure even if JPEG encoding returned bytes.

## 15. Fixture Strategy

Fixtures are project-owned deterministic RGB or encoded image inputs with documented provenance. The implementation phase must define files, generated source, licence/provenance and independent expected results before relying on them.

| Fixture | Plan/result assertion | Primary oracle |
| --- | --- | --- |
| 1. Single placement | One valid rectangle, count one | Geometry/count and pixel reference |
| 2. Multiple placements | Ordered non-overlapping rectangles | Placement order/count and sampled pixels |
| 3. Fewer copies than capacity | Plan has fewer rectangles than source grid capacity | Geometry/count; no implicit placements |
| 4. Exact capacity | All planned grid-derived rectangles present | Geometry/count |
| 5. Too many copies | Rust plan rejects before native entry | Error/no-native-work evidence |
| 6. Horizontal letterbox | FIT leaves left/right or top/bottom background according aspect | Pixel/background oracle |
| 7. Vertical letterbox | Opposite aspect case | Pixel/background oracle |
| 8. Exact aspect | Fitted image equals destination dimensions | Geometry and pixel oracle |
| 9. Infeasible geometry | Plan rejects; no silent shrink | Error/entry-not-called oracle |
| 10. Malformed input | Safe native decode failure/cleanup | Error/cleanup oracle |
| 11. Output ownership | Success release once; failure has no output | allocator/release/ASAN oracle |

Fixtures 3 through 5 exercise Rust plan semantics; native C still separately validates the supplied list/count. The initial C-private adapter does not receive grid capacity.

## 16. Oracle Strategy

| Property | Oracle |
| --- | --- |
| Conversion/plan geometry | Independent integer geometry oracle using section 4 arithmetic. |
| Rectangle validity/order/count | Machine-checkable placement golden. |
| FIT dimensions/offsets | Scalar integer FIT reference oracle. |
| Background/letterbox | Small RGB pixel golden with intentionally distinct source/background colours. |
| Exact aspect | Pixel/geometry golden with no background interior. |
| JPEG artifact | Decode it and compare dimensions plus selected semantic pixels/regions; not byte identity. |
| Failure categories | Expected stable category plus null output and count invariant. |
| Ownership | Explicit release-once test plus ASAN/UBSAN/leak evidence. |

A tiny scalar FIT/geometry reference is required for test code. It must be simple, non-SIMD, use the frozen integer formulas, and remain separate from production optimized code. It is a correctness oracle, not an alternate renderer. Existing SIMD-001 remains separate: no grayscale operation is part of this capability, and SIMD must not define FIT semantics.

## 17. Security Invariants

- All dimensions/counts required positive and all byte/coordinate arithmetic checked.
- Pointer/count pairs are validated before dereference or multiplication; alignment is checked for typed rectangle access.
- Encoded input is never trusted for dimensions/type; C decoder establishes source facts.
- No overlap, out-of-bounds placement, overflow, unknown enum/version, silent clamp, clipping or partial success.
- C owns and cleans decoded/plan/canvas/intermediate state; Rust owns one returned output handle only.
- No source bytes, output bytes, keys, paths, addresses or arbitrary diagnostics appear in public result/logging.
- Resource ceilings remain future Rust policy; native safety checks are mandatory regardless of those values.

## 18. Decisions Frozen

| Question | Frozen answer |
| --- | --- |
| Does FIT preserve the full source? | Yes; no crop. |
| Is scaling uniform? | Yes; smaller rational scale, integer floor rule in section 4. |
| Is fitted content centred? | Yes. |
| Odd residual distribution? | Extra pixel right/bottom. |
| Background representation/area/order? | Required explicit 8-bit RGB; fill full canvas then FIT targets then placements. |
| Rectangle convention? | x/y/width/height, half-open, upper-left origin. |
| Multiple placements/overlap? | Ordered explicit list; exact count; overlap rejects. |
| Encoded-source owner? | Rust borrows only through synchronous C decode; C never queues that pointer. |
| RenderPlan owner? | Rust; C copies queued execution metadata before submit. |
| Decoded/canvas/temp owner? | C under existing native memory rules. |
| JPEG output owner/release? | C allocates; ownership transfers only on success; Rust calls matching native release once. |
| Request minimum? | Source slice, concrete RGB canvas/background, FIT, placement slice/count, expected count, JPEG requirement/version. |
| Result minimum? | Category/version/counts/dimensions and owned output pointer/length only on success. |
| Exact completion representation? | expected == planned == executed is mandatory for success. |
| Fixture/oracle approach? | Mandatory 1-11 set; geometry/count and small RGB pixel oracles, not JPEG byte identity. |

## 19. Decisions Deferred

- FILL, crop anchors and source-crop semantics;
- decorations, paper/preset/orientation catalogues and physical-unit parsing;
- PDF/PNG output, streaming, metadata/DPI metadata;
- public ABI generation, registry/discovery and Rust FFI surface;
- asynchronous native jobs and native cancellation;
- compact grid representation;
- product resource ceilings and capacity claims;
- exact resize sampling/filter scalar oracle, which must be selected in the implementation test design before asserting pixel goldens beyond FIT geometry/background.

## 20. Implementation Gate

| Area | Readiness |
| --- | --- |
| FIT semantics/background/rectangle semantics | READY |
| Source/RenderPlan scheduler lifetime | READY |
| C-owned output/release semantics | READY for C-private implementation; public ABI exposure remains DEFERRED |
| C-private request/result semantic fields | READY |
| Completion/no-partial invariant | READY |
| Fixture/oracle strategy | READY WITH ONE SPECIFIC DECISION: implementation must select exact project-owned fixture files/generator and freeze resize sampling oracle |
| Native scheduler integration | READY WITH ONE SPECIFIC DECISION: implementation design must identify the C-private entry that decodes before submit and owns the copied execution object |
| Public ABI/Rust/HTTP/Nuxt integration | DEFERRED |
| Overall C-private implementation | READY WITH ONE SPECIFIC DECISION: approve the two implementation-local selections above |

This gate does not mark production readiness or a public layout API as ready.

## 21. Recommended Next Step

Request explicit approval for a narrowly scoped P7.2.7 C-private implementation plan that identifies the precise internal entry files, finalizes the native request/result header layout and allocator wrapper, and selects project-owned fixture generator/files plus resize reference oracle. It may then implement only the C-private FIT/JPEG capability and native tests. It must not modify ABI v1, Rust, HTTP, Nuxt, the scheduler architecture, or deferred layout features.
