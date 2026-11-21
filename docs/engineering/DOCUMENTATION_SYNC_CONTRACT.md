# IMGENGINE — mandatory documentation synchronization contract

**Effective from adoption of this PR.** Applies to human engineers, Codex, autonomous agents and tooling that proposes source, test, configuration, infrastructure, CI, security, contract or documentation changes. The current canonical branch is `main`; GitHub PR state and CI evidence outrank dated prose.

## 1. Four-file atomic documentation transaction

Every **substantive** change must change all four root files **in the same branch/PR**:

1. `AGENTS.md` — update binding agent rules, affected boundary, or a dated pointer to the verified change and relevant scoped `AGENTS.md`.
2. `next.md` — update the **human-maintained** checkpoint, exact evidence, milestone state and next eligible bounded task; never hand-edit the `BEGIN/END AUTO-GENERATED PR MILESTONES` block.
3. `README.md` — update the user/operator-facing current behavior or state; clearly distinguish implemented, tested, approved and released.
4. `skills.md` — update the reproducible engineering/verification recipe or link to the updated procedure with exact command and limitations.

Do this **before requesting review**, again when scope changes materially, and reconcile merged SHA and actual checks in the next authorized PR after merge. A comment or unchecked PR checkbox cannot replace a required file change. Add scoped `AGENTS.md`, architecture/ADR/HLD/LLD, API/ABI, security, SRE/observability, finance/CA-MBA, legal, cost, network/CCIE and tests **where affected**. Do not blindly rewrite 40 KB of durable policy or fabricate changes: a concise, accurate cross-reference and meaningful state delta suffice. Avoid future-tense promises written as completed facts.

The only automated exception is the existing **review-only generated status snapshot**, whose changed paths are confined to `next.md` and/or `docs/engineering/live-status.md`. The bot may update the generated `next.md` block and live status; it must **never** automatically edit `AGENTS.md`, `README.md`, `skills.md`, source, CI, security approvals or release authorization. All other paths (including docs-only contract/engineering edits) require the four-file transaction. The gate checks **presence**, not the truth of prose; reviewers must verify semantic accuracy.

## 2. CI enforcement and honest exception handling

`scripts/docs-sync-check.mjs` reads an exact Git diff (including deletions) and **fails closed** if any non-generated file changes without all four required files changed. It forbids deleting a canonical file or using a generated-only change as a bypass. Unit tests exercise meaningful and malformed diffs. This check runs inside the existing **CI policy and diff validation** job; its failure consequently fails the existing `CI verification gate`. The GitHub Actions token for this PR check is read-only; no bot privilege is needed.

**Important:** A check is not an unavoidable merge block until the repository owner configures a branch ruleset/protection for `main`, requiring **CI verification gate** on current head, no bypass/force-push, and appropriate reviews. The repository was observed **unprotected on 2026-11-21**. A direct commit to an unprotected branch can bypass this policy; no documentation can claim otherwise.

## 3. Standard agent/engineer workflow

- **Start:** Inspect current `main` head, open PRs, CI, root/scoped rules, milestone registry and canonical contracts. Identify already implemented work; no duplicate branches or blind historical claims.
- **Plan:** One bounded milestone and traceability ID, dependencies, tests, threat model, API/ABI compatibility, rollback, cost/privacy effects. Use seven engineering levels L1–L7 **proportionately**, plus CISSP, CCIE, Linux/SRE and CA/MBA/management reviews when their domains are affected.
- **Change:** Implement smallest secure modular diff (C17/Rust/Nuxt architecture intact); synchronize four files and affected contracts **in the same PR**, with evidence and next READY task.
- **Verify:** Run focused deterministic tests and full/selected exact-head CI, privacy/CSP, Docker/native ABI/FFI as applicable. No fabricated PASS; distinguish `NOT RUN`, `FAIL`, `PASS` and `RELEASED`.
- **Review and merge:** Confirm exact-head `CI verification gate`, the docs gate and human review. Do **not** automatically merge, deploy, release, tag, spend, change secrets, lower safety gates, or change account/branch settings based on this document.
- **After merge:** Confirm the actual merge SHA and `main` CI separately. Reconcile any now-stale status in a new reviewed change; milestone bot may only propose generated status.
- **Failure/recovery:** If the milestone bot cannot open PRs, its failed run is **not success**. Explicitly surface `Settings → Actions → General → Workflow permissions → Allow GitHub Actions to create and approve pull requests` for an owner decision; use a separate human-reviewed status PR if owner declines. Never push generated docs directly to `main`.

## 4. Evidence by discipline

| Review lens | Minimum documentation evidence when affected |
| --- | --- |
| L1–L2 | Reproduction, tests, user-facing feature, input/error behavior |
| L3–L4 | HLD/LLD, API/ABI/FFI compatibility, edge cases, maintainability, ownership |
| L5–L7 | Cross-product trade-offs, dependencies, alternatives, measurable capacity, migration/rollback |
| CISSP/security | Threat/abuse model, privacy boundaries, supply-chain provenance, advisory disposition, least privilege |
| CCIE/network | HTTP/TLS, DNS, network trust, ingress/egress, backpressure and failure handling |
| Linux/SRE | Docker/runtime, resource/cgroup constraints, observability, on-call/runbook, recovery and SLO evidence |
| CA/MBA/management | Unit economics, free/paid cost ceilings, vendor risk, scope/value, owner GO/NO-GO authority |

No change to the existing Free v1.0.0 **NOT_RELEASED** state or independent legal, publication, hosting, owner, security and financial gates is authorized by this policy.

## 5. Acceptance and operational commands

```sh
node --test scripts/docs-sync-check.test.mjs
node scripts/docs-sync-check.mjs HEAD^ HEAD
```

Required check: **CI verification gate**. After owner-enforced branch protection and verified real workflow executions, use the PR template to ensure both the four-file sync and all affected additional docs/contracts have been reviewed. Roll back by reverting this reviewed PR; do not disable security/ABI/release gates.
