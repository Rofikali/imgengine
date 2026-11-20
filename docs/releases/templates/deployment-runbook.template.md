# <Product> <Version> — Controlled deployment and rollback

**State:** PROCEDURE ONLY / NOT DEPLOYED. Adapt to the actual verified artifact and provider before any production action.

## Authorizations

Deployment requires independently recorded owner approval for one SHA-256 artifact digest, verified production target and permitted public scope. Development `go ahead`, green CI, account creation, a PR merge and the existence of a release tag are not that approval. Keep provider credentials outside Git and logs.

## Preflight

1. Confirm exact frozen source SHA, generated artifact and provenance/signature/SBOM as appropriate; match planned capabilities and security/data-flow contract.
2. Re-evaluate affected dependencies, current advisories, privacy/legal requirements, browser/device compatibility and all required full release gates; review exceptions/expiry.
3. Verify DNS/TLS, security headers, access restrictions, monitoring, no secrets and support/incident ownership.
4. Identify prior known-good artifact and practice restoring it. For a first production deployment, document an emergency unpublish/traffic-disabling plan because there is no previous version.
5. Separately approve provider cost/rate limits and possible usage beyond Free tier.

## Approved deployment (do not perform from this document)

Record: UTC time, operator, release owner, source SHA, tag, immutable artifact digest, command/process, hosting project/account (non-secret identifiers), deployment ID, URL, environment and observed results. Prohibit an implicit build or deploy on every `main` push without an approved control.

## Smoke, rollback and incident

- Check routes, static assets / documented API contract, browser results, unexpected network egress, headers/TLS and exposure of secrets; include negative tests.
- Roll back to the **verified** previous artifact/deployment ID, retest and record cause/timestamp; on initial deployment use explicit disable/withdraw plan.
- Escalate customer privacy/security issues, status and support communication; retain evidence without image bytes/credentials in logs.

## Audit

Compare deployed artifact ID/hash to approved release record. A publication attempt is not RELEASED until actual approved public availability and smoke tests are evidenced.
