# IMGENGINE Security Operations

## Implemented Controls

- Public API keys are hashed before job ownership is stored.
- Job status, output, and audit-log retrieval require the same owner key.
- `Idempotency-Key` is scoped to the key owner and prevents duplicate queue work.
- Per-key generation rate limits, upload-size bounds, private storage keys, worker resource limits, and signature-validated JPEG/PNG ingress are enforced.
- The browser receives only the Nuxt server-side API key; worker status updates require a separate internal token.

## Beta Exit Criteria

1. Replace shared API keys with authenticated tenant principals.
2. Add a durable per-tenant quota ledger for accepted jobs, bytes, and concurrent work.
3. Provide API-key creation, expiry, rotation, revocation, and last-used audit metadata.
4. Define malware-scanning and content-retention policies before accepting untrusted production traffic.
5. Validate backup restoration for PostgreSQL and artifact storage, then run an access-control review.

## Operational Rules

- Do not use filenames as an authorization, tenancy, or content-type signal.
- Do not retry a failed job automatically unless the failure is classified as transient and the idempotency boundary is preserved.
- Treat API-key exposure, cross-owner access, signature-validation bypass, and unbounded uploads as security incidents.
