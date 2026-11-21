# IMGENGINE Free v1.0.0 — Cloudflare launch gate, sign-off and live smoke record

**Record type:** operator-filled release worksheet. **Initial state 2026-11-21: NO GO / UNAPPROVED.** This is a **blank template**, not approval, a signed attestation, or evidence that a Cloudflare deployment exists. Do **not** commit personal contact details, credentials, token values, customer images, filenames or raw image-bearing network traffic. Keep the final immutable approval and artifact hashes in a protected external release record; a Git PR is not independent sign-off.

See [owner A-to-Z guide](HOST_ON_CLOUDFLARE_A_TO_Z.md), [policy runbook](CLOUDFLARE_PAGES_RELEASE_PLAYBOOK.md), [release plan](release-manifest.json), [security evidence](evidence-matrix.md).

**Candidate reproducibility milestone:** [F5-RC-ARCHIVE-01](F5_RC_ARCHIVE_01.md) prepares a normalized deterministic static tar + separately re-audited SHA-256 evidence. Only `candidate-archive.json` and `candidate-manifest.json` are CI-uploaded for review, never an unapproved website. A reproducible candidate is **not** an immutable owner-approved archive, independent SBOM/license sign-off, or permission to create `free-v1.0.0`.

## A. Verified release identity (fill after independent frozen build)

