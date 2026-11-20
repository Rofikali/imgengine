# IMGENGINE Free v1.0.0 — Release evidence and go/no-go

**Status:** PLANNED / **NOT RELEASED**; no static Free release candidate built, tagged, approved or deployed. Read [release overview](README.md), [static contract](static-only-contract.md) and [shared release governance](../../README.md).

## Verified historical milestones (not public-release checks)

| Evidence | State | Proof/limits |
| --- | --- | --- |
| Five feature implementations | MERGED | Resize #57; Crop #59; Rotate #62; Compress #72; Convert #76. This does **not** certify static export. |
| F5-NAV-01 navigation and full local-tool browser matrix | PASS **for feature PR head only** | [PR #80](https://github.com/Rofikali/imgengine/pull/80) head `1fce2098fe4a0ff1d6bf1e7c29581c0d0892822c`; [all 12 CI jobs passed](https://github.com/Rofikali/imgengine/actions/runs/44678857538), including browser quality, native security checks and gate. Merged as `f65fde4f8e2794a10b78f936bb0f2c035c32301f`. |
| Post-merge `main` CI | PASS **for current integration, not static artifact** | [Run #44679764303](https://github.com/Rofikali/imgengine/actions/runs/44679764303). Reconfirm latest main during next engineering task. |
| C-A0 malformed PNG defensive fix | MERGED | [PR #81](https://github.com/Rofikali/imgengine/pull/81) merge `a50c6e1469022eb8b0d3cbe5d76e1290f5db1d45`; not a Free launch waiver. |

**F5-STATIC-01 implementation MERGED (2026-11-20):** [PR #83](https://github.com/Rofikali/imgengine/pull/83) merged at `f1f61cfeecc565cbf3c44de57a747d1369b11000`; exact PR head `772e9375bbe638efe9771e9b0a5d025feeca30f6` passed [12/12 selected CI checks](https://github.com/Rofikali/imgengine/actions/runs/44686554819), including 84/84 existing server-backed browser and **100/100 separate static-output browser tests** and policy scan. CI generated 36 files, 292,571 bytes and a deterministic static-tree inventory digest. These are **historical implementation checks**, not a frozen candidate, independent hosting test, legal signoff or authorized public release.

## Required release-candidate gates — none presumed passed

**F5-TRUST-01 implementation candidate (2026-11-20):** /privacy, /terms and /support pages and footer belong **only to the Free static target**, not the existing Rust/C/Nuxt server experience. All three pages are visibly **PRE-LAUNCH DRAFT**. No verified operator name, private security/privacy contact, support response owner, terms signoff, effective date, provider analytics setting or privacy retention schedule has been supplied. Publication preflight is intentionally **BLOCKED**; development CI tests that blocking behavior rather than waiving it.

**Picomatch status change to investigate:** [GitHub withdrew GHSA-fc5q-39xc-vjpf on 2026-11-18](https://github.com/advisories/GHSA-fc5q-39xc-vjpf), stating it is not a security issue. This supersedes the advisory premise of the [2026-11-18 temporary nonproduction exception](../../../FRONTEND_PICOMATCH_TEMPORARY_NONPROD_RISK_ACCEPTANCE_2026-11-18.md). Do **not** silently remove the waiver, declare all dependencies secure or change critical/high policy without a fresh exact-lockfile `pnpm audit --prod`, advisory provenance reconciliation and security review. Independently examine other current findings and the production supply chain.

**Legal review reference:** [India's DPDP Act](https://www.indiacode.nic.in/indiacode/handle/123456789/22037?view_type=browse) and [DPDP Rules 2025 Gazette](https://egazette.gov.in/WriteReadData/2025/267650.pdf) have phased commencement; evaluate obligations applicable to the **actual launch date and data processing**, including CDN metadata, rather than inserting a generic compliance claim. Legal and customer-privacy decisions remain owner/reviewer responsibilities.

| Gate | Required evidence at **exact static build SHA + digest** | State | Owner / next work |
| --- | --- | --- | --- |
| F5-STATIC-01 generation | Five browser routes in standalone generated output, no server/backend, clean manifest/digest and static hard-refresh 404 | **NOT_RUN** | Frontend/platform PR |
| Image privacy and data transfer | Actual static-asset test confirms no image/filename/EXIF/server payload upload, safe error/cancel/Worker behavior | **NOT_RUN** | Security + frontend |
| Functionality & correctness | Pixel bounds/EXIF/alpha/metadata/format fixtures, all 5 tools, download and cleanup on *released artifact* | **NOT_RUN** | QA |
| Four-browser, mobile, a11y | Chromium desktop/mobile, Firefox, WebKit, keyboard, responsive, accessibility, static navigation, browser limits | **NOT_RUN** | QA + accessibility |
| Static artifact security | File allowlist, no Pages Functions/`_worker.js`, secrets or Node/Nitro runtime; CSP/headers/TLS/404; external asset review | **NOT_RUN** | CISSP/Platform/SRE |
| Dependencies, licenses & provenance | New `pnpm audit` and applicable threat/license review; SBOM/third-party rights; frozen SHA and SHA-256; time-bounded risk decisions | **BLOCKED** | Security/legal: [picomatch non-production exception](../../../FRONTEND_PICOMATCH_TEMPORARY_NONPROD_RISK_ACCEPTANCE_2026-11-18.md) is not a public-launch approval |
| Privacy & legal notices | Accurate CDN/access metadata disclosure, privacy policy, terms, no government affiliation, applicable Indian obligations, owner/legal review | **PENDING** | Legal/product/owner |
| Support and incidents | Real user contact and support handling, bug reports, emergency withdrawal/incident contact without sensitive image logging | **PENDING** | Operations/product |
| Hosting and rollback | Cloudflare Free limits/budget, controlled preview protection, static route & headers, deployment/rollback test and DNS/TLS plan | **NOT_RUN** | SRE/CCIE/platform |
| Financial/management gate | ₹0-tier operating assumptions verified; potential usage beyond free budgets and support obligations recorded | **PENDING** | CA/MBA/owner |
| Required CI and merge protection | Exact source checks, always-on verification gate and reviews; verify `main` branch protections actually enforce it | **PENDING** | Maintainer |
| Explicit owner authorization | Named approved static artifact digest, target Cloudflare project, approved timing and public launch decision | **NOT_GIVEN** | Owner |
| Actual production publication | Deployment ID, URL, UTC time and post-deploy smoke results | **NOT_DONE** | Only after owner go/no-go |

**Not applicable to static-only Free:** on-host C/Rust production resource-admission/load evidence and P14 VPS/Caddy/secret management for public servers. This is NOT_APPLICABLE **only if F5-STATIC-01 proves there are no server endpoints or Functions**. If the release depends on public Rust/C/Cloudflare Functions, the classification changes and host/security gates must be added rather than waived.

## Required test result record after real execution

For each gate store: `status`, UTC timestamp, revision SHA, immutable static artifact digest, exact executed command/job/environment, retained run/artifact URL, reviewer, and any failing/NOT_RUN reason. The final post-freeze generated artifact manifest and Cloudflare deployment evidence live outside the source-controlled plan JSON to avoid a self-referencing commit SHA.

**No public release, cloud upload, tag, domain, payments or automatic merge is authorized by this file.**
