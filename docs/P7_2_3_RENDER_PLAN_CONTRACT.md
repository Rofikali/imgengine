# P7.2.3 - RenderPlan Contract

## 1. Purpose

This is a design-only contract for a future immutable, versioned RenderPlan between the Rust control plane and a future native layout capability. It does not implement RenderPlan, alter ABI v1, add an endpoint, or expose historical layout fields.

The contract preserves P7.2.2's resolved semantics:

- a requested physical photo size is exact after one deterministic conversion, or the request rejects;
- AUTO_FIT is not an implicit fallback and remains deferred;
- copies and grid capacity are distinct;
- a successful render has exactly the requested, planned, and executed placement count;
- Rust owns product policy and plan compilation; C retains scheduler and execution ownership.

## 2. Current Architecture Boundary

    Browser
      -> Nuxt same-origin proxy and advisory UI validation
      -> Rust/Axum authentication, validation, normalization, admission, lifecycle
      -> immutable RenderPlan (future)
      -> versioned native layout capability (future)
      -> retained C scheduler and raster execution
      -> encoded artifact

Current verified P5/P6 behavior is not this capability: POST /api/v1/render accepts one JPEG or PNG file and synchronously returns JPEG. The public header include/imgengine/api/v1/imgengine.h exposes image.jpeg_encode through imgengine_process_encoded_image_to_jpeg, not a layout operation.

Historical CLI/internal flow is CLI -> img_job_t -> C scheduler -> A4 canvas -> resize/layout -> placement -> post-processing -> encoding. It is evidence, not a public contract. In particular, img_job_t uses float centimetres and A4-coupled internal semantics.

## 3. RenderRequest Conceptual Model

RenderRequest is user/product intent. It is mutable only until Rust has authenticated, validated, normalized, resolved policy, and compiled the plan. It is not the native execution structure and must not be reconstructed in C.

| Conceptual group | Product meaning | Status |
| --- | --- | --- |
| source | One untrusted encoded source image; no client path is authoritative. | Current ingress supports JPEG/PNG; future layout source policy requires capability design. |
| photo | Physical width/height, fit mode, and eventually crop policy for one repeated photo. | Physical dimensions and FIT/FILL are architecturally defined; crop positioning deferred. |
| paper | A reviewed paper identity or approved custom physical dimensions plus possible orientation. | Future; no preset catalogue selected. |
| layout | Single-page copies; manual grid or a future automatic-layout policy; margins and gaps. | Copies/capacity semantics defined; automatic mode/product shape deferred. |
| print | Positive integer DPI and deterministic conversion semantics. | Architecturally defined; product maximum deferred. |
| decorations | Border, bleed, crop marks, background. | Historical native concepts; public subset and units deferred. |
| output | Requested enabled artifact capability. | Current verified output JPEG; future layout output set deferred. |
| preset | Reviewed product identity and allowed override policy. | Future; catalogue/version ownership deferred. |

Names in this table are conceptual categories, not final JSON, Rust, C, or ABI field names. A request never includes API credentials, internal filesystem paths, native stride/allocation values, generated workspace names, raw decoded pixels, or claimed source dimensions.

## 4. RenderPlan Conceptual Model

RenderPlan is created once by Rust after all authoritative request validation and before bounded admission. It is immutable for its full lifecycle. It holds concrete execution facts, not vague product terms such as passport size, A4, small margin, or large photo.

    RenderPlan
      identity: plan version, required capability, optional safe plan identifier
      source policy: accepted encoded input and decoder constraints
      requested physical geometry: normalized photo and paper values
      resolved raster geometry: canvas, cell, footprint and placement rectangles
      resolved rendering policy: FIT/FILL, defined crop policy, background, enabled decorations
      output policy: selected verified output capability and policy reference
      execution invariants: requested count, planned count, expected completed count
      checked resource facts/estimates: pixels, bytes, work, workspace and output
      provenance: preset/catalogue version when a preset was used

The plan contains neither uploaded bytes nor a browser path. Source image facts discovered only at native decode are inputs to execution validation/result reporting, not browser assertions embedded as truth in the plan.

The plan must be serializable only if a future ABI/capability design needs it; this document does not choose a wire format, Rust type, C struct layout, or ownership encoding.

## 5. Field and Category Definitions

