# <Product> <Version> — Release evidence matrix

**State:** NOT_RELEASED. This is a signoff record, not a substitute for live test execution.

Use exact revision-specific links and the status vocabulary `PASS`, `FAIL`, `BLOCKED`, `PENDING`, `NOT_RUN`, `NOT_APPLICABLE`. NEVER report a test or check as passing solely because it was planned, discovered, skipped or passed at an earlier unrelated SHA.

| Gate | Required outcome | Status | Exact source SHA / artifact SHA-256 | CI/proof link | Accountable reviewer |
| --- | --- | --- | --- | --- | --- |
| Functionality/correctness | Acceptance fixtures | NOT_RUN | — | — | Engineering |
| Browser, mobile and a11y | Release artifact matrix | NOT_RUN | — | — | Quality |
| Privacy and data paths | Captured transport/storage assertions | NOT_RUN | — | — | Security |
| Supply chain/license | Current production dependency findings reviewed | NOT_RUN | — | — | Security/legal |
| Deployment boundary | Prohibited runtime/keys absent from artifact | NOT_RUN | — | — | Platform |
| Operations | Monitoring, rollback, incident drill | NOT_RUN | — | — | SRE |
| Legal/support | Applicable requirements, contact, terms/privacy | NOT_RUN | — | — | Owner/legal |
| Economics/budget | Cost constraints and support ownership | NOT_RUN | — | — | Owner/finance |
| Owner go/no-go | Explicit exact-artifact approval | PENDING | — | — | Owner |

Record: fixture/environment, build provenance and deviations; vulnerability advisory IDs/expiry if applicable; host/domain approval; residual risks and the independent **actual deploy** record only when performed. N/A requires a reason and reviewer (e.g. a truly static site needs no real C/Rust host-capacity gate). Link existing canonical test/security documentation; do not copy HLD/LLD wholesale.
