# P8 release audit and constrained-resource evidence

## Scope and starting state

This record is the evidence ledger for the P8 FIT-only layout release-candidate
audit. It is not a production-readiness claim.

The audit began on 2026-10-18 in the dedicated P8 worktree:

| Item | Recorded value |
| --- | --- |
| repository root | `E:\imgengine-p8-first-online-layout` |
| starting branch | `codex/p8-first-online-layout` |
| starting commit | `8b4c215d4b35d919d75e269397dee31416e83582` |
| canonical remote | `https://github.com/Rofikali/imgengine.git` |
| starting P8 work | uncommitted, isolated to this worktree; native ABI-v2/layout, Rust lifecycle/API, Nuxt proxy/UI/tests, P8 contract/docs, and frontend integration scripts |
| unrelated root worktree | `E:\imgengine` on `codex/p7-5-grayscale-observability`, with a documentation-table formatting edit, a deleted historical note, and two untracked P7.5 Docker files; deliberately untouched |
| Docker environment | Docker Desktop 4.40.0, Engine 28.0.4, Linux containers, cgroup v1, 4 GiB Linux VM, 12 visible CPUs |

The initial P8 worktree had no staged files. Its modified and untracked files
were reviewed as one coherent P8 implementation set before this audit branch
was created. No files in the unrelated root worktree are part of this effort.

## Gate A: implementation trace

| Boundary | Confirmed fact | Evidence |
| --- | --- | --- |
| browser | The Vue workflow constructs the declared layout multipart fields and consumes the returned JPEG as the preview/download object. | `imgengine-saas/frontend/app.vue`, `composables/useLayoutRenderWorkflow.ts`, `utils/render-form.ts` |
| same-origin proxy | `POST /api/layout-render` forwards only declared multipart fields to `/api/v1/layout-render`; `X-API-Key` is read from Nuxt private runtime config. | `server/api/layout-render.post.ts` |
| Axum | The endpoint authenticates, limits multipart input, validates the FIT-only plan, delegates to `RequestLifecycleService`, and maps stable errors without native diagnostics. | `imgengine/rust/imgengine-api/src/lib.rs` |
| lifecycle/admission | P8 reuses existing workspace, deadline, response-abandonment, bounded-admission, and worker ownership; it does not create a second queue. | `imgengine-supervisor/src/{request_lifecycle.rs,admission.rs,lib.rs}` |
| native | Rust passes resolved pixels and placements to additive `IMGENGINE_2.0`; C serializes via the existing opaque engine lock and retained scheduler, then returns a releaseable JPEG buffer. | `imgengine/src/api/imgengine_v2_layout.c`, `include/imgengine/api/v2/layout.h` |
| response | Successful output is a complete JPEG with safe attachment behavior and layout-count/dimension headers; the frontend uses that exact response. | API handler and Nuxt workflow above |

### Confirmed contract facts

- P8 accepts one JPEG or PNG, uses `px` or `mm` with checked half-up rounding,
  and supports only `output_format=jpeg`.
- Cell width/height, gap, margin, copies, optional columns, checked grid math,
  deterministic row-major placement, opaque-white background, and FIT-only
  rendering are defined in `P8_PRODUCT_CONTRACT.md`.
- The existing provisional 1 MiB body/input limit, queue capacity one, and
  10-second post-dequeue deadline apply. P8 additionally bounds copies,
  columns, dimensions, canvas pixels, and estimated RGB canvas bytes.
- The endpoint maps invalid requests to 400, media to 415, malformed images to
  422, resource bounds to 413, admission saturation to 429, drain to 503, and
  expired work to 504. Native work is not cancelled by deadline or disconnect.

### Findings and limits of this audit

| ID | Classification | Severity | Finding / evidence | Remediation priority |
| --- | --- | --- | --- | --- |
| P8-A01 | confirmed fact | medium | No configured encoded-output-byte ceiling or aggregate workspace-byte accounting exists. Current bounds constrain the plan but not final JPEG memory or all temporary bytes. | production-limit gate |
| P8-A02 | confirmed fact | medium | There is no dedicated health/readiness route. The simulation can only prove reachability through the existing render route (`GET` returns 405). | deployment/API decision |
| P8-A03 | confirmed fact | medium | The existing frontend Compose path is development-oriented (bind mount and `cargo run`/Nuxt dev), so it is not a production image or whole-VPS proof. | production packaging gate |
| P8-A04 | inference | medium | A 640 MiB Rust container plus 256 MiB frontend leaves too little headroom for a true 1 GiB VPS once OS, Docker, TLS proxy, and host services are included. The constrained run is service evidence only; a real-VPS smoke test is required. | deployment sizing gate |
| P8-A05 | partially measured | medium | The bounded run observed no OOM/restart and verified empty per-request workspace after completion, but its short periodic sample does not establish output peak memory, steady-state RSS, or a complete temporary-disk budget. | production-limit gate |

