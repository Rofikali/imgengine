# P8-SIM-02 — P12/P13 sheet/crop simulation and cgroup evidence

**State: implementation proposed / runtime unverified (2026-11-16).** Based on **main `730d843c672c62ed55240d204adc1f2afe8412f4`**, after [P8-SIM-01 merged PR #51](https://github.com/Rofikali/imgengine/pull/51). This is a *local Docker Desktop Ubuntu-engine simulation*, not a real 1 GiB VPS or approved public deployment. Follow [infra AGENTS](../infra/AGENTS.md), [P8-SIM-01 runbook](P8_SIM_01_RUNTIME_REPRODUCIBILITY.md), [P12 contract](P12_PHYSICAL_SHEET_CONTRACT.md), [P13 contract](P13_INTERACTIVE_CROP_CONTRACT.md), [M1.3 parity oracle](P13_M1_3_CROP_PARITY.md), and [agent CI policy](CI_FIRST_MINIMAL_AI_CONTEXT_POLICY.md).

## Outcome, source of truth, and boundaries

Keep P8 FIT-only original workload intact. Add a small, **separate P12/P13 smoke suite** inside the existing one-shot `workload` profile, hitting the *same-origin Nuxt endpoint* `/api/sheet-render`, which goes through Rust/Axum and the native C ABI. This is **not a replacement** for M1.3's independent every-pixel browser oracle, native security tests, or the seven required PR CI jobs. No new database, queue, user storage, credentials, production image/deployment, or paid infrastructure.

- **Valid P12:** omit all crop fields; assert HTTP 200, JPEG 100×100 and `x-imgengine-executed-count: 1`.
- **Valid P13:** submit a crop with all four normalized fields selecting a top-right square from a 128×128 PNG; assert HTTP 200, 100×100 output, executed count 1, and output differs from the P12 default (necessary but not sufficient as pixel-parity proof).
- **Invalid P13:** partial, repeated, negative, exponent-valued and out-of-bounds crop forms must return 400 with a problem+json Content-Type; a DPI above the P12 ceiling must return 413.
- **Unchanged P8:** original JPEG/PNG FIT, bad media, limits, malformed input, warmups, bounded concurrent burst and workspace cleanup remain in the same run. P8 request-level counters and separate P12/P13 success/rejection counts are in the final JSON. Uploaded test images and response bodies exist only in private temporary container files and are not included in reports.
- **Resource measurements:** P8 PowerShell harness still records sampled `docker stats` (500ms, not a peak), actual cgroup version, exact Docker/Compose versions, image IDs and configured limits, plus per-service `memory.peak` (when cgroup v2 supports it) and `memory.events` for OOM observations. `unavailable` is explicitly not zero. These are *container-lifetime observations*, not peak RSS of one render or aggregate host RAM.
- **Security:** retain non-root UID 10001, read-only roots, cap drop, no-new-privileges, no Rust host port and Nuxt loopback-only binding. No real images, customer files or production keys in workload. No generated files stored persistently.

## Reproduction on an isolated local Docker machine

Use an isolated checkout at exact PR SHA, then run the existing harness. Its **`-Build`** flag rebuilds all three simulation images (potentially expensive); the default runs already prepared images. The harness creates and cleans up a random **simulation-only** API key and Docker resources:

```powershell
git status -sb
git rev-parse HEAD
./infra/scripts/run-p8-vps-sim.ps1 -Build
```

The workload checks must be executed inside the profile; do **not** run them directly against a public URL. The report path defaults to a temporary directory. When the run fails, preserve the bounded failing Docker step log, not secrets, raw response bodies or large successful build transcripts.

## Completion gates — mark actual results, never guess

- [ ] `docker compose --profile workload config --quiet` PASS at PR revision, with only expected services/ports/network/limits.
- [ ] Clean/reusable-image Docker build PASS and all Rust/Nuxt healthchecks PASS, or precise failure captured.
- [ ] Original P8 workload: same expected successful and invalid HTTP responses, concurrent burst, cleanup PASS.
- [ ] New P12 and P13 smoke: successful 100×100 JPEGs/counts, different crop/default outputs, and all five invalid-input cases plus DPI limit PASS.
- [ ] JSON report includes `workload.sheet_crop`, real Docker/Compose/cgroup version, resource limits, cgroup high-water mark when available, memory events and OOM/exit fields.
- [ ] Verify cgroup peak/event values are meaningful for this Docker host; `unavailable` must not be labeled a passing measured peak. Investigate positive OOM counters rather than automatically accept the workload.
- [ ] All seven existing GitHub PR CI jobs PASS at exact PR head; **those CI jobs do not execute this dedicated P8 simulation** unless independently configured later.
- [ ] Distinguish `TESTED`, `NOT RUN`, `PENDING`, `FAILED`, `UNSUPPORTED` in handoff, and record actual SHA, Docker host settings, limits, result JSON and risks.

**EXIF coverage:** This P8 simulator checks server-side requests on a square generated PNG, **not EXIF orientation**. Browser EXIF 1–8 and native pixel parity remain in [M1.3 parity suite](P13_M1_3_CROP_PARITY.md). Do not claim the simulator alone verifies orientation or color management.

**Release gate:** P14 needs actual host CPU/RAM/overload, input abuse, edge protection, TLS/DNS, secret mount permissions, monitoring, rollback and owner approval. Simulation memory ceilings of 640+256+128 MiB sum to 1,024 MiB **without host overhead**, and are not proof of fitting a 1 GiB VPS.

## Engineering and management review

Junior/Mid: readable isolated contract cases and exact errors. Senior/Staff: preserves P8 tests, independently checks P12 omission and P13 selection, avoids duplication with M1.3 parity. Senior Staff/Principal: versioned HTTP/C17 contract boundaries, clean image/profile behavior and no unapproved infra. Security/SRE: bounded inputs, problem responses, non-root profiles, cleanup, kernel OOM evidence and truthful unmeasured data. CTO/CA/MBA: repeatable low-cost local verification; record actual build/compute cost and successful-image throughput before business claims.

**Milestone acceptance requires a real Docker-enabled run, not only code review.** No merge/deploy/production secret or spend is authorized by this document. After actual review and approval, reconcile `next.md` and select the next eligible task using the [agent continuation runbook](AGENT_MILESTONE_CONTINUATION_RUNBOOK.md).