| Item | Actual value / evidence | Current decision |
| --- | --- | --- |
| Public product | IMGENGINE **Free** v1.0.0, five browser-local tools only | SCOPE DEFINED, NOT RELEASED |
| Proposed release tag | `free-v1.0.0` (not yet created) | NO GO |
| Frozen source Git SHA | **NOT ASSIGNED**; use exact 40-character immutable commit | NOT VERIFIED |
| Complete audited static archive SHA-256 | **NOT ASSIGNED**; compute over the actual approved archive | NOT VERIFIED |
| Detached candidate JSON/tree digest | **NOT APPROVAL**; record exact workflow run and file provenance | CANDIDATE ONLY |
| Complete license/SBOM provenance review | **NOT ATTACHED** | NOT VERIFIED |
| Clean production lockfile audit on frozen SHA | **NOT ATTACHED**; link run and severities | NOT VERIFIED |
| Exact-head main/full CI | Current main `889be673220d4489325ff63a7a1ad6642b17fa2b` [full CI #44888710794](https://github.com/Rofikali/imgengine/actions/runs/44888710794) passed after #103's Firefox/WebKit test-lifecycle repair; no frozen release SHA has been selected | PASS ON CURRENT MAIN / NOT A RELEASE ATTESTATION |
| Owner security/privacy/legal written approvals | **NOT ATTACHED** | NOT VERIFIED |
| Hosting account / Free-only product selection | **NOT ENTERED** (non-secret reference only) | NOT VERIFIED |
| Cloudflare project & deployment ID | **NOT CREATED / NOT OBSERVED** | NOT DEPLOYED |
| Owner's explicit production GO | **NOT GIVEN** | NO GO |

The source-controlled `release-manifest.json` intentionally uses null fields rather than embedding its own commit SHA. A manifest on `main` does **not** sign or certify the final archive.

## B. Mandatory go/no-go gates

Mark each item **PASS** only with exact evidence URL/identifier, UTC timestamp and separate reviewer; **FAIL** or **UNKNOWN** means NO GO:

- [ ] Exact frozen SHA reviewed, branch protection satisfied, PR diffs/merged dependencies accounted for
- [ ] CI verification gate **PASS** on the frozen SHA, including all selected Rust/C/Docker/Linux/CI policy jobs
- [ ] Browser-local and full static four-project matrix **PASS** on frozen SHA, including Firefox six-route privacy navigation, WebKit, CSP and negative API routes
- [ ] `pnpm install --frozen-lockfile` plus source-locked `verify:free-release-dependencies` **PASS** with zero critical/high/moderate production advisories; cross-major override compatibility reviewed
- [ ] `build:free-static` and `verify:free-static` **PASS**; exactly nine routes, true `404.html`, root `_headers`; zero server/Functions/Workers/Rust or hidden credential output
- [ ] Frozen source, independent immutable public-file archive, deterministic digest, SBOM/licenses and external provenance reproducibly match; no private customer payloads
- [ ] Privacy, Terms and Support public copy finished; actual operator legal/support contact and effective date approved; hosting metadata, cookie/analytics/retention facts accurate
- [ ] The deliberately fail-closed `check-free-publication-ready.mjs` is replaced/enhanced **only through a separately reviewed implementation with verifiable detached post-freeze approval**, and the final gate truly passes; no `|| true`, mock approval or test skip
- [ ] Cloudflare account Free plan, project, permissions, MFA, recovery, domain strategy, zero paid add-ons and usage monitoring reviewed by owner
- [ ] Real Pages CDN smoke and rollback/withdrawal plan approved (may involve a separately authorized access-protected prelaunch rehearsal)
- [ ] A dated, separate **owner GO** authorizes the exact artifact digest, intended Pages account/project, first public exposure and roll-forward/withdrawal operator

**Required reviewers:** engineering/security owner; privacy/legal/support owner; hosting/SRE operator; financial/cost approver; final accountable release owner. These may be the same individual in a solo project, but **every decision and its role must be documented independently**. The responsibility matrix does not assert external professional certifications.

## C. Approved deployment execution (complete only after all above PASS)

| Field | Record |
| --- | --- |
| Freeze source SHA | PENDING |
| Archive SHA-256 | PENDING |
| Exact full CI run URL and outcome | PENDING |
| Approval record URL/ID and UTC | PENDING |
| Actual Cloudflare account/project (no tokens) | PENDING |
| Verified Free pricing/optional services | PENDING |
| Static upload method | Dashboard Direct Upload, after owner GO |
| Cloudflare deployment ID / URL | PENDING — do not invent |
| UTC deployment timestamp | PENDING |
| Operator and independent verifier | PENDING |
| Previous verified deployment / first-launch exposure control | PENDING |

## D. Live `pages.dev` HTTP/browser smoke tests (after authorized deploy)

Capture **actual** Cloudflare URL and browser/device/build versions, UTC time, HTTP status/headers and redacted evidence. Do not log or transmit customer filenames/image data.

| Live test | Expected | Result |
| --- | --- | --- |
| TLS/HTTPS | Valid certificate, HTTP→HTTPS as applicable | NOT RUN |
| Routes `/`, `/resize-image`, `/crop-image`, `/compress-image`, `/rotate-image`, `/convert-image`, `/privacy`, `/terms`, `/support` | All deliver expected static HTML; approved final notices | NOT RUN |
| Missing `/does-not-exist` and forbidden `/api/render`, `/api/sheet-render` | **HTTP 404**, not HTTP 200 fallback or dynamic response | NOT RUN |
| CSP and response headers | `default-src 'none'`, `script-src 'self'`, `connect-src 'self' blob:`, `frame-ancestors 'none'`, nosniff, DENY, no-referrer; no unexpected violations | NOT RUN |
| Confidential browser work | No image-bearing request/filename/metadata leakage, local processing only, no analytics unexpected by approval | NOT RUN |
| PNG/JPEG operations | Five tools functional; EXIF, alpha, bounds, cancellation and URL cleanup per contracted behavior | NOT RUN |
| Compatibility & a11y | Desktop/mobile Chromium, Firefox and WebKit; keyboard and accessibility regression | NOT RUN |
| Infrastructure & finance | No functions/Workers/server, unexpected paid bindings, domain charges or unapproved analytics | NOT RUN |
| Recovery | Actual rollback previous good deploy **or** first-deployment withdrawal method tested | NOT RUN |
| Artifact integrity | Observed CDN asset inventory and headers correspond to approved immutable build | NOT RUN |

Optional operator HTTP inspection **after** a public deployment has been authorized (substitute only an actual URL, never a guessed project name):

```bash
# Trusted operator terminal. Real HTTPS host assigned by Cloudflare only:
export BASE_URL="https://<ACTUAL_PROJECT>.pages.dev"
curl -sS -D - -o /dev/null "$BASE_URL/"
curl -sS -D - -o /dev/null "$BASE_URL/privacy"
curl -sS -D - -o /dev/null "$BASE_URL/does-not-exist"
curl -sS -D - -o /dev/null "$BASE_URL/api/render"
# Inspect response codes and actual CSP/security headers; do not count curl as browser QA.
```

## E. Finance / SRE controls

- Cloudflare Pages **Free static requests**: provider says free and unlimited; Free site limit **20,000 files**; individual asset maximum **25 MiB**. Validate current billing/plan and every optional service on the owner's actual account. The lack of server costs is conditional on **not deploying Functions/Workers/backend/analytics paid add-ons**.
- No optional custom domain needed for first `pages.dev` deployment. A purchased domain is a separate commercial decision and can incur cost.
- Monitor availability, security headers, error statuses, caching, deployment health and spending without collecting image bytes, EXIF or filenames. Write an incident log and preapprove release withdrawal/rollback; the first deployment has no prior good version.
- Protect GitHub and Cloudflare accounts with MFA and least privilege. Never store Cloudflare auth material in source files, logs or unreviewed third-party services.

## F. Final decision and record preservation

**Decision today: NO GO**. Final owner go/no-go must identify source, full audit and tests, immutable digest, hosting project/account, signed reviewer records, UTC time, and a live smoke plan. Publication may be announced as IMGENGINE Free v1.0.0 only after verified deployment, rollback and approval. The tagged version identity `free-v1.0.0` must refer to the exact approved source and must never be silently retargeted.

**Official references:**
- <https://developers.cloudflare.com/pages/get-started/direct-upload/>
- <https://developers.cloudflare.com/pages/platform/limits/>
- <https://developers.cloudflare.com/pages/functions/pricing/>
- <https://developers.cloudflare.com/pages/configuration/headers/>
- <https://developers.cloudflare.com/pages/configuration/serving-pages/>
- <https://developers.cloudflare.com/pages/configuration/rollbacks/>
