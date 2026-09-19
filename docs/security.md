# IMGENGINE Security Design

**Status:** Planned target controls; retain existing controls until the Rust cutover is complete.

## Security Principles

- Accept untrusted bytes once, with bounded streaming and fail closed.
- Keep image decoding outside the internet-facing process.
- Use generated identifiers and private directories; never trust a client path or filename.
- Minimize retained data and make deletion the normal completion path.
- Log events and identifiers, never image bytes, credentials, cookies, or filesystem paths.

## Required Controls

| Area | Target control |
| --- | --- |
| Upload ingress | `Content-Length` cap plus streaming byte counter; accept JPEG/PNG only; validate magic, decode dimensions, pixel count, aspect/layout bounds, and decompression ratio before render. |
| Execution | Dedicated non-root UID, no network, generated `0700` temporary directory, `no_new_privs`, resource limits, timeout, process-group kill, and platform sandboxing where available. |
| HTTP | TLS at Caddy, strict CORS allow-list, request/body timeouts, rate/concurrency limits, security headers, and a versioned error schema. |
| Authorization | Programmatic API keys are hashed at rest/configured through a secret mount. Browser traffic must use a per-user session or an explicitly anonymous, rate-limited mode; never a shared privileged key. |
| Secrets | Docker/host secret files with restrictive permissions; no credentials in images, logs, browser runtime configuration, or repository. |
| Supply chain | Pinned image digests, locked Rust/Node dependencies, SBOM, vulnerability scan, provenance, and immutable deployment digest. |
| Operations | Only `80/443` public; private backend network; firewall allow-list for SSH; non-root containers; read-only root filesystem where compatible. |

## Security Gates

The Rust service must not become the default path until it has tests for malformed JPEG/PNG, oversized content, decompression bombs, path traversal, symlink races, process timeout, cancellation, output overflow, cross-request isolation, unauthorized access, rate limiting, and secret redaction. Native sanitizer, fuzz, regression, and ABI checks remain required release gates.

## Legacy Risk Treatment

Until retirement, treat the Python worker queue as a separate security boundary. Fix terminal retries, lifecycle cleanup for all non-terminal jobs, dimension/pixel validation, and shared-browser-key authorization before expanding public usage. Existing logs and persistent artifacts require explicit retention and restricted backup access.