| Plan category | Concrete values required | Authority |
| --- | --- | --- |
| Identity | render_plan_version; required_native_capability; optional non-secret identifier | Rust |
| Physical request record | requested photo/paper dimensions in um and DPI | Rust normalization |
| Canvas | concrete canvas width_px, height_px, channels/colour representation and byte/stride expectations | Rust calculation; C verifies |
| Cell and footprint | photo rectangle pixels, decoration outsets and final occupied footprint | Rust calculation; C verifies |
| Placement | ordered count and concrete valid rectangle semantics | Rust calculation; C executes/verifies |
| Raster policy | FIT/FILL, crop anchor only if defined, background, decoration instructions only if enabled | Rust resolves; C executes |
| Resource accounting | checked exact values and clearly labelled estimates | Rust estimates; C protects actual allocation |
| Output | capability identifier, resolved output parameters and output-policy reference | Rust selects; C encodes |
| Completion contract | expected requested/planned/executed count equality | Both layers enforce |

All future fields must name their unit/domain. No generic cross-layer width, height, margin, gap, padding, or size field is acceptable.

## 6. Units

| Value family | Canonical contract representation | Requirements |
| --- | --- | --- |
| Physical photo/paper/margin/gap/decorative distance | integer micrometres (um) | Dimensions positive; clearances non-negative; parse decimal presentation input without binary-float authority. |
| DPI | positive integer pixels per inch | Zero and fractional values reject. Product maximum is deferred. |
| Raster dimension and coordinate | non-negative integer pixels (px) | Positive for required rectangles; no float coordinate contract. |
| Bytes, stride, allocation | named byte integers | Calculated with checked arithmetic, independently of pixels. |
| Colour | explicit format/channel values | Do not infer from a vague background field. |

The historical float centimetres in include/pipeline/job.h are not an authoritative future request, plan, or ABI unit.

## 7. Coordinate System

Raster coordinates are pixel-edge coordinates. Canvas origin is (0,0) at upper-left; x increases right and y increases down. A rectangle (x, y, width, height) denotes:

    [x, x + width) x [y, y + height)

It is valid only when width and height are positive, each addition is checked, and x + width <= canvas_width_px and y + height <= canvas_height_px. Rectangles must not overlap unless a separately declared decoration rule explicitly permits and describes that overlap. No plan relies on native clipping to make an invalid rectangle valid.

## 8. Physical-to-Raster Conversion

For every non-negative physical_um and positive integer dpi:

    px_from_um(physical_um, dpi) =
        floor((physical_um * dpi + 12,700) / 25,400)

This is round-to-nearest, ties upward. Multiplication, addition, division input and every dependent geometry calculation are checked. A positive physical dimension resolving to zero pixels rejects.

The conversion happens once at plan compilation. The plan records requested physical geometry and resolved raster geometry so quantisation is auditable. It never applies feasibility scaling. Example: 35,000 um x 45,000 um at 300 DPI resolves to 413 x 531 px. The associated physical extent is px * 25,400 / dpi; a future output contract must separately define DPI metadata/print calibration.

## 9. Paper and Canvas Semantics

Paper identity, paper physical dimensions, and raster canvas are separate:

    reviewed preset or approved custom paper
      -> concrete paper_width_um and paper_height_um
      -> chosen orientation, if supported
      -> canvas_width_px and canvas_height_px

Rust resolves any future paper/product choice. C receives concrete geometry, not A4/A5/A6 labels. The historical IMG_A4_W_CM and IMG_A4_H_CM usage in include/pipeline/canvas.h proves current A4 coupling, not a future catalogue.

Current support: no layout paper contract is public. Future/deferred: paper presets, custom paper, orientation, and their range/policy. The RenderPlan design must accommodate them without claiming they are enabled.

## 10. Grid and Copies Semantics

For an explicit single-page grid:

    capacity = checked(rows * columns)
    1 <= copies <= capacity

Empty cells are permitted when copies are lower than capacity. They are not implicit copies. Zero copies rejects. Copies above capacity reject before admission.

With concrete pixel values, initial geometry is:

    grid_width  = columns * photo_width_px  + (columns - 1) * gap_x_px
    grid_height = rows    * photo_height_px + (rows    - 1) * gap_y_px

    required_width  = margin_left_px + grid_width  + margin_right_px
    required_height = margin_top_px  + grid_height + margin_bottom_px

Feasibility requires checked required dimensions not exceeding the canvas. Rows, columns, photo dimensions, gaps, margins, pixel count, stride and canvas bytes all require checked multiplication/addition. No fit calculation changes cell dimensions to recover feasibility.

