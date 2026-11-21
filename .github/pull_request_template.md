# IMGENGINE bounded engineering change — review transport only

A pull request, when used, is **only** for code review/merge. It must **never generate docs** or determine `IN_PROGRESS`, `VERIFIED`, `COMPLETED` or `RELEASED`. The implementation agent must already have updated docs **while writing code**, before asking for review. See [agent-owned documentation procedure](../docs/engineering/DOCUMENTATION_SYNC_CONTRACT.md).

## Engineering evidence
- Work/task ID and current agent-maintained status (from `next.md`):
- Actual changed source/config/test paths and acceptance criteria:
- Scope, exact tested SHA/environment, commands: PASS / FAIL / NOT RUN:
- Risk/security/privacy, C ABI/Rust FFI, Linux/SRE, CCIE/network, CA/MBA/operating costs as applicable:
- Rollback, remaining blocker and next work:

## Confirm agent-owned documentation already updated during work
- [ ] `AGENTS.md` records task implications and current handoff
- [ ] `next.md` records actual engineering state and one next action
- [ ] `README.md` describes changed behavior/limits or task checkpoint
- [ ] `skills.md` records verification recipe and impact
- [ ] `docs/engineering/live-status.md` has a dated, task-linked change record and evidence
- [ ] Affected scoped AGENTS, HLD/LLD/ADR, API/ABI, security, SRE and finance docs are accurate or explicitly not applicable
- [ ] Check `node --test scripts/docs-sync-check.test.mjs` and `node scripts/docs-sync-check.mjs HEAD^ HEAD` if locally available; record NOT RUN instead of guessing
- [ ] Exact-head CI verification and reviewer checks meet existing security and release controls

**Do not open a standalone documentation-status PR as a substitute for the implementing engineer's work.** Review and merge remain subject to owner permissions. A merged PR is neither completion proof nor product release.
