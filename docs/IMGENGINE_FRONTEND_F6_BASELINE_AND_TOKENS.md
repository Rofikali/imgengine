# F6-00 / F6-01A — source baseline and first design tokens

**Owner override — 27 November 2026, GO AHEAD ONLY:** `go ahead` is the sole manual continuation keyword (case-insensitive, optionally followed by IMGENGINE). Retire `next` as a shortcut; `next.md` remains the live board, not a command. Apply the five-step execution cycle and maximum five opened PR attempts per stable feature ID in [continuation policy](IMGENGINE_FIVE_STEP_CONTINUATION_POLICY.md). Count all authors, including closed/failed PRs; same-PR fixes and ordinary development pushes do not consume another slot. Finish earlier if accepted, stop on required blockers, and do not reset counts by renaming. Preserve explicit merge/deploy/spend approval boundaries, no new autopilot, no security bypass. This owner instruction supersedes any older shortcut aliases.

27 November 2026. F6-00 source inventory PARTIAL; F6-01A IMPLEMENTED development candidate, runtime verification NOT RUN. Base ab1112f0f1a014a3a93ebb8c2c614e70066d4114, includes unmerged CI-EFF-01 and roadmap. Main 1f57493bb0d0b6545bc03da12aed195bc5a87b1e. [PR #127](https://github.com/Rofikali/imgengine/pull/127) exact head 9937aeeeb80eec7f04e35a327f6800fd832f7108 [CI run 45967764812](https://github.com/Rofikali/imgengine/actions/runs/45967764812) passed all 12 jobs; this is evidence for that documentation PR only. PR #127 subsequently merged as cfb788635b8ff553794b90707c24d93946f93fdf; PR #128 is open at 95b85f0 and its CI remains in progress. No release/deploy authorization.

## Source baseline

| Component | Existing CSS characters | Candidate CSS characters | Substitutions |
| --- | ---: | ---: | ---: |
| app.vue | 4086 | 4249 | 12 |
| free-static/FreeSiteHeader.vue | 1253 | 1307 | 3 |
| free-static/landing.vue | 922 | 983 | 4 |

Only three inspected SFC style blocks, not entire frontend or compressed assets. New token sheet 350 characters. Nuxt 4.6.4 and Vue 3.5.44 direct production dependencies. Nine static routes including five tools, homepage and three notices. Global app CSS owns typography/form/notice/editor defaults, header scoped CSS owns navigation/mobile native disclosure, landing scoped CSS owns cards. Existing browser tests cover navigation, focus, no-upload, static routes, privacy notices and CSP. Actual compressed bundle, field/synthetic CWV, mobile peak memory, screen reader, visual regression and complete source inventory still NOT RUN. Do not infer faster runtime from token extraction.

## Candidate

Import one plain CSS token sheet from app.vue. Seven semantic custom properties for background, surface, text, muted text, accent, link/focus color and existing system fallback font stack. Replace exact literals only in the three SFC style blocks. No layout values, selectors, HTML, JS behavior, dependencies, routes, legal text, native/Rust code, server isolation or CSP changed. Intentionally leave distinct color values and additional token categories for later bounded work. No remote fonts or new requests. The original CSS style bodies are reconstructible byte-for-character from candidate values; static string round-trip verified for each block during preparation, not a browser equivalence test.

## Required verification before integration/release

Ubuntu 24.04 Docker only; locked install, pnpm test, typecheck, build, normal local-tool browser matrix, static build, verify:free-static, verify:free-publication-ready, full static browser matrix; compare computed token values, keyboard focus, desktop/mobile screenshots, and compressed CSS/JS baseline vs candidate. Full CI required because batch includes CI selector changes and new docs. Test commands NOT RUN for this candidate. Reconcile branch ancestry after #127 owner merge; do not skip tests or merge/deploy on ordinary go ahead. F5 owner-reported Workers URL could not be fetched by current browsing tool (DisabledError), so live HTTPS/CSP/provenance/billing remain unverified; no live changes. No F6 completion claim.

Cross-functional review: L1–L7 bounded reuse and separation of responsibilities; CISSP no new executable dependencies or trust boundary; CCIE no network change; Linux/SRE reproducible verification; CA/MBA/management no additional service or spending, no measured savings. Native/ABI, Rust, deployment and privacy contracts reviewed — no change. Domain, finance and monetization remain deferred.

## F6-00 compiled asset measurement candidate (27 November)

New dependency-free Node tool and fixture tests in frontend/scripts. Build in approved Ubuntu 24.04 Docker from the exact checked-out SHA, then run from frontend:

    node --test scripts/measure-free-static-assets.node-check.mjs
    pnpm run build:free-static
    node scripts/measure-free-static-assets.mjs .output/public "$(git rev-parse HEAD)"

Retain stdout as detached evidence outside public output. Audit static tree before/after measurement, record claimed source SHA (operator must independently verify build provenance), tree hash, Node/zlib/Brotli versions, file count, raw/gzip level 6/Brotli quality 5 totals and per-asset values. The totals cover the entire CSS/JS distribution, not initial-route transfer; compare matching environments and separately measure actual route requests/CWV. Missing assets, symlinks, invalid SHA and audit failures reject. Tests and measurements NOT RUN; no Docker available locally. Keep tool outside browser bundle and CI trigger unchanged. Reconcile branch after #128 owner merge; do not publish or bypass F5. L1–L7: explicit measurement contract; CISSP: existing static audit preserved; CCIE: estimated compression not network measurement; Linux/SRE: reproducible command; CA/MBA/management: no service cost or speedup claim. Native, ABI, Rust and release contracts unchanged.

## F6-00 CI integration preparation

The next candidate attaches the measurement fixture to existing frontend verification and runs measurement after existing static/publication checks, before the final static browser matrix. JSON stdout goes to RUNNER_TEMP, uploaded as imgengine-free-static-size-evidence for 14 days. No extra build or job. The source identifier is the actual checkout SHA (on PRs typically synthetic merge SHA, not PR head); the artifact is measurement evidence, not release provenance or approval. Positive audited fixture and forbidden-worker rejection added. All changes are NOT RUN until reviewed in Ubuntu CI; do not assert actual compression savings or performance. Existing matrix is 2 Chromium invocations plus 55 sequential isolated Firefox and 55 WebKit invocations (112 total); investigate actual per-shard durations and isolation history before proposing parallelism, do not reduce coverage or change shard count here. The current integration PR #128 remains untouched. Next: finish #128, reconcile this branch, open bounded integration PR, verify artifact and compare baseline. No production or billing change.