The current historical grid uses rows * columns as effective placement count; the new plan must not do so.

## 11. Placement Semantics

Initial grid order is deterministic row-major: row zero through final row, and within each row column zero through final column. The first copies cells are occupied. Each planned rectangle is individually validated before admission.

The required success invariant is:

    requested_copies == planned_placement_count == executed_placement_count

A bounds failure, failed placement, allocation failure, or native invariant failure is an execution failure with no successful layout artifact. It is never a smaller successful output. This directly avoids the current historical behavior where img_layout_grid_place_cells() continues past invalid cells and img_layout_grid() returns IMG_SUCCESS with a reduced count.

### Representation analysis

| Option | Advantages | Risks/limits |
| --- | --- | --- |
| Deterministic grid specification | Small representation; matches historical execution shape; easy human audit for regular sheets. | C must reproduce geometry exactly; a mismatch can reintroduce divergent arithmetic/partial placement; does not directly represent non-grid layouts. |
| Explicit ordered rectangle list | Each requested execution target is concrete, independently validated and auditable; naturally supports future non-grid plans; makes count equality direct. | Memory grows with copies; list validation and ABI encoding require bounds/ownership design. |

There is no performance evidence sufficient to select a final native encoding. The architectural recommendation is that the canonical Rust plan has explicit ordered placement rectangles, optionally retaining a grid summary for provenance/debugging. A future native capability may use compact deterministic-grid encoding only if it proves equivalent geometry, count, overflow and bounds behavior against that canonical plan. This is an ARCHITECTURAL DECISION, not an ABI layout decision.

## 12. FIT, FILL, and Crop Semantics

The source image and destination photo cell are distinct:

- source dimensions/channels are C decoder facts;
- requested photo physical dimensions are normalized Rust intent;
- resolved photo cell pixels are immutable plan geometry;
- source crop rectangle is an execution value constrained by decoded source bounds;
- destination rectangle is the plan's cell rectangle.

FIT uniformly scales by min(target_width/source_width, target_height/source_height), preserves source aspect ratio, and uses the specified background for remaining cell pixels. FILL uniformly scales by max of those ratios, preserves aspect ratio, and crops overflow to fill the target exactly.

Historical source indicates centered FILL commit behavior, but P7.2.2 left crop-position product semantics open. Therefore crop positioning beyond a future explicitly named and tested policy is DEFERRED. The plan must not rely on an undocumented centre fallback. FIT/FILL cannot alter physical cell size, copies, paper, margins, gaps, or feasibility.

## 13. Decorations

| Historical concept | Future plan treatment | State |
| --- | --- | --- |
| Border | May be an explicit inside-cell decoration with exact unit, width, colour and order. | Deferred public semantics |
| Bleed | Requires defined physical purpose, explicit reserved outset and overlap/clipping policy. | Deferred |
| Crop marks | Requires referenced edge, length, thickness, offset, clearance and draw order. | Deferred |
| Background | Required concrete colour representation when FIT/empty cells expose it. | Capability-dependent |

Historical order is border, bleed, then crop marks in src/runtime/job_postfx.c. Historical pixels and clipping behavior are not adequate future public semantics. No decoration is enabled merely because internal C code can draw it. If a decoration is enabled later, its resolved footprint/outset must be included in plan feasibility and resource calculations.

## 14. Source Image Contract

| Value | Authority | Plan relation |
| --- | --- | --- |
| Encoded source file and MIME claim | Untrusted ingress | Validated by Rust/native policy; not embedded as raw bytes. |
| Decoded width/height/channels/actual format | C decoder truth | Must meet source policy; may be safely returned as result facts. |
| Decoded stride/allocation layout | C/native only | Must not originate from the client or be treated as product geometry. |
| Requested photo physical size | Rust-normalized request | Immutable plan input, independent of source dimensions. |
| Planned photo raster size | Rust conversion | Immutable target cell geometry. |
| Crop rectangle | Future native execution fact | Must be within decoded source bounds and conform to selected crop policy. |

C still rejects malformed or unsafe images and validates actual source facts. Rust does not assume that a client-supplied filename, MIME type, dimensions, or metadata is true.

## 15. Resource Estimation

Before bounded admission Rust must calculate, with checked arithmetic:

    decoded-source-policy facts
    canvas pixels and canvas bytes
    cell pixels, footprint pixels and placement count
    placement descriptor/list bytes
    resize/intermediate working bytes
    decoration/outset pixels and work
    estimated encoded output bytes
    workspace/transient bytes
    estimated resize/blit/post-processing/encode work

