# IMGENGINE Threat Model

**Assets:** service availability, host integrity, temporary image bytes, API/session credentials, configuration secrets, audit-safe telemetry, and released binaries.

| Threat | Primary mitigation | Verification |
| --- | --- | --- |
| Image parser exploit | Decode in a non-root, networkless child; fuzz/sanitize C; OS sandboxing | Fuzz corpus, sanitizer CI, sandbox integration test |
| Decompression/resource bomb | Stream, byte, dimension, pixel, ratio, timeout, CPU/RSS/output limits | Boundary/property tests and load tests |
| Path traversal/symlink race | Generated directory/file names; no user paths; safe create semantics; private temp root | Adversarial filesystem tests |
| Request flood | Per-principal/IP rate limits and bounded concurrent renders | Load test and metrics alert |
| Cross-user artifact access | No durable output by default; authenticated ownership for any future durable job | Authorization contract tests |
| Credential exposure | Secret mounts, redaction, server-only configuration, rotation | Log scanning and deployment review |
| Supply-chain compromise | Lockfiles, digest pinning, SBOM, scan, provenance, signed release policy | CI evidence review |
| Host/network compromise | Caddy TLS, firewall, private network, non-root/rootless posture, patching | Deployment checklist |
| Log/telemetry privacy leak | Allow-listed structured fields and bounded retention | Redaction tests and log review |

## Residual Risk

Native decoders process adversarial formats; sandboxing reduces but cannot eliminate that risk. A small VPS has limited isolation and capacity, so overload protection and fast cleanup are availability requirements, not optional tuning.

