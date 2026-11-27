# ADR-PORTFOLIO-005 — Five product families, one shared core, independent releases

**Current checkpoint — 27 November:** #131 merged as `cc4041e6e1738b7a0e1bf2271ec597996a4baa07`; [PR CI 45977976019](https://github.com/Rofikali/imgengine/actions/runs/45977976019) succeeded; [main CI 45979597557](https://github.com/Rofikali/imgengine/actions/runs/45979597557) running at inspection. Owner-approved detached portfolio documentation cafbc734 is integrated into this development candidate against the identical merged tree. CI-EFF-01 / GOV-FREE-SCOPE-01 candidate: exact reviewed local-tool source/test and shared frontend paths can select both Nuxt jobs; only existing isolated static presentation and static-specific spec opt into Free-static mode. Protected ordinary main pushes use previous main as trusted selector/gate checkout, verify exact base/head and fast-forward ancestry, and select only known frontend/planning paths; unknown/native/policy/mixed/deleted/renamed/invalid changes and merge queue remain full. Main runs no longer cancel earlier main runs, preventing loss of prior required coverage. Policy rollout itself requires full CI under old trusted base. New candidate fixtures and actual eligible PR/main runs NOT RUN; no claimed savings. Free release remains independently gated. F5 live evidence and F6 baseline outstanding; no paid product, deploy, C/Rust/Wasm or cost change.


**Owner-approved architecture direction — 27 November 2026:** One protected `main`, short-lived product-scoped feature branches, five product families with independently versioned, tested and released artifacts. Free remains the only active product; reuse visual UI without implementing paid logic. No permanent per-product branches, bulk directory moves, new toolchains, public paid source, new services or spending. This decision approves architecture direction and documentation, not releases or new product implementation. #130 merged `92703496208572f1ff07ec1cedb065bcf9b5b8af`, [main CI passed](https://github.com/Rofikali/imgengine/actions/runs/45976692568). #131 exact head `8818d6fa2258a3f4acfb066050535a7e008eb07d` has policy and frontend verification PASS, browser quality running at inspection ([run](https://github.com/Rofikali/imgengine/actions/runs/45977976019)). Preserve active CI; this documentation candidate is staged as an unreferenced commit for later integration, not pushed to the active branch or main. Candidate tests NOT RUN.

## Owner-approved operating model (27 November 2026; overrides earlier proposed status)