Canvas dimensions, placement count and descriptor count may be exact values for a selected plan representation. JPEG encoded bytes, timing, allocator overhead, compression scratch, temporary file use, and streaming/network behavior are estimates unless independently bounded. An estimate is not a hard guarantee.

No production values are selected here. Resource policy inputs must eventually come from reproducible representative-sheet measurements, sanitizer/security evidence, workspace/output behavior and capacity characterization. Current P5/P6 limits for JPEG/PNG-to-JPEG are not layout product limits.

## 16. Failure Semantics

| Category | Example | Required outcome |
| --- | --- | --- |
| Invalid request | malformed physical value, invalid enum, zero copies | Stable safe rejection before admission. |
| Unsupported capability | paper/output/decoration/crop policy not enabled | Stable safe rejection; no substitution. |
| Invalid geometry | zero pixel result, overflow, impossible fit, invalid rectangle | Reject before admission. |
| Capacity violation | copies > rows * columns | Reject before admission. |
| Resource policy rejection | checked estimate exceeds selected future policy | Reject before admission. |
| Admission overload | bounded queue cannot accept valid work | Existing 429 overloaded behavior. |
| Native execution failure | decode, arithmetic, allocation, placement, encoder failure | Safe failure; no successful artifact; cleanup. |
| Partial placement | completed count differs from planned count | Native invariant failure; no success. |
| Output policy failure | encoded result violates a future enforced output policy | Safe failure and cleanup. |

HTTP status/error token details for a future layout endpoint are not chosen in this document. They must preserve existing fail-closed authentication, redaction, lifecycle and cleanup properties. Deadline remains lifecycle behavior; ABI v1 has no mid-operation native cancellation.

## 17. Rust Ownership

Rust owns authentication; request/schema validation; product/preset policy; unit normalization; checked geometry/resource calculation; RenderRequest-to-RenderPlan compilation; plan immutability; bounded admission; workspace lifecycle; safe public errors; and selection of an available native capability.

Rust must reject invalid or unsupported intent before admission where possible. It must not delegate product-policy ambiguity to C or rely on Nuxt validation. It cannot treat its plan as a reason to remove C defenses.

## 18. C Ownership

C retains decode truth, native image/raster work, FIT/FILL execution, placement and decoration execution when a capability supports them, encoding, memory/arena/slab ownership, checked arithmetic, stride/allocation/bounds validation, execution-count verification, and the C scheduler.

C receives concrete normalized execution information. It must reject inconsistent/corrupt/unexecutable plans and fail rather than silently correcting or skipping work. This contract does not migrate or redesign the scheduler.

## 19. ABI Relationship

Historical internal layout capability is not current public ABI capability:

    historical img_job_t/layout pipeline != ABI v1 image.jpeg_encode

ABI v1 remains unchanged. Internal img_job_t must not become a public FFI/HTTP structure because it carries float-centimetre, template/A4 and pixel-field ambiguities. A layout capability needs a separately reviewed versioned public contract with explicit ownership, error and compatibility rules.

## 20. Versioning

These identifiers have independent scopes:

| Identifier | Scope | Compatibility rule |
| --- | --- | --- |
| HTTP API version | Browser/proxy to Rust public wire contract | Does not imply plan or ABI compatibility. |
| RenderRequest version | Product request vocabulary | Rust must reject or explicitly migrate unsupported request versions. |
| RenderPlan version | Normalized immutable plan semantics | Rust emits only a version it can prove and the selected native capability can execute. |
| Native capability version | A named layout execution contract | C rejects unsupported/invalid plans before execution. |
| ABI version | Public binary ABI stability | ABI v1 remains image.jpeg_encode; it is not silently extended. |
| Preset/catalogue version | Product metadata/provenance | A plan records the resolved version when a preset is used. |

A Rust producer must not send RenderPlan v2 to a native capability that only supports v1. The plan declares its exact required capability/version. Compatibility negotiation, binary encoding, backward migration and multi-version retention are DEFERRED until native capability design.

## 21. Security Model