No exhaustive security audit is claimed. Controls are enforced where code and
tests demonstrate them; planned controls in `docs/security.md` remain planned.

## Constrained Docker Desktop profile

Run from the repository root on Windows PowerShell:

```powershell
pwsh -File infra/scripts/run-p8-vps-sim.ps1 -Build
```

The script creates a cryptographically random process-local test key, starts
only its `imgengine-p8-vps-sim` Compose project, waits for both services,
runs project-owned fixture requests, samples `docker stats`, writes a
machine-readable result outside the repository by default, and stops only the
containers it created. It neither commits a key nor removes volumes/images.

`infra/docker-compose.p8-vps-sim.yml` is a standalone constrained profile
that uses the established P7/P8 source boundary without bind mounts. It
enforces the following service limits on the current Docker Desktop runtime:

| Service | Memory / swap | CPU | PIDs | FDs | temp workspace |
| --- | --- | --- | --- | --- | --- |
| Rust API | 640 MiB / 640 MiB | 1.0 | 128 | 1024 | 48 MiB tmpfs, `0700`, `nosuid,nodev,noexec` |
| Nuxt frontend | 256 MiB / 256 MiB | 0.50 | 128 | 1024 | no application workspace |

These are enforced Docker service limits where the Desktop daemon reports
memory, swap, CPU CFS, and PID-limit support. The application's one-slot
admission and 1 MiB request limit are code-level facts, not invented Compose
environment settings. Docker Desktop here uses cgroup v1, so cgroup-v2 files
such as `memory.peak` are unavailable; the harness records effective Docker
limits and periodic `docker stats` samples instead. Periodic samples are not
exact peak measurements.

The two service limits sum to 896 MiB. Docker Desktop’s Linux VM is currently
4 GiB and its host accounting differs from a VPS. Therefore a successful run
does **not** prove that the whole system fits safely on a 1 GiB VPS; it is
evidence about constrained application containers. A real Ubuntu VPS smoke
test with the intended reverse proxy and OS budget remains required.

## Workload and acceptance criteria

The bounded harness uses the project-owned CC0 JPEG fixture and a locally
derived PNG. It checks decodable output for valid JPEG/PNG and runs:

- a multi-copy, non-zero-gap/margin layout;
- invalid output format, malformed input, excessive calculated canvas, and
  missing-required-field rejections;
- ten sequential valid requests; and
- a four-request concurrent burst, where only documented completion/rejection
  status classes are accepted because ordering is intentionally not assumed.

It never creates customer data, uses a fixed bounded request count, and leaves
generated files inside container `/tmp`. The run fails on unexpected HTTP
classes or invalid successful JPEG output.

Acceptance requires no container OOM/restart, no unexpected HTTP class, no
invalid JPEG, and no residual request workspace after the service’s normal
cleanup window. It also records status distribution and p50/p95/p99 from its
small sample. Percentiles are characterization values, not an SLA.

## Results, reproducibility, and cost decision

The profile executed successfully on 2026-10-18 against the reviewed candidate
at `8b4c215d4b35d919d75e269397dee31416e83582` on branch
`codex/p8-vps-resource-audit`. The machine-readable result was intentionally
written outside version control; this table preserves the reproducible facts.

