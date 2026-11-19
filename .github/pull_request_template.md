# IMGENGINE bounded engineering PR

## Task and evidence
- Milestone / parent, exact head SHA, changed boundary:
- Tests PASS / FAIL / NOT RUN, exact SHA and reproducible commands:
- Seven GitHub CI job results (native build, sanitizer, fuzz, Docker, Rust API, Nuxt, SaaS) and link:
- Security/privacy/ABI, rollback and remaining risks:

## Documentation synchronization
- [ ] Required milestone documentation-impact transaction completed: actual tested SHA, code/tests, `next.md` and next READY job are synchronized; statuses are not prematurely MERGED/RELEASED.
- [ ] Docs impact summary: updated paths/reasons; root/scoped AGENTS, skills, READMEs, contracts, security/CI/finance/ops reviewed as relevant; unrelated docs N/A.
- [ ] Any CI optimization preserves all existing security/browser projects and must report actual before/after measured times, not inferred speedups.

- [ ] `next.md` contains truthful proposed PR status and one next READY task, not a premature MERGED claim.
- [ ] `README.md`, `skills.md`, root/scoped `AGENTS.md`, `docs/README.md` checked for affected stale facts; updated or N/A explained.
- [ ] Relevant HLD/LLD, security, contract, finance and release documents updated only if applicable.
- [ ] Temporary security exceptions show exact advisory, scope, expiry, non-production restrictions and remediation trigger; waived is not patched.
- [ ] After owner merges externally, next authorized review PR reconciles actual merge SHA and affected docs. This checklist is **not an automatic agent**.

**Docs updated (why):**

**Docs reviewed, not applicable:**

**After owner merge, sync needed:**

## Selective CI verification (CI-SCOPE-02)
- [ ] Record `CI scope selection (trusted base)` version/mode and selected lanes for this exact PR SHA; any fork, unknown, ABI/FFI, dependency or workflow change must fail closed to the full matrix.
- [ ] Confirm the **`CI verification gate` actually passed** and that all selected jobs have real SUCCESS. A skipped job is N/A, **never** a passed test.
- [ ] For release/readiness, verify full matrix at the exact candidate SHA; document branch protection/required-check status and any pending owner decision.

## Approval
All seven CI jobs remain required. Pushing, merging, production deployment, billing, account or cloud settings and security policy changes need appropriate owner approval.