| Threat | Rust control-plane defense | C execution-plane defense |
| --- | --- | --- |
| Huge DPI/paper/photo/copies/rows/columns | Parse, normalize, checked estimate and selected policy rejection before admission. | Checked conversion/arithmetic, allocation/slab constraints. |
| Integer overflow or zero/negative value | Closed schema/domain validation and checked arithmetic. | Validate received dimensions/strides/counts; reject inconsistent values. |
| Invalid coordinates/overlap | Validate canonical placement rectangles. | Revalidate all bounds and execution invariants. |
| Malformed/hostile source | Authenticated bounded ingress and source policy. | Decoder truth, existing native input guards and memory safety checks. |
| Output amplification/workspace exhaustion | Future measured policy and resource estimate. | Actual allocation/encoder failure handling; ownership cleanup. |
| Client path/secret leakage | Never put paths/keys/raw bytes in request/plan/logs. | No client-controlled filesystem path or leaked diagnostics. |

Both layers are required. Rust preflight is product/admission security; C validation is corruption and native-safety defense in depth.

## 22. Test and Evidence Matrix

| Area | Required evidence before implementation is accepted |
| --- | --- |
| Conversion | Unit tests for exact 35x45 mm at 300 DPI = 413x531 px, half-pixel boundaries, zero, overflow and deterministic repeated calculation. |
| Geometry | Independent reference geometry tests for margins, gaps, canvas, 1x1, 1xN, Nx1, near boundaries and impossible layouts. |
| Copies | One, fewer than capacity, exact capacity, capacity plus one, zero and extreme checked multiplication; row-major order goldens. |
| Placement | Every rectangle in bounds/non-overlap; fault-injected failed cell proves no partial success. |
| FIT/FILL | Same/wide/tall/extreme source aspect fixtures, crop bounds, reference pixel/geometry goldens once anchor is chosen. |
| Paper | Multiple concrete paper geometries when enabled; C receives only resolved canvas geometry. |
| Decorations | Disabled/valid/excessive/overlap/boundary tests after each decoration's semantic contract is approved. |
| Resources | Checked calculation/property tests plus representative-sheet memory, output, workspace, latency and admission measurements before values are selected. |
| Boundary | ABI/FFI ownership/error/version compatibility tests; real JPEG/PNG end-to-end tests. |
| Native correctness | Sanitizer, fuzz, regression and scalar/reference equivalence evidence where optimized code executes. SIMD-001 remains separate unless later code makes it part of layout. |
| Lifecycle | Existing P5/P6 cleanup, deadline, disconnect and graceful-drain evidence remains required for admitted layout work. |

## 23. Proven, Proposed, and Deferred Decisions

| Topic | Classification |
| --- | --- |
| Historical A4/float internal job model; silent shrink; partial placement success; ABI v1 lacks layout | PROVEN BY SOURCE |
| Existing narrow JPEG/PNG-to-JPEG route and P5/P6 lifecycle behavior | PROVEN BY SOURCE/TEST evidence documented elsewhere |
| Deterministic conversion formula; exact-size rejection; copies/count invariant; row-major initial grid; Rust/C responsibility split | ARCHITECTURAL DECISION from P7.2.2 |
| Explicit rectangles as canonical plan semantics, with any compact grid encoding proving equivalence | ARCHITECTURAL RECOMMENDATION |
| Paper catalogue, custom paper, orientation, output formats, decoration subset/units, crop position, AUTO_FIT, product resource ceilings | PRODUCT DECISION / DEFERRED |
| Binary plan encoding, public ABI/capability version, compatibility negotiation and native ownership layout | DEFERRED |
| Production capacity/print calibration/output budget | UNKNOWN pending evidence |

## 24. Open Questions

1. Is an explicit AUTO_FIT policy commercially required after exact-size MVP evidence?
2. Which paper/preset catalogue, custom paper rules and orientation support are justified?
3. Which decoration subset and physical-unit/clearance semantics should be exposed first?
4. Is a centre crop anchor sufficient, and how will any other anchor be specified and previewed?
5. Does initial layout remain one source and one page?
6. Which output capability, DPI metadata policy, streaming model and output ceiling are supported?
7. Which measured resource budgets permit safe layout admission?
8. Does the future native capability receive an explicit rectangle list, an equivalent compact grid encoding, or both?
9. What stable public ownership/error/versioning model is appropriate for that capability?

## 25. Recommended Next Implementation Gate

Do not implement layout yet. The next evidence-based gate is **P7.2.4 - Native Layout Capability Design**: map this canonical plan to a deliberately versioned native capability; define native ownership, error and count-completion behavior; choose no public ABI shape until compatibility/FFI evidence is designed; and identify the minimum exact-geometry native changes/tests. It must preserve ABI v1 and the retained C scheduler. Production readiness is not claimed.