| Measurement | Actual result |
| --- | --- |
| Docker / Compose | Engine `28.0.4`; Compose `2.34.0-desktop.1` |
| startup to both reachability checks | 14.149 s |
| enforced service budgets | API 640 MiB / 1.0 CPU / 128 PIDs; frontend 256 MiB / 0.5 CPU / 128 PIDs; both have equal swap ceilings |
| bounded workload | 21 requests in 4.037439 s (5.201317 requests/s across the complete mixed matrix) |
| successful / expected-rejection / unexpected | 15 / 6 / 0 |
| required valid paths | JPEG 200; PNG 200; non-zero copy/gap/margin layout 200; every successful response passed `identify` validation |
| required negative paths | unsupported output 400; declared-JPEG malformed bytes 422; excessive plan 413; missing required field 400 |
| bounded concurrent burst | two 200 and two documented 429 responses; no unexpected class |
| latency characterization | p50 0.132987 s; p95 0.239419 s; p99 0.335545 s across 21 mixed requests |
| cleanup evidence | 0 entries under the live API's `/tmp/imgengine-workspaces` after the workload, before Compose shutdown |
| periodic observed usage | one 500-ms sample: frontend 30.74 MiB / 256 MiB and API 49.91 MiB / 640 MiB; API CPU was 100.12% for that sample |
| container outcome | no OOM/restart was observed; the harness stopped and removed only its own containers/network after the checks |

The periodic sampler collected only two lines in the earlier run and one usable
line in the final run because this intentionally small workload completes in a
few seconds. It is therefore **not** an RSS peak measurement. The test images
are local Docker measurement artifacts of roughly 1.45--1.47 GB because they
inherit the existing verified development build image; their size is neither a
production-image claim nor evidence that a 1 GiB VPS has sufficient disk.

The 896 MiB service-limit sum already leaves no credible operating-system,
Docker-daemon, TLS-proxy, logging, or page-cache budget on a genuine 1 GiB VPS.
This result demonstrates correct behavior under per-service cgroup limits in a
4 GiB Docker Desktop VM only. It does not establish a minimum VPS size,
production throughput, peak memory, or production readiness.

No provider quote was inspected in this audit; monthly VPS, domain, DNS,
backup, monitoring, payment, bandwidth, and storage prices are **unknown**.
No capacity or cost-per-request claim is made. The evidence-based next step is
a real Ubuntu host smoke test after production image, health/readiness,
reverse-proxy, output-size, and workspace-budget decisions exist.

## Security review

The P8 route has a narrow, source-reviewed boundary: it accepts only a single
declared image part and declared scalar layout fields, authenticates before
multipart parsing, keeps the API key in Nuxt private runtime configuration,
uses server-generated workspace names, and returns stable client errors rather
than native diagnostics. The constrained profile runs as a non-root user with
a read-only root filesystem, dropped capabilities, `no-new-privileges`, a
private bounded tmpfs workspace, and loopback-only published frontend port.

This is not a complete deployment-security certification. In particular,
P8-A01 and P8-A02 remain open, no TLS/reverse-proxy configuration was tested,
and the Docker Desktop profile does not replace a real-host security review.

## Verification actually executed

The following checks were run after the P8 candidate and constrained-profile
changes. The initial offline verifier attempt was blocked by a missing
disposable Cargo cache entry; the source was unchanged. After fetching the
locked dependencies into the image's actual `/root/.cargo` cache, the unchanged
authoritative command completed with exit code zero.

| Gate | Result |
| --- | --- |
| Compose syntax for constrained profile | PASS |
| constrained P8 Docker workload | PASS; 15 expected 200 responses, 6 expected rejections, 0 unexpected; cleanup check passed |
| frontend unit tests | PASS; 26 tests |
| frontend type check and production build | PASS |
| P7/P8 Nuxt-to-Rust Compose integration | PASS; P5 render and P8 JPEG/PNG FIT-only layout contracts |
| Ubuntu 24.04 normal native CTest | PASS; 13/13 |
| native security, output-format, geometry, progressive regressions | PASS |
| Rust FFI smoke / supervisor / API | PASS; FFI smoke, 17 supervisor tests, 13 API tests |
| normal real-image verification | PASS |
| ASAN/UBSAN CTest and real-image verification | PASS; 13/13 and no sanitizer finding |
| sandbox lifecycle | PASS |
| libFuzzer targets | PASS; input validator 5,000 runs and decoder 5,000 runs |
| whitespace check | PASS (`git diff --check`) |

## Required release verification

The profile is additional evidence only. Before release review, run focused
P8 tests, frontend unit/typecheck/build, the P7/P8 Compose integration script,
and the unchanged Ubuntu 24.04 `imgengine/scripts/verify_linux.sh` gate with
its native, ABI, FFI, supervisor, API, real-image, sanitizer, sandbox, and
both required fuzz campaigns. Record any environment block exactly; do not
substitute this profile for that gate.
