# Security Policy

## Scope

`imgengine` processes image data and untrusted input at a native-code boundary. Security issues involving memory safety, malformed-input handling, resource exhaustion, sandboxing, ABI/FFI boundaries, or unsafe filesystem behavior are especially important.

## Reporting a vulnerability

Please do not disclose an exploitable vulnerability in a public issue.

Use GitHub's private vulnerability reporting/security advisory mechanism for this repository when available. Include:

- a clear description of the issue
- affected component or version/commit
- reproduction steps or a minimal proof of concept
- expected and observed behavior
- any relevant sanitizer, fuzzing, or crash output

Avoid including real user images, credentials, tokens, private paths, or other sensitive data in a report.

## Security engineering

The project aims to validate security-sensitive behavior through:

- ASan/UBSan testing
- fuzzing of untrusted input paths
- bounded dimensions and resource validation
- explicit ownership and release rules at the public C ABI
- controlled filesystem lifecycle
- sandboxing where applicable
- regression tests for previously discovered failures

Security fixes should preserve the documented native API contract unless a breaking change is explicitly required and versioned.
