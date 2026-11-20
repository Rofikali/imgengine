# IMGENGINE — Product release records

**Status:** Release-documentation framework proposed for review in F5-RELEASE-DOCS-01 (2026-11-20). Creating this index or a version directory does **not** create a Git tag, production build, Cloudflare project, deployment, release approval or public website. Check [live milestone status](../../next.md) and current GitHub metadata before acting.

## Source of truth and ownership

- [ADR-PORTFOLIO-005](../ADR_PORTFOLIO_FIVE_PRODUCT_RELEASE_STRATEGY_2026-11-19.md) is the shared-platform/independent-product release design; [compatibility matrix](../IMGENGINE_FIVE_PRODUCT_RELEASE_COMPATIBILITY_MATRIX.md) owns native C ABI, Rust/API and frontend package interoperability.
- Root [AGENTS](../../AGENTS.md), [Docker-only policy](../IMGENGINE_DOCKER_ONLY_DEVELOPMENT_POLICY.md), [change-scoped CI contract](../CI_SCOPE_02_SELECTION_GATE.md) and [milestone transaction](../AGENT_MILESTONE_CONTINUATION_RUNBOOK.md) remain authoritative; **do not duplicate** engineering implementation policies in each release dossier.
- The [server-backed P12 MVP checklist](../MVP_RELEASE_CHECKLIST.md) and [legacy durable-job checklist](../RELEASE_CHECKLIST.md) are separate products/architectures. Do not treat a browser-local Free static release as an approval to expose server APIs, and do not impose P14 real-VPS tests on a deployment with **no server runtime**.
- Each product/version dossier is an **auditable release plan**. Individual features/PRs and passing CI jobs are evidence inputs, not approval. The final exact-source, artifact digest/provenance and deployment identity must be generated **after** the source freeze, and archived as immutable release/deployment evidence.

## Actual release dossiers

| Product/version | Release dossier | Status |
| --- | --- | --- |
| IMGENGINE Free v1.0.0 | [Free v1.0.0](free/v1.0.0/README.md) | **PLANNED / NOT RELEASED**. Five feature implementations are merged; remaining F5 gates are not closed. |

Create new product/version folders only when implementation and a real release candidate justify them. Future lanes may include `pro/`, `studio/`, `api/`, `sdk/` and `desktop/`; a directory does not mean a purchasable or supported product exists.

## Standard dossier files

1. `README.md`: purpose, capabilities, known limitations, status, decision and links.
2. `release-manifest.json`: **source-controlled release PLAN**, stable product/version and proposed policy; never write the file's own commit SHA into it.
3. `static-only-contract.md` for Free static distribution; server-based products require their **own** runtime/security contracts rather than copying the static prohibition.
4. `evidence-matrix.md`: actual PASS/FAIL/BLOCKED/NOT_RUN/PENDING, owner, timestamp, exact SHA and retained evidence URL. Unknown is **NOT_RUN**, not PASS.
5. `deployment-runbook.md`: environment/approval boundary, preflight, deploy, smoke test, rollback, incident handling and post-release record.

Machine-readable plan validation is specified by [release plan JSON schema](schemas/release-plan.schema.json); schema presence alone is not automated enforcement. The release CI must validate it in a later bounded milestone.

Shared human templates: [release README](templates/README.template.md), [evidence matrix](templates/evidence-matrix.template.md), [deployment runbook](templates/deployment-runbook.template.md). Duplicate only the version-specific scaffolding. Avoid a new database or CI system solely to track releases.

## Release and evidence lifecycle

`PLANNED` → `CANDIDATE` → `READY_FOR_RELEASE_REVIEW` → `APPROVED_FOR_DEPLOYMENT` → `RELEASED` → optional `SUPERSEDED` or `ROLLED_BACK`. A failed/rejected candidate remains blocked; never mark it RELEASED because the PR merged or the site was previewed.

- **PLANNED**: documentation and proposed version; source/artifact/deployment IDs may be null.
- **CANDIDATE**: exact source SHA and bounded frozen scope; complete missing evidence or mark BLOCKED.
- **READY_FOR_RELEASE_REVIEW**: release-level browser, security, privacy, legal/support, artifact and operational evidence has actually passed; remaining risks explicitly dispositioned by accountable owners.
- **APPROVED_FOR_DEPLOYMENT**: independent explicit owner authorization for one immutable artifact and public target. Account creation or `go ahead` on a development task is not approval to publish.
- **RELEASED**: approved artifact live and post-deployment smoke checks observed; record deployed URL, deployment ID and timestamp in a dated operational record. Do not retroactively edit a signed Git tag or artifact manifest.
- **SUPERSEDED/ROLLED_BACK**: retain original digest, incident and replacement references.

The source-controlled JSON file is **not** the final generated build attestation. Do not self-reference its own Git SHA. Once frozen, generate a separate post-build `artifact-manifest.json` and content digest (and attestations/SBOM when applicable), then retain them with the release archive and verified Cloudflare deployment record. Never invent successful build results or actual URLs.

## Git and deployment rules

Single governed `main`, short-lived `feat/free/*`, `feat/pro/*`, `feat/studio/*`, `feat/api/*`, `feat/sdk/*`, `feat/desktop/*` and `docs/*`. Create `release/free/v1.0.0` only for a true owner-reviewed freeze, then the immutable `free-v1.0.0` tag after verification. No permanent product forks. Full repository, including C/Rust and infra sources, stays in Git; only the statically verified distribution is uploaded to Cloudflare Pages.

**Account and public traffic are separate authorities.** No GitHub PR, merge, CI PASS or documentation update authorizes Cloudflare Direct Upload, domain purchase, payment, Rust/C public hosting, server image uploads or a production release. Restrict credentials and require an independently recorded owner go/no-go.

## Maintenance checks

Review only impacted files for a milestone; keep `next.md` as the live queue and link to version-specific records. Before approving a release-documentation PR, validate Markdown links, parse the JSON manifest, inspect all status/CI claims, ensure no secrets or public URL claims, and verify the PR exact-head `CI verification gate`. Later F5-STATIC-01 must add **automated fail-closed checks of the generated artifact**; documentation alone does not enforce deployment isolation.