### Git and product ownership
- Maintain one protected trunk. Branch from fresh main: feat/free/*, feat/pro/*, feat/studio/*, feat/api/*, feat/sdk/*, feat/desktop/*, feat/shared-ui/*, feat/core/*, fix/* or docs/*. Merge reviewed scoped changes and delete short-lived branches after merge. Release/support branches only for actual freeze or supported-version backports, with explicit end date. No five permanent forks. An ordinary merge does not activate a product or deploy it.
- Keep existing physical directories; define logical product boundaries before migrations. Free: browser-local Nuxt/Vue/TS/HTML/CSS, Canvas/Worker. Shared UI: tokens, shell, navigation, accessible primitives, product registry; no paid processing or secret entitlements. Pro/Studio: planned, isolated feature modules and real consumer contracts. API: versioned Rust service, native C only as required. SDK family: distinct C native SDK, Rust wrapper, HTTP clients and desktop app artifacts. No browser loading Linux .so/.a directly.
- Public repository and branches are not confidentiality controls. Before adding proprietary code, decide licence, visibility and private-repository/package boundary; never put secrets or enforcement-only licensing logic in client assets. No repository split now without separate owner decision.

### CI/CD and independent release
- Always-on policy, trusted selector and aggregate gate; product dependency graph determines jobs. Free-only changes run Free static build, locked frontend install, audits, units/typecheck, privacy/CSP/no-upload, accessibility and affected browser checks only. Pro/Studio UI changes run their affected frontend consumers when they exist. Shared UI changes run actual frontend consumers; API/ABI, native, Rust, security or configuration changes run applicable transitive checks. Unknown impact fails closed. No C/Rust/Wasm for isolated Free. No global path-filter skip of required checks.
- Current selector is narrow and main/merge queue full. After #131 integrates, CI-EFF-01 / GOV-FREE-SCOPE-01 must test exact ownership/dependencies, ordinary main-push diff from trusted previous main, invalid/deleted/renamed/mixed/unknown fallbacks, and gate truth tables. Verify real eligible PR and main runs and record elapsed/runner minutes; preserve policy and separate exact-artifact release gates. Parallel experiment is opt-in, not verified. No automatic deploy.
- Independently version product artifacts: free-vX.Y.Z, pro-vX.Y.Z, studio-vX.Y.Z, api-vX.Y.Z (HTTP contract separately versioned), sdk-c-vX.Y.Z, sdk-rust-vX.Y.Z, sdk-ts-vX.Y.Z, desktop-vX.Y.Z. Tags are examples, not created. Product SemVer does not reset C ABI or schema versions. Record exact source SHA, dependency versions, artifact digest, SBOM/licence, compatibility, tests, approvals and rollback. Release only intended artifact; Free static hosting remains separate from future server/desktop systems. Roll back to previously verified artifact, never deploy all products on a main merge.

### Seven-level and cross-discipline review
L1 correctness, naming, tests; L2 cohesive components and errors; L3 contracts, resource bounds, integration; L4 shared UI ownership and dependency-aware CI; L5 compatibility, versioned migration and backport; L6 architecture and threat models; L7 portfolio evolution and cost-aware simplicity. CISSP: CSP/no-upload, secrets and tenant boundaries, least privilege, signed/provenance-verifiable packages; public source is not confidential. CCIE: Free static HTTPS only; future API authenticated ingress, quotas, private processing and egress controls. Linux/SRE: per-product Ubuntu Docker toolchains, deterministic builds, release rollback, measured latency and resource use. CA: separate cost centres (Free, Pro, Studio, API, SDK/Desktop and shared platform); allocate shared expenses transparently, track contribution, refunds, fees, support and taxes; no adopted pricing. MBA/management: Free adoption, repeat usage, then validate Pro and Studio demand before investment; API/SDK/desktop later. No paid service, domain, marketing, analytics or monetization authorization.

### Ordered work
1. Resolve and verify #131; owner merge; reconcile all exact-source evidence and ledger.
2. Finish bounded Free-only CI selection and main-push design, verify actual affected jobs; no policy bypass.
3. F5 live evidence reconciliation and F6-00 baseline; finish shared tokens/layout, product dropdown with Free active and Pro/Studio clearly planned, footer/home/editor and accessibility/performance. No fake links or paid implementation.
4. Independent Free release review, exact artifact, legal/hosting/cost and owner approval.
5. Only after explicit separate owner product decision: Pro, Studio, API, SDK/Desktop with distinct contracts and releases. Domain, finance/marketing and monetization remain deferred. Retain go ahead as sole manual shortcut, five-step cycle, maximum five opened PR attempts per stable feature, same-PR repair first, manual synchronized docs.



**Historical status (2026-11-19):** PROPOSED / OWNER-REQUESTED DOCUMENTATION ADD-ON. **No code, branches-per-product, release tags, SDK, desktop app, billing, public endpoint or deployment implemented by this ADR.** The owner requested that the five-product strategy be documented and cross-linked; approving the documents for review is not approval to implement paid services or publish releases. Preserve the four owner-selected lines in [the existing portfolio](IMGENGINE_FOUR_PRODUCT_PORTFOLIO.md) and the live facts in [next.md](../next.md).

**Decision to review:** Represent IMGENGINE as **five product families sharing a governed engineering platform**: Free, Pro, Studio, Developer API, and **SDK/Desktop**. The fifth is a proposed portfolio extension, with **Native SDK, HTTP API client SDK and Desktop application as independently packaged/versioned deliverables**, not a claim that they exist. Retain one primary `main` integration branch, short-lived changes, optional time-bounded release/support branches, and independent product/version tags. Do **not** create five permanent product forks/branches.

## 1. Source of truth and compatibility with existing decisions

This ADR supplements, rather than supersedes:

- [Four-product portfolio](IMGENGINE_FOUR_PRODUCT_PORTFOLIO.md) — current Free/Pro/Studio/Developer API scope, customer promises, pricing hypotheses and existing status.
- [Technology/repository policy](TECHNOLOGY_AND_REPOSITORY_POLICY.md) — strict C17 native core, Rust/Axum/Tokio + safe FFI, Nuxt/Vue/TypeScript, no **new** Python/Redis/PostgreSQL; retained legacy scripts/services are not to be removed by this ADR.
- [Product/infra milestone protocol](IMGENGINE_PRODUCT_INFRA_MILESTONE_AND_CODEX_USAGE_PROTOCOL.md) and [live board](../next.md) — product vs feature vs infrastructure completion and exact-SHA evidence.
- [API metering blueprint](IMGENGINE_DEVELOPER_API_USAGE_BILLING_CAPACITY_BLUEPRINT.md), [commercial gates](IMGENGINE_COMMERCIAL_READINESS_AND_UNIT_ECONOMICS.md), [seven-level review model](SEVEN_LEVEL_ENGINEERING_OPERATING_MODEL.md), [native ABI](ABI.md), [production checklist](MVP_RELEASE_CHECKLIST.md), and [P14 real-host evidence](P14_ACTUAL_VPS_EVIDENCE_RUNBOOK.md).
- [Product-release and compatibility matrix](IMGENGINE_FIVE_PRODUCT_RELEASE_COMPATIBILITY_MATRIX.md) — implementation-facing proposals and release gate inventory.

**Priority remains unchanged:** Free v1 includes **exactly five** browser-local tools (Crop, Resize, Compress, Rotate, Convert), without default image upload. The latest checked 2026-11-18 board says Resize/Crop/Rotate merged, F2c Compress next, F4 Convert afterward, Free not released. This dated context must not be treated as fresh verification. Pro, Studio and paid public API are not implemented/publicly launched by their existing design documents.

## 2. Product families and independently releasable artifacts

| Family | Customer job | Distribution / isolation | Initial commercial posture |
| --- | --- | --- | --- |
| **Free** | Privacy-first one-image tools | Nuxt/browser APIs, local file processing, no default upload/account | Free, potential ads only after separate approval |
| **Pro** | Creator recipes, repeatable bounded batch operations | Browser-first; license/entitlement and paid plans require separate decisions | Customer validation before subscription |
| **Studio** | Print-shop operator workflow, physical presets, proofs, sheet accuracy | Local where feasible; server image processing only by informed choice and passed host/native gates | Paid shop/seat/job-pack hypothesis |
| **Developer API** | Integrate reliable transforms into another application | Versioned HTTPS contract, private Rust admission/C17 execution, future tenant keys/quotas/metering | Separate metered B2B offering after verified capacity |
| **SDK/Desktop** | Integrate local native execution or operate a standalone app | Native C/Rust SDK, small HTTP API clients, and desktop app **must have separate lifecycles** | Discovery and licensing/provenance review first |

**Do not conflate:** an HTTP API *client SDK* (TypeScript/Rust examples using a hosted API) is not the locally embeddable *native SDK* (`.so`/`.a`/headers or Rust wrapper), and neither is a signed desktop installer. Distinct customers, contracts, security boundaries and release artifacts justify separate milestones `PRODUCT-SDK-01` and `PRODUCT-DESKTOP-01` *under* the fifth family, both **PROPOSED/NOT READY**, not new Free v1 work.

**Packaging hypothesis:** a Studio Desktop offline-first variant may be valuable for print shops, but it must not be marketed as available, assumed to contain all Pro entitlements, or implemented without operator evidence, Windows/Linux packaging checks, calibrated print tests and a privacy/licensing review. No automatic unrestricted API credits inside any UI product.

## 3. Architecture — one core, distinct adapters and trust boundaries

```text
                       Shared product governance/contracts
                                      |
             +------------+-----------+------------+------------+
             |            |           |            |            |
          Free UI      Pro UI      Studio UI    Developer API  SDK/Desktop
             |            |           |            |            |
      Browser-native  Local recipe  Local or     HTTPS Rust    Native SDK
      transforms      & batch      consented    Axum policy    or desktop
             |            |       Rust proxy       |            |
             +------------+           +------------+            |
                 (browser-specific)    |                          |
                              Rust application / safe Rust FFI   |
                                            |                    |
                                 Versioned C ABI <---------------+
                                            |
                                C17 image processing core
```

The illustration describes **component relationships**, not that a browser directly loads Linux `libimgengine.so`. Browser APIs and any future tested Wasm module are separate execution builds; Rust/Axum does not become an automatic upload fallback for local Free. Preserve native ownership/allocation/release rules and existing ABI compatibility: **commercial Platform 1.x does not reset the already versioned native ABI (e.g., existing ABI 2.3 references)**.

**Domain ownership:** C17 owns decode/geometry/layout/render/encode/scheduling, not accounts or pricing. Rust application services own HTTP authorization, quotas, admission, errors, billing/metering *if separately approved*. Nuxt owns client UX/local processing/safe proxy. Entitlements are enforced at appropriate trusted boundaries; browser-only license checks are not assumed tamper-proof. Durable *billing metadata* may be required; durable *image payload storage* is not assumed and requires a separate opt-in/privacy contract.

Favor **modular monolith, clear dependency boundaries and measured complexity**; use hexagonal adapters around billing/auth/providers where needed, strategy/capability registries for codecs, deterministic fixtures and typed errors. Do not force CQRS, Kafka, event buses, Kubernetes or a redundant image engine into simple local tools.

## 4. Git development and maintenance policy (proposed)

**Recommended:** `main` is the single integration source of truth; use short-lived `feat/free/*`, `feat/pro/*`, `feat/studio/*`, `feat/api/*`, `feat/sdk/*`, `feat/desktop/*`, `fix/*`, and `docs/*` branches; review one bounded PR per milestone. Create `release/<artifact>/vX.Y.Z` only for an actual freeze; retain `support/<line>` only with a declared support window and a backport policy. Product families are **not** permanent branches.

- Require existing CI plus focused changed-scope tests. Exact PR-head checks and branch protection are authoritative; no bypass of the seven existing mandatory checks. Record reviewed SHA, artifact identity, owner and release gates.
- A native/security correction affecting multiple products is merged once into the governed source and **backported** to supported release lines only after compatibility review/tests; never assume one green branch validates all shipped binaries.
- PR merged = feature merged; docs merged = docs merged; neither = publicly released. No automated tag, merge, deploy, vendor purchase or payment action authorized by this document.
- Use **signed/provenance-verifiable artifact identity** when supported, minimal permissions, pinned dependencies, SBOM and release reproducibility appropriate to each supported target. These are target release requirements, not claims of current automation.
- Alternatives rejected for now: (1) five permanent branches require repeated cherry-picks and conflict/security tracking; (2) five separate repositories duplicate a shared C/Rust contract; (3) tightly synchronized all-product versions force unnecessary releases. Reevaluate only with independent teams and proven divergent requirements.

## 5. Versioning — marketing family is not binary/API compatibility

Use SemVer `MAJOR.MINOR.PATCH` **independently per released artifact**. A proposed `Platform 1.x` can be a **marketing/documentation compatibility family**, never a forced numeric prefix or proof that every artifact is 1.x. Native public ABI version, Rust crate version, HTTP `/v1/` contract, schema version, desktop installer/app version and product entitlement/catalog version are distinct dimensions.

**Illustrative tags only — none asserted to exist:** `free-v1.0.0`, `pro-v1.0.0`, `studio-v1.0.0`, `api-contract-v1.0.0`, `sdk-c-v0.1.0`, `sdk-rust-v0.1.0`, `desktop-v0.1.0-beta.1`, `platform-v1.0.0`. Prefixes identify artifact owners, not guaranteed compatibility.

A release manifest should pin: product/artifact name and SemVer, Git commit, signed artifact digest, target OS/architecture, C ABI minimum/maximum, HTTP API contract/schema, frontend/runtime/FFI versions, image codecs and provenance, known limitations, exact test/CI evidence, migrations, support and rollback. See the [example matrix](IMGENGINE_FIVE_PRODUCT_RELEASE_COMPATIBILITY_MATRIX.md). No actual tags or manifest automation created here.

## 6. Review perspectives and secure-by-design gates

Use the existing [seven-level operating model](SEVEN_LEVEL_ENGINEERING_OPERATING_MODEL.md) **proportionally**:

1. Junior: exact requirements, reproducible test, naming, docs.
2. Mid-level: cohesive vertical slice, errors, scoped tests.
3. Senior: contract ownership, concurrency, failure and resource model.
4. Staff: cross-product integration, versioning and ABI/API compatibility.
5. Senior Staff: phased delivery, migration, backport and incident readiness.
6. Principal: shared platform boundaries, maintainability, threat model and long-horizon risk.
7. Distinguished: portfolio-level simplicity, evolution, irreversible trade-offs.

**Security/CISSP lens:** sensitive ID/passport images never cross product/tenant boundaries accidentally; no default image retention or Free upload; dangerous input bytes, decompression bombs, malicious image metadata and supply-chain codecs need budgets, validation, sanitizer/fuzz/negative evidence. API needs authenticated scopes, tenant isolation, rotatable keys, per-tenant **and global** memory/CPU/pixel/egress limits, redacted logs, durable idempotent billing events. Browser and Wasm are not security sandboxes. Desktop needs least privilege, signed artifacts/updates, OS/package permissions and a tested offline path.

**Networking/CCIE lens:** public ingress TLS, correctly bounded edge/body/timeouts, private Rust/network namespace, no exposed internal image processor, egress and DNS policies, overload/backpressure, incident monitoring and real-host rollback. On-host P14 evidence and C-A0/native disposition remain mandatory before public server-backed commercial work. The 2026-11-18 temporary picomatch exception expires **2026-12-02 UTC** and is **non-production only**, not a patch or a general high-severity waiver.

## 7. CA/MBA/management review

| Product | Validate first | Commercial measure |
| --- | --- | --- |
| Free | Five real useful tools, no-upload and accessibility, repeat visitors | Successful local download, retained users, CDN/support cost |
| Pro | Repeat creator tasks and saved minutes | Conversion, retention/churn, net contribution/user |
| Studio | Print shop timing, calibrations, spoiled paper/reprints and consent | Rework prevented, paid shops, contribution/shop |
| API | Developer integrations, throughput/latency/peak RSS and usage ledger | Cost per billable success including failure/abuse overhead, collected revenue |
| SDK/Desktop | Native integration/desktop adoption and support channels | Licenses, activation, distribution/support and signing costs |

No adopted rate card, refund/entitlement decision, tax conclusion, SDK license, subscription mechanism, ad approval or foreign-exchange assumption arises here. Gross cash collected, revenue recognized, variable contribution and operating profit are different. **Break-even active customers = fixed monthly cost / positive average contribution per active customer**; validate tax and legal treatment with appropriate professional review before charges. Confirm actual third-party/codec and root project license before redistributing proprietary binary artifacts; repository license statements require reconciliation from files and owner intent, not assumption.

## 8. Migration order and decision log

1. **Documentation/architecture only (this proposal):** cross-link this ADR and compatibility matrix; add fifth family as proposed in the live board without displacing the next READY Free feature.
2. **Continue Free:** verify F2c Compress actual implementation/tests, then F4 Convert, then F5 full release review. No SDK/Wasm/API infrastructure prerequisite unless evidence proves otherwise.
3. **Research:** Pro/Studio customer interviews; SDK/Desktop market/portability/packaging/printing-license feasibility; Developer API capacity/customer discovery.
4. **Implementation only after separate scoped approvals:** individual product feature PRs; native SDK packaging/API binding correctness; desktop UI/signed packaging; entitlements, metering and durable financial metadata ADRs.
5. **Release:** independent security/compatibility/operational/legal/finance gate, exact-SHA testing, owner go/no-go and real availability. `READY_FOR_RELEASE_REVIEW` precedes `RELEASED`.

**Open decisions:** confirm fifth-family customer demand; whether native SDK is source/binary/commercial and on which platforms; Desktop target OS, shell framework, local signing/update and offline license design; Studio Desktop SKU overlap; API client SDK language policy; semver support/backport windows; entitlement/storage/provider design; licensing/IP rights. Until resolved, mark SDK/Desktop `PROPOSED / NOT READY / NOT RELEASED`.

**Documentation-only acceptance criteria:** new ADR + matrix link resolves; `README.md`, `AGENTS.md`, `skills.md`, `next.md` and portfolio cross-reference where appropriate; original Free scope/current task unmodified; no CI/build/security policy changes, source moves or public release claims.
